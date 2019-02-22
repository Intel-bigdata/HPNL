#ifndef COMMON_H
#define COMMON_H

#define BUFFER_SIZE 65536
#define MEM_SIZE 65536
#define MAX_WORKERS 10

#include <system_error>

inline void throw_system_error(bool condition, const char* what) {
  if (condition) {
    throw std::system_error(errno, std::system_category(), what);
  }
}

#endif
