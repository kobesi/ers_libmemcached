/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Libmemcached library
 *
 *  Copyright (C) 2011-2013 Data Differential, http://datadifferential.com/
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
#include <stdio.h>
extern "C"{
#include "../Jerasure-1.2A/cauchy.h"
#include "../Jerasure-1.2A/reed_sol.h"
#include "../Jerasure-1.2A/jerasure.h"
#include "../Jerasure-1.2A/galois.h"
}
/*
  What happens if no servers exist?
*/

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
/* get functions by the ERS code and the SRS code */
static char *memcached_get_ers(memcached_st *ptr, const char *key,
                    size_t key_length,
                    size_t *value_length,
                    uint32_t *flags,
                    memcached_return_t *error)
{
  uint32_t k = ptr->number_of_k;
  uint32_t s = ptr->number_of_s;
  uint32_t l = lcm(k, s);

  char **key_ptrs = new char*[l];
  size_t *key_len_ptrs = new size_t[l];

  // generate new keys for each data block
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

  // multiple get the data blocks
  memcached_return_t unused;
  if(error == NULL) {
      error = &unused;
  }

  //FILE *fp = fopen("./wusi", "a");
  size_t temp_value_len = 0;
  char **sub_values = new char*[l];
  size_t block_size;
  size_t last_data_request_size;

  *error = memcached_mget(ptr, (const char * const *)key_ptrs, key_len_ptrs, l);

  if(*error == MEMCACHED_SUCCESS) {
      if(ptr->srs_specified > 0) {
        //fprintf(fp, "srs, multiple get success !\n");
      }
      if(ptr->ers_specified > 0) {
        //fprintf(fp, "ers, multiple get success !\n");
      }
      memcached_result_st *result_buffer = &ptr->result;
      char *temp_key = new char[key_length + 4];
      while((result_buffer = memcached_fetch_result(ptr, result_buffer, error)) != NULL) {
        strncpy(temp_key, result_buffer->item_key, result_buffer->key_length);
        temp_key[key_length + 3] = '\0';
        //fprintf(fp, "key: %s\n", temp_key);
        uint32_t i;
        for(i = 0; i < l; ++i) {
          if(strcmp(temp_key, key_ptrs[i]) == 0) break;
        }
        //fprintf(fp, "index: %d\n", i);

        if(i == 0) {
          block_size = memcached_result_length(result_buffer);
        }
        if(i == l-1) {
          last_data_request_size = memcached_result_length(result_buffer);
        }
		temp_value_len += memcached_result_length(result_buffer);
        sub_values[i] = new char[memcached_result_length(result_buffer)];
        memcpy(sub_values[i], memcached_result_value(result_buffer), memcached_result_length(result_buffer));

        if(flags) {
          *flags= result_buffer->item_flags;
        }
      }
      *error = MEMCACHED_SUCCESS;
      value_length = &temp_value_len;
      //fprintf(fp, "value size: %d\n", temp_value_len);
      //fprintf(fp, "\n");
      //fclose(fp);
      char *temp_value = new char[temp_value_len];
      for(uint32_t i = 0; i < l; ++i) {
        if(i < l-1) {
          memcpy(temp_value + i*block_size, sub_values[i], block_size);
        } else {
          memcpy(temp_value + i*block_size, sub_values[i], last_data_request_size);
        }
      }
      if(sub_values != NULL) {
        for(uint32_t i = 0; i < l; ++i) {
          if(sub_values[i] != NULL) {
            delete sub_values[i];
          }
        }
        delete sub_values;
      }
      delete temp_key;
      if(key_ptrs != NULL) {
        for(uint32_t i = 0; i < l; ++i) {
          if(key_ptrs[i] != NULL) {
            delete key_ptrs[i];
          }
        }
        delete key_ptrs;
      }
      if(key_len_ptrs != NULL) {
        delete key_len_ptrs;
      }
      return temp_value;
  } else {
      if(ptr->srs_specified > 0) {
        //fprintf(fp, "srs, multiple get error !\n");
      }
      if(ptr->ers_specified > 0) {
        //fprintf(fp, "ers, multiple get error !\n");
      }
      //fprintf(fp, "\n");
      //fclose(fp);
      if(sub_values != NULL) {
        for(uint32_t i = 0; i < l; ++i) {
          if(sub_values[i] != NULL) {
            delete sub_values[i];
          }
        }
        delete sub_values;
      }
      if(key_ptrs != NULL) {
        for(uint32_t i = 0; i < l; ++i) {
          if(key_ptrs[i] != NULL) {
            delete key_ptrs[i];
          }
        }
        delete key_ptrs;
      }
      if(key_len_ptrs != NULL) {
        delete key_len_ptrs;
      }
      return NULL;
  }
}

