/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2009 Brian Aker All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <libmemcached/common.h>
extern "C"{
#include "../Jerasure-1.2A/cauchy.h"
#include "../Jerasure-1.2A/reed_sol.h"
#include "../Jerasure-1.2A/jerasure.h"
#include "../Jerasure-1.2A/galois.h"
}
#include <stdio.h>
#include <stdlib.h>

enum memcached_storage_action_t {
  SET_OP,
  REPLACE_OP,
  ADD_OP,
  PREPEND_OP,
  APPEND_OP,
  CAS_OP
};

/* Inline this */
static inline const char *storage_op_string(memcached_storage_action_t verb)
{
  switch (verb)
  {
  case REPLACE_OP:
    return "replace ";

  case ADD_OP:
    return "add ";

  case PREPEND_OP:
    return "prepend ";

  case APPEND_OP:
    return "append ";

  case CAS_OP:
    return "cas ";

  case SET_OP:
    break;
  }

  return "set ";
}

static inline uint8_t can_by_encrypted(const memcached_storage_action_t verb)
{
  switch (verb)
  {
  case SET_OP:
  case ADD_OP:
  case CAS_OP:
  case REPLACE_OP:
    return true;
    
  case APPEND_OP:
  case PREPEND_OP:
    break;
  }

  return false;
}

static inline uint8_t get_com_code(const memcached_storage_action_t verb, const bool reply)
{
  if (reply == false)
  {
    switch (verb)
    {
    case SET_OP:
      return PROTOCOL_BINARY_CMD_SETQ;

    case ADD_OP:
      return PROTOCOL_BINARY_CMD_ADDQ;

    case CAS_OP: /* FALLTHROUGH */
    case REPLACE_OP:
      return PROTOCOL_BINARY_CMD_REPLACEQ;

    case APPEND_OP:
      return PROTOCOL_BINARY_CMD_APPENDQ;

    case PREPEND_OP:
      return PROTOCOL_BINARY_CMD_PREPENDQ;
    }
  }

  switch (verb)
  {
  case SET_OP:
    break;

  case ADD_OP:
    return PROTOCOL_BINARY_CMD_ADD;

  case CAS_OP: /* FALLTHROUGH */
  case REPLACE_OP:
    return PROTOCOL_BINARY_CMD_REPLACE;

  case APPEND_OP:
    return PROTOCOL_BINARY_CMD_APPEND;

  case PREPEND_OP:
    return PROTOCOL_BINARY_CMD_PREPEND;
  }

  return PROTOCOL_BINARY_CMD_SET;
}

// ~~~ subfunctions for RS, ERS, SRS encoding, begin ~~~
//ERSadd
static size_t determine_block_size_rs(const size_t value_length, uint32_t k) {
  size_t block_size = 0;
  size_t temp_value_length = 0;
  int size_of_long = sizeof(long);
  int mod = value_length % (k*size_of_long);
  if(mod == 0) {
    block_size = value_length / k;
  } else {
    temp_value_length = value_length + k*size_of_long - mod;
    block_size = temp_value_length / k;
  }
  return block_size;
}

//ERSadd
static uint32_t lcd(uint32_t a, uint32_t b) {
  uint32_t largerOne = a>=b?a:b;
  uint32_t smallerOne = a>=b?b:a;
  while(smallerOne != 0) {
    uint32_t remain = largerOne%smallerOne;
    largerOne = smallerOne;
    smallerOne = remain;
  }
  return largerOne;
}

//ERSadd
static uint32_t lcm(uint32_t a, uint32_t b) {
  return a*b/lcd(a, b);
}

//ERSadd
static size_t determine_block_size_ers(const size_t value_length, uint32_t k, uint32_t s) {
  uint32_t l = lcm(k, s);
  size_t block_size = 0;
  size_t temp_value_length = 0;
  int size_of_long = sizeof(long);
  int mod = value_length % (l*size_of_long);
  if(mod == 0) {
    block_size = value_length / l;
  } else {
    temp_value_length = value_length + l*size_of_long - mod;
    block_size = temp_value_length / l;
  }
  return block_size;
}

//ERSadd
static void fill_data_ptrs(char **data_ptrs, uint32_t size, size_t block_size, const char *value, const size_t value_length) {
  size_t bytes_remained = value_length;
  for(uint32_t i = 0; i < size; ++i) {
    if(block_size <= bytes_remained) {
      memcpy(data_ptrs[i], value + i*block_size, block_size);
      bytes_remained -= block_size;
    } else {
      memcpy(data_ptrs[i], value + i*block_size, bytes_remained);
      bytes_remained -= bytes_remained;
    }
  }
}

//ERSadd
static void calculate_parity_ptrs_rs(char **data_ptrs, uint32_t k, char **coding_ptrs, uint32_t m, size_t block_size) {
  int *matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
  jerasure_matrix_encode(k, m, 8, matrix, data_ptrs, coding_ptrs, block_size);
  free(matrix);
}

//ERSadd
static void print_enhanced_placement_ers_to_file(uint32_t l, uint32_t row_num, uint32_t *h_ids, uint32_t *v_ids) {
  uint32_t column_num = l / row_num;
  FILE *fp = fopen("./wusi", "a");
  fprintf(fp, "ers, enhanced placement: \n");
  uint32_t *positions = new uint32_t[l];
  for(uint32_t i = 0; i < row_num; ++i) {
    for(uint32_t j = 0; j < column_num; ++j) {
      uint32_t idx = i*column_num + j;
      uint32_t hid = h_ids[idx];
      uint32_t vid = v_ids[idx];
      positions[vid*column_num + hid] = idx;
    }
  }
  for(uint32_t i = 0; i < row_num; ++i) {
    for(uint32_t j = 0; j < column_num; ++j) {
      fprintf(fp, "%d ", positions[i*column_num + j]);
    }
    fprintf(fp, "\n");;
  }
  fclose(fp);
  delete positions;
}

//ERSadd
/* designed placement for the ERS code */
static void enhanced_placement_ers(uint32_t *h_ids, uint32_t *v_ids, uint32_t k, uint32_t s) {
  uint32_t l = lcm(k, s);
  uint32_t row_num = l/s;
  uint32_t col_num = s;
  uint32_t x = s / k;
  uint32_t y = s % k;
  uint32_t first_class_row_num = l / k - x * l / s;
  uint32_t second_class_row_num = row_num - first_class_row_num;

  // FIRST-CLASS rows
  for(uint32_t i = 0; i < first_class_row_num; ++i) {
    uint32_t rotate_num = i*(k-y);
    uint32_t full_stripe_id = i*x;
    uint32_t partial_stripe_id = x*row_num + i;
    uint32_t start_idx = rotate_num % s;
      // fill x full stripes [length k*x]
    for(uint32_t tt = 0; tt < k*x; ++tt) {
      uint32_t hid = (start_idx + tt) % s;
      uint32_t vid = i;
      h_ids[full_stripe_id*k + tt] = hid;
      v_ids[full_stripe_id*k + tt] = vid;
    }
      // fill a partial stripe [length y]
    uint32_t end_idx = (start_idx + k*x) % s;
    for(uint32_t tt = 0; tt < y; ++tt) {
      uint32_t hid = (end_idx + tt) % s;
      uint32_t vid = i;
      h_ids[partial_stripe_id*k + tt] = hid;
      v_ids[partial_stripe_id*k + tt] = vid;
    }
  }

  // SECOND-CLASS rows
  uint32_t partial_stripe_id = x*row_num;
  uint32_t partial_stripe_start_idx = y;
  for(uint32_t j = 0; j < second_class_row_num; ++j) {
    uint32_t rotate_num = (j+1)*y;
    uint32_t full_stripe_id = (j + first_class_row_num)*x;
    uint32_t start_idx = rotate_num % s;
      // fill x full stripes [length k*x]
    for(uint32_t tt = 0; tt < k*x; ++tt) {
      uint32_t hid = (start_idx + tt) % s;
      uint32_t vid = j + first_class_row_num;
      h_ids[full_stripe_id*k + tt] = hid;
      v_ids[full_stripe_id*k + tt] = vid;
    }
      // fill the partial stripes [each of which is of length k - y]
    uint32_t end_idx = (start_idx + k*x) % s;
    for(uint32_t tt = 0; tt < y; ++tt) {
      uint32_t hid = (end_idx + tt) % s;
      uint32_t vid = j + first_class_row_num;
      h_ids[partial_stripe_id*k + partial_stripe_start_idx] = hid;
      v_ids[partial_stripe_id*k + partial_stripe_start_idx] = vid;
      partial_stripe_start_idx++;
      if(partial_stripe_start_idx == k) {
        partial_stripe_id++;
        partial_stripe_start_idx = y;
      }
    }
  }
  
  //print_enhanced_placement_ers_to_file(l, row_num, h_ids, v_ids);
}

//ERSadd
static void calculate_parity_ptrs_ers(Memcached *ptr, uint32_t *h_ids, uint32_t *v_ids, char **data_ptrs, char **coding_ptrs, uint32_t k, uint32_t s, uint32_t m, size_t block_size) {
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_pre = l/k;

  bool enhanced = (ptr->ers_enhanced_placement > 0 && s % k != 0);
  if(enhanced) enhanced_placement_ers(h_ids, v_ids, k, s);

  char **temp_data_ptrs = new char*[s];
  for(uint32_t i = 0; i < s; ++i) {
    temp_data_ptrs[i] = new char[block_size];
  }
  char **temp_coding_ptrs = new char*[m];
  for(uint32_t i = 0; i < m; ++i) {
    temp_coding_ptrs[i] = new char[block_size];
  }

  // an m*s dimensional big matrix
  int *matrix = reed_sol_vandermonde_coding_matrix(s, m, 8);

  for(uint32_t i = 0; i < num_stripes_pre; ++i) {
    for(uint32_t j = 0; j < s; ++j) {
      memset(temp_data_ptrs[j], 0, block_size);
    }
    for(uint32_t j = 0; j < m; ++j) {
      memset(temp_coding_ptrs[j], 0, block_size);
    }

    for(uint32_t j = 0; j < k; ++j) {
      uint32_t data_idx = i*k + j;
      uint32_t hid;
      if(enhanced) {
        hid = h_ids[data_idx];
      } else {
        hid = data_idx % s;
      }
        // k data blocks and s -k dummy blocks
      memcpy(temp_data_ptrs[hid], data_ptrs[data_idx], block_size);
    }

    jerasure_matrix_encode(s, m, 8, matrix, temp_data_ptrs, temp_coding_ptrs, block_size);

    for(uint32_t j = 0; j < m; ++j) {
      memcpy(coding_ptrs[i*m + j], temp_coding_ptrs[j], block_size);
    }
  }

  for(uint32_t i = 0; i < s; ++i) {
    delete temp_data_ptrs[i];
  }
  delete temp_data_ptrs;
  for(uint32_t i = 0; i < m; ++i) {
    delete temp_coding_ptrs[i];
  }
  delete temp_coding_ptrs;
  free(matrix);
}

//ERSadd
static void calculate_parity_ptrs_srs(Memcached *ptr, char **data_ptrs, char **coding_ptrs, uint32_t k, uint32_t s, uint32_t m, size_t block_size) {
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_pre = l/k;

  char **temp_data_ptrs = new char*[k];
  for(uint32_t i = 0; i < k; ++i) {
    temp_data_ptrs[i] = new char[block_size];
  }
  char **temp_coding_ptrs = new char*[m];
  for(uint32_t i = 0; i < m; ++i) {
    temp_coding_ptrs[i] = new char[block_size];
  }

  int *matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);

  for(uint32_t i = 0; i < num_stripes_pre; ++i) {
    for(uint32_t j = 0; j < k; ++j) {
      memset(temp_data_ptrs[j], 0, block_size);
    }
    for(uint32_t j = 0; j < m; ++j) {
      memset(temp_coding_ptrs[j], 0, block_size);
    }

    for(uint32_t j = 0; j < k; ++j) {
      uint32_t data_idx = j*num_stripes_pre + i;
      memcpy(temp_data_ptrs[j], data_ptrs[data_idx], block_size);
    }

    jerasure_matrix_encode(k, m, 8, matrix, temp_data_ptrs, temp_coding_ptrs, block_size);

    for(uint32_t j = 0; j < m; ++j) {
      memcpy(coding_ptrs[i*m + j], temp_coding_ptrs[j], block_size);
    }
  }

  for(uint32_t i = 0; i < k; ++i) {
    delete temp_data_ptrs[i];
  }
  delete temp_data_ptrs;
  for(uint32_t i = 0; i < m; ++i) {
    delete temp_coding_ptrs[i];
  }
  delete temp_coding_ptrs;
  free(matrix);
}

