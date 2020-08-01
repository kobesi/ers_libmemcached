#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <libmemcached/memcached.h>

using namespace std;

void replication_upload_download_test(string key, int value_size, string input_file, string output_file, uint64_t r) {
  memcached_st* memc;
  memcached_return_t rc;
  memcached_server_list_st servers;
  time_t expiration = 0;
  uint32_t flags = 0;
  memc = memcached_create(NULL);
  servers = memcached_server_list_append(NULL, "192.168.10.24", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.10.25", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.10.26", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.10.28", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.10.30", 8888, &rc);
  rc = memcached_server_push(memc, servers);
  if(rc != MEMCACHED_SUCCESS) {
    cout<<"Add servers failure!"<<endl;
  }
  memcached_server_list_free(servers);
  // change behavior, MEMCACHED_BEHAVIOR_DISTRIBUTION
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
  // change behavior, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, (uint64_t)1);
  // set number_of_replicas
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS, r);

  double write_time = 0.0;
  double read_time = 0.0;
  struct timeval start_time, end_time;

  size_t key_length = key.length();
  char* value = new char[value_size];
  FILE* fp = fopen(input_file.c_str(), "r");
  fread(value, 1, value_size, fp);
  fclose(fp);
  size_t value_length = value_size;
  gettimeofday(&start_time, NULL);
  rc = memcached_set(memc, key.c_str(), key_length, value, value_length, expiration, flags);
  gettimeofday(&end_time, NULL);
  if(rc == MEMCACHED_SUCCESS) {
    cout<<"Save data:<"<<key<<endl;
  }
  write_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  fprintf(stderr, "~~~~~~ replication write time: %.6lf s\n", write_time);

  gettimeofday(&start_time, NULL);
  char *result = memcached_get(memc, key.c_str(), key_length, &value_length, &flags, &rc);
  gettimeofday(&end_time, NULL);
  if(rc == MEMCACHED_SUCCESS || MEMCACHED_END) {
    cout<<"Get value length: "<<value_length<<endl;
    FILE* fp2 = fopen(output_file.c_str(), "w");
    fwrite(result, 1, value_length, fp2);
    fclose(fp2);
  } else {
    cout<<"Get error!"<<endl;
  }
  read_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  fprintf(stderr, "~~~~~~ replication read time: %.6lf s\n", read_time);

  memcached_free(memc);
  delete value;
}

void srs_ers_upload_download_test(string key, int value_size, string input_file, string output_file, uint64_t k, uint64_t m, uint64_t s, int srs, int enhanced) {
  memcached_st* memc;
  memcached_return_t rc;
  memcached_server_list_st servers;
  time_t expiration = 0;
  uint32_t flags = 0;
  memc = memcached_create(NULL);
  servers = memcached_server_list_append(NULL, "192.168.0.24", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.0.25", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.0.26", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.0.28", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.0.30", 8888, &rc);
  if(s >= 5) {
    servers = memcached_server_list_append(servers, "192.168.0.31", 8888, &rc);
    if(s >= 6) {
      servers = memcached_server_list_append(servers, "192.168.0.32", 8888, &rc);
    }
  }
  rc = memcached_server_push(memc, servers);
  if(rc != MEMCACHED_SUCCESS) {
    cout<<"Add servers failure!"<<endl;
  }
  memcached_server_list_free(servers);
  // change behavior, MEMCACHED_BEHAVIOR_DISTRIBUTION
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
  // change behavior, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, (uint64_t)1);
  // set k, m, s
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_K, k);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_M, m);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_S, s);
  if(srs > 0) {
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SRS_SPECIFIED, uint64_t(1));
  } else {
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_ERS_SPECIFIED, uint64_t(1));
    if(enhanced > 0) memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_ERS_ENHANCED_PLACEMENT, uint64_t(1));
  }

  double write_time = 0.0;
  double read_time = 0.0;
  struct timeval start_time, end_time;

  size_t key_length = key.length();
  char* value = new char[value_size];
  FILE* fp = fopen(input_file.c_str(), "r");
  fread(value, 1, value_size, fp);
  fclose(fp);
  size_t value_length = value_size;
  gettimeofday(&start_time, NULL);
  rc = memcached_set(memc, key.c_str(), key_length, value, value_length, expiration, flags);
  gettimeofday(&end_time, NULL);
  if(rc == MEMCACHED_SUCCESS) {
    cout<<"Save data:"<<key<<endl;
  }
  write_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;

  gettimeofday(&start_time, NULL);
  char *result = memcached_get(memc, key.c_str(), key_length, &value_length, &flags, &rc);
  gettimeofday(&end_time, NULL);
  if(rc == MEMCACHED_SUCCESS || MEMCACHED_END) {
    cout<<"Get value length: "<<value_length<<endl;
    FILE* fp2 = fopen(output_file.c_str(), "w");
    fwrite(result, 1, value_length, fp2);
    fclose(fp2);
  } else {
    cout<<"Get error!"<<endl;
  }
  read_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;

  if(srs > 0) {
    fprintf(stderr, "~~~~~~ srs write time: %.6lf s\n", write_time);
    fprintf(stderr, "~~~~~~ srs read time: %.6lf s\n", read_time);
  } else {
    if(enhanced) {
      fprintf(stderr, "~~~~~~ ers enhanced write time: %.6lf s\n", write_time);
      fprintf(stderr, "~~~~~~ ers enhanced read time: %.6lf s\n", read_time);
    } else {
      fprintf(stderr, "~~~~~~ ers basic write time: %.6lf s\n", write_time);
      fprintf(stderr, "~~~~~~ ers basic read time: %.6lf s\n", read_time);
    }
  }


  memcached_free(memc);
  delete value;
}

