#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"
#include "HPNL/ChunkMgr.h"
#include "service/Service.h"

TEST_CASE("msg server") {
  int total_buffer_num = 512;
  int single_buffer_num = 16;
  int buffer_size = 65536;
  auto bufMgr = new DefaultChunkMgr(total_buffer_num, buffer_size);
  auto service = new Service(1, single_buffer_num, true);
  SECTION("init") {
    REQUIRE(service->init(true) == 0);
  }
  SECTION("init->start->listen") {
    REQUIRE(service->init(true) == 0);
    service->set_buf_mgr(bufMgr);
    service->start();
    REQUIRE(service->listen(nullptr, nullptr) == -1);
    service->shutdown();
  }
  SECTION("init->listen->start") {
    REQUIRE(service->init(true) == 0);
    service->set_buf_mgr(bufMgr);
    REQUIRE(service->listen(nullptr, nullptr) == -1);
    service->start();
    service->shutdown();
  }
  delete service;
  delete bufMgr;
}

TEST_CASE("msg client") {
  int total_buffer_num = 512;
  int single_buffer_num = 16;
  int buffer_size = 65536;
  auto bufMgr = new DefaultChunkMgr(total_buffer_num, buffer_size);
  auto service = new Service(1, single_buffer_num, false);
  SECTION("init") {
    REQUIRE(service->init(true) == 0);
  }
  SECTION("init->start->connect") {
    REQUIRE(service->init(true) == 0);
    service->set_buf_mgr(bufMgr);
    service->start();
    REQUIRE(service->connect(nullptr, nullptr) == -1);
    service->shutdown();
  }
  SECTION("init->connect->start") {
    REQUIRE(service->init(true) == 0);
    service->set_buf_mgr(bufMgr);
    REQUIRE(service->connect(nullptr, nullptr) == -1);
    service->start();
    service->shutdown();
  }
  delete service;
  delete bufMgr;
}

TEST_CASE("rdm server") {
  int total_buffer_num = 512;
  int single_buffer_num = 16;
  int buffer_size = 65536;
  auto bufMgr = new DefaultChunkMgr(total_buffer_num, buffer_size);
  auto service = new Service(1, single_buffer_num, true);
  SECTION("init") {
    REQUIRE(service->init(false) == 0);
  }
  SECTION("init->start->listen") {
    REQUIRE(service->init(false) == 0);
    service->set_buf_mgr(bufMgr);
    service->start();
    REQUIRE(service->listen(nullptr, nullptr) == -1);
    service->shutdown();
  }
  SECTION("init->listen->start") {
    REQUIRE(service->init(false) == 0);
    service->set_buf_mgr(bufMgr);
    REQUIRE(service->listen(nullptr, nullptr) == -1);
    service->start();
    service->shutdown();
  }
  delete service;
  delete bufMgr;
}

TEST_CASE("rdm client") {
  int total_buffer_num = 512;
  int single_buffer_num = 16;
  int buffer_size = 65536;
  auto bufMgr = new DefaultChunkMgr(total_buffer_num, buffer_size);
  auto service = new Service(1, single_buffer_num, false);
  SECTION("init") {
    REQUIRE(service->init(false) == 0);
  }
  SECTION("init->start->connect") {
    REQUIRE(service->init(false) == 0);
    service->set_buf_mgr(bufMgr);
    service->start();
    REQUIRE(service->connect(nullptr, nullptr) == -1);
    service->shutdown();
  }
  SECTION("init->connect->start") {
    REQUIRE(service->init(false) == 0);
    service->set_buf_mgr(bufMgr);
    service->start();
    REQUIRE(service->connect(nullptr, nullptr) == -1);
    service->shutdown();
  }
  delete service;
  delete bufMgr;
}