//ERSadd
/* get function by the RS code */
static char *memcached_get_rs(memcached_st *ptr, const char *key,
                    size_t key_length,
                    size_t *value_length,
                    uint32_t *flags,
                    memcached_return_t *error)
{
  uint32_t k = ptr->number_of_k;

  char **key_ptrs = new char*[k];
  size_t *key_len_ptrs = new size_t[k];

  // generate new keys for each data block
  for(uint32_t i = 0; i < k; ++i) {
      key_ptrs[i] = new char[key_length + 3];
      memset(key_ptrs[i], 0, key_length+3);
      memcpy(key_ptrs[i], key, key_length);
      int ten = i / 10;
      int one = i - ten * 10;
      char ten_bit_char = '0' + ten;
      char one_bit_char = '0' + one;
      key_ptrs[i][key_length] = ten_bit_char;
      key_ptrs[i][key_length+1] = one_bit_char;
      key_ptrs[i][key_length+2] = '\0';
      key_len_ptrs[i] = key_length+2;
  }

  // multiple get the data blocks
  memcached_return_t unused;
  if(error == NULL) {
      error = &unused;
  }

  //FILE *fp = fopen("./wusi", "a");
  size_t temp_value_len = 0;
  char **sub_values = new char*[k];
  size_t block_size;
  size_t last_data_request_size;

  *error = memcached_mget(ptr, (const char * const *)key_ptrs, key_len_ptrs, k);

  if(*error == MEMCACHED_SUCCESS) {
      //fprintf(fp, "rs, multiple get success !\n");
      memcached_result_st *result_buffer = &ptr->result;
      char *temp_key = new char[key_length + 3];
      while((result_buffer = memcached_fetch_result(ptr, result_buffer, error)) != NULL) {
        strncpy(temp_key, result_buffer->item_key, result_buffer->key_length);
        temp_key[key_length + 2] = '\0';
        //fprintf(fp, "key: %s\n", temp_key);
        uint32_t i;
        for(i = 0; i < k; ++i) {
          if(strcmp(temp_key, key_ptrs[i]) == 0) break;
        }
        //fprintf(fp, "index: %d\n", i);
        if(i == 0) {
          block_size = memcached_result_length(result_buffer);
        }
        if(i == k-1) {
          last_data_request_size = memcached_result_length(result_buffer);
        }
		temp_value_len += memcached_result_length(result_buffer);
        sub_values[i] = new char[memcached_result_length(result_buffer)];
        memcpy(sub_values[i], memcached_result_value(result_buffer), memcached_result_length(result_buffer));
        if(flags) {
          *flags= result_buffer->item_flags;
        }
      }
      *error = MEMCACHED_SUCCESS;
      value_length = &temp_value_len;
      //fprintf(fp, "value size: %d\n", temp_value_len);
      //fprintf(fp, "\n");
      //fclose(fp);
      char *temp_value = new char[temp_value_len];
      for(uint32_t i = 0; i < k; ++i) {
        if(i < k-1) {
          memcpy(temp_value + i*block_size, sub_values[i], block_size);
        } else {
          memcpy(temp_value + i*block_size, sub_values[i], last_data_request_size);
        }
      }
      if(sub_values != NULL) {
        for(uint32_t i = 0; i < k; ++i) {
          if(sub_values[i] != NULL) {
            delete sub_values[i];
          }
        }
        delete sub_values;
      }
      delete temp_key;
      if(key_ptrs != NULL) {
        for(uint32_t i = 0; i < k; ++i) {
          if(key_ptrs[i] != NULL) {
            delete key_ptrs[i];
          }
        }
        delete key_ptrs;
      }
      if(key_len_ptrs != NULL) {
        delete key_len_ptrs;
      }
      return temp_value;
  } else {
      //fprintf(fp, "rs, multiple get error !\n");
      //fprintf(fp, "\n");
      //fclose(fp);
      if(sub_values != NULL) {
        for(uint32_t i = 0; i < k; ++i) {
          if(sub_values[i] != NULL) {
            delete sub_values[i];
          }
        }
        delete sub_values;
      }
      if(key_ptrs != NULL) {
        for(uint32_t i = 0; i < k; ++i) {
          if(key_ptrs[i] != NULL) {
            delete key_ptrs[i];
          }
        }
        delete key_ptrs;
      }
      if(key_len_ptrs != NULL) {
        delete key_len_ptrs;
      }
      return NULL;
  }
}

//ERSadd
/*
 * get the parity blocks after redundancy transitioning,
 * both the ERS code and the SRS code call this function to get the parity blocks after redundancy transitioning,
 * e.g.,
 * d0, d1, d2, p0,
 * d3, d4, d5, p1,
 *             p2,
 * we first call "memcached_move" to transition p0, p1, p2 into p'0, p'1,
 * then call this function to get p'0, p'1.
 */
void memcached_get_parity_after_transition(memcached_st *ptr, 
                                           const char *key, 
                                           size_t key_length)
{
  uint32_t k = ptr->number_of_k;
  uint32_t s = ptr->number_of_s;
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_post = l/s;
  uint32_t m = ptr->number_of_m;

  char **key_ptrs = new char*[m*num_stripes_post];
  size_t *key_len_ptrs = new size_t[m*num_stripes_post];

  for(int i = 0; i < m*num_stripes_post; ++i) {
    key_ptrs[i] = new char[key_length + 4];
    memset(key_ptrs[i], 0, key_length + 4);
    memcpy(key_ptrs[i], key, key_length);
    int handred = (i+l) / 100;
    int ten = ((i+l) - handred*100) / 10;
    int one = (i+l) - handred*100 - ten*10;
    char handred_bit_char = '0' + handred;
    char ten_bit_char = '0' + ten;
    char one_bit_char = '0' + one;
    key_ptrs[i][key_length] = handred_bit_char;
    key_ptrs[i][key_length + 1] = ten_bit_char;
    key_ptrs[i][key_length + 2] = one_bit_char;
    key_ptrs[i][key_length + 3] = '\0';

    key_len_ptrs[i] = key_length + 3;
  }

  // multiple get the data blocks
  memcached_return_t unused;
  unused = memcached_mget(ptr, (const char * const *)key_ptrs, key_len_ptrs, m*num_stripes_post);

  if(unused == MEMCACHED_SUCCESS) {
      memcached_result_st *result_buffer = &ptr->result;
      char *temp_key = new char[key_length + 4];
      while((result_buffer = memcached_fetch_result(ptr, result_buffer, &unused)) != NULL) {
        strncpy(temp_key, result_buffer->item_key, result_buffer->key_length);
        temp_key[key_length + 3] = '\0';

        char *temp_value = new char[memcached_result_length(result_buffer)];
        memcpy(temp_value, memcached_result_value(result_buffer), memcached_result_length(result_buffer));

        FILE *fp = fopen(temp_key, "w");
        fwrite(temp_value, 1, memcached_result_length(result_buffer), fp);
        fclose(fp);
        delete temp_value;
      }
      delete temp_key;
  }

  if(key_ptrs != NULL) {
    for(uint32_t i = 0; i < m*num_stripes_post; ++i) {
      if(key_ptrs[i] != NULL) {
        delete key_ptrs[i];
      }
    }
    delete key_ptrs;
  }
  if(key_len_ptrs != NULL) {
    delete key_len_ptrs;
  }
}

