#ifndef LOG_H
#define LOG_H

#include <ostream>

#include "log/NanoLog.h"

enum LOG_LEVEL {
  INFO,
  DEBUG,
  CRIT
};

class Log {
  public:
    Log(std::string const log_directory, std::string const log_file_name, nanolog::LogLevel level_) : directory(log_directory), name(log_file_name), level(level_), started(false) {}
    void start() {
      started = true;
      nanolog::initialize(nanolog::GuaranteedLogger(), directory, name, 1);
      nanolog::set_log_level(level);
    }
    void log(LOG_LEVEL log_level, const std::string content) {
      if (started) {
        if (log_level == CRIT) {
          LOG_CRIT << content; 
        } else if (log_level == INFO) {
          LOG_INFO << content; 
        } else if (log_level == DEBUG) {
          LOG_DEBUG << content; 
        } else {
        
        }
      }
    }

  private:
    std::string directory;
    std::string name;
    nanolog::LogLevel level;
    bool started;
};

#endif