//ERSadd
static void generate_new_keys_rs(char **key_ptrs, uint32_t size, const char *key, const size_t key_length) {
  for(uint32_t i = 0; i < size; ++i) {
    memcpy(key_ptrs[i], key, key_length);
    int ten = i / 10;
    int one = i - ten * 10;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    key_ptrs[i][key_length] = ten_bit_char;
    key_ptrs[i][key_length+1] = one_bit_char;
    key_ptrs[i][key_length+2] = '\0';
  }
}
// ~~~ subfunctions for RS, ERS, SRS encoding, end ~~~



// ~~~ subfunctions for RS, ERS, SRS transitioning, begin ~~~
//ERSadd
static void right_part(uint32_t start_data_idx, uint32_t start_parity_idx, uint32_t dl, uint32_t rdl, uint32_t k, uint32_t num_stripes_pre, uint32_t* data_blocks_required_tag, 
                       uint32_t p_id, uint32_t **parity_update_vector) {
  if(dl <= rdl) {
    for(int i = start_data_idx; i <= (int)(start_data_idx + dl - 1); ++i) {
      data_blocks_required_tag[i] = 1;
      parity_update_vector[p_id][i + num_stripes_pre] = 1;
    }
  } else {
    parity_update_vector[p_id][start_parity_idx] = 1;
    for(int i = start_data_idx + dl; i <= (int)(start_data_idx + k - 1); ++i) {
      data_blocks_required_tag[i] = 1;
      parity_update_vector[p_id][i + num_stripes_pre] = 1;
    }
  }
}

//ERSadd
static void left_part(uint32_t start_data_idx, uint32_t start_parity_idx, uint32_t dl, uint32_t rdl, uint32_t k, uint32_t num_stripes_pre, uint32_t* data_blocks_required_tag, 
                      uint32_t p_id, uint32_t **parity_update_vector) {
  if(dl <= rdl) {
    parity_update_vector[p_id][start_parity_idx] = 1;
    for(int i = start_data_idx; i <= (int)(start_data_idx + dl - 1); ++i) {
      data_blocks_required_tag[i] = 1;
      parity_update_vector[p_id][i + num_stripes_pre] = 1;
    }
  } else {
    for(int i = start_data_idx + dl; i <= (int)(start_data_idx + k - 1); ++i) {
      data_blocks_required_tag[i] = 1;
      parity_update_vector[p_id][i + num_stripes_pre] = 1;
    }
  }
}

//ERSadd
static void ERS_basic_calculate_update(uint32_t k, uint32_t m, uint32_t s, 
            uint32_t *data_blocks_required_tag, /* size: l */
            uint32_t **parity_update_vector /* size: num_stripes_post * (num_stripes_pre + l) */ ) {
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_pre = l/k;
  uint32_t num_stripes_post = l/s;
  uint32_t x = s / k;
  uint32_t y = s % k;

  if(y == 0) {
    for(uint32_t i = 0; i < num_stripes_post; ++i) {
      for(uint32_t j = 0; j < x; ++j) {
        parity_update_vector[i][i*x+j] = 1;
      }
    }
    return;
  }

  uint32_t start_data_idx = 0;
  uint32_t start_parity_idx = 0;
  uint32_t dl = 0;
  uint32_t rdl = k - dl;

  for(uint32_t p_id = 0; p_id < num_stripes_post; ++p_id) {
    start_parity_idx = p_id*s / k;
    start_data_idx = start_parity_idx * k;
    if(y == rdl) {
      left_part(start_data_idx, start_parity_idx, dl, rdl, k, num_stripes_pre, data_blocks_required_tag, p_id, parity_update_vector);
      for(int i = start_parity_idx+1; i <= (int)(start_parity_idx+x); ++i) {
        parity_update_vector[p_id][i] = 1;
      }

    } else if(y < rdl) {
      left_part(start_data_idx, start_parity_idx, dl, rdl, k, num_stripes_pre, data_blocks_required_tag, p_id, parity_update_vector);
      for(int i = start_parity_idx+1; i <= (int)(start_parity_idx+x-1); ++i) {
        parity_update_vector[p_id][i] = 1;
      }
      dl = k + y - rdl;
      rdl = k - dl;
      right_part(start_data_idx + x*k, start_parity_idx+x, dl, rdl, k, num_stripes_pre, data_blocks_required_tag, p_id, parity_update_vector);

    } else {
      left_part(start_data_idx, start_parity_idx, dl, rdl, k, num_stripes_pre, data_blocks_required_tag, p_id, parity_update_vector);
      for(int i = start_parity_idx+1; i <= (int)(start_parity_idx+x); ++i) {
        parity_update_vector[p_id][i] = 1;
      }
      dl = y - rdl;
      rdl = k - dl;
      right_part(start_data_idx + (x+1)*k, start_parity_idx+x+1, dl, rdl, k, num_stripes_pre, data_blocks_required_tag, p_id, parity_update_vector);

    }
  } // end of for

  /*FILE* fp = fopen("./kobe", "a");
  fprintf(fp, "ers basic update:\n");
  fprintf(fp, "k %d, m %d, s %d\n", k, m, s);
  for(uint32_t i = 0; i < l; ++i) {
    if(data_blocks_required_tag[i] == 1) {
      fprintf(fp, "data block %d, ", i);
    }
  }
  fprintf(fp, "are required for parity update\n");
  for(uint32_t i = 0; i < num_stripes_post; ++i) {
    fprintf(fp, "P'%d = ", i);
    for(uint32_t j = 0; j < num_stripes_pre + l; ++j) {
      if(parity_update_vector[i][j] == 1) {
        if(j < num_stripes_pre) {
          fprintf(fp, "P%d, ", j);
        } else {
          fprintf(fp, "D%d, ", j - num_stripes_pre);
        }
      }
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
  fclose(fp);*/

}

//ERSadd
static void ERS_enhanced_calculate_update(uint32_t k, uint32_t m, uint32_t s, 
            uint32_t *data_blocks_required_tag, /* size: l */
            uint32_t **parity_update_vector, /* size: num_stripes_post * (num_stripes_pre + l) */ 
            uint32_t *h_ids, uint32_t *v_ids ) {
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_pre = l/k;
  uint32_t num_stripes_post = l/s;
  uint32_t row_num = l/s;
  uint32_t x = s / k;
  uint32_t y = s % k;
  uint32_t first_class_row_num = l / k - x*row_num;
  uint32_t second_class_row_num = row_num - first_class_row_num;

  if(y == 0) {
    for(uint32_t i = 0; i < num_stripes_post; ++i) {
      for(uint32_t j = 0; j < x; ++j) {
        parity_update_vector[i][i*x+j] = 1;
      }
    }
    return;
  }

  // FIRST-CLASS rows
  for(uint32_t i = 0; i < first_class_row_num; ++i) {
    uint32_t rotate_num = i*(k-y);
    uint32_t full_stripe_id = i*x;
    uint32_t partial_stripe_id = x*row_num + i;
    uint32_t start_idx = rotate_num % s;
      // fill x full stripes [length k*x]
    for(uint32_t tt = 0; tt < k*x; ++tt) {
      uint32_t hid = (start_idx + tt) % s;
      uint32_t vid = i;
      h_ids[full_stripe_id*k + tt] = hid;
      v_ids[full_stripe_id*k + tt] = vid;
    }
    for(uint32_t pp = 0; pp < x; ++pp) {
      parity_update_vector[i][full_stripe_id + pp] = 1;
    }
      // fill a partial stripe [length y]
    uint32_t end_idx = (start_idx + k*x) % s;
    parity_update_vector[i][partial_stripe_id] = 1;
    for(uint32_t tt = 0; tt < y; ++tt) {
      uint32_t hid = (end_idx + tt) % s;
      uint32_t vid = i;
      h_ids[partial_stripe_id*k + tt] = hid;
      v_ids[partial_stripe_id*k + tt] = vid;
    }
    for(uint32_t tt = y; tt < k; ++tt) {
      parity_update_vector[i][partial_stripe_id*k + tt + num_stripes_pre] = 1;
    }
  }

  // SECOND-CLASS rows
  uint32_t partial_stripe_id = x*row_num;
  uint32_t partial_stripe_start_idx = y;
  for(uint32_t j = 0; j < second_class_row_num; ++j) {
    uint32_t rotate_num = (j+1)*y;
    uint32_t full_stripe_id = (j + first_class_row_num)*x;
    uint32_t start_idx = rotate_num % s;
      // fill x full stripes [length k*x]
    for(uint32_t tt = 0; tt < k*x; ++tt) {
      uint32_t hid = (start_idx + tt) % s;
      uint32_t vid = j + first_class_row_num;
      h_ids[full_stripe_id*k + tt] = hid;
      v_ids[full_stripe_id*k + tt] = vid;
    }
    for(uint32_t pp = 0; pp < x; ++pp) {
      parity_update_vector[j + first_class_row_num][full_stripe_id + pp] = 1;
    }
      // fill the partial stripes [each of which is of length k - y]
    uint32_t end_idx = (start_idx + k*x) % s;
    for(uint32_t tt = 0; tt < y; ++tt) {
      uint32_t hid = (end_idx + tt) % s;
      uint32_t vid = j + first_class_row_num;
      h_ids[partial_stripe_id*k + partial_stripe_start_idx] = hid;
      v_ids[partial_stripe_id*k + partial_stripe_start_idx] = vid;
      data_blocks_required_tag[partial_stripe_id*k + partial_stripe_start_idx] = 1;
      parity_update_vector[j + first_class_row_num][partial_stripe_id*k + partial_stripe_start_idx + num_stripes_pre] = 1;
      partial_stripe_start_idx++;
      if(partial_stripe_start_idx == k) {
        partial_stripe_id++;
        partial_stripe_start_idx = y;
      }
    }
  }

  /*FILE* fp = fopen("./kobe", "a");
  fprintf(fp, "ers enhanced !!! update:\n");
  fprintf(fp, "k %d, m %d, s %d\n", k, m, s);
  for(uint32_t i = 0; i < l; ++i) {
    if(data_blocks_required_tag[i] == 1) {
      fprintf(fp, "data block %d, ", i);
    }
  }
  fprintf(fp, "are required for parity update\n");
  for(uint32_t i = 0; i < num_stripes_post; ++i) {
    fprintf(fp, "P'%d = ", i);
    for(uint32_t j = 0; j < num_stripes_pre + l; ++j) {
      if(parity_update_vector[i][j] == 1) {
        if(j < num_stripes_pre) {
          fprintf(fp, "P%d, ", j);
        } else {
          fprintf(fp, "D%d, ", j - num_stripes_pre);
        }
      }
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
  fclose(fp);*/

}

//ERSadd
static void ERS_update_parity(uint32_t k, uint32_t m, uint32_t s, size_t block_size, 
            char **data_ptrs, uint32_t number_data_blocks_required, 
            uint32_t *data_blocks_required_index, /* size: number_data_blocks_required */
            char **coding_ptrs, /* size: m*num_stripes_pre */
            char **new_codings, /* size: m*num_stripes_post */
            uint32_t **parity_update_vector, /* size: num_stripes_post * (num_stripes_pre + l) */
            int *matrix, /*size: m*s */
            uint32_t *h_ids, uint32_t *v_ids, bool enhanced) {
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_pre = l/k;
  uint32_t num_stripes_post = l/s;
  //FILE *fp = fopen("./kewen", "a");
  //fprintf(fp, "k %d, m %d, s %d\n", k, m, s);
  //if(enhanced) fprintf(fp, "ers enhanced placement...\n");
  for(uint32_t parity_offset = 0; parity_offset < num_stripes_post; ++parity_offset) {
    for(uint32_t parity_node_id = 0; parity_node_id < m; ++parity_node_id) {
      // [[new parity id]]: parity_offset*m + parity_node_id
      for(uint32_t r = 0 ; r < num_stripes_pre + l; ++r) {
        if(parity_update_vector[parity_offset][r] == 1) {
          if(r < num_stripes_pre) {
            // [[old parity id]]: r*m + parity_node_id
            galois_region_xor(new_codings[parity_offset*m + parity_node_id],         /* Region 1 */
                              coding_ptrs[r*m + parity_node_id],         /* Region 2 */
                              new_codings[parity_offset*m + parity_node_id],         /* Sum region (r3 = r1 ^ r2) -- can be r1 or r2 */
                              block_size);
            //fprintf(fp, "P'%d <- P%d \n", parity_offset*m + parity_node_id, r*m + parity_node_id);
          } else {
            // [[data id]]: r - num_stripes_pre
            int hid;
            if(enhanced) {
              hid = h_ids[r - num_stripes_pre];
            } else {
              hid = (r - num_stripes_pre) % s;
            }
            char *temp_buf = new char[block_size];
            int index = 0;
            for(; index < number_data_blocks_required; ++index) {
              if(data_blocks_required_index[index] == r - num_stripes_pre) break;
            }
            memcpy(temp_buf, data_ptrs[index], block_size);
            galois_w08_region_multiply(temp_buf,       /* Region to multiply */
                                  matrix[parity_node_id*s + hid],       /* Number to multiply by */
                                  block_size,       /* Number of bytes in region */
                                  temp_buf,         /* If r2 != NULL, products go here.
                                                       Otherwise region is overwritten */
                                  0);         /* If (r2 != NULL && add) the products is XOR'd with r2 */

            galois_region_xor(new_codings[parity_offset*m + parity_node_id],
                              temp_buf,
                              new_codings[parity_offset*m + parity_node_id],
                              block_size);
            //fprintf(fp, "P'%d <- (%d) D%d \n", parity_offset*m + parity_node_id, 
            //matrix[parity_node_id*s + hid], r - num_stripes_pre);
            delete temp_buf;
          }
        }
      }
    }
  }
  //fclose(fp);
}

