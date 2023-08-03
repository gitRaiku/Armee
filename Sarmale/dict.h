#ifndef DICT_H
#define DICT_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

// template = {'pos': '', 'word': '', 'sounds': [], 'senses': []}
// sounds = {'ipa: ''}
// stempl = {'links': [], 'glosses': []}
// links = ["einem", "einem#German"], ["'n", "'n#German"]
// glosses = ['', '']

struct sense {
  uint16_t glossel;
  char *__restrict *__restrict glosses;
  uint16_t linkl;
  char *__restrict *__restrict links;
};

struct wikte {
  char *__restrict word;
  char *__restrict pos;
  uint16_t soundl;
  char *__restrict *__restrict sounds;
  uint16_t sensel;
  struct sense *__restrict senses;
};

extern uint32_t entrl;
extern struct wikte *__restrict entries;

void load_dict();


#endif