void srs_ers_transition_test(string key, uint64_t k, uint64_t m, uint64_t s, int srs, int enhanced) {
  memcached_st* memc;
  memcached_return_t rc;
  memcached_server_list_st servers;
  time_t expiration = 0;
  uint32_t flags = 0;
  memc = memcached_create(NULL);
  servers = memcached_server_list_append(NULL, "192.168.0.24", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.0.25", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.0.26", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.0.28", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.0.30", 8888, &rc);
  if(s >= 5) {
    servers = memcached_server_list_append(servers, "192.168.0.31", 8888, &rc);
    if(s >= 6) {
      servers = memcached_server_list_append(servers, "192.168.0.32", 8888, &rc);
    }
  }
  rc = memcached_server_push(memc, servers);
  if(rc != MEMCACHED_SUCCESS) {
    cout<<"Add servers failure!"<<endl;
  }
  memcached_server_list_free(servers);
  // change behavior, MEMCACHED_BEHAVIOR_DISTRIBUTION
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
  // change behavior, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, (uint64_t)1);
  // set k, m, s
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_K, k);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_M, m);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_S, s);
  if(srs > 0) {
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SRS_SPECIFIED, uint64_t(1));
  } else {
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_ERS_SPECIFIED, uint64_t(1));
    if(enhanced > 0) memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_ERS_ENHANCED_PLACEMENT, uint64_t(1));
  }

  double transition_time = 0.0;
  struct timeval start_time, end_time;

  size_t key_length = key.length();

  gettimeofday(&start_time, NULL);
  rc = memcached_move(memc, key.c_str(), key_length);
  gettimeofday(&end_time, NULL);
  if(rc != MEMCACHED_SUCCESS) {
    cout<<"Transition object error!"<<endl;
  }
  transition_time = end_time.tv_sec-start_time.tv_sec+(end_time.tv_usec-start_time.tv_usec)*1.0/1000000;
  fprintf(stderr, "~~~~~~ transition time: %.6lf s\n", transition_time);

  memcached_get_parity_after_transition(memc, key.c_str(), key_length);

  memcached_free(memc);
}