//ERSadd
static memcached_return_t ERS_upload_new_parity_1p(memcached_st *ptr, const char *key, size_t key_length, 
            uint32_t k, uint32_t m, uint32_t s, size_t block_size, 
            char **new_codings /* size: m*num_stripes_post */) {
  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, true)))
  {
    return rc;
  }
  
  uint32_t server_key= memcached_generate_hash_with_redistribution(ptr, key, key_length);
  memcached_instance_st* server= memcached_instance_fetch(ptr, server_key);

  WATCHPOINT_SET(instance->io_wait_count.read= 0);
  WATCHPOINT_SET(instance->io_wait_count.write= 0);

  bool flush= true;
  if (memcached_is_buffering(server->root))
  {
    flush= false;
  }

  bool reply= memcached_is_replying(ptr);

  memcached_storage_action_t verb = SET_OP;
  time_t expiration = 0;
  uint32_t flags = 0;
  uint64_t cas = 0;

  uint32_t l = lcm(k, s);
  uint32_t num_stripes_pre = l/k;
  uint32_t num_stripes_post = l/s;

  size_t *value_length = new size_t[m*num_stripes_post];
  for(uint32_t i = 0; i < m*num_stripes_post; ++i) {
    value_length[i] = block_size;
  }
  for(uint32_t i = 0; i < m*num_stripes_post; ++i) {
    hashkit_string_st* destination= NULL;

    if (memcached_is_encrypted(ptr))
    {
      if (can_by_encrypted(verb) == false)
      {
        return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT, 
                                   memcached_literal_param("Operation not allowed while encyrption is enabled"));
      }

      if ((destination= hashkit_encrypt(&ptr->hashkit, new_codings[i], block_size)) == NULL)
      {
        return rc;
      }
      const char *temp_value = hashkit_string_c_str(destination);
      delete new_codings[i];
      new_codings[i] = new char[strlen(temp_value) + 1];
      strcpy(new_codings[i], temp_value);
	  delete temp_value;
      value_length[i]= hashkit_string_length(destination);
    }
  } // end of encyrption

  for(uint32_t i = 0; i < m*num_stripes_post; ++i) {
    uint32_t hid = i % m;

    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = (i+l) / 100;
    int ten = ((i+l) - handred*100) / 10;
    int one = (i+l) - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t parity_server = (server_key + s + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, parity_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
	//request.message.header.request.opcode= PROTOCOL_BINARY_CMD_SET_PREPARE;
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { new_codings[i], value_length[i] }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    rc = memcached_response(instance, NULL, 0, NULL);
    if(rc == MEMCACHED_SUCCESS) {
    } else {
      return rc;
    }
  } // end of for

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }

  //return memcached_response(server, NULL, 0, NULL);
  return MEMCACHED_SUCCESS;
}

//ERSadd
static memcached_return_t ERS_upload_new_parity_2p(memcached_st *ptr, const char *key, size_t key_length, 
            uint32_t k, uint32_t m, uint32_t s) {
  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, true)))
  {
    return rc;
  }
  
  uint32_t server_key= memcached_generate_hash_with_redistribution(ptr, key, key_length);
  memcached_instance_st* server= memcached_instance_fetch(ptr, server_key);

  WATCHPOINT_SET(instance->io_wait_count.read= 0);
  WATCHPOINT_SET(instance->io_wait_count.write= 0);

  bool flush= true;
  if (memcached_is_buffering(server->root))
  {
    flush= false;
  }

  bool reply= memcached_is_replying(ptr);

  memcached_storage_action_t verb = SET_OP;
  time_t expiration = 0;
  uint32_t flags = 0;
  uint64_t cas = 0;

  uint32_t l = lcm(k, s);
  uint32_t num_stripes_post = l/s;

  char *temp_value = new char[3];
  temp_value[0] = 'c';
  temp_value[1] = 'o';
  temp_value[2] = '\0';
  size_t temp_value_length = 2;

  for(uint32_t i = 0; i < m*num_stripes_post; ++i) {
    uint32_t hid = i % m;

    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = (i+l) / 100;
    int ten = ((i+l) - handred*100) / 10;
    int one = (i+l) - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t parity_server = (server_key + s + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, parity_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
	//request.message.header.request.opcode= PROTOCOL_BINARY_CMD_SET_COMMIT;
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + temp_value_length +
                                                            request.message.header.request.extlen));

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { temp_value, temp_value_length }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    rc = memcached_response(instance, NULL, 0, NULL);
    if(rc == MEMCACHED_SUCCESS) {
    } else {
      return rc;
    }
  } // end of for

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }

  //return memcached_response(server, NULL, 0, NULL);
  return MEMCACHED_SUCCESS;
}

//ERSadd
static memcached_return_t ERS_revert_to_old_parity(memcached_st *ptr, const char *key, size_t key_length, 
            uint32_t k, uint32_t m, uint32_t s, size_t block_size, 
            char **coding_ptrs /* size: m*num_stripes_pre */) {
  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, true)))
  {
    return rc;
  }
  
  uint32_t server_key= memcached_generate_hash_with_redistribution(ptr, key, key_length);
  memcached_instance_st* server= memcached_instance_fetch(ptr, server_key);

  WATCHPOINT_SET(instance->io_wait_count.read= 0);
  WATCHPOINT_SET(instance->io_wait_count.write= 0);

  bool flush= true;
  if (memcached_is_buffering(server->root))
  {
    flush= false;
  }

  bool reply= memcached_is_replying(ptr);

  memcached_storage_action_t verb = SET_OP;
  time_t expiration = 0;
  uint32_t flags = 0;
  uint64_t cas = 0;

  uint32_t l = lcm(k, s);
  uint32_t num_stripes_pre = l/k;
  uint32_t num_stripes_post = l/s;

  size_t *value_length = new size_t[m*num_stripes_pre];
  for(uint32_t i = 0; i < m*num_stripes_pre; ++i) {
    value_length[i] = block_size;
  }
  for(uint32_t i = 0; i < m*num_stripes_pre; ++i) {
    hashkit_string_st* destination= NULL;

    if (memcached_is_encrypted(ptr))
    {
      if (can_by_encrypted(verb) == false)
      {
        return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT, 
                                   memcached_literal_param("Operation not allowed while encyrption is enabled"));
      }

      if ((destination= hashkit_encrypt(&ptr->hashkit, coding_ptrs[i], block_size)) == NULL)
      {
        return rc;
      }
      const char *temp_value = hashkit_string_c_str(destination);
      delete coding_ptrs[i];
      coding_ptrs[i] = new char[strlen(temp_value) + 1];
      strcpy(coding_ptrs[i], temp_value);
	  delete temp_value;
      value_length[i]= hashkit_string_length(destination);
    }
  } // end of encyrption

  for(uint32_t i = 0; i < m*num_stripes_pre; ++i) {
    uint32_t hid = i % m;

    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = (i+l) / 100;
    int ten = ((i+l) - handred*100) / 10;
    int one = (i+l) - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t parity_server = (server_key + s + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, parity_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { coding_ptrs[i], value_length[i] }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    rc = memcached_response(instance, NULL, 0, NULL);
    if(rc == MEMCACHED_SUCCESS) {
    } else {
      return rc;
    }
  } // end of for

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }

  //return memcached_response(server, NULL, 0, NULL);
  return MEMCACHED_SUCCESS;
}

//ERSadd
static void ERS_add_moving_key(memcached_st *ptr, const char *key, size_t key_length) {
  // insert into moving_key_list
  struct key_st *new_key = (struct key_st *)malloc(sizeof(struct key_st));
  new_key->key_name = new char[key_length + 1];
  strncpy(new_key->key_name, key, key_length);
  new_key->key_name[key_length] = '\0';
  new_key->next = NULL;

  if(ptr->moving_key_list == NULL) {
    ptr->moving_key_list = new_key;
  } else {
    struct key_st *temp = ptr->moving_key_list;
    while(temp->next != NULL) {
      temp = temp->next;
    }
    temp->next = new_key;
  }
}

//ERSadd
static void ERS_delete_moving_key(memcached_st *ptr, const char *key) {
  // delete from moving_key_list
  struct key_st *previous = NULL;
  struct key_st *temp = ptr->moving_key_list;
  while(temp != NULL) {
    if(strcmp(temp->key_name, key) == 0) {
      break;
    }
    previous = temp;
    temp = temp->next;
  }
  if(previous == NULL) {
    ptr->moving_key_list = temp->next;
    free(temp);
  } else {
    previous->next = temp->next;
    free(temp);
  }
}

//ERSadd
static bool ERS_is_moving(memcached_st *ptr, const char *key) {
  struct key_st *temp = ptr->moving_key_list;
  while(temp != NULL) {
    if(strcmp(temp->key_name, key) == 0) {
      break;
    }
    temp = temp->next;
  }
  if(temp != NULL) {
    return true;
  } else {
    return false;
  }
}

//ERSadd
static void ERS_add_moved_key(memcached_st *ptr, const char *key, size_t key_length) {
  // insert into moved_key_list
  struct key_st *new_key = (struct key_st *)malloc(sizeof(struct key_st));
  new_key->key_name = new char[key_length + 1];
  strncpy(new_key->key_name, key, key_length);
  new_key->key_name[key_length] = '\0';
  new_key->next = NULL;

  if(ptr->moved_key_list == NULL) {
    ptr->moved_key_list = new_key;
  } else {
    struct key_st *temp = ptr->moved_key_list;
    while(temp->next != NULL) {
      temp = temp->next;
    }
    temp->next = new_key;
  }
}

//ERSadd
static bool ERS_is_moved(memcached_st *ptr, const char *key) {
  struct key_st *temp = ptr->moved_key_list;
  while(temp != NULL) {
    if(strcmp(temp->key_name, key) == 0) {
      break;
    }
    temp = temp->next;
  }
  if(temp != NULL) {
    return true;
  } else {
    return false;
  }
}

//ERSadd
static void ERS_traverse_moving_and_moved_list(memcached_st *ptr){
  //FILE *fp = fopen("./fxy", "a");
  //fprintf(fp, "moving key list:\n");
  struct key_st *temp = ptr->moving_key_list;
  while(temp != NULL) {
    //fprintf(fp, "%s\n", temp->key_name);
    temp = temp->next;
  }
  //fprintf(fp, "\nmoved key list:\n");
  temp = ptr->moved_key_list;
  while(temp != NULL) {
    //fprintf(fp, "%s\n", temp->key_name);
    temp = temp->next;
  }
  //fprintf(fp, "\n");
  //fclose(fp);
}

