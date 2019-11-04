CFLAGS = -Wall -g
CC = gcc
INCLUDES = -I/usr/local/include -I/usr/include/c++/4.8.2 -I/usr/include/c++/4.8.2/x86_64-redhat-linux -I/opt/jdk1.8.0_202/include -I/opt/jdk1.8.0_202/include/linux
LDFLAGS = -L/usr/local/lib

all: 
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) stack.c -lfabric -o stack.o
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) test.c -lfabric -o test.o
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) scalable_endpoint.c -lfabric -o scalable_endpoint.o
gni:
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) scalable_endpoint_gni.c -lfabric -o scalable_endpoint_gni.o
rdma:
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) scalable_rdma.c -lfabric -o scalable_rdma.o
gni:	
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) com_intel_hpnl_buffer_Utils.c -fPIC -shared -o test.so

clean:
	rm -rf *.o

