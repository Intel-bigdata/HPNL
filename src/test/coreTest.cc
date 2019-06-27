#define CATCH_CONFIG_MAIN

#include <vector>

#include "catch2/catch.hpp"
#include "core/MsgStack.h"
#include "core/RdmStack.h"
#include "core/MsgConnection.h"
#include "HPNL/Common.h"
#include "HPNL/HpnlBufMgr.h"

TEST_CASE("msg server") {
  MsgStack *stack = new MsgStack(1, 6, true);
  SECTION("init") {
    REQUIRE(stack->init() == 0);
  }
  SECTION("init->bind->listen") {
    REQUIRE(stack->init() == 0);
    REQUIRE(stack->bind(NULL, NULL, NULL) == NULL);
    REQUIRE(stack->listen() == -1);
  }
  SECTION("init->listen") {
    REQUIRE(stack->init() == 0);
    REQUIRE(stack->listen() == -1);
  }
  SECTION("bind->listen") {
    REQUIRE(stack->bind(NULL, NULL, NULL) == NULL);
    REQUIRE(stack->listen() == -1);
  }
  SECTION("listen") {
    REQUIRE(stack->listen() == -1);
  }

  delete stack;
}

TEST_CASE("msg client") {
  int buffer_num_per_connection = 16;
  MsgStack *stack = new MsgStack(1, buffer_num_per_connection, false);
  SECTION("init") {
    REQUIRE(stack->init() == 0);
  }
  SECTION("init->connect") {
    REQUIRE(stack->init() == 0);
    REQUIRE(stack->connect(NULL, NULL, NULL) == NULL);
  }
  SECTION("connect") {
    REQUIRE(stack->connect(NULL, NULL, NULL) == NULL);
  }
  delete stack;
}

#if defined(LOCAL_IP) && defined(LOCAL_PORT)
void connect(MsgStack *stack, TestBufMgr *mgr, int buffer_num_per_connection) {
  fid_eq *eq = stack->connect(LOCAL_IP, LOCAL_PORT, mgr);
  MsgConnection *con = stack->get_connection(&eq->fid);
  REQUIRE(con != NULL);
  REQUIRE(con->get_send_buffer().size() == buffer_num_per_connection);
}
#endif

TEST_CASE("msg connect operation") {
  int parallel_num = 10;
  int buffer_num_per_connection = 16;
  int total_buffer_num = parallel_num*buffer_num_per_connection*2;

  MsgStack *stack = new MsgStack(1, buffer_num_per_connection, false);
  REQUIRE(stack->init() == 0);

  HpnlBufMgr *mgr = new HpnlBufMgr();
  for (int i = 0; i < total_buffer_num; i++) {
    Chunk *ck = new Chunk();
    ck->buffer_id = mgr->get_id();
    ck->buffer = std::malloc(65536);
    ck->capacity = 65536;
    mgr->put(ck->buffer_id, ck);
  }

#if defined(LOCAL_IP) && defined(LOCAL_PORT)
  SECTION("multiple connection") {
    std::vector<std::thread> threads;
    for (int i = 0; i < parallel_num; i++) {
      threads.push_back(std::thread(connect, stack, mgr, buffer_num_per_connection));
      // Don't support parallel connection
      threads[i].join();
    }
  }
#endif

  delete stack;
  REQUIRE(mgr->free_size() == total_buffer_num);

  for (int i = 0; i < 64; i++) {
    Chunk *ck = mgr->get(i);
    free(ck->buffer);
  }
}

void reg_rma_buffer(MsgStack *stack, char* test, int i) {
  stack->reg_rma_buffer(test, i*10, i);
}

TEST_CASE("rma buffer registration") {
  char test[10] = "12345";
  int parallel_num = 100;
  MsgStack *stack = new MsgStack(1, 6, false);
  stack->init();

  SECTION("async rma buffer registration") {
    std::vector<std::thread> threads;
    for (int i = 0; i < parallel_num; i++) {
      threads.push_back(std::thread(reg_rma_buffer, stack, test, i));
    }

    for (int i = 0; i < parallel_num; i++) {
      threads[i].join();
    }

    REQUIRE(stack->get_rma_chunk(5)->capacity == 50);
    REQUIRE(stack->get_rma_chunk(2)->capacity == 20);

    for (int i = 0; i < parallel_num; i++) {
      stack->unreg_rma_buffer(i);
    }

    REQUIRE(stack->get_rma_chunk(5) == NULL);
    REQUIRE(stack->get_rma_chunk(2) == NULL);
  }

  SECTION("sycn rma buffer registration") {
    REQUIRE(stack->reg_rma_buffer(NULL, 1, 2) == -1);
    REQUIRE(stack->reg_rma_buffer(test, 10, 1) > 0);
    REQUIRE(stack->reg_rma_buffer(test, 20, 2) > 0);
    REQUIRE(stack->reg_rma_buffer(test, 30, 3) > 0);
    REQUIRE(stack->reg_rma_buffer(test, 40, 4) > 0);
    REQUIRE(stack->get_rma_chunk(1)->capacity == 10);
    REQUIRE(stack->get_rma_chunk(2)->capacity == 20);
    REQUIRE(stack->get_rma_chunk(3)->capacity == 30);
    REQUIRE(stack->get_rma_chunk(4)->capacity == 40);
    stack->unreg_rma_buffer(1);
    stack->unreg_rma_buffer(2);
    stack->unreg_rma_buffer(3);
    stack->unreg_rma_buffer(4);
    REQUIRE(stack->get_rma_chunk(1) == NULL);
    REQUIRE(stack->get_rma_chunk(2) == NULL);
    REQUIRE(stack->get_rma_chunk(3) == NULL);
    REQUIRE(stack->get_rma_chunk(4) == NULL);
  }

  delete stack;
}

TEST_CASE("rdm server") {
  RdmStack *stack = new RdmStack(16, true);
  SECTION("init->init") {
    REQUIRE(stack->init() == 0);
    REQUIRE(stack->bind(NULL, NULL, NULL) == NULL);
  }
  SECTION("bind") {
    REQUIRE(stack->bind(NULL, NULL, NULL) == NULL);
  }
  delete stack;
}

TEST_CASE("rdm client") {
  RdmStack *stack = new RdmStack(16, false);
  SECTION("init->get_con") {
    REQUIRE(stack->init() == 0);
    REQUIRE(stack->get_con(NULL, NULL, NULL) == NULL);
  }
  SECTION("get_con") {
    REQUIRE(stack->get_con(NULL, NULL, NULL) == NULL);
  }
  delete stack;
}


