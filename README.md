In this file, we briefly introduce how to deploy and run the ers kv store prototype. More design details can be referred to our paper "__Si Wu, Zhirong Shen, Patrick P. C. Lee__, Enabling I/O-Efficient Redundancy Transitioning in Erasure-Coded KV Stores via Elastic Reed-Solomon Codes" which appears at _SRDS 2020_. If you have any questions regarding the deployment and the code, please feel free to contact me at siwu5938@gmail.com.

## 1. Descriptions

The ers kv store prototype is implemented based on libmemcached-1.0.18. We integrate the ers/srs/rs codes, the normal set/get/update operations of ers/srs/rs codes, and the redundancy transitioning operations into libmemcached. Note that ers/srs/rs codes are added by modifying the read/write pathes of libmemcached. To show how we modify libmemcached, we have a file, _modified_file_list_, which tells what files we have modified in the libmemcached-1.0.18/ directory. This file, i.e., modified_file_list, can also be regarded as a guide for how to integrate a erasure coding strategy into libmemcached.

To test the encoding and transitioning performance of ers/srs codes, we can call the _memcached_set_, _memcached_get_, and _memcached_move_ functions. As an example, we have a test file, _test_libmem.cc_, in the root directory. This file sets several configurations of memcached and calls set/get/transition functions for replication/ers/srs. Users can modify this file slightly (i.e., modify the server ips) to test the read/write/transitioning performance of replication/ers/srs.

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
$/memcached -m 128 -p 8888 --max-item-size=5242880 -vv
```

In this command, -m 128 means the allocated buffer size is 128 MB. -p 8888 is the default port. --max-item-size=5242880 means the largest object size can be 5 MB. -vv means that the the terminal will print detailed messages.

#### 3.2. Installation of the libmemcached proxy

- Downloading our sourcecode:

()

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

## 4. Running