//ERSadd
/* directly calculate the parity blocks after transitioning, ERS code (horizontal placement + designed matrix) */
static void ers_calculate_parity_blocks_after_transition_basic(const char *file, uint32_t k, uint32_t s, uint32_t m) {
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_post = l / s;

  // w is set to be 8 in our encoding
  FILE* fp = fopen(file, "r");

  // determining file size
  size_t file_size = 0;
  if(fp != NULL) {
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fclose(fp);
  }
  fp = fopen(file, "r");
  char *whole_file = new char[file_size];
  fread(whole_file, 1, file_size, fp);

  // determining block size
  size_t block_size = 0;
  size_t temp_file_size = 0;
  int size_of_long = sizeof(long);
  int mod = file_size % (l*size_of_long);
  if(mod == 0) {
    block_size = file_size / l;
    temp_file_size = file_size;
  } else {
    temp_file_size = file_size + l*size_of_long - mod;
    block_size = temp_file_size / l;
  }

  // fill data blocks
  char **data_ptrs = new char*[l];
  size_t bytes_remained = file_size;
  for(uint32_t i = 0; i < l; ++i) {
    data_ptrs[i] = new char[block_size];
    memset(data_ptrs[i], 0, block_size);
    if(block_size <= bytes_remained) {
      memcpy(data_ptrs[i], whole_file + i*block_size, block_size);
      bytes_remained -= block_size;
    } else {
      memcpy(data_ptrs[i], whole_file + i*block_size, bytes_remained);
      bytes_remained -= bytes_remained; 
    }
  }

  // calculate parity blocks
  char *parity_key = new char[3];
  parity_key[0] = 'p';
  int *matrix = reed_sol_vandermonde_coding_matrix(s, m, 8);
  for(uint32_t x = 0; x < num_stripes_post; ++x) {
    char **sub_data_ptrs = new char*[s];
    for(uint32_t i = 0; i < s; ++i) {
      sub_data_ptrs[i] = new char[block_size];
      memcpy(sub_data_ptrs[i], data_ptrs[x*s + i], block_size);
    }

    char **coding_ptrs = new char*[m];
    for(uint32_t i = 0; i < m; ++i) {
      coding_ptrs[i] = new char[block_size];
      memset(coding_ptrs[i], 0, block_size);
    }

    jerasure_matrix_encode(s, m, 8, matrix, sub_data_ptrs, coding_ptrs, block_size);
    for(uint32_t i = 0; i < m; ++i) {
      parity_key[1] = '0' + (x*m + i);
      parity_key[2] = '\0';
      FILE* fp2 = fopen(parity_key, "w");
      fwrite(coding_ptrs[i], 1, block_size, fp2);
      fclose(fp2);
    }

    for(uint32_t i = 0; i < s; ++i) {
      delete sub_data_ptrs[i];
    }
    delete sub_data_ptrs;
    for(uint32_t i = 0; i < m; ++i) {
      delete coding_ptrs[i];
    }
    delete coding_ptrs;
  }

  delete whole_file;
  for(uint32_t i = 0; i < l; ++i) {
    delete data_ptrs[i];
  }
  delete data_ptrs;
  fclose(fp);
}

static void enhanced_placement_ers(uint32_t *h_ids, uint32_t *v_ids, uint32_t k, uint32_t s);

//ERSadd
/* directly calculate the parity blocks after transitioning, ERS code (designed matrix + designed placement) */
static void ers_calculate_parity_blocks_after_transition_enhanced(const char *file, uint32_t k, uint32_t s, uint32_t m) {
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_post = l / s;
  uint32_t num_stripes_pre = l / k;

  // w is set to be 8 in our encoding
  FILE* fp = fopen(file, "r");

  // determining file size
  size_t file_size = 0;
  if(fp != NULL) {
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fclose(fp);
  }
  fp = fopen(file, "r");
  char *whole_file = new char[file_size];
  fread(whole_file, 1, file_size, fp);

  // determining block size
  size_t block_size = 0;
  size_t temp_file_size = 0;
  int size_of_long = sizeof(long);
  int mod = file_size % (l*size_of_long);
  if(mod == 0) {
    block_size = file_size / l;
    temp_file_size = file_size;
  } else {
    temp_file_size = file_size + l*size_of_long - mod;
    block_size = temp_file_size / l;
  }

  // fill data blocks
  char **data_ptrs = new char*[l];
  size_t bytes_remained = file_size;
  for(uint32_t i = 0; i < l; ++i) {
    data_ptrs[i] = new char[block_size];
    memset(data_ptrs[i], 0, block_size);
    if(block_size <= bytes_remained) {
      memcpy(data_ptrs[i], whole_file + i*block_size, block_size);
      bytes_remained -= block_size;
    } else {
      memcpy(data_ptrs[i], whole_file + i*block_size, bytes_remained);
      bytes_remained -= bytes_remained; 
    }
  }

  uint32_t *h_ids = new uint32_t[l];
  uint32_t *v_ids = new uint32_t[l];
  for(uint32_t i = 0; i < l; ++i) {
    h_ids[i] = v_ids[i] = 0;
  }
  enhanced_placement_ers(h_ids, v_ids, k, s);

  // calculate parity blocks
  char *parity_key = new char[3];
  parity_key[0] = 'p';
  int *matrix = reed_sol_vandermonde_coding_matrix(s, m, 8);
  for(uint32_t x = 0; x < num_stripes_post; ++x) {
    char **sub_data_ptrs = new char*[s];
    for(uint32_t i = 0; i < s; ++i) {
      sub_data_ptrs[i] = new char[block_size];
      memset(sub_data_ptrs[i], 0, block_size);

      uint32_t vid = x;
      uint32_t hid = i;

      uint32_t index = 0;
      for(; index < l; ++index) {
        if(h_ids[index] == hid && v_ids[index] == vid) {
          break;
        }
      }

      memcpy(sub_data_ptrs[i], data_ptrs[index], block_size);
    }

    char **coding_ptrs = new char*[m];
    for(uint32_t i = 0; i < m; ++i) {
      coding_ptrs[i] = new char[block_size];
      memset(coding_ptrs[i], 0,block_size);
    }

    jerasure_matrix_encode(s, m, 8, matrix, sub_data_ptrs, coding_ptrs, block_size);
    for(uint32_t i = 0; i < m; ++i) {
      parity_key[1] = '0' + (x*m + i);
      parity_key[2] = '\0';
      FILE* fp2 = fopen(parity_key, "w");
      fwrite(coding_ptrs[i], 1, block_size, fp2);
      fclose(fp2);
    }

    for(uint32_t i = 0; i < l/num_stripes_post; ++i) {
      delete sub_data_ptrs[i];
    }
    delete sub_data_ptrs;
    for(uint32_t i = 0; i < m; ++i) {
      delete coding_ptrs[i];
    }
    delete coding_ptrs;
  }

  delete whole_file;
  for(uint32_t i = 0; i < l; ++i) {
    delete data_ptrs[i];
  }
  delete data_ptrs;
  fclose(fp);

  delete h_ids;
  delete v_ids;
}