//ERSadd
static void calculate_parity_ptrs_srs_after_transition(Memcached *ptr, char **data_ptrs, char **coding_ptrs, uint32_t k, uint32_t s, uint32_t m, size_t block_size) {
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_post = l/s;

  char **temp_data_ptrs = new char*[s];
  for(uint32_t i = 0; i < s; ++i) {
    temp_data_ptrs[i] = new char[block_size];
  }
  char **temp_coding_ptrs = new char*[m];
  for(uint32_t i = 0; i < m; ++i) {
    temp_coding_ptrs[i] = new char[block_size];
  }

  int *matrix = reed_sol_vandermonde_coding_matrix(s, m, 8);

  for(uint32_t i = 0; i < num_stripes_post; ++i) {
    for(uint32_t j = 0; j < s; ++j) {
      memset(temp_data_ptrs[j], 0, block_size);
    }
    for(uint32_t j = 0; j < m; ++j) {
      memset(temp_coding_ptrs[j], 0, block_size);
    }

    for(uint32_t j = 0; j < s; ++j) {
      uint32_t data_idx = j*num_stripes_post + i;
      memcpy(temp_data_ptrs[j], data_ptrs[data_idx], block_size);
    }

    jerasure_matrix_encode(s, m, 8, matrix, temp_data_ptrs, temp_coding_ptrs, block_size);

    for(uint32_t j = 0; j < m; ++j) {
      memcpy(coding_ptrs[i*m + j], temp_coding_ptrs[j], block_size);
    }
  }

  for(uint32_t i = 0; i < s; ++i) {
    delete temp_data_ptrs[i];
  }
  delete temp_data_ptrs;
  for(uint32_t i = 0; i < m; ++i) {
    delete temp_coding_ptrs[i];
  }
  delete temp_coding_ptrs;
  free(matrix);
}

//ERSadd
static memcached_return_t memcached_srs_send_binary_after_transition(Memcached *ptr,
                                                   memcached_instance_st* server,
                                                   uint32_t server_key,
                                                   const char *key,
                                                   const size_t key_length,
                                                   const char *value,
                                                   const size_t value_length,
                                                   const time_t expiration,
                                                   const uint32_t flags,
                                                   const uint64_t cas,
                                                   const bool flush,
                                                   const bool reply,
                                                   memcached_storage_action_t verb)
{
  // determining block size
  uint32_t k = ptr->number_of_k;
  uint32_t s = ptr->number_of_s;
  uint32_t m = ptr->number_of_m;
  size_t block_size = determine_block_size_ers(value_length, k, s);
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_post = l/s;

  // fill data blocks
  char **data_ptrs = new char*[l];
  for(uint32_t i = 0; i < l; ++i) {
    data_ptrs[i] = new char[block_size];
    memset(data_ptrs[i], 0, block_size);		
  }
  fill_data_ptrs(data_ptrs, l, block_size, value, value_length);

  // calculate parity blocks
  char **coding_ptrs = new char*[m*num_stripes_post];
  for(uint32_t i = 0; i < m*num_stripes_post; ++i) {
    coding_ptrs[i] = new char[block_size];
    memset(coding_ptrs[i], 0, block_size);
  }
  calculate_parity_ptrs_srs_after_transition(ptr, data_ptrs, coding_ptrs, k, s, m, block_size);

  // generate l + m*num_stripes_post requests for 
  // l data blocks and 
  // m*num_stripes_post parity blocks
  size_t last_data_request_size = value_length - block_size * (l - 1);
  char *last_data_request = new char[last_data_request_size];
  memcpy(last_data_request, data_ptrs[l-1], last_data_request_size);
  for(uint32_t i = 0; i < l; ++i) {
    uint32_t hid = i / num_stripes_post;
    //uint32_t vid = i % num_stripes_post;
    
    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = i / 100;
    int ten = (i - handred*100) / 10;
    int one = i - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t data_server = (server_key + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, data_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    if(i < l - 1) {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));
    } else {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + last_data_request_size +
                                                            request.message.header.request.extlen));
    }

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { i<(l-1)?data_ptrs[i]:last_data_request, i<(l-1)?block_size:last_data_request_size }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    if((rc = memcached_response(instance, NULL, 0, NULL)) == MEMCACHED_SUCCESS) {
    }
  } // end of for
  if(data_ptrs[l-1] != NULL) delete data_ptrs[l-1];

  for(uint32_t i = l; i < l + m*num_stripes_post; ++i) {
    uint32_t hid = (i - l) % m;
    //uint32_t vid = (i - l) / m;

    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = i / 100;
    int ten = (i - handred*100) / 10;
    int one = i - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t parity_server = (server_key + s + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, parity_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { coding_ptrs[i-l], block_size }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    if((rc = memcached_response(instance, NULL, 0, NULL)) == MEMCACHED_SUCCESS) {
    }
  } // end of for

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }

  return MEMCACHED_SUCCESS;
}

//ERSadd
static void calculate_parity_ptrs_ers_after_transition(Memcached *ptr, uint32_t *h_ids, uint32_t *v_ids, char **data_ptrs, char **coding_ptrs, uint32_t k, uint32_t s, uint32_t m, size_t block_size) {
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_post = l/s;

  bool enhanced = (ptr->ers_enhanced_placement > 0 && s % k != 0);
  if(enhanced) enhanced_placement_ers(h_ids, v_ids, k, s);

  char **temp_data_ptrs = new char*[s];
  for(uint32_t i = 0; i < s; ++i) {
    temp_data_ptrs[i] = new char[block_size];
  }
  char **temp_coding_ptrs = new char*[m];
  for(uint32_t i = 0; i < m; ++i) {
    temp_coding_ptrs[i] = new char[block_size];
  }

  // an m*s dimensional big matrix
  int *matrix = reed_sol_vandermonde_coding_matrix(s, m, 8);

  for(uint32_t i = 0; i < num_stripes_post; ++i) {
    for(uint32_t j = 0; j < s; ++j) {
      memset(temp_data_ptrs[j], 0, block_size);
    }
    for(uint32_t j = 0; j < m; ++j) {
      memset(temp_coding_ptrs[j], 0, block_size);
    }

    if(enhanced) {
      for(uint32_t j = 0; j < s; ++j) {
        uint32_t vid = i;
        uint32_t hid = j;

        uint32_t index = 0;
        for(; index < l; ++index) {
          if(h_ids[index] == hid && v_ids[index] == vid) {
            break;
          }
        }

        memcpy(temp_data_ptrs[j], data_ptrs[index], block_size);
      }
    } else {
      for(uint32_t j = 0; j < s; ++j) {
        memcpy(temp_data_ptrs[j], data_ptrs[i*s + j], block_size);
      }
    } //

    jerasure_matrix_encode(s, m, 8, matrix, temp_data_ptrs, temp_coding_ptrs, block_size);

    for(uint32_t j = 0; j < m; ++j) {
      memcpy(coding_ptrs[i*m + j], temp_coding_ptrs[j], block_size);
    }
  }

  for(uint32_t i = 0; i < s; ++i) {
    delete temp_data_ptrs[i];
  }
  delete temp_data_ptrs;
  for(uint32_t i = 0; i < m; ++i) {
    delete temp_coding_ptrs[i];
  }
  delete temp_coding_ptrs;
  free(matrix);
}

//ERSadd
static memcached_return_t memcached_ers_send_binary_after_transition(Memcached *ptr,
                                                   memcached_instance_st* server,
                                                   uint32_t server_key,
                                                   const char *key,
                                                   const size_t key_length,
                                                   const char *value,
                                                   const size_t value_length,
                                                   const time_t expiration,
                                                   const uint32_t flags,
                                                   const uint64_t cas,
                                                   const bool flush,
                                                   const bool reply,
                                                   memcached_storage_action_t verb)
{
  // determining block size
  uint32_t k = ptr->number_of_k;
  uint32_t s = ptr->number_of_s;
  uint32_t m = ptr->number_of_m;
  size_t block_size = determine_block_size_ers(value_length, k, s);
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_post = l/s;

  // store the positions of each data block
  // in the data store array
  uint32_t *h_ids = new uint32_t[l];
  uint32_t *v_ids = new uint32_t[l];
  for(uint32_t i = 0; i < l; ++i) {
    h_ids[i] = v_ids[i] = 0;
  }

  // fill data blocks
  char **data_ptrs = new char*[l];
  for(uint32_t i = 0; i < l; ++i) {
    data_ptrs[i] = new char[block_size];
    memset(data_ptrs[i], 0, block_size);		
  }
  fill_data_ptrs(data_ptrs, l, block_size, value, value_length);

  // calculate parity blocks
  char **coding_ptrs = new char*[m*num_stripes_post];
  for(uint32_t i = 0; i < m*num_stripes_post; ++i) {
    coding_ptrs[i] = new char[block_size];
    memset(coding_ptrs[i], 0, block_size);
  }
  calculate_parity_ptrs_ers_after_transition(ptr, h_ids, v_ids, data_ptrs, coding_ptrs, k, s, m, block_size);
  bool enhanced = (ptr->ers_enhanced_placement > 0 && s % k != 0);

  // generate l + m*num_stripes_post requests for 
  // l data blocks and 
  // m*num_stripes_post parity blocks
  size_t last_data_request_size = value_length - block_size * (l - 1);
  char *last_data_request = new char[last_data_request_size];
  memcpy(last_data_request, data_ptrs[l-1], last_data_request_size);
  for(uint32_t i = 0; i < l; ++i) {
    uint32_t hid;
    if(enhanced) {
      hid = h_ids[i];
    } else {
      hid = i % s;
    }
    //uint32_t vid = i / s;

    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = i / 100;
    int ten = (i - handred*100) / 10;
    int one = i - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t data_server = (server_key + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, data_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    if(i < l - 1) {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));
    } else {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + last_data_request_size +
                                                            request.message.header.request.extlen));
    }

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { i<(l-1)?data_ptrs[i]:last_data_request, i<(l-1)?block_size:last_data_request_size }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    if((rc = memcached_response(instance, NULL, 0, NULL)) == MEMCACHED_SUCCESS) {
    }
  } // end of for
  if(data_ptrs[l-1] != NULL) delete data_ptrs[l-1];

  for(uint32_t i = l; i < l + m*num_stripes_post; ++i) {
    uint32_t hid = (i - l) % m;
    //uint32_t vid = (i - l) / m;

    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = i / 100;
    int ten = (i - handred*100) / 10;
    int one = i - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t parity_server = (server_key + s + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, parity_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { coding_ptrs[i-l], block_size }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    if((rc = memcached_response(instance, NULL, 0, NULL)) == MEMCACHED_SUCCESS) {
    }
  } // end of for

  delete h_ids;
  delete v_ids;

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }

  return MEMCACHED_SUCCESS;
}
// ~~~ subfunctions for RS, ERS, SRS transitioning, end ~~~



//ERSadd
/*
 * ERS, horizontal placement + designed matrix, transition
 */
