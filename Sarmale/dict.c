#include "dict.h"

#include "../Libs/log.h"

uint32_t entrl;
struct wikte *__restrict entries;
static FILE *__restrict log_file;

char a[4];
uint16_t glen(int32_t d) {
  if (read(d, a, 2) < 0) {
      logg(10, log_file, "Error reading from rdict! %m");
      exit(1);
  }
  fprintf(stdout, "Got len: %u\n", ((uint16_t)a[0] << 8) |
         ((uint16_t)a[1] << 0));
  return ((uint16_t)a[0] << 8) |
         ((uint16_t)a[1] << 0);
}

char *__restrict reads(int32_t d, uint32_t l) {
  char *__restrict c = malloc(l + 1);
  if (read(d, c, l) < 0) {
      logg(10, log_file, "Error reading from rdict! %m");
      exit(1);
  }
  c[l + 1] = '\0';
  fprintf(stdout, "Read: %u[%s]\n", l, c);
  return c;
}

void load_dict(FILE *__restrict lfile) {
  log_file = lfile;
  int32_t d = open("rdict", O_RDONLY);
  if (d < 0) {
    logg(10, log_file, "Could not open rdict! %m");
    exit(1);
  }

  {
    //char a[4];
    if (read(d, a, 4) < 0) {
      logg(10, log_file, "Could not read from rdict! %m");
      exit(1);
    }

    entrl = ((uint32_t)a[0] << 24) | 
            ((uint32_t)a[1] << 16) | 
            ((uint32_t)a[2] <<  8) | 
            ((uint32_t)a[3] <<  0);
  }

  entries = malloc(sizeof(struct wikte) * entrl);
  int32_t i, j, k;
// template = {'pos': '', 'word': '', 'sounds': [], 'senses': []}
// sounds = {'ipa: ''}
// stempl = {'links': [], 'glosses': []}
// links = ["einem", "einem#German"], ["'n", "'n#German"]
// glosses = ['', '']
#define rd(a) entries[i]. a = reads(d, glen(d))
#define rrd(a, b, c) entries[i]. a ##l = glen(d); \
                  entries[i]. a ##s = malloc(sizeof(char *__restrict) * entries[i]. a ##l); \
                  for(b = 0; b < entries[i]. a ##l; ++(b)) { c; }
                  
// for(i = 0; i < entrl; ++i) {
  for(i = 0; i < 12; ++i) {
    entries[i].word = reads(d, glen(d));
    entries[i].pos = reads(d, glen(d));

    entries[i].soundl = glen(d); 
    entries[i].sounds = malloc(sizeof(char *__restrict) * entries[i]. soundl); 
    for(j = 0; j < entries[i]. soundl; ++(j)) { 
      entries[i].sounds[j] = reads(d, glen(d));
    };
    entries[i].sensel = glen(d); 
    entries[i].senses = malloc(sizeof(char *__restrict) * entries[i].sensel); 
    for(j = 0; j < entries[i]. sensel; ++(j)) { 
      entries[i].senses[j].glossel = glen(d);
      entries[i].senses[j].glosses = malloc(sizeof(char *__restrict) * entries[i]. senses[j].glossel);
      for(k = 0; k < entries[i]. senses[j].glossel; ++(k)) {
        entries[i].senses[j].glosses[k] = reads(d, glen(d));
      };
      fprintf(stdout, "ss: %p\n", entries[i].senses[0].glosses[0]);
      /// TODO: i == 11; j = 1 -> WHAT THE FUCK
      entries[i].senses[j].linkl = glen(d);
      entries[i].senses[j].links = malloc(sizeof(char *__restrict) * entries[i]. senses[j].linkl);
      for(k = 0; k < entries[i]. senses[j].linkl; ++(k)) {
        entries[i].senses[j].links[k] = reads(d, glen(d));
      }
    }








    /*
    rd(word);
    rd(pos);

    rrd(sound, j, rd(sounds[j]));
    rrd(sense, j, 
        rrd(senses[j].glosse, k, rd(senses[j].glosses[k]));
        fprintf(stdout, "ss: %p\n", entries[i].senses[0].glosses[0]);
        rrd(senses[j].link, k, rd(senses[j].links[k]));
        );


    fprintf(stdout, "word: %s; pos: %s\n", entries[i].word, entries[i].pos);
    for(j = 0; j < entries[i].soundl; ++j) {
      fprintf(stdout, "\tsound: %s\n", entries[i].sounds[j]);
    }
    for(j = 0; j < entries[i].sensel; ++j) {
      for(k = 0; k < entries[i].senses[j].glossel; ++k) {
        fprintf(stdout, "\tgloss: %s\n", entries[i].senses[j].glosses[k]);
      }
      for(k = 0; k < entries[i].senses[j].linkl; ++k) {
        fprintf(stdout, "\tlink: %s\n", entries[i].senses[j].links[k]);
      }
    }*/
  }


  /*
  for(i = 0; i < entrl; ++i) {

    fgetc(stdin);
  }*/

  close(d);
}