//ERSadd
/* directly calculate the parity blocks after transitioning, SRS code */
static void srs_calculate_parity_blocks_after_transition(const char *file, uint32_t k, uint32_t s, uint32_t m) {
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_post = l / s;

  // w is set to be 8 in our encoding
  FILE* fp = fopen(file, "r");

  // determining file size
  size_t file_size = 0;
  if(fp != NULL) {
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fclose(fp);
  }
  fp = fopen(file, "r");
  char *whole_file = new char[file_size];
  fread(whole_file, 1, file_size, fp);

  // determining block size
  size_t block_size = 0;
  size_t temp_file_size = 0;
  int size_of_long = sizeof(long);
  int mod = file_size % (l*size_of_long);
  if(mod == 0) {
    block_size = file_size / l;
    temp_file_size = file_size;
  } else {
    temp_file_size = file_size + l*size_of_long - mod;
    block_size = temp_file_size / l;
  }

  // fill data blocks
  char **data_ptrs = new char*[l];
  size_t bytes_remained = file_size;
  for(uint32_t i = 0; i < l; ++i) {
    data_ptrs[i] = new char[block_size];
    memset(data_ptrs[i], 0, block_size);
    if(block_size <= bytes_remained) {
      memcpy(data_ptrs[i], whole_file + i*block_size, block_size);
      bytes_remained -= block_size;
    } else {
      memcpy(data_ptrs[i], whole_file + i*block_size, bytes_remained);
      bytes_remained -= bytes_remained; 
    }
  }

  // calculate parity blocks
  char *parity_key = new char[3];
  parity_key[0] = 'p';
  int *matrix = reed_sol_vandermonde_coding_matrix(s, m, 8);
  for(uint32_t x = 0; x < num_stripes_post; ++x) {
    char **sub_data_ptrs = new char*[s];
    for(uint32_t i = 0; i < s; ++i) {
      sub_data_ptrs[i] = new char[block_size];
      memcpy(sub_data_ptrs[i], data_ptrs[i*num_stripes_post + x], block_size);
    }

    char **coding_ptrs = new char*[m];
    for(uint32_t i = 0; i < m; ++i) {
      coding_ptrs[i] = new char[block_size];
      memset(coding_ptrs[i], 0, block_size);
    }

    jerasure_matrix_encode(s, m, 8, matrix, sub_data_ptrs, coding_ptrs, block_size);
    for(uint32_t i = 0; i < m; ++i) {
      parity_key[1] = '0' + (x*m + i);
      parity_key[2] = '\0';
      FILE* fp2 = fopen(parity_key, "w");
      fwrite(coding_ptrs[i], 1, block_size, fp2);
      fclose(fp2);
    }

    for(uint32_t i = 0; i < s; ++i) {
      delete sub_data_ptrs[i];
    }
    delete sub_data_ptrs;
    for(uint32_t i = 0; i < m; ++i) {
      delete coding_ptrs[i];
    }
    delete coding_ptrs;
  }

  delete whole_file;
  for(uint32_t i = 0; i < l; ++i) {
    delete data_ptrs[i];
  }
  delete data_ptrs;
  fclose(fp);
}

//ERSadd
/*
 * directly calculate the parity blocks after transitioning,
 * e.g.,
 * d0, d1, d2, p0,
 * d3, d4, d5, p1,
 *             p2,
 * we directly use d0, d1, d2 to encode into p'0, and use d3, d4, d5 to encode into p'1,
 * this function is to verify the correctness of redundancy transitioning by comparing with "memcached_get_parity_after_transition".
 */
void memcached_calculate_parity_after_transition(memcached_st *ptr, 
                                                 const char *file)
{
  uint32_t k = ptr->number_of_k;
  uint32_t s = ptr->number_of_s;
  uint32_t l = lcm(k, s);
  uint32_t num_stripes_post = l/s;
  uint32_t num_stripes_pre = l/k;
  uint32_t m = ptr->number_of_m;

  if(ptr->ers_specified > 0) {
    if(ptr->ers_enhanced_placement > 0) {
      ers_calculate_parity_blocks_after_transition_enhanced(file, k, s, m);
    } else {
      ers_calculate_parity_blocks_after_transition_basic(file, k, s, m);
    }		
  }
  if(ptr->srs_specified > 0) {
    srs_calculate_parity_blocks_after_transition(file, k, s, m);
  }
}