static memcached_return_t ERS_basic_move(memcached_st *ptr, const char *key, size_t key_length) {
  ERS_add_moving_key(ptr, key, key_length);
  ERS_traverse_moving_and_moved_list(ptr);

  uint32_t k = ptr->number_of_k;
  uint32_t m = ptr->number_of_m;
  uint32_t s = ptr->number_of_s;
  uint32_t l = lcm(k, s);
  uint32_t *data_blocks_required_tag = new uint32_t[l];
  for(uint32_t i = 0; i < l; ++i) {
    data_blocks_required_tag[i] = 0;
  }
  uint32_t num_stripes_pre = l/k;
  uint32_t num_stripes_post = l/s;
  uint32_t **parity_update_vector = new uint32_t*[num_stripes_post];
  for(uint32_t i = 0; i < num_stripes_post; ++i) {
    parity_update_vector[i] = new uint32_t[num_stripes_pre + l];
    for(uint32_t j = 0; j < num_stripes_pre + l; ++j) {
      parity_update_vector[i][j] = 0;
    }
  }

  // calculate the parity update functions
  ERS_basic_calculate_update(k, m, s, data_blocks_required_tag, parity_update_vector);
  uint32_t number_data_blocks_required = 0;
  for(uint32_t i = 0; i < l; ++i) {
    if(data_blocks_required_tag[i] == 1) {
      number_data_blocks_required++;
    }
  }
  uint32_t *data_blocks_required_index = new uint32_t[number_data_blocks_required];
  uint32_t temp = 0;
  for(uint32_t i = 0; i < l; ++i) {
    if(data_blocks_required_tag[i] == 1) {
      data_blocks_required_index[temp] = i;
      temp++;
    }
  }

  // generate sub keys
  char **key_ptrs = new char*[number_data_blocks_required + num_stripes_pre*m];
  size_t *key_len_ptrs = new size_t[number_data_blocks_required + num_stripes_pre*m];
  char **data_ptrs = new char*[number_data_blocks_required];
  char **coding_ptrs = new char*[m*num_stripes_pre];
  for(uint32_t i = 0; i < number_data_blocks_required + num_stripes_pre*m; ++i) {
      key_ptrs[i] = new char[key_length + 4];
      memset(key_ptrs[i], 0, key_length + 4);
      memcpy(key_ptrs[i], key, key_length);
      int original_index;
      if(i < number_data_blocks_required) {
        original_index = data_blocks_required_index[i];
      } else {
        original_index = i - number_data_blocks_required + l;
      }
      int handred = original_index / 100;
      int ten = (original_index - handred*100) / 10;
      int one = original_index - handred*100 - ten*10;
      char handred_bit_char = '0' + handred;
      char ten_bit_char = '0' + ten;
      char one_bit_char = '0' + one;
      key_ptrs[i][key_length] = handred_bit_char;
      key_ptrs[i][key_length + 1] = ten_bit_char;
      key_ptrs[i][key_length + 2] = one_bit_char;
      key_ptrs[i][key_length + 3] = '\0';

      key_len_ptrs[i] = key_length + 3;
  }
  
  // multiple get
  memcached_return_t error = memcached_mget(ptr, (const char * const *)key_ptrs, key_len_ptrs, 
      number_data_blocks_required + num_stripes_pre*m);

  // fetch result
  size_t block_size;
  size_t last_data_request_size;
  if(error == MEMCACHED_SUCCESS) {
    memcached_result_st *result_buffer = &ptr->result;
    char *temp_key = new char[key_length + 4];
    while((result_buffer = memcached_fetch_result(ptr, result_buffer, &error)) != NULL) {
        strncpy(temp_key, result_buffer->item_key, result_buffer->key_length);
        temp_key[key_length + 3] = '\0';
        uint32_t i;
        for(i = 0; i < number_data_blocks_required + num_stripes_pre*m; ++i) {
          if(strcmp(temp_key, key_ptrs[i]) == 0) break;
        }

        if(i == number_data_blocks_required) {
          block_size = memcached_result_length(result_buffer);
        }
        if(i < number_data_blocks_required && data_blocks_required_index[i] == l-1) {
          last_data_request_size = memcached_result_length(result_buffer);
        }

        if(i < number_data_blocks_required) {
          data_ptrs[i] = new char[memcached_result_length(result_buffer)];
          memcpy(data_ptrs[i], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
        } else {
          coding_ptrs[i - number_data_blocks_required] = new char[memcached_result_length(result_buffer)];
          memcpy(coding_ptrs[i - number_data_blocks_required], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
        }
    }
  }

  // handle the last data request
  for(uint32_t i = 0; i < number_data_blocks_required; ++i) {
    if(data_blocks_required_index[i] == l-1) {
      char* temp_buf = new char[block_size];
      memset(temp_buf, 0, block_size);
      memcpy(temp_buf, data_ptrs[i], last_data_request_size);
      delete data_ptrs[i];
      data_ptrs[i] = new char[block_size];
      memset(data_ptrs[i], 0, block_size);
      memcpy(data_ptrs[i], temp_buf, block_size);
      delete temp_buf;
    }
  }

  // generate the encoding matrix
  // an m*s dimensional big matrix
  int *matrix = reed_sol_vandermonde_coding_matrix(s, m, 8);

  // update parity blocks
  char **new_codings = new char*[m*num_stripes_post];
  for(uint32_t i = 0; i < m*num_stripes_post; ++i) {
    new_codings[i] = new char[block_size];
    memset(new_codings[i], 0, block_size);
  }

  uint32_t *h_ids = new uint32_t[l];
  uint32_t *v_ids = new uint32_t[l];
  for(uint32_t i = 0; i < l; ++i) {
    h_ids[i] = v_ids[i] = 0;
  }
  bool enhanced = (ptr->ers_enhanced_placement > 0 && ptr->number_of_s % ptr->number_of_k != 0);

  ERS_update_parity(k, m, s, block_size, 
            data_ptrs, number_data_blocks_required, 
            data_blocks_required_index, 
            coding_ptrs, new_codings, 
    		parity_update_vector, matrix, h_ids, v_ids, enhanced);

  // upload new parity blocks
  error = ERS_upload_new_parity_1p(ptr, key, key_length, 
            k, m, s, block_size, new_codings);
  if(error == MEMCACHED_SUCCESS) {
    //ERS_upload_new_parity_2p(ptr, key, key_length, 
    //        k, m, s);
    ERS_delete_moving_key(ptr, key);
    ERS_add_moved_key(ptr, key, key_length);
    ERS_traverse_moving_and_moved_list(ptr);
  } else {
    ERS_revert_to_old_parity(ptr, key, key_length, 
            k, m, s, block_size, coding_ptrs);
    ERS_delete_moving_key(ptr, key);
    ERS_traverse_moving_and_moved_list(ptr);
    return error;
  }

  // free operations
  if(data_blocks_required_tag != NULL) delete data_blocks_required_tag;
  if(parity_update_vector != NULL) {
    for(uint32_t i = 0; i < num_stripes_post; ++i) {
      if(parity_update_vector[i] != NULL) delete parity_update_vector[i];
    }
    delete parity_update_vector;
  }
  if(data_blocks_required_index != NULL) delete data_blocks_required_index;

  if(key_ptrs != NULL) {
    for(uint32_t i = 0; i < number_data_blocks_required + num_stripes_pre*m; ++i) {
      if(key_ptrs[i] != NULL) delete key_ptrs[i];
    }
    delete key_ptrs;
  }
  if(key_len_ptrs != NULL) delete key_len_ptrs;

  if(data_ptrs != NULL) {
    for(uint32_t i = 0; i < number_data_blocks_required; ++i) {
      if(data_ptrs[i] != NULL) delete data_ptrs[i];
    }
    delete data_ptrs;
  }
  if(coding_ptrs != NULL) {
    for(uint32_t i = 0; i < num_stripes_pre*m; ++i) {
      if(coding_ptrs[i] != NULL) delete coding_ptrs[i];
    }
    delete coding_ptrs;
  }
  if(matrix != NULL) free(matrix);

  delete h_ids;
  delete v_ids;

  return MEMCACHED_SUCCESS;
}

//ERSadd
/*
 * ERS, designed matrix + designed placement, transition
 */
static memcached_return_t ERS_enhanced_move(memcached_st *ptr, const char *key, size_t key_length) {
  ERS_add_moving_key(ptr, key, key_length);
  ERS_traverse_moving_and_moved_list(ptr);

  uint32_t k = ptr->number_of_k;
  uint32_t m = ptr->number_of_m;
  uint32_t s = ptr->number_of_s;
  uint32_t l = lcm(k, s);
  uint32_t *data_blocks_required_tag = new uint32_t[l];
  for(uint32_t i = 0; i < l; ++i) {
    data_blocks_required_tag[i] = 0;
  }
  uint32_t num_stripes_pre = l/k;
  uint32_t num_stripes_post = l/s;
  uint32_t **parity_update_vector = new uint32_t*[num_stripes_post];
  for(uint32_t i = 0; i < num_stripes_post; ++i) {
    parity_update_vector[i] = new uint32_t[num_stripes_pre + l];
    for(uint32_t j = 0; j < num_stripes_pre + l; ++j) {
      parity_update_vector[i][j] = 0;
    }
  }

  uint32_t *h_ids = new uint32_t[l];
  uint32_t *v_ids = new uint32_t[l];
  for(uint32_t i = 0; i < l; ++i) {
    h_ids[i] = v_ids[i] = 0;
  }
  bool enhanced = (ptr->ers_enhanced_placement > 0 && ptr->number_of_s % ptr->number_of_k != 0);

  // calculate the parity update functions
  ERS_enhanced_calculate_update(k, m, s, data_blocks_required_tag, parity_update_vector, h_ids, v_ids);
  uint32_t number_data_blocks_required = 0;
  for(uint32_t i = 0; i < l; ++i) {
    if(data_blocks_required_tag[i] == 1) {
      number_data_blocks_required++;
    }
  }
  uint32_t *data_blocks_required_index = new uint32_t[number_data_blocks_required];
  uint32_t temp = 0;
  for(uint32_t i = 0; i < l; ++i) {
    if(data_blocks_required_tag[i] == 1) {
      data_blocks_required_index[temp] = i;
      temp++;
    }
  }

  // generate sub keys
  char **key_ptrs = new char*[number_data_blocks_required + num_stripes_pre*m];
  size_t *key_len_ptrs = new size_t[number_data_blocks_required + num_stripes_pre*m];
  char **data_ptrs = new char*[number_data_blocks_required];
  char **coding_ptrs = new char*[m*num_stripes_pre];
  for(uint32_t i = 0; i < number_data_blocks_required + num_stripes_pre*m; ++i) {
      key_ptrs[i] = new char[key_length + 4];
      memset(key_ptrs[i], 0, key_length + 4);
      memcpy(key_ptrs[i], key, key_length);
      int original_index;
      if(i < number_data_blocks_required) {
        original_index = data_blocks_required_index[i];
      } else {
        original_index = i - number_data_blocks_required + l;
      }
      int handred = original_index / 100;
      int ten = (original_index - handred*100) / 10;
      int one = original_index - handred*100 - ten*10;
      char handred_bit_char = '0' + handred;
      char ten_bit_char = '0' + ten;
      char one_bit_char = '0' + one;
      key_ptrs[i][key_length] = handred_bit_char;
      key_ptrs[i][key_length + 1] = ten_bit_char;
      key_ptrs[i][key_length + 2] = one_bit_char;
      key_ptrs[i][key_length + 3] = '\0';

      key_len_ptrs[i] = key_length + 3;
  }
  
  // multiple get
  memcached_return_t error = memcached_mget(ptr, (const char * const *)key_ptrs, key_len_ptrs, 
      number_data_blocks_required + num_stripes_pre*m);

  // fetch result
  size_t block_size;
  size_t last_data_request_size;
  if(error == MEMCACHED_SUCCESS) {
    memcached_result_st *result_buffer = &ptr->result;
    char *temp_key = new char[key_length + 4];
    while((result_buffer = memcached_fetch_result(ptr, result_buffer, &error)) != NULL) {
        strncpy(temp_key, result_buffer->item_key, result_buffer->key_length);
        temp_key[key_length + 3] = '\0';
        uint32_t i;
        for(i = 0; i < number_data_blocks_required + num_stripes_pre*m; ++i) {
          if(strcmp(temp_key, key_ptrs[i]) == 0) break;
        }

        if(i == number_data_blocks_required) {
          block_size = memcached_result_length(result_buffer);
        }
        if(i < number_data_blocks_required && data_blocks_required_index[i] == l-1) {
          last_data_request_size = memcached_result_length(result_buffer);
        }

        if(i < number_data_blocks_required) {
          data_ptrs[i] = new char[memcached_result_length(result_buffer)];
          memcpy(data_ptrs[i], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
        } else {
          coding_ptrs[i - number_data_blocks_required] = new char[memcached_result_length(result_buffer)];
          memcpy(coding_ptrs[i - number_data_blocks_required], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
        }
    }
  }

  // handle the last data request
  for(uint32_t i = 0; i < number_data_blocks_required; ++i) {
    if(data_blocks_required_index[i] == l-1) {
      char* temp_buf = new char[block_size];
      memset(temp_buf, 0, block_size);
      memcpy(temp_buf, data_ptrs[i], last_data_request_size);
      delete data_ptrs[i];
      data_ptrs[i] = new char[block_size];
      memset(data_ptrs[i], 0, block_size);
      memcpy(data_ptrs[i], temp_buf, block_size);
      delete temp_buf;
    }
  }

  // generate the encoding matrix
  // an m*s dimensional big matrix
  int *matrix = reed_sol_vandermonde_coding_matrix(s, m, 8);

  // update parity blocks
  char **new_codings = new char*[m*num_stripes_post];
  for(uint32_t i = 0; i < m*num_stripes_post; ++i) {
    new_codings[i] = new char[block_size];
    memset(new_codings[i], 0, block_size);
  }
  ERS_update_parity(k, m, s, block_size, 
            data_ptrs, number_data_blocks_required, 
            data_blocks_required_index, 
            coding_ptrs, new_codings, 
    		parity_update_vector, matrix, h_ids, v_ids, enhanced);

  // upload new parity blocks
  error = ERS_upload_new_parity_1p(ptr, key, key_length, 
            k, m, s, block_size, new_codings);
  if(error == MEMCACHED_SUCCESS) {
    ERS_delete_moving_key(ptr, key);
    ERS_add_moved_key(ptr, key, key_length);
    ERS_traverse_moving_and_moved_list(ptr);
  } else {
    ERS_revert_to_old_parity(ptr, key, key_length, 
            k, m, s, block_size, coding_ptrs);
    ERS_delete_moving_key(ptr, key);
    ERS_traverse_moving_and_moved_list(ptr);
    return error;
  }

  // free operations
  if(data_blocks_required_tag != NULL) delete data_blocks_required_tag;
  if(parity_update_vector != NULL) {
    for(uint32_t i = 0; i < num_stripes_post; ++i) {
      if(parity_update_vector[i] != NULL) delete parity_update_vector[i];
    }
    delete parity_update_vector;
  }
  if(data_blocks_required_index != NULL) delete data_blocks_required_index;

  if(key_ptrs != NULL) {
    for(uint32_t i = 0; i < number_data_blocks_required + num_stripes_pre*m; ++i) {
      if(key_ptrs[i] != NULL) delete key_ptrs[i];
    }
    delete key_ptrs;
  }
  if(key_len_ptrs != NULL) delete key_len_ptrs;

  if(data_ptrs != NULL) {
    for(uint32_t i = 0; i < number_data_blocks_required; ++i) {
      if(data_ptrs[i] != NULL) delete data_ptrs[i];
    }
    delete data_ptrs;
  }
  if(coding_ptrs != NULL) {
    for(uint32_t i = 0; i < num_stripes_pre*m; ++i) {
      if(coding_ptrs[i] != NULL) delete coding_ptrs[i];
    }
    delete coding_ptrs;
  }
  if(matrix != NULL) free(matrix);

  delete h_ids;
  delete v_ids;

  return MEMCACHED_SUCCESS;
}

//ERSadd
/*
 * SRS, vertical placement, transition
 */
static memcached_return_t SRS_move(memcached_st *ptr, const char *key, size_t key_length) {
  ERS_add_moving_key(ptr, key, key_length);
  ERS_traverse_moving_and_moved_list(ptr);

  uint32_t k = ptr->number_of_k;
  uint32_t m = ptr->number_of_m;
  uint32_t s = ptr->number_of_s;
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_pre = l/k;
  uint32_t num_stripes_post = l/s;

  // retrieve all data blocks
  // generate sub keys
  char **key_ptrs = new char*[l];
  size_t *key_len_ptrs = new size_t[l];
  char **data_ptrs = new char*[l];
  for(uint32_t i = 0; i < l; ++i) {
      key_ptrs[i] = new char[key_length + 4];
      memset(key_ptrs[i], 0, key_length + 4);
      memcpy(key_ptrs[i], key, key_length);
      int handred = i / 100;
      int ten = (i - handred*100) / 10;
      int one = i - handred*100 - ten*10;
      char handred_bit_char = '0' + handred;
      char ten_bit_char = '0' + ten;
      char one_bit_char = '0' + one;
      key_ptrs[i][key_length] = handred_bit_char;
      key_ptrs[i][key_length + 1] = ten_bit_char;
      key_ptrs[i][key_length + 2] = one_bit_char;
      key_ptrs[i][key_length + 3] = '\0';

      key_len_ptrs[i] = key_length + 3;
  }
  
  // multiple get
  memcached_return_t error = memcached_mget(ptr, (const char * const *)key_ptrs, key_len_ptrs, 
      l);

  // fetch result
  size_t block_size;
  size_t last_data_request_size;
  if(error == MEMCACHED_SUCCESS) {
    memcached_result_st *result_buffer = &ptr->result;
    char *temp_key = new char[key_length + 4];
    while((result_buffer = memcached_fetch_result(ptr, result_buffer, &error)) != NULL) {
        strncpy(temp_key, result_buffer->item_key, result_buffer->key_length);
        temp_key[key_length + 3] = '\0';
        uint32_t i;
        for(i = 0; i < l; ++i) {
          if(strcmp(temp_key, key_ptrs[i]) == 0) break;
        }

        if(i == 0) {
          block_size = memcached_result_length(result_buffer);
        }
        if(i == l-1) {
          last_data_request_size = memcached_result_length(result_buffer);
        }

        data_ptrs[i] = new char[memcached_result_length(result_buffer)];
        memcpy(data_ptrs[i], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
    }
  }

  // handle the last data request
  char* temp_buf = new char[block_size];
  memset(temp_buf, 0, block_size);
  memcpy(temp_buf, data_ptrs[l-1], last_data_request_size);
  delete data_ptrs[l-1];
  data_ptrs[l-1] = new char[block_size];
  memset(data_ptrs[l-1], 0, block_size);
  memcpy(data_ptrs[l-1], temp_buf, block_size);
  delete temp_buf;

  // old parity blocks
  char **coding_ptrs = new char*[m*num_stripes_pre];
  for(uint32_t i = 0; i < m*num_stripes_pre; ++i) {
    coding_ptrs[i] = new char[block_size];
    memset(coding_ptrs[i], 0, block_size);
  }
  calculate_parity_ptrs_srs(ptr, data_ptrs, coding_ptrs, k, s, m, block_size);

  // new parity blocks
  char **new_codings = new char*[m*num_stripes_post];
  for(uint32_t i = 0; i < m*num_stripes_post; ++i) {
    new_codings[i] = new char[block_size];
    memset(new_codings[i], 0, block_size);
  }
  calculate_parity_ptrs_srs_after_transition(ptr, data_ptrs, new_codings, k, s, m, block_size);

  // upload new parity blocks
  error = ERS_upload_new_parity_1p(ptr, key, key_length, 
            k, m, s, block_size, new_codings);
  if(error == MEMCACHED_SUCCESS) {
    ERS_delete_moving_key(ptr, key);
    ERS_add_moved_key(ptr, key, key_length);
    ERS_traverse_moving_and_moved_list(ptr);
  } else {
    ERS_revert_to_old_parity(ptr, key, key_length, 
            k, m, s, block_size, coding_ptrs);
    ERS_delete_moving_key(ptr, key);
    ERS_traverse_moving_and_moved_list(ptr);
    return error;
  }

  // free operations
  if(key_ptrs != NULL) {
    for(uint32_t i = 0; i < l; ++i) {
      if(key_ptrs[i] != NULL) delete key_ptrs[i];
    }
    delete key_ptrs;
  }
  if(key_len_ptrs != NULL) delete key_len_ptrs;

  if(data_ptrs != NULL) {
    for(uint32_t i = 0; i < l; ++i) {
      if(data_ptrs[i] != NULL) delete data_ptrs[i];
    }
    delete data_ptrs;
  }
  if(coding_ptrs != NULL) {
    for(uint32_t i = 0; i < num_stripes_pre*m; ++i) {
      if(coding_ptrs[i] != NULL) delete coding_ptrs[i];
    }
    delete coding_ptrs;
  }

  return MEMCACHED_SUCCESS;
}

//ERSadd
/* * * * * * * * * *
 * memcached_move  *
 * * * * * * * * * *
*/
memcached_return_t memcached_move(memcached_st *ptr, const char *key, size_t key_length) {
  if(ptr->number_of_k > 0 and ptr->number_of_m > 0 and ptr->number_of_s > 0) {
    if(ptr->srs_specified > 0) {
      // SRS
      return SRS_move(ptr, key, key_length);
    } 
    if(ptr->ers_specified > 0) {
      // ERS
      if(ptr->ers_enhanced_placement > 0) {
        // designed matrix + designed placement
        return ERS_enhanced_move(ptr, key, key_length);
      } else {
        // horizontal placement + designed matrix
        return ERS_basic_move(ptr, key, key_length);
      }
    }
  }
}



//ERSadd
/* * * * * * * * * * * * *
 * Elastic Reed-Solomon  *
 * * * * * * * * * * * * *
*/
static memcached_return_t memcached_ers_send_binary(Memcached *ptr,
                                                   memcached_instance_st* server,
                                                   uint32_t server_key,
                                                   const char *key,
                                                   const size_t key_length,
                                                   const char *value,
                                                   const size_t value_length,
                                                   const time_t expiration,
                                                   const uint32_t flags,
                                                   const uint64_t cas,
                                                   const bool flush,
                                                   const bool reply,
                                                   memcached_storage_action_t verb)
{
  if(ERS_is_moving(ptr, key)) {
    return MEMCACHED_WRITE_FAILURE;
  }
  if(ERS_is_moved(ptr, key)) {
    return memcached_ers_send_binary_after_transition(ptr, server, server_key, 
                                    key, key_length, value, value_length, 
                                    expiration, flags, cas, flush, reply, verb);
  }

  /* Apply erasure coding on this object.
   * Unlike replication which uses one request, erasure coding uses k+m requests, each of which corresponds
   * to a data (parity) block.
   */
  
  // determining block size
  uint32_t k = ptr->number_of_k;
  uint32_t s = ptr->number_of_s;
  uint32_t m = ptr->number_of_m;
  size_t block_size = determine_block_size_ers(value_length, k, s);
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_pre = l/k;

  // store the positions of each data block
  // in the data store array
  uint32_t *h_ids = new uint32_t[l];
  uint32_t *v_ids = new uint32_t[l];
  for(uint32_t i = 0; i < l; ++i) {
    h_ids[i] = v_ids[i] = 0;
  }

  // fill data blocks
  char **data_ptrs = new char*[l];
  for(uint32_t i = 0; i < l; ++i) {
    data_ptrs[i] = new char[block_size];
    memset(data_ptrs[i], 0, block_size);		
  }
  fill_data_ptrs(data_ptrs, l, block_size, value, value_length);
  //FILE *fp = fopen("./wusi", "a");
  //fprintf(fp, "ers, k: %d, s: %d, m: %d, key: %s, value_length:%d, block_size: %d\n", k, s, m, key, value_length, block_size);

  // calculate parity blocks
  char **coding_ptrs = new char*[m*num_stripes_pre];
  for(uint32_t i = 0; i < m*num_stripes_pre; ++i) {
    coding_ptrs[i] = new char[block_size];
    memset(coding_ptrs[i], 0, block_size);
  }
  calculate_parity_ptrs_ers(ptr, h_ids, v_ids, data_ptrs, coding_ptrs, k, s, m, block_size);
  bool enhanced = (ptr->ers_enhanced_placement > 0 && s % k != 0);

  // generate l + m*num_stripes_pre requests for 
  // l data blocks and 
  // m*num_stripes_pre parity blocks
  size_t last_data_request_size = value_length - block_size * (l - 1);
  char *last_data_request = new char[last_data_request_size];
  memcpy(last_data_request, data_ptrs[l-1], last_data_request_size);
  for(uint32_t i = 0; i < l; ++i) {
    uint32_t hid;
    if(enhanced) {
      hid = h_ids[i];
    } else {
      hid = i % s;
    }
    //uint32_t vid = i / s;

    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = i / 100;
    int ten = (i - handred*100) / 10;
    int one = i - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t data_server = (server_key + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, data_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    if(i < l - 1) {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));
    } else {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + last_data_request_size +
                                                            request.message.header.request.extlen));
    }

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { i<(l-1)?data_ptrs[i]:last_data_request, i<(l-1)?block_size:last_data_request_size }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    if((rc = memcached_response(instance, NULL, 0, NULL)) == MEMCACHED_SUCCESS) {
    }
  } // end of for
  if(data_ptrs[l-1] != NULL) delete data_ptrs[l-1];

  for(uint32_t i = l; i < l + m*num_stripes_pre; ++i) {
    uint32_t hid = (i - l) % m;
    //uint32_t vid = (i - l) / m;

    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = i / 100;
    int ten = (i - handred*100) / 10;
    int one = i - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t parity_server = (server_key + s + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, parity_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { coding_ptrs[i-l], block_size }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    if((rc = memcached_response(instance, NULL, 0, NULL)) == MEMCACHED_SUCCESS) {
    }
  } // end of for

  // store key -> value_length mapping, added in 19-10-14
  /*for(uint32_t i = 0; i < s; ++i) {
    char *val_len_val = new char[sizeof(size_t)];
    memcpy(val_len_val, &value_length, sizeof(size_t));

    uint32_t data_server = (server_key + i) % memcached_server_count(ptr);
    
    memcached_instance_st *instance = memcached_instance_fetch(ptr, data_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    request.message.header.request.bodylen= htonl((uint32_t) (key_length + memcached_array_size(ptr->_namespace) + sizeof(size_t) +
                                                            request.message.header.request.extlen));

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { key, key_length },
      { val_len_val, sizeof(size_t) }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }
  }*/ // end of store key -> value_length mapping

  //fclose(fp);
  delete h_ids;
  delete v_ids;

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }

  return MEMCACHED_SUCCESS;
  //return memcached_response(server, NULL, 0, NULL);
}

//ERSadd
/* * * * * * * * *
 * Reed-Solomon  *
 * * * * * * * * *
*/
static memcached_return_t memcached_rs_send_binary(Memcached *ptr,
                                                   memcached_instance_st* server,
                                                   uint32_t server_key,
                                                   const char *key,
                                                   const size_t key_length,
                                                   const char *value,
                                                   const size_t value_length,
                                                   const time_t expiration,
                                                   const uint32_t flags,
                                                   const uint64_t cas,
                                                   const bool flush,
                                                   const bool reply,
                                                   memcached_storage_action_t verb)
{
  /* Apply erasure coding on this object.
   * Unlike replication which uses one request, erasure coding uses k+m requests, each of which corresponds
   * to a data (parity) block.
   */
  
  // determining block size
  uint32_t k = ptr->number_of_k;
  uint32_t m = ptr->number_of_m;
  size_t block_size = determine_block_size_rs(value_length, k);
  // fill data blocks
  char **data_ptrs = new char*[k];
  for(uint32_t i = 0; i < k; ++i) {
    data_ptrs[i] = new char[block_size];
    memset(data_ptrs[i], 0, block_size);		
  }
  fill_data_ptrs(data_ptrs, k, block_size, value, value_length);
  //FILE *fp = fopen("./wusi", "a");
  //fprintf(fp, "rs, k: %d, m: %d, key: %s, value_length:%d, block_size: %d\n", k, m, key, value_length, block_size);

  // calculate parity blocks
  char **coding_ptrs = new char*[m];
  for(uint32_t i = 0; i < m; ++i) {
    coding_ptrs[i] = new char[block_size];
    memset(coding_ptrs[i], 0, block_size);
  }
  calculate_parity_ptrs_rs(data_ptrs, k, coding_ptrs, m, block_size);

  // generate new keys for each block
  // Note, we cannot use the same key for each block, since multiple blocks of 
  // a same item would fall into one physical machine.
  char **key_ptrs = new char*[k+m];
  for(uint32_t i = 0; i < k+m; ++i) {
    key_ptrs[i] = new char[key_length + 3];
    memset(key_ptrs[i], 0, key_length+3);
  }
  generate_new_keys_rs(key_ptrs, k+m, key, key_length);

  // generate k+m requests for k+m data and parity blocks
  uint32_t storage_key = server_key;
  size_t last_data_request_size = value_length - block_size * (k - 1);
  char *last_data_request = new char[last_data_request_size];
  memcpy(last_data_request, data_ptrs[k-1], last_data_request_size);
  for(uint32_t i = 0; i < k+m; ++i) {
    if(i > 0) {
      ++storage_key;
    }
    if (storage_key == memcached_server_count(ptr))
    {
      storage_key= 0;
    }

    memcached_instance_st *instance = memcached_instance_fetch(ptr, storage_key);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 2 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    if(i != k - 1) {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 2 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));
    } else {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 2 + memcached_array_size(ptr->_namespace) + last_data_request_size +
                                                            request.message.header.request.extlen));
    }

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    if(i != k - 1) {
      libmemcached_io_vector_st vector[] = 
	  {
        { NULL, 0 },
        { request.bytes, send_length },
        { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
        { key_ptrs[i], key_length+2 },
        { i<k?data_ptrs[i]:coding_ptrs[i-k], block_size }
      };
	  // write the block
      memcached_return_t rc;
      if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
      {
        memcached_io_reset(instance);
        #if 0
          if (memcached_has_error(ptr))
          {
            memcached_set_error(*instance, rc, MEMCACHED_AT);
          }
        #endif

        assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
        return memcached_last_error(instance->root);
      }
    } else {
      libmemcached_io_vector_st vector[] = 
	  {
        { NULL, 0 },
        { request.bytes, send_length },
        { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
        { key_ptrs[i], key_length+2 },
        { last_data_request, last_data_request_size }
      };
      // write the block
      memcached_return_t rc;
      if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
      {
        memcached_io_reset(instance);
        #if 0
          if (memcached_has_error(ptr))
          {
            memcached_set_error(*instance, rc, MEMCACHED_AT);
          }
        #endif

        assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
        return memcached_last_error(instance->root);
      }
    }
  } // end of for

  // store key -> value_length mapping, added in 19-10-14
  /*for(uint32_t i = 0; i < k; ++i) {
    char *val_len_val = new char[sizeof(size_t)];
    memcpy(val_len_val, &value_length, sizeof(size_t));

    uint32_t data_server = (server_key + i) % memcached_server_count(ptr);
    
    memcached_instance_st *instance = memcached_instance_fetch(ptr, data_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    request.message.header.request.bodylen= htonl((uint32_t) (key_length + memcached_array_size(ptr->_namespace) + sizeof(size_t) +
                                                            request.message.header.request.extlen));

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { key, key_length },
      { val_len_val, sizeof(size_t) }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }
  }*/ // end of store key -> value_length mapping

  //fclose(fp);

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }

  return memcached_response(server, NULL, 0, NULL);
}

