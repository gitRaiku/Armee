#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../Libs/socket.h"
#include "../Libs/log.h"
#include "../Libs/dict.h"

FILE *__restrict log_file;
void die_horribly() {
  fclose(log_file);
  exit(1);
}

enum FUNCTIONS { FUNC_NONE, FUNC_WORD, FUNC_DEEPL, FUNC_COUNT };

int pint(char *v, uint32_t i) {
  v[0] = ((i & 0xFF00) >>  8);
  v[1] = ((i & 0x00FF) >>  0);
  return 2;
}

int pstr(char *v, char *s) {
  uint32_t l = strlen(s);
  pint(v, l);
  memcpy(v + 2, s, l);
  return 2 + l;
}

struct rpacket search_word(char *word, uint32_t wl) {
  uint32_t p1, pl1;
  uint32_t p2, pl2;
  struct rpacket p = {0};
  
  p.data = malloc(2048);
  if (isupper(word[0])) {
    search_dict(word, wl, &p1, &pl1);
    word[0] = tolower(word[0]);
    search_dict(word, wl, &p2, &pl2);
  } else {
    search_dict(word, wl, &p1, &pl1);
    word[0] = toupper(word[0]);
    search_dict(word, wl, &p2, &pl2);
  }

  uint16_t cp = 0;

  cp += pint(p.data + cp, pl1 + pl2);
  int32_t i, j, k;
#define PS(x) cp += pstr(p.data + cp, g(entries[p1 + i].x))
#define PI(x) cp += pint(p.data + cp, entries[p1 + i].x);
  for(i = 0; i < pl1; ++i) {
    print_entry(p1 + i);
    PS(word);
    PS(pos);
    PI(soundl);
    for(j = 0; j < entries[p1 + i].soundl; ++j) {
      PS(sounds[j]);
    }
    PI(sensel);
    for(j = 0; j < entries[p1 + i].sensel; ++j) {
      PI(senses[j].glossel);
      for(k = 0; k < entries[p1 + i].senses[j].glossel; ++k) {
        PS(senses[j].glosses[k]);
      }
      PI(senses[j].linkl);
      for(k = 0; k < entries[p1 + i].senses[j].linkl; ++k) {
        PS(senses[j].links[k]);
      }
    }
  }

#undef PS
#define PS(x) cp += pstr(p.data + cp, g(entries[p2 + i].x))
#undef PI
#define PI(x) cp += pint(p.data + cp, entries[p2 + i].x);
  for(i = 0; i < pl2; ++i) {
    print_entry(p2 + i);
    PS(word);
    PS(pos);
    PI(soundl);
    for(j = 0; j < entries[p2 + i].soundl; ++j) {
      PS(sounds[j]);
    }
    PI(sensel);
    for(j = 0; j < entries[p2 + i].sensel; ++j) {
      PI(senses[j].glossel);
      for(k = 0; k < entries[p2 + i].senses[j].glossel; ++k) {
        PS(senses[j].glosses[k]);
      }
      PI(senses[j].linkl);
      for(k = 0; k < entries[p2 + i].senses[j].linkl; ++k) {
        PS(senses[j].links[k]);
      }
    }
  }
#undef PS
#undef PI
  p.data[cp] = '\0';

  p.func = 0x11;
  p.len = cp;
  p.data = realloc(p.data, cp + 1);

  return p;
}

struct rpacket deepl_word(char *word, uint32_t wl) {
  return make_empty_packet();
}

struct rpacket parse_packet(struct rpacket p) {
  logg(0, log_file, "Got new packet! %u %u %s", p.func, p.len, p.data);
  switch (p.func) {
    case FUNC_WORD:
      logg(0, log_file, "Search word %s", p.data);
      return search_word(p.data, p.len);
    case FUNC_DEEPL:
      logg(0, log_file, "Deepl word %s", p.data);
      return deepl_word(p.data, p.len);
    default:
      logg(1, log_file, "Got broken packet! Sending empty packet pack!");
      hexprint(log_file, p.data, p.len);
      return make_empty_packet();
  }
}

uint8_t run = 1;
int main(void) {
  if (1) {
    set_logging_level(10);
    log_file = fopen("/var/log/sarmale.log", "a");
    if (log_file == NULL) {
      logg(10, stderr, "Could not open the log file! [%m]");
      exit(1);
    }
  } else {
    set_logging_level(0);
    log_file = stdout;
  }
  set_logging_string("Sarmale");

  load_dict(log_file);
  /*
  print_entry(113970);
  print_entry(113971);
  print_entry(113972);
  char a[] = "Würfel";
  search_word(a, strlen(a));
  */

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
      struct rpacket resp = parse_packet(p);
      free(p.data);

      send_packet(c, resp);
      free(resp.data);
    }
    close_connection(c);
  }

  unload_dict();
  return 0;
}

/*

int cmpff(const void *o1, const void *o2) { /// > means ascending; < means descending
  return ((*(struct wikte *)o1).hash) > ((*(struct wikte *)o2).hash);
}

char u[40894464];
uint32_t al;
void pint4(uint32_t w, uint32_t i) {
  u[al + 0] = ((i & 0xFF000000) >> 24);
  u[al + 1] = ((i & 0x00FF0000) >> 16);
  u[al + 2] = ((i & 0x0000FF00) >>  8);
  u[al + 3] = ((i & 0x000000FF) >>  0);
  al += 4;
  // write(w, &i, 4);
}

void pint2(uint32_t w, uint16_t i) {
  u[al + 0] = ((i & 0xFF00) >>  8);
  u[al + 1] = ((i & 0x00FF) >>  0);
  al += 2;
  // write(w, &i, 2);
}

void pstr(uint32_t w, char *__restrict c) {
  uint32_t l = strlen(c);
  pint2(w, l);
  memcpy(u + al, c, l);
  al += l;
  // write(w, c, l);
}

    load_dict(log_file, 1);

    qsort(entries, entrl, sizeof(entries[0]), cmpff);
    uint32_t w = open("new_rdict", O_CREAT | O_WRONLY, S_IRWXU);

    pint4(w, entrl);
    {
      int32_t i, j, k;
      for(i = 0; i < entrl; ++i) {
        pstr(w, g(entries[i].word));
        pstr(w, g(entries[i].pos));
        pint2(w, entries[i].soundl);
        for(j = 0; j < entries[i].soundl; ++j) {
          pstr(w, g(entries[i].sounds[j]));
        }
        pint2(w, entries[i].sensel);
        for(j = 0; j < entries[i].sensel; ++j) {
          pint2(w, entries[i].senses[j].glossel);
          for(k = 0; k < entries[i].senses[j].glossel; ++k) {
            pstr(w, g(entries[i].senses[j].glosses[k]));
          }
          pint2(w, entries[i].senses[j].linkl);
          for(k = 0; k < entries[i].senses[j].linkl; ++k) {
            pstr(w, g(entries[i].senses[j].links[k]));
          }
        }
      }
    }

    if (write(w, u, al));
    close(w);

    unload_dict();
    return 0;
*/