char *memcached_get(memcached_st *ptr, const char *key,
                    size_t key_length,
                    size_t *value_length,
                    uint32_t *flags,
                    memcached_return_t *error)
{
  //ERSadd
  if(ptr->number_of_k > 0 and ptr->number_of_m > 0) {
    if(ptr->number_of_s > 0) {
      return memcached_get_ers(ptr, key,
                    key_length,
                    value_length,
                    flags,
                    error);
    } else {
      return memcached_get_rs(ptr, key,
                    key_length,
                    value_length,
                    flags,
                    error);
    }
  }

  return memcached_get_by_key(ptr, NULL, 0, key, key_length, value_length,
                              flags, error);
}

static memcached_return_t __mget_by_key_real(memcached_st *ptr,
                                             const char *group_key,
                                             size_t group_key_length,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             size_t number_of_keys,
                                             const bool mget_mode);
char *memcached_get_by_key(memcached_st *shell,
                           const char *group_key,
                           size_t group_key_length,
                           const char *key, size_t key_length,
                           size_t *value_length,
                           uint32_t *flags,
                           memcached_return_t *error)
{
  Memcached* ptr= memcached2Memcached(shell);
  memcached_return_t unused;
  if (error == NULL)
  {
    error= &unused;
  }

  uint64_t query_id= 0;
  if (ptr)
  {
    query_id= ptr->query_id;
  }

  /* Request the key */
  *error= __mget_by_key_real(ptr, group_key, group_key_length,
                             (const char * const *)&key, &key_length, 
                             1, false);
  if (ptr)
  {
    assert_msg(ptr->query_id == query_id +1, "Programmer error, the query_id was not incremented.");
  }

  if (memcached_failed(*error))
  {
    if (ptr)
    {
      if (memcached_has_current_error(*ptr)) // Find the most accurate error
      {
        *error= memcached_last_error(ptr);
      }
    }

    if (value_length) 
    {
      *value_length= 0;
    }

    return NULL;
  }

  char *value= memcached_fetch(ptr, NULL, NULL,
                               value_length, flags, error);
  assert_msg(ptr->query_id == query_id +1, "Programmer error, the query_id was not incremented.");

  /* This is for historical reasons */
  if (*error == MEMCACHED_END)
  {
    *error= MEMCACHED_NOTFOUND;
  }
  if (value == NULL)
  {
    if (ptr->get_key_failure and *error == MEMCACHED_NOTFOUND)
    {
      memcached_result_st key_failure_result;
      memcached_result_st* result_ptr= memcached_result_create(ptr, &key_failure_result);
      memcached_return_t rc= ptr->get_key_failure(ptr, key, key_length, result_ptr);

      /* On all failure drop to returning NULL */
      if (rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED)
      {
        if (rc == MEMCACHED_BUFFERED)
        {
          uint64_t latch; /* We use latch to track the state of the original socket */
          latch= memcached_behavior_get(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS);
          if (latch == 0)
          {
            memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 1);
          }

          rc= memcached_set(ptr, key, key_length,
                            (memcached_result_value(result_ptr)),
                            (memcached_result_length(result_ptr)),
                            0,
                            (memcached_result_flags(result_ptr)));

          if (rc == MEMCACHED_BUFFERED and latch == 0)
          {
            memcached_behavior_set(ptr, MEMCACHED_BEHAVIOR_BUFFER_REQUESTS, 0);
          }
        }
        else
        {
          rc= memcached_set(ptr, key, key_length,
                            (memcached_result_value(result_ptr)),
                            (memcached_result_length(result_ptr)),
                            0,
                            (memcached_result_flags(result_ptr)));
        }

        if (rc == MEMCACHED_SUCCESS or rc == MEMCACHED_BUFFERED)
        {
          *error= rc;
          *value_length= memcached_result_length(result_ptr);
          *flags= memcached_result_flags(result_ptr);
          char *result_value=  memcached_string_take_value(&result_ptr->value);
          memcached_result_free(result_ptr);

          return result_value;
        }
      }

      memcached_result_free(result_ptr);
    }
    assert_msg(ptr->query_id == query_id +1, "Programmer error, the query_id was not incremented.");

    return NULL;
  }

  return value;
}

memcached_return_t memcached_mget(memcached_st *ptr,
                                  const char * const *keys,
                                  const size_t *key_length,
                                  size_t number_of_keys)
{
  return memcached_mget_by_key(ptr, NULL, 0, keys, key_length, number_of_keys);
}

static memcached_return_t binary_mget_by_key(memcached_st *ptr,
                                             const uint32_t master_server_key,
                                             const bool is_group_key_set,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             const size_t number_of_keys,
                                             const bool mget_mode);

