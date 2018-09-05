# HPNL
HPNL is a **Fast**, **CPU-Efficient** network library desinged for [modern network technology](#modern_network).

## Contents
- [Introduction](#introduction)
- [Installation](#installation)
- [Benchmark](#benchmark)
- [Usage](#usage)
- [Contact](#contact)

## Introduction

### High level design

<h3 id="modern_network">Support modern network technology</h3>

## Installation

### Build prerequisites
HPNL depends on Libfabric library. For more infomation, please refer to [https://github.com/ofiwg/libfabric](https://github.com/ofiwg/libfabric).

### Build for C++
This project supports CMake out of box.

```shell
git clone https://github.com/Intel-bigdata/HPNL.git
cd HPNL; mkdir build; cd build; cmake ..
make && make install
```

By default, the **header file** and **shared library file** will be generated in /usr/local. Please see the CMakeLists.txt for more advanced usage. 

### Build for Java
The HPNL Java interface comes with libhpnl JNI wrapper. Make sure have the libhpnl.so and header file installed before building the jar.

```shell
cd HPNL/java/hpnl; mvn package
```

## Benchmark

## Usage
To get started, refer to the mini tutorial. Simple C/S examples can be found in [HPNL/test](https://github.com/Intel-bigdata/HPNL/tree/master/test) directory. 

## Contact

