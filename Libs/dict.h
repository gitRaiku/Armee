#ifndef DICT_H
#define DICT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

// template = {'pos': '', 'word': '', 'sounds': [], 'senses': []}
// sounds = {'ipa: ''}
// stempl = {'links': [], 'glosses': []}
// links = ["einem", "einem#German"], ["'n", "'n#German"]
// glosses = ['', '']

typedef uint32_t strp;

struct sense {
  uint16_t glossel;
  strp *__restrict glosses;
  uint16_t linkl;
  strp *__restrict links;
};

struct wikte {
  uint64_t hash;
  strp word;
  strp pos;
  uint16_t soundl;
  strp *__restrict sounds;
  uint16_t sensel;
  struct sense *__restrict senses;
};

#define g(x) (strings + (x))
#include "dictsize.h"
// #define STRINGS_SIZE 40054251
#define STRINGS_SMALL_SIZE 8192
extern char *__restrict strings;

extern uint32_t entrl;
extern struct wikte *__restrict entries;

void load_dict(FILE *__restrict lfile);
void load_dicts(FILE *__restrict lfile, char *__restrict s, uint32_t l);
void print_entry(uint32_t i);
void search_dict(char *word, uint32_t l, uint32_t *__restrict p, uint32_t *__restrict pl);
void unload_dict();

#endif