//ERSadd
/* * * * * * * * * * * * *
 * Stretch Reed-Solomon  *
 * * * * * * * * * * * * *
*/
static memcached_return_t memcached_srs_send_binary(Memcached *ptr,
                                                   memcached_instance_st* server,
                                                   uint32_t server_key,
                                                   const char *key,
                                                   const size_t key_length,
                                                   const char *value,
                                                   const size_t value_length,
                                                   const time_t expiration,
                                                   const uint32_t flags,
                                                   const uint64_t cas,
                                                   const bool flush,
                                                   const bool reply,
                                                   memcached_storage_action_t verb)
{
  if(ERS_is_moving(ptr, key)) {
    return MEMCACHED_WRITE_FAILURE;
  }
  if(ERS_is_moved(ptr, key)) {
    return memcached_srs_send_binary_after_transition(ptr, server, server_key, 
                                    key, key_length, value, value_length, 
                                    expiration, flags, cas, flush, reply, verb);
  }

  /* Apply erasure coding on this object.
   * Unlike replication which uses one request, erasure coding uses k+m requests, each of which corresponds
   * to a data (parity) block.
   */
  
  // determining block size
  uint32_t k = ptr->number_of_k;
  uint32_t s = ptr->number_of_s;
  uint32_t m = ptr->number_of_m;
  size_t block_size = determine_block_size_ers(value_length, k, s);
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_pre = l/k;
  uint32_t num_stripes_post = l/s;

  // fill data blocks
  char **data_ptrs = new char*[l];
  for(uint32_t i = 0; i < l; ++i) {
    data_ptrs[i] = new char[block_size];
    memset(data_ptrs[i], 0, block_size);		
  }
  fill_data_ptrs(data_ptrs, l, block_size, value, value_length);
  //FILE *fp = fopen("./wusi", "a");
  //fprintf(fp, "srs, k: %d, s: %d, m: %d, key: %s, value_length:%d, block_size: %d\n", k, s, m, key, value_length, block_size);

  // calculate parity blocks
  char **coding_ptrs = new char*[m*num_stripes_pre];
  for(uint32_t i = 0; i < m*num_stripes_pre; ++i) {
    coding_ptrs[i] = new char[block_size];
    memset(coding_ptrs[i], 0, block_size);
  }
  calculate_parity_ptrs_srs(ptr, data_ptrs, coding_ptrs, k, s, m, block_size);

  // generate l + m*num_stripes_pre requests for 
  // l data blocks and 
  // m*num_stripes_pre parity blocks
  size_t last_data_request_size = value_length - block_size * (l - 1);
  char *last_data_request = new char[last_data_request_size];
  memcpy(last_data_request, data_ptrs[l-1], last_data_request_size);
  for(uint32_t i = 0; i < l; ++i) {
    uint32_t hid = i / num_stripes_post;
    //uint32_t vid = i % num_stripes_post;

    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = i / 100;
    int ten = (i - handred*100) / 10;
    int one = i - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t data_server = (server_key + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, data_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    if(i < l - 1) {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));
    } else {
      request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + last_data_request_size +
                                                            request.message.header.request.extlen));
    }

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { i<(l-1)?data_ptrs[i]:last_data_request, i<(l-1)?block_size:last_data_request_size }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    if((rc = memcached_response(instance, NULL, 0, NULL)) == MEMCACHED_SUCCESS) {
    }
  } // end of for
  if(data_ptrs[l-1] != NULL) delete data_ptrs[l-1];

  for(uint32_t i = l; i < l + m*num_stripes_pre; ++i) {
    uint32_t hid = (i - l) % m;
    //uint32_t vid = (i - l) / m;

    char *temp_key = new char[key_length + 4];
    memset(temp_key, 0, key_length + 4);
    memcpy(temp_key, key, key_length);
    int handred = i / 100;
    int ten = (i - handred*100) / 10;
    int one = i - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    temp_key[key_length] = handred_bit_char;
    temp_key[key_length + 1] = ten_bit_char;
    temp_key[key_length + 2] = one_bit_char;
    temp_key[key_length + 3] = '\0';

    uint32_t parity_server = (server_key + s + hid) % memcached_server_count(ptr);

    memcached_instance_st *instance = memcached_instance_fetch(ptr, parity_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + 3 + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    request.message.header.request.bodylen= htonl((uint32_t) (key_length + 3 + memcached_array_size(ptr->_namespace) + block_size +
                                                            request.message.header.request.extlen));

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { temp_key, key_length+3 },
      { coding_ptrs[i-l], block_size }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }

    if((rc = memcached_response(instance, NULL, 0, NULL)) == MEMCACHED_SUCCESS) {
    }
  } // end of for

  // store key -> value_length mapping, added in 19-10-14
  /*for(uint32_t i = 0; i < s; ++i) {
    char *val_len_val = new char[sizeof(size_t)];
    memcpy(val_len_val, &value_length, sizeof(size_t));

    uint32_t data_server = (server_key + i) % memcached_server_count(ptr);
    
    memcached_instance_st *instance = memcached_instance_fetch(ptr, data_server);

    protocol_binary_request_set request= {};
    size_t send_length= sizeof(request.bytes);

    initialize_binary_request(instance, request.message.header);

    request.message.header.request.opcode= get_com_code(verb, reply);
    request.message.header.request.keylen= htons((uint16_t)(key_length + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);

    request.message.header.request.bodylen= htonl((uint32_t) (key_length + memcached_array_size(ptr->_namespace) + sizeof(size_t) +
                                                            request.message.header.request.extlen));

    if (cas)
    {
      request.message.header.request.cas= memcached_htonll(cas);
    }

    libmemcached_io_vector_st vector[]=
    {
      { NULL, 0 },
      { request.bytes, send_length },
      { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
      { key, key_length },
      { val_len_val, sizeof(size_t) }
    };

    // write the block
    memcached_return_t rc;
    if ((rc= memcached_vdo(instance, vector, 5, flush)) != MEMCACHED_SUCCESS)
    {
      memcached_io_reset(instance);
      #if 0
        if (memcached_has_error(ptr))
        {
          memcached_set_error(*instance, rc, MEMCACHED_AT);
        }
      #endif

      assert(memcached_last_error(instance->root) != MEMCACHED_SUCCESS);
      return memcached_last_error(instance->root);
    }
  }*/ // end of store key -> value_length mapping

  //fclose(fp);

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }

  return MEMCACHED_SUCCESS;
  //return memcached_response(server, NULL, 0, NULL);
}