static memcached_return_t __mget_by_key_real(memcached_st *ptr,
                                             const char *group_key,
                                             const size_t group_key_length,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             size_t number_of_keys,
                                             const bool mget_mode)
{
  bool failures_occured_in_sending= false;
  const char *get_command= "get";
  uint8_t get_command_length= 3;
  unsigned int master_server_key= (unsigned int)-1; /* 0 is a valid server id! */

  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, true)))
  {
    return rc;
  }

  if (memcached_is_udp(ptr))
  {
    return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT);
  }

  LIBMEMCACHED_MEMCACHED_MGET_START();

  if (number_of_keys == 0)
  {
    return memcached_set_error(*ptr, MEMCACHED_INVALID_ARGUMENTS, MEMCACHED_AT, memcached_literal_param("Numbers of keys provided was zero"));
  }

  if (memcached_failed((rc= memcached_key_test(*ptr, keys, key_length, number_of_keys))))
  {
    assert(memcached_last_error(ptr) == rc);

    return rc;
  }

  bool is_group_key_set= false;
  if (group_key and group_key_length)
  {
    master_server_key= memcached_generate_hash_with_redistribution(ptr, group_key, group_key_length);
    is_group_key_set= true;
  }

  /*
    Here is where we pay for the non-block API. We need to remove any data sitting
    in the queue before we start our get.

    It might be optimum to bounce the connection if count > some number.
  */
  for (uint32_t x= 0; x < memcached_server_count(ptr); x++)
  {
    memcached_instance_st* instance= memcached_instance_fetch(ptr, x);

    if (instance->response_count())
    {
      char buffer[MEMCACHED_DEFAULT_COMMAND_SIZE];

      if (ptr->flags.no_block)
      {
        memcached_io_write(instance);
      }

      while(instance->response_count())
      {
        (void)memcached_response(instance, buffer, MEMCACHED_DEFAULT_COMMAND_SIZE, &ptr->result);
      }
    }
  }

  if (memcached_is_binary(ptr))
  {
    return binary_mget_by_key(ptr, master_server_key, is_group_key_set, keys,
                              key_length, number_of_keys, mget_mode);
  }

  if (ptr->flags.support_cas)
  {
    get_command= "gets";
    get_command_length= 4;
  }

  /*
    If a server fails we warn about errors and start all over with sending keys
    to the server.
  */
  WATCHPOINT_ASSERT(rc == MEMCACHED_SUCCESS);
  size_t hosts_connected= 0;
  for (uint32_t x= 0; x < number_of_keys; x++)
  {
    uint32_t server_key;

    if (is_group_key_set)
    {
      server_key= master_server_key;
    }
    else
    {
      server_key= memcached_generate_hash_with_redistribution(ptr, keys[x], key_length[x]);
    }

    memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);

    libmemcached_io_vector_st vector[]=
    {
      { get_command, get_command_length },
      { memcached_literal_param(" ") },
      { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
      { keys[x], key_length[x] }
    };


    if (instance->response_count() == 0)
    {
      rc= memcached_connect(instance);

      if (memcached_failed(rc))
      {
        memcached_set_error(*instance, rc, MEMCACHED_AT);
        continue;
      }
      hosts_connected++;

      if ((memcached_io_writev(instance, vector, 1, false)) == false)
      {
        failures_occured_in_sending= true;
        continue;
      }
      WATCHPOINT_ASSERT(instance->cursor_active_ == 0);
      memcached_instance_response_increment(instance);
      WATCHPOINT_ASSERT(instance->cursor_active_ == 1);
    }

    {
      if ((memcached_io_writev(instance, (vector + 1), 3, false)) == false)
      {
        memcached_instance_response_reset(instance);
        failures_occured_in_sending= true;
        continue;
      }
    }
  }

  if (hosts_connected == 0)
  {
    LIBMEMCACHED_MEMCACHED_MGET_END();

    if (memcached_failed(rc))
    {
      return rc;
    }

    return memcached_set_error(*ptr, MEMCACHED_NO_SERVERS, MEMCACHED_AT);
  }


  /*
    Should we muddle on if some servers are dead?
  */
  bool success_happened= false;
  for (uint32_t x= 0; x < memcached_server_count(ptr); x++)
  {
    memcached_instance_st* instance= memcached_instance_fetch(ptr, x);

    if (instance->response_count())
    {
      /* We need to do something about non-connnected hosts in the future */
      if ((memcached_io_write(instance, "\r\n", 2, true)) == -1)
      {
        failures_occured_in_sending= true;
      }
      else
      {
        success_happened= true;
      }
    }
  }

  LIBMEMCACHED_MEMCACHED_MGET_END();

  if (failures_occured_in_sending and success_happened)
  {
    return MEMCACHED_SOME_ERRORS;
  }

  if (success_happened)
  {
    return MEMCACHED_SUCCESS;
  }

  return MEMCACHED_FAILURE; // Complete failure occurred
}

memcached_return_t memcached_mget_by_key(memcached_st *shell,
                                         const char *group_key,
                                         size_t group_key_length,
                                         const char * const *keys,
                                         const size_t *key_length,
                                         size_t number_of_keys)
{
  Memcached* ptr= memcached2Memcached(shell);
  return __mget_by_key_real(ptr, group_key, group_key_length, keys, key_length, number_of_keys, true);
}

memcached_return_t memcached_mget_execute(memcached_st *ptr,
                                          const char * const *keys,
                                          const size_t *key_length,
                                          size_t number_of_keys,
                                          memcached_execute_fn *callback,
                                          void *context,
                                          unsigned int number_of_callbacks)
{
  return memcached_mget_execute_by_key(ptr, NULL, 0, keys, key_length,
                                       number_of_keys, callback,
                                       context, number_of_callbacks);
}

memcached_return_t memcached_mget_execute_by_key(memcached_st *shell,
                                                 const char *group_key,
                                                 size_t group_key_length,
                                                 const char * const *keys,
                                                 const size_t *key_length,
                                                 size_t number_of_keys,
                                                 memcached_execute_fn *callback,
                                                 void *context,
                                                 unsigned int number_of_callbacks)
{
  Memcached* ptr= memcached2Memcached(shell);
  memcached_return_t rc;
  if (memcached_failed(rc= initialize_query(ptr, false)))
  {
    return rc;
  }

  if (memcached_is_udp(ptr))
  {
    return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT);
  }

  if (memcached_is_binary(ptr) == false)
  {
    return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT,
                               memcached_literal_param("ASCII protocol is not supported for memcached_mget_execute_by_key()"));
  }

  memcached_callback_st *original_callbacks= ptr->callbacks;
  memcached_callback_st cb= {
    callback,
    context,
    number_of_callbacks
  };

  ptr->callbacks= &cb;
  rc= memcached_mget_by_key(ptr, group_key, group_key_length, keys,
                            key_length, number_of_keys);
  ptr->callbacks= original_callbacks;

  return rc;
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
}

