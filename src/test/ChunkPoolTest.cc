#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"
#include "HPNL/ChunkMgr.h"

TEST_CASE("chunk pool") {
  int request_chunk_number = 32;
  std::vector<Chunk*> vec;
  auto cp = new ChunkPool(4096, request_chunk_number, 1024);
  for (int i = 0; i < request_chunk_number*2; i++) {
    auto ck = reinterpret_cast<Chunk*>(cp->malloc());
    REQUIRE(ck != nullptr);
    REQUIRE(ck->buffer_id == i);
    REQUIRE(cp->get(ck->buffer_id) == ck);
    vec.push_back(ck);
  }
  delete cp;
}
