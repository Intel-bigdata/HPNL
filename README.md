# HPNL
HPNL is a **Fast**, **CPU-Efficient** network library designed for modern network technology([#1](https://www.intel.com/content/www/us/en/ethernet-products/iwarp-rdma-here-and-now-technology-brief.html) [#2](https://www.intel.com/content/www/us/en/high-performance-computing-fabrics/omni-path-driving-exascale-computing.html))

## Contents
- [Introduction](#introduction)
- [Installation](#installation)
- [Usage](#usage)
- [Contact](#contact)

## Introduction
Nowadays data is growing at a faster rate than ever before and large-scale data analytics present new challenge. HPNL is 
a light-weight network library built on Libfabric for big data application. It provides C/JAVA API and high level abstraction 
to let developer easily replace other TCP/IP based network library, like ASIO or Netty, without knowing the low level 
details of RDMA programing model. Ease to use API, functionality and performance is of primary design concern. 

### High level design

## Installation

### Build prerequisites
Library dependencies:
- C++ 11
- Libfabric 1.6+
- Boost 1.58+
- PMDK (optional)
- JDK 1.8+ (optional)

HPNL depends on [Libfabri](https://github.com/ofiwg/libfabric). Please make sure the Libfabric is installed in your setup.
Persistent Memory over Fabric is another targeting feature of HPNL with which you can use HPNL interface to communicate with Persistent Memory over different kinds of network fabrics. If you want to try
the PMoF example, please install [PMDK](https://github.com/pmem/pmdk), a library to manage and access persistent memory devices. 

On Ubuntu/Debian, install other requirements with:

```shell
sudo apt-get install cmake \
                     libboost-dev \
                     libboost-system-dev \
```

### Build for C/C++
This project supports CMake out of box.

```shell
git clone https://github.com/Intel-bigdata/HPNL.git
cd HPNL; mkdir build; cd build; cmake ..
make && make install
```

By default, HPNL runs over TCP/IP protocol, in this mode, you may expect similar performance to other sockets based network library. Thanks to Libfabric, HPNL also supports 
RDMA, Omni-Path protocol, you can switch between different protocols by adding cmake build option as follows.

```shell
cmake -DWITH_VERBS=ON ..     // for RDMA Protocol
cmake -DWITH_PSM2=ON ..      // for Omni-Path Protocol
```

### Build for Java
The HPNL Java API comes with HPNL C/C++ shared library, the native code won't be included without explicit cmake option.

```shell
cmake -DWITH_JAVA=ON ..      // to include native code for Java API
cd ${project_root_path}/java/hpnl; mvn package
```

### Test
```shell
cd ${project_root_path}/build
ctest                        // cmake ctest is binded to Catch2 unit-test framework
```

## Usage
To get started, refer to the mini tutorial. Simple C/S examples can be found in [HPNL/example](https://github.com/Intel-bigdata/HPNL/tree/master/example) directory. 

## Contact
jian.zhang@intel.com

haodong.tang@intel.com