static memcached_return_t simple_binary_mget(memcached_st *ptr,
                                             const uint32_t master_server_key,
                                             bool is_group_key_set,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             const size_t number_of_keys, const bool mget_mode)
{
  memcached_return_t rc= MEMCACHED_NOTFOUND;

  bool flush= (number_of_keys == 1);

  if (memcached_failed(rc= memcached_key_test(*ptr, keys, key_length, number_of_keys)))
  {
    return rc;
  }

  /*
    If a server fails we warn about errors and start all over with sending keys
    to the server.
  */
  //ERSadd
  uint32_t server_key_ec;
  uint32_t l = lcm(ptr->number_of_k, ptr->number_of_s);
  uint32_t *h_ids = new uint32_t[l];
  uint32_t *v_ids = new uint32_t[l];
  for(uint32_t i = 0; i < l; ++i) {
    h_ids[i] = v_ids[i] = 0;
  }
  if(ptr->number_of_k > 0 && ptr->number_of_m > 0 && ptr->number_of_s > 0 && ptr->ers_specified > 0) {
    // calculate the placement
    bool enhanced = (ptr->ers_enhanced_placement > 0 && ptr->number_of_s % ptr->number_of_k != 0);
    if(enhanced) enhanced_placement_ers(h_ids, v_ids, ptr->number_of_k, ptr->number_of_s);
  }

  for (uint32_t x= 0; x < number_of_keys; ++x)
  {
    uint32_t server_key;

    //ERSadd
    if(number_of_keys > 1 && ptr->number_of_k > 0 && ptr->number_of_m > 0) {
        /* number_of_keys > 1 is to distinguish get key or get sub-keys, 19-10-14 */
      if(ptr->number_of_s > 0) {
        uint32_t s = ptr->number_of_s;
        uint32_t k = ptr->number_of_k;
        uint32_t m = ptr->number_of_m;

        bool enhanced = (ptr->ers_enhanced_placement > 0 && s % k != 0);

        char handred_bit_char = keys[x][key_length[0] - 3];
        char ten_bit_char = keys[x][key_length[0] - 2];
        char one_bit_char = keys[x][key_length[0] - 1];
        int handred = handred_bit_char - '0';
        int ten = ten_bit_char - '0';
        int one = one_bit_char - '0';
        uint32_t index = handred*100 + ten*10 + one;

        if(x == 0) {
          char *temp_key = new char[key_length[0] - 2];
          memcpy(temp_key, keys[0], key_length[0] - 3);
          temp_key[key_length[0] - 3] = '\0';
          server_key_ec = memcached_generate_hash_with_redistribution(ptr, temp_key, key_length[0]-3);
          if(temp_key != NULL) delete temp_key;
        }

        uint32_t hid;
        if(index < l) {
          if(ptr->srs_specified > 0) {
            uint32_t num_stripes_post = l/s;
            hid = index / num_stripes_post;
          } else if(enhanced) {
            hid = h_ids[index];
          } else {
            hid = index % s;
          }

          server_key = (server_key_ec + hid) % memcached_server_count(ptr);
		} else {
          hid = (index - l) % m;
          server_key = (server_key_ec + s + hid) % memcached_server_count(ptr);
        }

      } else {
        char ten_bit_char = keys[x][key_length[0] - 2];
        char one_bit_char = keys[x][key_length[0] - 1];
        int ten = ten_bit_char - '0';
        int one = one_bit_char - '0';
        uint32_t index = ten*10 + one;

        if(x == 0) {
          char *temp_key = new char[key_length[0]-1];
          memcpy(temp_key, keys[0], key_length[0]-2);
          temp_key[key_length[0]-2] = '\0';
          server_key_ec = memcached_generate_hash_with_redistribution(ptr, temp_key, key_length[0]-2);
          if(temp_key != NULL) delete temp_key;
        }

        server_key = (server_key_ec + index) % memcached_server_count(ptr);

      }

    } else {

      if (is_group_key_set)
      {
        server_key= master_server_key;
      }
      else
      {
        server_key= memcached_generate_hash_with_redistribution(ptr, keys[x], key_length[x]);
      }
    }

    memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);

    if (instance->response_count() == 0)
    {
      rc= memcached_connect(instance);
      if (memcached_failed(rc))
      {
        continue;
      }
    }

    protocol_binary_request_getk request= { }; //= {.bytes= {0}};
    initialize_binary_request(instance, request.message.header);
    if (mget_mode)
    {
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETKQ;
    }
    else
    {
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETK;
    }

#if 0
    {
      memcached_return_t vk= memcached_validate_key_length(key_length[x], ptr->flags.binary_protocol);
      if (memcached_failed(rc= memcached_key_test(*memc, (const char **)&key, &key_length, 1)))
      {
        memcached_set_error(ptr, vk, MEMCACHED_AT, memcached_literal_param("Key was too long."));

        if (x > 0)
        {
          memcached_io_reset(instance);
        }

        return vk;
      }
    }
#endif

    request.message.header.request.keylen= htons((uint16_t)(key_length[x] + memcached_array_size(ptr->_namespace)));
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
    request.message.header.request.bodylen= htonl((uint32_t)( key_length[x] + memcached_array_size(ptr->_namespace)));

    libmemcached_io_vector_st vector[]=
    {
      { request.bytes, sizeof(request.bytes) },
      { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
      { keys[x], key_length[x] }
    };

    if (memcached_io_writev(instance, vector, 3, flush) == false)
    {
      memcached_server_response_reset(instance);
      rc= MEMCACHED_SOME_ERRORS;
      continue;
    }

    /* We just want one pending response per server */
    memcached_server_response_reset(instance);
    memcached_server_response_increment(instance);
    if ((x > 0 and x == ptr->io_key_prefetch) and memcached_flush_buffers(ptr) != MEMCACHED_SUCCESS)
    {
      rc= MEMCACHED_SOME_ERRORS;
    }
  }

  if (mget_mode)
  {
    /*
      Send a noop command to flush the buffers
    */
    protocol_binary_request_noop request= {}; //= {.bytes= {0}};
    request.message.header.request.opcode= PROTOCOL_BINARY_CMD_NOOP;
    request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;

    for (uint32_t x= 0; x < memcached_server_count(ptr); ++x)
    {
      memcached_instance_st* instance= memcached_instance_fetch(ptr, x);

      if (instance->response_count())
      {
        initialize_binary_request(instance, request.message.header);
        if ((memcached_io_write(instance) == false) or
            (memcached_io_write(instance, request.bytes, sizeof(request.bytes), true) == -1))
        {
          memcached_instance_response_reset(instance);
          memcached_io_reset(instance);
          rc= MEMCACHED_SOME_ERRORS;
        }
      }
    }
  }

  delete h_ids;
  delete v_ids;
  return rc;
}

