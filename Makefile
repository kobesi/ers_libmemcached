CC = g++ -std=c++11
CLIBS = -pthread
CFLAGS = -g -Wall -O2 -lm -lrt
dir_jerasure_te = libmemcached-1.0.18/Jerasure-1.2A
JERASURE_LIB_O = $(dir_jerasure_te)/cauchy.o $(dir_jerasure_te)/reed_sol.o $(dir_jerasure_te)/jerasure.o $(dir_jerasure_te)/galois.o

test_libmem: test_libmem.cc
	$(CC) $(CFLAGS) -o $@ $^ $(CLIBS) $(JERASURE_LIB_O) -lmemcached

clean_test_libmem:
	rm test_libmem
