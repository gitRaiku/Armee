#include "dict.h"
#include "log.h"

uint32_t entrl;
struct wikte *__restrict entries;
static FILE *__restrict log_file;

char *__restrict strings;
uint32_t cstp = 0;

#define rbl 65536
char rbuf[rbl];
uint32_t crb = 0;
int32_t d;

uint64_t whs(char *s, uint32_t l) {
  uint64_t hash = 0;

  int32_t i;
  for(i = 0; i < l; ++i) {
    hash = s[i] + (hash << 6) + (hash << 16) - hash;
  }

  return hash;
}

uint64_t wh(strp s, uint32_t l) {
  uint64_t hash = 0;

  int32_t i;
  for(i = 0; i < l; ++i) {
    hash = *g(s + i) + (hash << 6) + (hash << 16) - hash;
  }

  return hash;
}

void gs(char *b, uint32_t l) {
  if (crb == 0) {
    if (read(d, rbuf, rbl) < 0) {
      logg(10, log_file, "Error reading from rdict! %m");
      exit(1);
    }
  }

  if (rbl - crb < l) {
    memcpy(b, rbuf + crb, rbl - crb);
    int32_t cd = rbl - crb;
    crb = 0;
    gs(b + cd, l - cd);
  } else {
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
  strings[c + l - (strings[c + l - 1] == '\n')] = '\0';
  return c;
}

void print_entry(uint32_t i) {
  int32_t j, k;
  logg(0, log_file, "Entry %u", i);
  logg(0, log_file, "word: %s(%lu); pos: %s", g(entries[i].word), entries[i].hash, g(entries[i].pos));
  for(j = 0; j < entries[i].soundl; ++j) {
    logg(0, log_file, "\tsound: %s", g(entries[i].sounds[j]));
  }
  for(j = 0; j < entries[i].sensel; ++j) {
    for(k = 0; k < entries[i].senses[j].glossel; ++k) {
      logg(0, log_file, "\tgloss: %s", g(entries[i].senses[j].glosses[k]));
    }
    for(k = 0; k < entries[i].senses[j].linkl; ++k) {
      logg(0, log_file, "\tlink: %s", g(entries[i].senses[j].links[k]));
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

int cmp(const void *o1, const void *o2) { /// `>` means ascending; `<` means descending
  return (*(struct wikte *)o1).hash > (*(struct wikte *)o2).hash;
}

void load_dict(FILE *__restrict lfile) {
  log_file = lfile;
  d = open("/usr/share/dicts/rgdict", O_RDONLY);
  if (d < 0) {
    logg(10, log_file, "Could not open rdict! %m");
    exit(1);
  }

  cstp = 0;
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
    entries[i].hash = wh(entries[i].word, wl);
    rd(pos);

    rrd(sound, j, rd(sounds[j]));
    rrd(sense, j, 
        rrd(senses[j].glosse, k, rd(senses[j].glosses[k]));
        rrd(senses[j].link, k, rd(senses[j].links[k]));
        );
  }
  qsort(entries, entrl, sizeof(entries[0]), cmp);
#undef rd
#undef rrd

  close(d);
}

strp readss(char *s, uint32_t *cs, uint16_t l) {
  strp c = cstp;
  cstp += l + 1;
  memcpy(strings + c, s + *cs, l);
  *cs += l;
  strings[c + l - (strings[c + l - 1] == '\n')] = '\0';
  return c;
}

uint16_t glens(char *s, uint32_t *cs) {
  *cs += 2;
  return ((uint8_t)s[*cs - 2] << 8) | 
         ((uint8_t)s[*cs - 1] << 0);
}

void load_dicts(FILE *__restrict lfile, char *__restrict s, uint32_t l) {
  log_file = lfile;

  cstp = 0;
  strings = calloc(STRINGS_SMALL_SIZE, 1);
  uint32_t cs = 0;

  entrl = ((uint8_t)s[cs + 0] << 8) | 
          ((uint8_t)s[cs + 1] << 0);

  cs += 2;

  entries = malloc(sizeof(struct wikte) * entrl);
  int32_t i, j, k;

#define rd(a) {uint32_t CGLS = glens(s, &cs); entries[i]. a = readss(s, &cs, CGLS);}
#define rrd(a, b, c) entries[i]. a ##l = glens(s, &cs); \
                  entries[i]. a ##s = malloc(sizeof(struct sense) * entries[i]. a ##l); \
                  for(b = 0; b < entries[i]. a ##l; ++(b)) { c; }
  uint32_t wl;
  for(i = 0; i < entrl; ++i) {
    wl = glens(s, &cs);
    entries[i].word = readss(s, &cs, wl);
    entries[i].hash = wh(entries[i].word, wl);
    rd(pos);

    rrd(sound, j, rd(sounds[j]));
    rrd(sense, j, 
        rrd(senses[j].glosse, k, rd(senses[j].glosses[k]));
        rrd(senses[j].link, k, rd(senses[j].links[k]));
        );
  }
#undef rd
#undef rrd
}

uint32_t search_hash(uint64_t hash) {
  int32_t cpos;
  int32_t lbound = 0;
  int32_t ubound = entrl;
  int32_t answer = lbound;
  while (lbound <= ubound) {
    cpos = (ubound + lbound) / 2;
    fprintf(stdout, "Searching hash %lu at %u(%lu)\n", hash, cpos, entries[cpos].hash);
    if (entries[cpos].hash >= hash) {
      answer = cpos;
      ubound = cpos - 1;
    } else {
      lbound = cpos + 1;
    }
  }
  return answer;
}

void search_dict(char *word, uint32_t l, uint32_t *__restrict p, uint32_t *__restrict pl) {
  uint64_t hash = whs(word, l);
  *p = search_hash(hash);
  if (entries[*p].hash != hash) {
    *p = 0;
    *pl = 0;
    return;
  }
  *pl = 1;
  while (((*p + *pl) < entrl) && (entries[*p].hash == entries[*p + *pl].hash)) { ++*pl; }
}

void unload_dict() {
  int32_t i, j;
  if (entries) {
    for(i = 0; i < entrl; ++i) {
      free(entries[i].sounds);
      for(j = 0; j < entries[i].sensel; ++j) {
        free(entries[i].senses[j].glosses);
        free(entries[i].senses[j].links);
      }
      free(entries[i].senses);
    }
    free(entries);
  }
  if (strings) {
    free(strings);
  }
  strings = NULL;
  cstp = 0;
  entries = NULL;
  entrl = 0;
}
