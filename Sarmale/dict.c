#include "dict.h"

#include "../Libs/log.h"

uint32_t entrl;
struct wikte *__restrict entries;
static FILE *__restrict log_file;

char *__restrict strings;
uint32_t cstp = 0;

#define rbl 65536
char rbuf[rbl];
uint32_t crb = 0;
int32_t d;

uint64_t wh(strp s, uint32_t l) {
  uint64_t hash = 0;

  int32_t i;
  for(i = 0; i < l; ++i) {
    hash = *g(s + i) + (hash << 6) + (hash << 16) - hash;
    //hash = ((hash << 5) + hash) + *g(s + i);
  }

  return hash;
}

void gs(char *b, uint32_t l) {
  if (crb == 0) {
    //fprintf(stdout, "Buffer read ran out! Reading again!\n");
    if (read(d, rbuf, rbl) < 0) {
      logg(10, log_file, "Error reading from rdict! %m");
      exit(1);
    }
  }

  if (rbl - crb < l) {
    memcpy(b, rbuf + crb, rbl - crb);
    int32_t cd = rbl - crb;
    crb = 0;
    //fprintf(stdout, "Reading %u and buffering again!\n", cd);
    gs(b + cd, l - cd);
  } else {
    //fprintf(stdout, "Reading %u!\n", l);
    memcpy(b, rbuf + crb, l);
    crb += l;
    if (crb >= rbl) {
      crb = 0;
    }
  }
}

char a[4];
uint16_t glen() {
  gs(a, 2);
  return ((uint8_t)a[0] << 8) |
         ((uint8_t)a[1] << 0);
}

strp reads(uint16_t l) {
  strp c = cstp;
  cstp += l + 1;
  gs(strings + c, l);
  strings[c + l + 1] = '\0';
  return c;
}

void print_entry(uint32_t i) {
  int32_t j, k;
  fprintf(stdout, "word: %s(%lu); pos: %s\n", g(entries[i].word), entries[i].whash, g(entries[i].pos));
  for(j = 0; j < entries[i].soundl; ++j) {
    fprintf(stdout, "\tsound: %s\n", g(entries[i].sounds[j]));
  }
  for(j = 0; j < entries[i].sensel; ++j) {
    for(k = 0; k < entries[i].senses[j].glossel; ++k) {
      fprintf(stdout, "\tgloss: %s\n", g(entries[i].senses[j].glosses[k]));
    }
    for(k = 0; k < entries[i].senses[j].linkl; ++k) {
      fprintf(stdout, "\tlink: %s\n", g(entries[i].senses[j].links[k]));
    }
  }
}

uint8_t wdif(uint32_t x, uint32_t y) {
  uint32_t l1 = strlen(g(entries[x].word));
  uint32_t l2 = strlen(g(entries[y].word));
  if (l1 != l2) {
    return 1;
  }
  int32_t i;
  for(i = 0; i < l1; ++i) {
    if (g(entries[x].word)[i] != g(entries[y].word)[i]) {
      return 1;
    }
  }
  return 0;
}

void load_dict(FILE *__restrict lfile) {
  log_file = lfile;
  d = open("rdict", O_RDONLY);
  if (d < 0) {
    logg(10, log_file, "Could not open rdict! %m");
    exit(1);
  }

  strings = malloc(STRINGS_SIZE);

  gs(a, 4);
  entrl = ((uint8_t)a[0] << 24) | 
          ((uint8_t)a[1] << 16) | 
          ((uint8_t)a[2] <<  8) | 
          ((uint8_t)a[3] <<  0);

  entries = malloc(sizeof(struct wikte) * entrl);
  int32_t i, j, k;
  //
// template = {'pos': '', 'word': '', 'sounds': [], 'senses': []}
// sounds = {'ipa: ''}
// stempl = {'links': [], 'glosses': []}
// links = ["einem", "einem#German"], ["'n", "'n#German"]
// glosses = ['', '']
//
#define rd(a) entries[i]. a = reads(glen())
#define rrd(a, b, c) entries[i]. a ##l = glen(); \
                  entries[i]. a ##s = malloc(sizeof(struct sense) * entries[i]. a ##l); \
                  for(b = 0; b < entries[i]. a ##l; ++(b)) { c; }
  uint32_t wl;
  for(i = 0; i < entrl; ++i) {
    wl = glen();
    entries[i].word = reads(wl);
    entries[i].whash = wh(entries[i].word, wl);
    rd(pos);

    rrd(sound, j, rd(sounds[j]));
    rrd(sense, j, 
        rrd(senses[j].glosse, k, rd(senses[j].glosses[k]));
        rrd(senses[j].link, k, rd(senses[j].links[k]));
        );
  }

  close(d);
}

void unload_dict() {
  int32_t i, j;
  for(i = 0; i < entrl; ++i) {
    free(entries[i].sounds);
    for(j = 0; j < entries[i].sensel; ++j) {
      free(entries[i].senses[j].glosses);
      free(entries[i].senses[j].links);
    }
    free(entries[i].senses);
  }
  free(entries);
  free(strings);
}
