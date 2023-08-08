#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "dict.h"

#include "../Libs/socket.h"
#include "../Libs/log.h"

FILE *__restrict log_file;
void die_horribly() {
  fclose(log_file);
  exit(1);
}

enum FUNCTIONS { FUNC_NONE, FUNC_WORD, FUNC_DEEPL, FUNC_COUNT };

struct rpacket search_word(char *word, uint32_t wl) {
  return make_empty_packet();
}

struct rpacket deepl_word(char *word, uint32_t wl) {
  return make_empty_packet();
}

struct rpacket parse_packet(struct rpacket p) {
  fprintf(stdout, "Got new packet! %u %u %s\n", p.func, p.len, p.data);
  switch (p.func) {
    case FUNC_WORD:
      logg(0, log_file, "Search word\n");
      return search_word(p.data, p.len);
    case FUNC_DEEPL:
      logg(0, log_file, "Deepl word\n");
      return deepl_word(p.data, p.len);
    default:
      logg(1, log_file, "Got broken packet! Sending empty packet pack!\n");
      hexprint(log_file, p.data, p.len);
      return make_empty_packet();
  }
}

uint8_t run = 1;
int main(void) {
  //log_file = fopen("/var/log/sarmale.log", "a");
  log_file = stdout;
  set_logging_level(0);

  load_dict(log_file);

  struct rsocket s = setup_server(log_file, "sarmale");

  struct rsocket c;
  struct rpacket p;
  while (run) {
    c = accept_connection(log_file, s);
    if (c.sock == -1) {
      continue;
    }
    
    while (1) {
      p = get_packet(log_file, c);

      if (p.func == RFUNC_NONE || p.func == RFUNC_ERR) {
        break;
      }
      send_packet(c, parse_packet(p));

      close_connection(c);
    }
  }

  unload_dict();
  return 0;
}
