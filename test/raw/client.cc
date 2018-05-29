#include "common.h"

int main(int argc, char *argv[]) {

   if (argc != 4) {
    fprintf(stderr, "usage: %s addr type\n", argv[0]);
    return 0;
  }
  
  common_init(argv[1], 0, atoi(argv[3]));

  client_connect();
 
  if (!strcmp(argv[2], "one_side")) {
    client_ping_pong_msg_one_side();     
  } else if (!strcmp(argv[2], "msg")) {
    client_ping_pong_msg(); 
  } else if (!strcmp(argv[2], "rma")) {
    recv_key();
    client_ping_pong_rma();     
  } else {
  
  }

  shutdown();

  join();

  return 0;
}