void calculate_parity_blocks_after_transition(string input_file, uint64_t k, uint64_t m, uint64_t s, int srs, int enhanced) {
  memcached_st* memc;
  memcached_return_t rc;
  memcached_server_list_st servers;
  time_t expiration = 0;
  uint32_t flags = 0;
  memc = memcached_create(NULL);
  servers = memcached_server_list_append(NULL, "192.168.10.24", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.10.25", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.10.26", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.10.28", 8888, &rc);
  servers = memcached_server_list_append(servers, "192.168.10.30", 8888, &rc);
  if(s >= 5) {
    servers = memcached_server_list_append(servers, "192.168.10.31", 8888, &rc);
    if(s >= 6) {
      servers = memcached_server_list_append(servers, "192.168.10.32", 8888, &rc);
    }
  }
  rc = memcached_server_push(memc, servers);
  if(rc != MEMCACHED_SUCCESS) {
    cout<<"Add servers failure!"<<endl;
  }
  memcached_server_list_free(servers);
  // change behavior, MEMCACHED_BEHAVIOR_DISTRIBUTION
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
  // change behavior, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, (uint64_t)1);
  // set k, m, s
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_K, k);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_M, m);
  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_S, s);
  if(srs > 0) {
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SRS_SPECIFIED, uint64_t(1));
  } else {
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_ERS_SPECIFIED, uint64_t(1));
    if(enhanced > 0) memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_ERS_ENHANCED_PLACEMENT, uint64_t(1));
  }

  memcached_calculate_parity_after_transition(memc, input_file.c_str());

  memcached_free(memc);
}

int main(int argc, char** argv) {
  if(argc != 6) {
    cout<<"./test_libmem code_name? enhanced_id_for_ers value_size operation parameter !"<<endl;
    cout<<"code_name: ers, srs"<<endl;
	cout<<"enhanced id: 0, 1"<<endl;
    cout<<"value size: xxxKB"<<endl;
    cout<<"operation: en(encode), tr(transition), ca(calculate parity blocks after transition)"<<endl;
    cout<<"parameter: p0, (2,1,3); p1, (4,1,5); p2, (5,1,6)"<<endl;
    cout<<"./test_libmem replication number_of_replicas value_size en p0 !"<<endl;
    exit(1);
  }

  if(strcmp(argv[1], "replication") == 0) {
    int replication_factor = argv[2][0] - '0';
    int value_size = atoi(argv[3]);
    string key = "rep_big_obj_k" + to_string(value_size);
    string input_file = "./input_item_" + to_string(value_size) + "K_REP";
    string output_file = "./output_item_" + to_string(value_size) + "K_REP";
    replication_upload_download_test(key, value_size*1024, 
	input_file, output_file, (uint64_t)replication_factor);
    return 1;
  }

  int srs;
  int enhanced;
  int value_size;
  srs = (strcmp(argv[1], "srs") == 0)?1:0;
  enhanced = argv[2][0] - '0';
  value_size = atoi(argv[3]);
  string key;
  string input_file;
  string output_file;
  if(srs > 0) {
    key = "srs_big_obj_k" + to_string(value_size);
    input_file = "./input_item_" + to_string(value_size) + "K_SRS";
    output_file = "./output_item_" + to_string(value_size) + "K_SRS";
  } else {
    key = "ers_big_obj_k" + to_string(value_size);
    input_file = "./input_item_" + to_string(value_size) + "K_ERS";
    output_file = "./output_item_" + to_string(value_size) + "K_ERS";
  }

  int k, m, s;
  if(strcmp(argv[5], "p0") == 0) {
    k = 2; m = 1; s = 3;
  }
  else if(strcmp(argv[5], "p1") == 0) {
    k = 4; m = 1; s = 5;
  }
  else if(strcmp(argv[5], "p2") == 0) {
    k = 5; m = 1; s = 6;
  }

  if(strcmp(argv[4], "en") == 0) {
    srs_ers_upload_download_test(key, value_size*1024, input_file, output_file, k, m, s, srs, enhanced);
  }
  else if (strcmp(argv[4], "tr") == 0) {
    srs_ers_transition_test(key, k, m, s, srs, enhanced);
  }
  else if (strcmp(argv[4], "ca") == 0) {
    calculate_parity_blocks_after_transition(input_file, k, m, s, srs, enhanced);
  }

  return 1;
}
