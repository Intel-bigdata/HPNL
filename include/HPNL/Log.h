#ifndef LOG_H
#define LOG_H

#include "NanoLog.h"

class Log {
  public:
    Log(std::string const log_directory, std::string const log_file_name, nanolog::LogLevel level_) : directory(log_directory), name(log_file_name), level(level_), started(false) {}
    void start() {
      started = true;
      nanolog::initialize(nanolog::GuaranteedLogger(), directory, name, 1);
      nanolog::set_log_level(level);
    }
    void log(const std::string content) {
      if (started) {
        if (level == nanolog::LogLevel::CRIT) {
          LOG_CRIT << content; 
        } else if (level == nanolog::LogLevel::INFO) {
          LOG_INFO << content; 
        } else if (level == nanolog::LogLevel::WARN) {
          LOG_WARN << content; 
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
