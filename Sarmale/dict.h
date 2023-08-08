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
  uint64_t whash;
  strp word;
  strp pos;
  uint16_t soundl;
  strp *__restrict sounds;
  uint16_t sensel;
  struct sense *__restrict senses;
};

#define g(x) (strings + (x))
#define STRINGS_SIZE 33969860
extern char *__restrict strings;

extern uint32_t entrl;
extern struct wikte *__restrict entries;

void load_dict(FILE *__restrict lfile);
void unload_dict();

#endif
