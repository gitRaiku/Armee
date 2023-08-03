#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "../Libs/socket.h"
#include "../Libs/log.h"

FILE *__restrict log_file;
void die_horribly() {
  fclose(log_file);
  exit(1);
}

enum FUNCTIONS { FUNC_NONE, FUNC_WORD, FUNC_DEEPL, FUNC_COUNT };

void search_word(char *word, uint32_t wl, char *resp, uint32_t *rl) {
  *rl = 0;
}

void deepl_word(char *word, uint32_t wl, char *resp, uint32_t *rl) {
  *rl = 0;
}

void interpret_data(char *buf, int32_t len, char *response, uint32_t *rl) {
  uint8_t function;
  uint16_t chlen;
  function = (uint8_t)buf[0];
  chlen = (uint8_t)buf[1] << 8u | (uint8_t)buf[2];
  logg(0, log_file, "Chlen: %u\n", chlen);

  if (chlen == 0) {
    logg(0, log_file, "Got an empty packet, sending one back!");
    make_empty_packet(response, rl);
    return;
  }

  if (chlen + 3 != len) {
    logg(1, log_file, "Length mismatch! Got %i, expected %hu!", len, chlen + 3);
    make_empty_packet(response, rl);
    hexprint(log_file, buf, len);
  }

  uint32_t respl;
  switch (function) {
    case FUNC_WORD:
      logg(0, log_file, "Search word\n");
      search_word(buf + 3, chlen, response + 3, &respl);
      response[0] = FUNC_WORD;
      response[1] = respl & 0xFF00;
      response[2] = respl & 0x00FF;
      logg(0, log_file, "[%s] %u\n", response + 3, respl);
      *rl = 3 + respl;
      break;
    case FUNC_DEEPL:
      logg(0, log_file, "Deepl word\n");
      deepl_word(buf + 3, chlen, response + 3, &respl);
      response[0] = FUNC_DEEPL;
      response[1] = respl & 0xFF00;
      response[2] = respl & 0x00FF;
      logg(0, log_file, "[%s] %u\n", response + 3, respl);
      *rl = 3 + respl;
      break;
    default:
      logg(1, log_file, "Got broken packet! Sending empty packet pack!\n");
      hexprint(log_file, buf, len);
      make_empty_packet(response, rl);
      break;
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

      close_connection(c);
    }
  }

  
  return 0;
}