static memcached_return_t memcached_send_binary(Memcached *ptr,
                                                memcached_instance_st* server,
                                                uint32_t server_key,
                                                const char *key,
                                                const size_t key_length,
                                                const char *value,
                                                const size_t value_length,
                                                const time_t expiration,
                                                const uint32_t flags,
                                                const uint64_t cas,
                                                const bool flush,
                                                const bool reply,
                                                memcached_storage_action_t verb)
{
  //ERSadd
  if(verb == SET_OP and ptr->number_of_k > 0 and ptr->number_of_m > 0) {
    if(ptr->number_of_s > 0) {
      if(ptr->srs_specified > 0) {
        // SRS
        return memcached_srs_send_binary(ptr, server, server_key, 
                                    key, key_length, value, value_length, 
                                    expiration, flags, cas, flush, reply, verb);
      } 
      if(ptr->ers_specified > 0) {
        // ERS
        return memcached_ers_send_binary(ptr, server, server_key, 
                                    key, key_length, value, value_length, 
                                    expiration, flags, cas, flush, reply, verb);
      }
	} else {
      // RS
      return memcached_rs_send_binary(ptr, server, server_key, 
                                    key, key_length, value, value_length, 
                                    expiration, flags, cas, flush, reply, verb);
    }
  }

  protocol_binary_request_set request= {};
  size_t send_length= sizeof(request.bytes);

  initialize_binary_request(server, request.message.header);

  request.message.header.request.opcode= get_com_code(verb, reply);
  request.message.header.request.keylen= htons((uint16_t)(key_length + memcached_array_size(ptr->_namespace)));
  request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
  if (verb == APPEND_OP or verb == PREPEND_OP)
  {
    send_length -= 8; /* append & prepend does not contain extras! */
  }
  else
  {
    request.message.header.request.extlen= 8;
    request.message.body.flags= htonl(flags);
    request.message.body.expiration= htonl((uint32_t)expiration);
  }

  request.message.header.request.bodylen= htonl((uint32_t) (key_length + memcached_array_size(ptr->_namespace) + value_length +
                                                            request.message.header.request.extlen));

  if (cas)
  {
    request.message.header.request.cas= memcached_htonll(cas);
  }

  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { request.bytes, send_length },
    { memcached_array_string(ptr->_namespace),  memcached_array_size(ptr->_namespace) },
    { key, key_length },
    { value, value_length }
  };

  /* write the header */
  memcached_return_t rc;
  if ((rc= memcached_vdo(server, vector, 5, flush)) != MEMCACHED_SUCCESS)
  {
    memcached_io_reset(server);

#if 0
    if (memcached_has_error(ptr))
    {
      memcached_set_error(*server, rc, MEMCACHED_AT);
    }
#endif

    assert(memcached_last_error(server->root) != MEMCACHED_SUCCESS);
    return memcached_last_error(server->root);
  }

  if (verb == SET_OP and ptr->number_of_replicas > 0)
  {
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_SETQ;
    WATCHPOINT_STRING("replicating");

    for (uint32_t x= 0; x < ptr->number_of_replicas; x++)
    {
      ++server_key;
      if (server_key == memcached_server_count(ptr))
      {
        server_key= 0;
      }

      memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);

      if (memcached_vdo(instance, vector, 5, false) != MEMCACHED_SUCCESS)
      {
        memcached_io_reset(instance);
      }
      else
      {
        memcached_server_response_decrement(instance);
      }
    }
  }

  if (flush == false)
  {
    return MEMCACHED_BUFFERED;
  }

  // No reply always assumes success
  if (reply == false)
  {
    return MEMCACHED_SUCCESS;
  }

  return memcached_response(server, NULL, 0, NULL);
}

