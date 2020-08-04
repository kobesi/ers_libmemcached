In this file, we briefly introduce how to deploy and run the ers kv store prototype. More design details can be referred to our paper "__Si Wu, Zhirong Shen, Patrick P. C. Lee__, Enabling I/O-Efficient Redundancy Transitioning in Erasure-Coded KV Stores via Elastic Reed-Solomon Codes" which appears at _SRDS 2020_. If you have any questions regarding the deployment and the code, please feel free to contact me at siwu5938@gmail.com.

## 1. Description

The ers kv store prototype is implemented based on libmemcached-1.0.18. We integrate the ers/srs/rs codes, the normal set/get/update operations of ers/srs/rs codes, and the redundancy transitioning operations into libmemcached. Note that ers/srs/rs codes are added by modifying the read/write pathes of libmemcached. To show how we modify libmemcached, we have a file, _modified_file_list_, which tells what files we have modified in the libmemcached-1.0.18/ directory. This file, i.e., modified_file_list, can also be regarded as a guide for how to integrate a erasure coding strategy into libmemcached.

We leverge the Jerasure library to realize ers/srs/rs codes. We include a Jerasure folder in the libmemcached-1.0.18/ directly. In enable Jerasure functions, we have modified the libmemcached-1.0.18/libmemcached/include.am file.

To test the encoding and transitioning performance of ers/srs codes, we can call the _memcached_set_, _memcached_get_, and _memcached_move_ functions. As an example, we have a test file, _test_libmem.cc_, in the root directory. This file sets several configurations (e.g., k, m, s of ers/srs codes) and calls set/get/transition functions for replication/ers/srs. Users can modify this file slightly (i.e., modify the server ips) to test the read/write/transitioning performance of replication/ers/srs.

The prototype is tested on Ubuntu 16.04.5, so we use Ubuntu as an example to show the deplyment.

## 2. Preparation

There are some required libraries that are listed as follows.

- make & cmake

- automake & aclocal & autoconf

- libevent

Users can use apt-get to install the required libraries.

Also, we give the versions of the libraries in our testbed for reference.

- gcc, g++, 5.4.0

- make, GNU 4.1, cmake, 3.8.1

- automake, aclocal, 1.14.1, autoconf, 2.69

- libevent, 2.1.11

## 3. Installation

We assume transitioning from ERS/SRS(k, m, k') = (2,1,3) to RS(k', m) = (3,1), so we should deploy four memcached servers (i.e., k' + m = 4), as well as one libmemcached proxy.

#### 3.1. Installation of the memcached servers

In each physical machine that runs as a memcached server, do the following.

- Downloading the sourcecode of memcached server:

(https://memcached.org/downloads)

- Unzipping:

```shell
$tar -zvxf memcached-1.5.20.tar.gz
```
- Entering the memcached directory, compling and installing:

```shell
$./configure --prefix=$(pwd)
```

```shell
$make & sudo make install
```

- Running memcached server:

A memcached executable is generated in the root directory. Note that by default, memcached does not support object size more than 1 MB. However, we should test object sizes of 1 MB and 4 MB in our experiments. We use the following command to run memcached server with the support of large object size.

```shell
$./memcached -m 128 -p 8888 --max-item-size=5242880 -vv
```

In this command, -m 128 means the allocated buffer size is 128 MB. -p 8888 is the default port. --max-item-size=5242880 means the largest object size can be 5 MB. -vv means that the the terminal will print detailed messages.

#### 3.2. Installation of the libmemcached proxy

- Downloading our sourcecode:

(https://github.com/kobesi/ers_libmemcached.git)

- Unzipping:

```shell
$tar -zvxf ers_libmemcached-1.0.0.tar.gz
```

- Entering the libmemcached-1.0.18/ directory, compiling and installing:

```shell
$./configure --prefix=$(pwd)
```

```shell
$make & sudo make install
```

- Going to the root directory (i.e., ers_libmemcached/) , compiling the test program:

```shell
$make test_libmem
```

A test_libmem executable is generated in the root directory.

## 4. Running and testing

The input command of test_libmem is like "./test_libmem code_name enhanced_tag value_size operation parameter", where the code_name could be 'srs' and 'ers', 'enhanced_tag' means whether the designed/enhanced placement of ers code is specified, 'value_size' means the size of the value (in KB), operation can be 'en' (encoding)/ 'tr' (transitioning)/ 'ca' (calculating), and 'parameter' means the transitioning parameter and it could be 'p0' (k=2, m=1, k'=3)/ 'p1' (k=4, m=1, k'=5)/ 'p2' ((k=5, m=1, k'=6)). We assume 1 KB value size and 
(k=2, m=1, k'=3) in the following.

At the root directory, we first generate two 1 KB sized files, called 'input_item_1K_ERS' and 'input_item_1K_SRS', which store the values of objects. The keys of objects are generated in test_libmem.cc. Then we start to test the encoding and transitioning performance of srs/ers codes.

- SRS encoding:

```shell
$./test_libmem srs 0 1 en p0
```

This is to encode the object with srs code and then download the object. We can get the read/write time of srs code. Besides, the value of the downloaded object is written into a file called 'output_item_1K_SRS'. We can compare 'input_item_1K_SRS' with 'output_item_1K_SRS' to check whether the read/write processes of srs code are right or not.

- SRS transitioning:

```shell
$./test_libmem srs 0 1 tr p0
```

This is to transition the object from srs code into rs code. We can get the transition time of srs code. Besides, we will download the new parity blocks (of rs(k', m)) and write each of them into a file, e.g., 'p0', 'p1'.

```shell
$./test_libmem srs 0 1 ca p0
```

This is to calculate the new parity blocks directly (i.e., calculating the new parity blocks from the data blocks, not from the old parity blocks as the transitioning does). We will get several files, e.g., 'srs_big_obj_k1006', 'srs_big_obj_k1007'. By comparing 'srs_big_obj_k1006' and 'srs_big_obj_k1007' with 'p0' and 'p1', we can validate the rightness of the transitioning process.

- ERS encoding:

```shell
$./test_libmem ers 0 1 en p0
```

This is to encode the object with ers code and then download the object. We can get the read/write time of ers code. _We can expect that the read/write time of ers code is similar to that of srs code_.

- ERS transitioning:

```shell
$./test_libmem ers 0 1 tr p0
```

This is to transition the object from ers code into rs code. We can get the transition time of ers code. _We can expect that the transitioning time of ers code is significantly lower than that of srs code_.

```shell
$./test_libmem ers 0 1 ca p0
```

This is also to validate the rightness of the transitioning process.