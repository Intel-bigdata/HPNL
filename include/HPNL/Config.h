#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>

#include <libconfig.h++>

class Config {
  public:
    Config() {
      init();
      try {
        cfg.readFile("/etc/hpnl.conf");
      } catch (const libconfig::FileIOException &e) {
        return;
      }
      try {
        worker_num = cfg.lookup("worker_num");
      } catch (...) {
      }
    }

    Config& operator=(const Config& config) {
      if (&config != this) {
        worker_num = config.worker_num;
      }
      return *this;
    }

    Config(const Config& config) {
      worker_num = config.worker_num; 
    }

    void init() {
      worker_num = 1; 
    }
  public:
    int worker_num;
  private:
    libconfig::Config cfg;
};

#endif