static memcached_return_t memcached_send_ascii(Memcached *ptr,
                                               memcached_instance_st* instance,
                                               const char *key,
                                               const size_t key_length,
                                               const char *value,
                                               const size_t value_length,
                                               const time_t expiration,
                                               const uint32_t flags,
                                               const uint64_t cas,
                                               const bool flush,
                                               const bool reply,
                                               const memcached_storage_action_t verb)
{
  char flags_buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
  int flags_buffer_length= snprintf(flags_buffer, sizeof(flags_buffer), " %u", flags);
  if (size_t(flags_buffer_length) >= sizeof(flags_buffer) or flags_buffer_length < 0)
  {
    return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                               memcached_literal_param("snprintf(MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH)"));
  }

  char expiration_buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
  int expiration_buffer_length= snprintf(expiration_buffer, sizeof(expiration_buffer), " %llu", (unsigned long long)expiration);
  if (size_t(expiration_buffer_length) >= sizeof(expiration_buffer) or expiration_buffer_length < 0)
  {
    return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                               memcached_literal_param("snprintf(MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH)"));
  }

  char value_buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
  int value_buffer_length= snprintf(value_buffer, sizeof(value_buffer), " %llu", (unsigned long long)value_length);
  if (size_t(value_buffer_length) >= sizeof(value_buffer) or value_buffer_length < 0)
  {
    return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                               memcached_literal_param("snprintf(MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH)"));
  }

  char cas_buffer[MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH +1];
  int cas_buffer_length= 0;
  if (cas)
  {
    cas_buffer_length= snprintf(cas_buffer, sizeof(cas_buffer), " %llu", (unsigned long long)cas);
    if (size_t(cas_buffer_length) >= sizeof(cas_buffer) or cas_buffer_length < 0)
    {
      return memcached_set_error(*instance, MEMCACHED_MEMORY_ALLOCATION_FAILURE, MEMCACHED_AT, 
                                 memcached_literal_param("snprintf(MEMCACHED_MAXIMUM_INTEGER_DISPLAY_LENGTH)"));
    }
  }

  libmemcached_io_vector_st vector[]=
  {
    { NULL, 0 },
    { storage_op_string(verb), strlen(storage_op_string(verb))},
    { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
    { key, key_length },
    { flags_buffer, size_t(flags_buffer_length) },
    { expiration_buffer, size_t(expiration_buffer_length) },
    { value_buffer, size_t(value_buffer_length) },
    { cas_buffer, size_t(cas_buffer_length) },
    { " noreply", reply ? 0 : memcached_literal_param_size(" noreply") },
    { memcached_literal_param("\r\n") },
    { value, value_length },
    { memcached_literal_param("\r\n") }
  };

  /* Send command header */
  memcached_return_t rc=  memcached_vdo(instance, vector, 12, flush);

  // If we should not reply, return with MEMCACHED_SUCCESS, unless error
  if (reply == false)
  {
    return memcached_success(rc) ? MEMCACHED_SUCCESS : rc; 
  }

  if (flush == false)
  {
    return memcached_success(rc) ? MEMCACHED_BUFFERED : rc; 
  }

  if (rc == MEMCACHED_SUCCESS)
  {
    char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];
    rc= memcached_response(instance, buffer, sizeof(buffer), NULL);

    if (rc == MEMCACHED_STORED)
    {
      return MEMCACHED_SUCCESS;
    }
  }

  if (rc == MEMCACHED_WRITE_FAILURE)
  {
    memcached_io_reset(instance);
  }

  assert(memcached_failed(rc));
#if 0
  if (memcached_has_error(ptr) == false)
  {
    return memcached_set_error(*ptr, rc, MEMCACHED_AT);
  }
#endif

  return rc;
}

static inline memcached_return_t memcached_send(memcached_st *shell,
                                                const char *group_key, size_t group_key_length,
                                                const char *key, size_t key_length,
                                                const char *value, size_t value_length,
                                                const time_t expiration,
                                                const uint32_t flags,
                                                const uint64_t cas,
                                                memcached_storage_action_t verb)
{
  Memcached* ptr= memcached2Memcached(shell);
  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, true)))
  {
    return rc;
  }

  if (memcached_failed(memcached_key_test(*ptr, (const char **)&key, &key_length, 1)))
  {
    return memcached_last_error(ptr);
  }

  uint32_t server_key= memcached_generate_hash_with_redistribution(ptr, group_key, group_key_length);
  memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);

  WATCHPOINT_SET(instance->io_wait_count.read= 0);
  WATCHPOINT_SET(instance->io_wait_count.write= 0);

  bool flush= true;
  if (memcached_is_buffering(instance->root) and verb == SET_OP)
  {
    flush= false;
  }

  bool reply= memcached_is_replying(ptr);

  hashkit_string_st* destination= NULL;

  if (memcached_is_encrypted(ptr))
  {
    if (can_by_encrypted(verb) == false)
    {
      return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT, 
                                 memcached_literal_param("Operation not allowed while encyrption is enabled"));
    }

    if ((destination= hashkit_encrypt(&ptr->hashkit, value, value_length)) == NULL)
    {
      return rc;
    }
    value= hashkit_string_c_str(destination);
    value_length= hashkit_string_length(destination);
  }

  if (memcached_is_binary(ptr))
  {
    rc= memcached_send_binary(ptr, instance, server_key,
                              key, key_length,
                              value, value_length, expiration,
                              flags, cas, flush, reply, verb);
  }
  else
  {
    rc= memcached_send_ascii(ptr, instance,
                             key, key_length,
                             value, value_length, expiration,
                             flags, cas, flush, reply, verb);
  }

  hashkit_string_free(destination);

  return rc;
}


memcached_return_t memcached_set(memcached_st *ptr, const char *key, size_t key_length,
                                 const char *value, size_t value_length,
                                 time_t expiration,
                                 uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_SET_START();
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, SET_OP);
  LIBMEMCACHED_MEMCACHED_SET_END();
  return rc;
}

memcached_return_t memcached_add(memcached_st *ptr,
                                 const char *key, size_t key_length,
                                 const char *value, size_t value_length,
                                 time_t expiration,
                                 uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_ADD_START();
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, ADD_OP);

  LIBMEMCACHED_MEMCACHED_ADD_END();
  return rc;
}

memcached_return_t memcached_replace(memcached_st *ptr,
                                     const char *key, size_t key_length,
                                     const char *value, size_t value_length,
                                     time_t expiration,
                                     uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_REPLACE_START();
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, REPLACE_OP);
  LIBMEMCACHED_MEMCACHED_REPLACE_END();
  return rc;
}

memcached_return_t memcached_prepend(memcached_st *ptr,
                                     const char *key, size_t key_length,
                                     const char *value, size_t value_length,
                                     time_t expiration,
                                     uint32_t flags)
{
  memcached_return_t rc;
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, PREPEND_OP);
  return rc;
}

memcached_return_t memcached_append(memcached_st *ptr,
                                    const char *key, size_t key_length,
                                    const char *value, size_t value_length,
                                    time_t expiration,
                                    uint32_t flags)
{
  memcached_return_t rc;
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, APPEND_OP);
  return rc;
}

memcached_return_t memcached_cas(memcached_st *ptr,
                                 const char *key, size_t key_length,
                                 const char *value, size_t value_length,
                                 time_t expiration,
                                 uint32_t flags,
                                 uint64_t cas)
{
  memcached_return_t rc;
  rc= memcached_send(ptr, key, key_length,
                     key, key_length, value, value_length,
                     expiration, flags, cas, CAS_OP);
  return rc;
}

memcached_return_t memcached_set_by_key(memcached_st *ptr,
                                        const char *group_key,
                                        size_t group_key_length,
                                        const char *key, size_t key_length,
                                        const char *value, size_t value_length,
                                        time_t expiration,
                                        uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_SET_START();
  rc= memcached_send(ptr, group_key, group_key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, SET_OP);
  LIBMEMCACHED_MEMCACHED_SET_END();
  return rc;
}

memcached_return_t memcached_add_by_key(memcached_st *ptr,
                                        const char *group_key, size_t group_key_length,
                                        const char *key, size_t key_length,
                                        const char *value, size_t value_length,
                                        time_t expiration,
                                        uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_ADD_START();
  rc= memcached_send(ptr, group_key, group_key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, ADD_OP);
  LIBMEMCACHED_MEMCACHED_ADD_END();
  return rc;
}

memcached_return_t memcached_replace_by_key(memcached_st *ptr,
                                            const char *group_key, size_t group_key_length,
                                            const char *key, size_t key_length,
                                            const char *value, size_t value_length,
                                            time_t expiration,
                                            uint32_t flags)
{
  memcached_return_t rc;
  LIBMEMCACHED_MEMCACHED_REPLACE_START();
  rc= memcached_send(ptr, group_key, group_key_length,
                     key, key_length, value, value_length,
                     expiration, flags, 0, REPLACE_OP);
  LIBMEMCACHED_MEMCACHED_REPLACE_END();
  return rc;
}

memcached_return_t memcached_prepend_by_key(memcached_st *ptr,
                                            const char *group_key, size_t group_key_length,
                                            const char *key, size_t key_length,
                                            const char *value, size_t value_length,
                                            time_t expiration,
                                            uint32_t flags)
{
  return memcached_send(ptr, group_key, group_key_length,
                        key, key_length, value, value_length,
                        expiration, flags, 0, PREPEND_OP);
}

memcached_return_t memcached_append_by_key(memcached_st *ptr,
                                           const char *group_key, size_t group_key_length,
                                           const char *key, size_t key_length,
                                           const char *value, size_t value_length,
                                           time_t expiration,
                                           uint32_t flags)
{
  return memcached_send(ptr, group_key, group_key_length,
                        key, key_length, value, value_length,
                        expiration, flags, 0, APPEND_OP);
}

memcached_return_t memcached_cas_by_key(memcached_st *ptr,
                                        const char *group_key, size_t group_key_length,
                                        const char *key, size_t key_length,
                                        const char *value, size_t value_length,
                                        time_t expiration,
                                        uint32_t flags,
                                        uint64_t cas)
{
  return  memcached_send(ptr, group_key, group_key_length,
                         key, key_length, value, value_length,
                         expiration, flags, cas, CAS_OP);
}