static memcached_return_t replication_binary_mget(memcached_st *ptr,
                                                  uint32_t* hash,
                                                  bool* dead_servers,
                                                  const char *const *keys,
                                                  const size_t *key_length,
                                                  const size_t number_of_keys)
{
  memcached_return_t rc= MEMCACHED_NOTFOUND;
  uint32_t start= 0;
  uint64_t randomize_read= memcached_behavior_get(ptr, MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ);

  if (randomize_read)
  {
    start= (uint32_t)random() % (uint32_t)(ptr->number_of_replicas + 1);
  }

  /* Loop for each replica */
  for (uint32_t replica= 0; replica <= ptr->number_of_replicas; ++replica)
  {
    bool success= true;

    for (uint32_t x= 0; x < number_of_keys; ++x)
    {
      if (hash[x] == memcached_server_count(ptr))
      {
        continue; /* Already successfully sent */
      }

      uint32_t server= hash[x] +replica;

      /* In case of randomized reads */
      if (randomize_read and ((server + start) <= (hash[x] + ptr->number_of_replicas)))
      {
        server+= start;
      }

      while (server >= memcached_server_count(ptr))
      {
        server -= memcached_server_count(ptr);
      }

      if (dead_servers[server])
      {
        continue;
      }

      memcached_instance_st* instance= memcached_instance_fetch(ptr, server);

      if (instance->response_count() == 0)
      {
        rc= memcached_connect(instance);

        if (memcached_failed(rc))
        {
          memcached_io_reset(instance);
          dead_servers[server]= true;
          success= false;
          continue;
        }
      }

      protocol_binary_request_getk request= {};
      initialize_binary_request(instance, request.message.header);
      request.message.header.request.opcode= PROTOCOL_BINARY_CMD_GETK;
      request.message.header.request.keylen= htons((uint16_t)(key_length[x] + memcached_array_size(ptr->_namespace)));
      request.message.header.request.datatype= PROTOCOL_BINARY_RAW_BYTES;
      request.message.header.request.bodylen= htonl((uint32_t)(key_length[x] + memcached_array_size(ptr->_namespace)));

      /*
       * We need to disable buffering to actually know that the request was
       * successfully sent to the server (so that we should expect a result
       * back). It would be nice to do this in buffered mode, but then it
       * would be complex to handle all error situations if we got to send
       * some of the messages, and then we failed on writing out some others
       * and we used the callback interface from memcached_mget_execute so
       * that we might have processed some of the responses etc. For now,
       * just make sure we work _correctly_
     */
      libmemcached_io_vector_st vector[]=
      {
        { request.bytes, sizeof(request.bytes) },
        { memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace) },
        { keys[x], key_length[x] }
      };

      if (memcached_io_writev(instance, vector, 3, true) == false)
      {
        memcached_io_reset(instance);
        dead_servers[server]= true;
        success= false;
        continue;
      }

      memcached_server_response_increment(instance);
      hash[x]= memcached_server_count(ptr);
    }

    if (success)
    {
      break;
    }
  }

  return rc;
}

static memcached_return_t binary_mget_by_key(memcached_st *ptr,
                                             const uint32_t master_server_key,
                                             bool is_group_key_set,
                                             const char * const *keys,
                                             const size_t *key_length,
                                             const size_t number_of_keys,
                                             const bool mget_mode)
{
  if (ptr->number_of_replicas == 0)
  {
    return simple_binary_mget(ptr, master_server_key, is_group_key_set,
                              keys, key_length, number_of_keys, mget_mode);
  }

  uint32_t* hash= libmemcached_xvalloc(ptr, number_of_keys, uint32_t);
  bool* dead_servers= libmemcached_xcalloc(ptr, memcached_server_count(ptr), bool);

  if (hash == NULL or dead_servers == NULL)
  {
    libmemcached_free(ptr, hash);
    libmemcached_free(ptr, dead_servers);
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  }

  if (is_group_key_set)
  {
    for (size_t x= 0; x < number_of_keys; x++)
    {
      hash[x]= master_server_key;
    }
  }
  else
  {
    for (size_t x= 0; x < number_of_keys; x++)
    {
      hash[x]= memcached_generate_hash_with_redistribution(ptr, keys[x], key_length[x]);
    }
  }

  memcached_return_t rc= replication_binary_mget(ptr, hash, dead_servers, keys,
                                                 key_length, number_of_keys);

  WATCHPOINT_IFERROR(rc);
  libmemcached_free(ptr, hash);
  libmemcached_free(ptr, dead_servers);

  return MEMCACHED_SUCCESS;
}
