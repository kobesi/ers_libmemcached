CC = g++ -std=c++11
CLIBS = -pthread
CFLAGS = -g -Wall -O2 -lm -lrt -Ilibmemcached-1.0.18 -Llibmemcached-1.0.18/lib

test_libmem: test_libmem.cc
	$(CC) $(CFLAGS) -o $@ $^ $(CLIBS) -lmemcached

clean_test_libmem:
	rm test_libmem
