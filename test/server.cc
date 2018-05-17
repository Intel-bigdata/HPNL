#include "common.h"

int main(int argc, char *argv[]) {

  if (argc != 4) {
    fprintf(stderr, "usage: %s addr type\n", argv[0]);
    return 0;
  }
  
  common_init(argv[1], FI_SOURCE, atoi(argv[3]));

  server_connect();

  if (!strcmp(argv[2], "one_side")) {
    server_ping_pong_msg_one_side();     
  } else if (!strcmp(argv[2], "msg")) {
    server_ping_pong_msg(); 
  } else if (!strcmp(argv[2], "rma")) {
    send_key();
  } else {
  
  }

  join();
  return 0;
}
