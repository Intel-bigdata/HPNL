#include "util/ByteBuffer.h"

#include <iostream>

ByteBuffer::ByteBuffer(int size) {
  this->size = size;
  limit_len = size;
  pos = 0;
  buff = (char*)std::malloc(size*sizeof(char));
}

ByteBuffer::~ByteBuffer() {
  size = 0;
  limit_len = 0;
  pos = 0;
  std::free(buff);
}

size_t ByteBuffer::get_size() {
  return size;
}

void ByteBuffer::position(int pos) {
  this->pos = pos;
}

void ByteBuffer::limit(int limit_len) {
  this->limit_len = limit_len;
}

void ByteBuffer::reset() {
  pos = 0;
}

char* ByteBuffer::get_bytes() {
  return buff;
}

char ByteBuffer::get() {
  return buff[pos++];
}

