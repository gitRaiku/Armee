#include <curses.h>
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <panel.h>
#include <signal.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "../Libs/log.h"
#include "../Libs/socket.h" 
#include "../Libs/dict.h" // Supplies entrl, entries and strings

FILE *__restrict log_file;

void help() {
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  ./armee \"<text>\" [path/to/audio]\n");
  fprintf(stderr, "\n");
  exit(1);
}

uint8_t RUNNING = 2;
struct rsocket s;
int32_t client_sock;
int32_t wx, wy;
char *text;
uint32_t textl;
uint32_t textdl;
char *audioPath;
uint32_t apathl;
uint32_t chpl, nlines;
char *selection;
uint32_t selt;
uint32_t selStart, selEnd;
struct wp {
  WINDOW *w;
  PANEL *p;
};
struct wp ma, se, re;

struct pointer {
  int32_t cy;   // Y Box
  int32_t cx;
  int32_t ccy;  // Sense
  int32_t cccy; // Gloss
  uint32_t pos;
  int32_t len;
  uint8_t sel;
};
struct pointer cp;

void del_win(WINDOW *w) {
  wborder(w, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(w);
  delwin(w);
}

uint32_t runel(char *__restrict str) {
  char *__restrict os = str;
  for (++str; (*str & 0xc0) == 0x80; ++str);
  return (uint32_t) (str - os);
}

uint32_t utf8_to_unicode(char *__restrict str, uint32_t l) {
  uint32_t res = 0;
  switch (l) {
    case 4:
      res |= *str & 0x7;
      break;
    case 3:
      res |= *str & 0xF;
      break;
    case 2:
      res |= *str & 0x1F;
      break;
    case 1:
      res |= *str & 0x7F;
      break;
  }

  --l;
  while (l) {
    ++str;
    res <<= 6;
    res |= *str & 0x3F;
    --l;
  }

  return res;
}

uint32_t strdlen(char *str) { /// TODO: Fix
  uint32_t len = 0;
  while (*str) { 
    str += runel(str);
    ++len;
  }
  return len;
}

uint32_t go_until(char *str, uint32_t strl, int pos) {
  uint32_t cmpos = 0;
  uint32_t chml = 0;
  uint32_t cdpos = 0;
  while (cdpos < pos && cmpos < strl) {
    chml = runel(str + cmpos);
    cmpos += chml;
    ++cdpos;
  }
  if (cmpos <= strl) {
    return cmpos;
  } else {
    return cmpos - chml;
  }
}

void draw_text() {
  int32_t i;
  uint32_t cmpos = 0;
  uint32_t lmpos = 0;

  for(i = 0; i < nlines; ++i) {
    cmpos += go_until(text + cmpos, textl - cmpos, chpl);
    mvwaddnstr(ma.w, i + 1, 1, text + lmpos, cmpos - lmpos);
    lmpos = cmpos;
  }
}

void clear_highlight() {
  int32_t i;
  for(i = 0; i < nlines; ++i) {
    mvwchgat(ma.w, i + 1, 1, chpl, 0, 0, NULL);
  }
}

void highlight_text() {
  clear_highlight();
  int32_t pointer = cp.pos;
  int32_t end     = cp.pos + cp.len;
  if (end < pointer) {
    int32_t t = end;
    end = pointer;
    pointer = t;
  }
  int32_t py = pointer / chpl;
  int32_t px = pointer - (py * chpl);
  int32_t ey = end / chpl;
  int32_t ex = end - (ey * chpl);
  if (cp.sel) {
    if (py == ey) {
      mvwchgat(ma.w, py + 1, px + 1, ex - px + 1, A_UNDERLINE, 0, NULL);
    } else  {
      mvwchgat(ma.w, py + 1, px + 1, chpl - px, A_UNDERLINE, 0, NULL);
      int32_t i;
      for(i = py + 1; i < ey; ++i) {
        mvwchgat(ma.w, i + 1, 1, chpl, A_UNDERLINE, 0, NULL);
      }
      mvwchgat(ma.w, ey + 1, 1, ex, A_UNDERLINE, 0, NULL);
    }
  }
  mvwchgat(ma.w, py + 1, px + 1, 1, (cp.sel ? A_UNDERLINE : 0) | (cp.cy == 0 ? A_REVERSE : A_BOLD), 0, NULL);
  mvwchgat(ma.w, ey + 1, ex + 1, 1, (cp.sel ? A_UNDERLINE : 0) | (cp.cy == 0 ? A_REVERSE : A_BOLD), 0, NULL);
  /*
  char a[1024];
  snprintf(a, sizeof(a), "chpl: %u; pos: %u; len: %u; sel: %u; po: %u;\n en: %u; py: %u; px: %u; ey: %u; ex: %u; cx: %u; cy: %u; ccy: %u; cccy: %u; st: %u; ed: %u       ", chpl, cp.pos, cp.len, cp.sel, pointer, end, py, px, ey, ex, cp.cx, cp.cy, cp.ccy, cp.cccy, selStart, selEnd);
  mvwaddstr(se.w, 25, 2, a);
  */
}

void setup_windows() {
  erase();
  if (ma.w) {
    del_panel(ma.p);
    del_win(ma.w);
  }
  if (se.w) {
    del_panel(se.p);
    del_win(se.w);
  }
  if (re.w) {
    del_panel(re.p);
    del_win(re.w);
  }
  getmaxyx(curscr, wy, wx);
  chpl = wx - 4;
  {
    int32_t i;
    textdl = 0;
    for(i = 0; i < textl; ) {
      i += runel(text + i);
      ++textdl;
    }
  }

  nlines = ((textdl - 1) / chpl) + 1;

  ma.w = newwin(nlines + 2, wx - 2, 0, 1);
  ma.p = new_panel(ma.w);
  box(ma.w, 0, 0);
  se.w = newwin(wy - nlines - 8, wx - 2, nlines + 2, 1);
  se.p = new_panel(se.w);
  box(se.w, 0, 0);
  re.w = newwin(6, wx - 2, wy - 6, 1);
  re.p = new_panel(re.w);
  box(re.w, 0, 0);
  draw_text();

  highlight_text();
  update_panels();
  doupdate();
}

int32_t bound(int32_t v, int32_t o1, int32_t o2) {
  return v < o1 ? o1 : (v > o2 ? o2 : v);
}

void get_selection() {
  uint32_t st = go_until(text, textl, cp.pos);
  uint32_t ed = go_until(text, textl, cp.pos + cp.len);
  if (ed < st) {
    uint32_t t = st;
    st = ed;
    ed = t;
  }
  selt = ed - st + 1;
  selStart = st;
  selEnd = ed;
  selection = strndup(text + st, selt);
}

struct entwin {
  WINDOW *w;
  PANEL *p;
  uint32_t h;
};

struct entwin *__restrict ents;
uint32_t entsl;

void show_entries() {
  werase(se.w);
  box(se.w, 0, 0);
  int32_t i, j, k;
  if (ents) {
    for(i = 0; i < entsl; ++i) {
      del_panel(ents[i].p);
      delwin(ents[i].w);
    }
    free(ents);
    ents = NULL;
  }

  if (entrl) {
    ents = malloc(sizeof(ents[0]) * entrl);
    entsl = entrl;
    {
      uint32_t ceh = 0;
      uint32_t cx, cy;
#define ce entries[i]
#define cce ents[i]
      for(i = 0; i < entrl; ++i) {
        //print_entry(i);
        cce.h = 3; // TEXT + BORDER
        if (ce.soundl) { ++cce.h; } // READING LENGTH
        for(j = 0; j < ce.sensel; ++j) { cce.h += ce.senses[j].glossel + 2; } // EVERY SENSE

        cce.w = newwin(cce.h, wx - 4, ceh + nlines + 2 + 1, 2);
        ceh += cce.h;
        cce.p = new_panel(cce.w);
        box(cce.w, 0, 0);

        mvwaddstr(cce.w, 1, 1, g(ce.word));
        mvwaddstr(cce.w, 1, strdlen(g(ce.word)) + 2, g(ce.pos));
        
        cx = 3;
        for(j = 0; j < ce.soundl; ++j) {
          mvwaddstr(cce.w, 2, cx, g(ce.sounds[j]));
          cx += strdlen(g(ce.sounds[j])) + 1;
        }

        cy = 2;
        if (ce.soundl) { ++cy; } // READING LENGTH
        char st[10];
        for(j = 0; j < ce.sensel; ++j) {
          snprintf(st, sizeof(st), "%u.", j + 1);
          mvwaddstr(cce.w, cy + 1, 1, st);
          for(k = 0; k < ce.senses[j].glossel; ++k) {
            mvwaddstr(cce.w, cy + k + 2, 1, g(ce.senses[j].glosses[k]));
            if (k == 0 && k < ce.senses[j].linkl) {
              uint32_t cd = strdlen(g(ce.senses[j].links[k]));
              mvwaddstr(cce.w, cy + k + 2, wx - 5 - cd, g(ce.senses[j].links[k]));
              mvwchgat(cce.w, cy + k + 2, wx - 5 - cd, cd, A_UNDERLINE, 0, NULL);
            }
          }

          cy += ce.senses[j].glossel + 2;
        }

/*
      wx
 |-----------------|
 ┌─────────────────┐  
 │der Alte würf Alt│ -
 │lt nicht Der nich│ | nlines
 │t Der Alte wüer A│ |
 │nicht            │ -
 └─────────────────┘  
 ┌─────────────────┐
 │┌───────────────┐│-
 ││DER pron       ││|              
 ││  /de(ː)r/ [d  ││| cce.h
 ││               ││|         
 ││  that  |  that││|             
 ││  him   | h / e││|                        
 ││  him   |      ││|             
 │└───────────────┘│-

*/




      }
    }
#undef ce
  }
  //exit(0);
}

void get_results() {
  unload_dict();
  struct rpacket p = get_packet(log_file, s);
  load_dicts(stdout, p.data, p.len);
  show_entries();
}

uint32_t glossc(uint32_t p) { /// I know this is inneficient but i do not care about this enough to rewrite it
  int32_t i;
  uint32_t res = 0;
  for(i = 0; i < entries[p].sensel; ++i) {
    res += entries[p].senses[i].glossel;
  }
  return res;
}

#define CE entries[cp.cy - 1]
strp ggloss(uint32_t k, int32_t p) {
  return CE.senses[k].glosses[p];
}

strp glink(int32_t p) {
  return CE.senses[p].links[0];
}

uint32_t glossh(uint32_t k, uint32_t p) {
  uint32_t ch = 2;
  if (CE.soundl) { ++ch; }

  int32_t i;
  for(i = 0; i < k; ++i) {
    ch += 2;
    ch += CE.senses[i].glossel;
  }
  ch += 2;
  ch += p;
  return ch;
}

void clear_dentry(uint32_t k, uint32_t p) {
  if (cp.cy) {
    if (CE.senses[0].glossel == 0) { // Why the fuck is the dictionary malformed for fucking Bär :(
      return;
    }
    mvwchgat(ents[cp.cy - 1].w, glossh(k, p), 1, strdlen(g(ggloss(k, p))), 0, 0, NULL);
  }
}

void clear_dlentry(uint32_t k) {
  if (cp.cy) {
    int32_t w = strdlen(g(glink(cp.ccy)));
    mvwchgat(ents[cp.cy - 1].w, glossh(k, 0), wx - 5 - w, w, A_UNDERLINE, 0, NULL);
    //mvwchgat(ents[cp.cy - 1].w, glossh(k, 0), 1, strdlen(g(glink(k))), A_UNDERLINE, 0, NULL);
  }
}

void highlight_selection() {
  if (cp.cx) {
    int32_t w = strdlen(g(glink(cp.ccy)));
    mvwchgat(ents[cp.cy - 1].w, glossh(cp.ccy, 0), wx - 5 - w, w, A_REVERSE | A_UNDERLINE, 0, NULL);
  } else if (cp.cy) {
    if (CE.senses[0].glossel == 0) { // Why the fuck is the dictionary malformed for fucking Bär :(
      return;
    }
    clear_dentry(cp.ccy, cp.cccy);
    mvwchgat(ents[cp.cy - 1].w, glossh(cp.ccy, cp.cccy), 1, strdlen(g(ggloss(cp.ccy, cp.cccy))), A_BOLD, 0, NULL);
  }
}

struct outp {
  uint64_t hash;
  uint32_t st, ed;
  char *word;
  char *strs[8];
  uint32_t strl;
};
struct outp outs[8];
uint32_t outsl = 0;

void update_outp() {
  int32_t i, j;
  for(i = 0; i < outsl; ++i) {
    mvwaddstr(re.w, i + 1, 1, outs[i].word);
    waddstr(re.w, ": ");
    for(j = 0; j < outs[i].strl; ++j) {
      waddstr(re.w, outs[i].strs[j]);
      if (j != outs[i].strl - 1) {
        waddstr(re.w, "; ");
      }
    }
  }
}

void add_sel_output() {
  char *s = g(ggloss(cp.ccy, cp.cccy));
  int32_t i;
  if (*s == '(') { while (*s != ')') { ++s; } s += 2; }
  uint32_t sl = strlen(s) - 1;
  uint8_t sssss = 0;
  if (*s) { 
    if (s[sl] == ']') { 
      sssss = 1; 
      while (s[sl] != '[') { 
        --sl; 
      } 
      --sl;
      s[sl] = '\0'; 
    } 
  }
  for(i = 0; i < outsl; ++i) {
    if (outs[i].hash == CE.hash) {
      if (outs[i].strl == 8) {
        return;
      }
      outs[i].strs[outs[i].strl] = strdup(s);
      *outs[i].strs[outs[i].strl] = toupper(*outs[i].strs[outs[i].strl]);
      ++outs[i].strl;
      update_outp();
      if (sssss) { s[sl] = ' '; }
      return;
    }
  }
  if (outsl == 8) {
    return;
  }
  outs[outsl].word = strdup(g(CE.word));
  outs[outsl].hash = CE.hash;
  outs[outsl].st = selStart;
  outs[outsl].ed = selEnd;
  outs[outsl].strs[0] = strdup(s);
  *outs[outsl].strs[0] = toupper(*outs[outsl].strs[0]);
  outs[outsl].strl = 1;
  ++outsl;
  if (sssss) { s[sl] = ' '; }
  update_outp();
}

void query_dict(char *st, uint32_t l) {
  struct rpacket p = {0};
  p.func = 0x01;
  p.len = l;
  p.data = malloc(l);
  memcpy(p.data, st, l);
  send_packet(s, p);
  get_results();
  free(p.data);
}

void handle_input(char ch) {
  switch (ch) {
    case 'q':
    case 'O':
      RUNNING = 0;
      return;
    case 'I':
      RUNNING = 1;
      return;
    case 'J':
      if (cp.cx) {
        clear_dlentry(cp.ccy);
      } else {
        clear_dentry(cp.ccy, cp.cccy);
      }
      ++cp.cy;
      cp.cy = bound(cp.cy, 0, entsl);
      cp.ccy = 0;
      cp.cccy = 0;
      highlight_selection();
      return;
    case 'K':
      if (cp.cx) {
        clear_dlentry(cp.ccy);
      } else {
        clear_dentry(cp.ccy, cp.cccy);
      }
      --cp.cy;
      cp.cy = bound(cp.cy, 0, entsl);
      cp.ccy = 0;
      cp.cccy = 0;
      if (cp.cy) {
        highlight_selection();
      }
      return;
  }

  if (cp.cy == 0) {
    cp.cx = 0;
    cp.ccy = 0;
    if (cp.sel) {
      switch (ch) {
        case 'w':
          uint32_t cmemp = go_until(text, textl, cp.len + cp.pos);
          while ((cp.len <= textdl - 1 - cp.pos) && text[cmemp] != ' ') { 
            ++cp.len; 
            cmemp = go_until(text, textl, cp.len + cp.pos);
          }
          --cp.len;
          return;
        case '0':
          cp.len = -cp.pos;
          return;
        case '$':
        case 'A':
          cp.len = textdl - 1 - cp.pos;
          return;
        case 0x1b:
        case 'v':
          cp.sel = 0;
          cp.pos += cp.len;
          cp.len = 0;
          return;
        case 'h':
          --cp.len;
          cp.len = bound(cp.len, -cp.pos, textdl - 1 - cp.pos);
          return;
        case 'j':
          cp.len += chpl;
          cp.len = bound(cp.len, -cp.pos, textdl - 1 - cp.pos);
          return;
        case 'k':
          cp.len -= chpl;
          cp.len = bound(cp.len, -cp.pos, textdl - 1 - cp.pos);
          return;
        case 'l':
          ++cp.len;
          cp.len = bound(cp.len, -cp.pos, textdl - 1 - cp.pos);
          return;
        case 'f':
          get_selection();
          selection[selt] = '\0';
          werase(se.w);
          box(se.w, 0, 0);
          mvwaddnstr(se.w, 15, 1, selection, selt);
          endwin();
          char intr[64];
          fprintf(stdout, "Got %s:\n", selection);
          if (fscanf(stdin, "%s", intr));
          query_dict(intr, strlen(intr));
          cp.sel = 0;
          cp.pos += cp.len;
          cp.len = 0;
          return;
        case ' ':
        case '\n':
          get_selection();
          selection[selt] = '\0';
          werase(se.w);
          box(se.w, 0, 0);
          mvwaddnstr(se.w, 15, 1, selection, selt);
          query_dict(selection, selt);
          cp.sel = 0;
          cp.pos += cp.len;
          cp.len = 0;
          return;
      }
    } else {
      switch (ch) {
        case '0':
          cp.pos = 0;
          return;
        case '$':
        case 'A':
          cp.pos = textdl - 1;
          return;
        case 'v':
          cp.sel = 1;
          cp.len = 0;
          return;
        case 'h':
          --cp.pos;
          cp.pos = bound(cp.pos, 0, textdl - 1);
          return;
        case 'j':
          cp.pos += chpl;
          cp.pos = bound(cp.pos, 0, textdl - 1);
          return;
        case 'k':
          cp.pos -= chpl;
          cp.pos = bound(cp.pos, 0, textdl - 1);
          return;
        case 'l':
          ++cp.pos;
          cp.pos = bound(cp.pos, 0, textdl - 1);
          return;
        case ' ':
        case '\n':
          return;
      }
    }
  } else {
    switch (ch) {
      case 'j':
        if (cp.cx) {
          clear_dlentry(cp.ccy);
          if (cp.ccy < CE.sensel - 1) {
            ++cp.ccy;
          }
        } else {
          clear_dentry(cp.ccy, cp.cccy);
          ++cp.cccy;
          if (cp.cccy == CE.senses[cp.ccy].glossel) {
            if (cp.ccy == CE.sensel - 1) {
              --cp.cccy;
            } else {
              cp.cccy = 0;
              ++cp.ccy;
            }
          }
        }
        highlight_selection();
        return;
      case 'k':
        if (cp.cx) {
          clear_dlentry(cp.ccy);
          if (cp.ccy) {
            --cp.ccy;
          }
        } else {
          clear_dentry(cp.ccy, cp.cccy);
          --cp.cccy;
          if (cp.cccy == -1) {
            if (cp.ccy == 0) {
              cp.cccy = 0;
            } else {
              --cp.ccy;
              cp.cccy = CE.senses[cp.ccy].glossel - 1;
            }
          }
        }
        highlight_selection();
        return;
      case 'h':
        clear_dentry(cp.ccy, cp.cccy);
        clear_dlentry(cp.ccy);
        --cp.cx;
        cp.cx = bound(cp.cx, 0, 1);
        highlight_selection();
        return;
      case 'l':
        clear_dentry(cp.ccy, cp.cccy);
        clear_dlentry(cp.ccy);
        ++cp.cx;
        cp.cx = bound(cp.cx, 0, 1);
        highlight_selection();
        return;
      case ' ':
        if (cp.cx == 0) {
          add_sel_output();
        } else {
          query_dict(g(glink(cp.ccy)), strlen(g(glink(cp.ccy))));
          cp.cy = entrl ? 1 : 0;
          cp.ccy = 0;
          cp.cccy = 0;
          cp.cx = 0;
          highlight_selection();
        }
        return;
      case '\n':
        clear_dentry(cp.ccy, cp.cccy);
        clear_dlentry(cp.ccy);
        cp.cy = cp.ccy = cp.cccy = cp.cx = 0;
    }
  }
}

void rscr() {
  endwin();
  refresh();
  clear();
  setup_windows();
  if (ents) {
    show_entries();
  }
  highlight_selection();
  update_outp();
  update_panels();
  doupdate();
}

int cmpf(const void *o1, const void *o2) { /// `>` means ascending; `<` means descending
  struct outp p1 = (*(struct outp *)o1);
  struct outp p2 = (*(struct outp *)o2);
  if (p1.st == p2.st) {
    return p1.ed > p2.ed;
  }
  return p1.st > p2.st;
}

void bold_text(char *res) {
  int32_t i;
  qsort(outs, outsl, sizeof(outs[0]), cmpf);

  uint32_t resl = 0;

  uint32_t led = 0;
  uint32_t len;
  for(i = 0; i < outsl; ++i) {
    while (i < outsl - 1 && outs[i].st == outs[i + 1].st) { ++i; }
    len = outs[i].st - led;
    memcpy(res + resl, text + led, len);
    resl += len;

    strncpy(res + resl, "<b>", 4);
    resl += 3;

    len = outs[i].ed - outs[i].st + 1;
    memcpy(res + resl, text + outs[i].st, len);
    resl += len;
    led = outs[i].ed + 1;

    strncpy(res + resl, "</b>", 5);
    resl += 4;
  }

  if (led < textl) {
    len = textl - led;
    memcpy(res + resl, text + led, len);
    resl += len;
  }
  res[resl] = '\0';
}

char *get_last_fname(char *s, int32_t l) {
  while (s[l] != '/' && l >= 0) { --l; }
  return s + l + 1;
}

void eescape(char *r, char *s) {
  while (*s) {
    if (*s == '\'') {
      *r = '&'; ++r;
      *r = 'a'; ++r;
      *r = 'p'; ++r;
      *r = 'o'; ++r;
      *r = 's'; ++r;
      *r = ';'; ++r;
      ++s;
      continue;
    } else if (*s =='\n') {
      *r = '<'; ++r;
      *r = 'b'; ++r;
      *r = 'r'; ++r;
      *r = '>'; ++r;
      ++s;
      continue;
    } else if (*s =='"') {
      *r = '&'; ++r;
      *r = 'q'; ++r;
      *r = 'u'; ++r;
      *r = 'o'; ++r;
      *r = 't'; ++r;
      *r = ';'; ++r;
      ++s;
      continue;
    }

    *r = *s; ++r;
    ++s;
  }
  *r = '\0';
}

void to_anki() {
  char action[12] = {0};
  if (RUNNING == 0) {
    sprintf(action, "addNote");
  } else {
    sprintf(action, "guiAddCards");
  }

  char res[1024] = {0};
  uint32_t resl = 0;
  {
    int32_t i, j;
    for(i = 0; i < outsl; ++i) {
      sprintf(res + resl, "%s: ", outs[i].word);
      resl += strlen(outs[i].word) + 2;
      for(j = 0; j < outs[i].strl; ++j) {
        sprintf(res + resl, "%s", outs[i].strs[j]);
        resl += strlen(outs[i].strs[j]);
        if (j != outs[i].strl - 1) {
          sprintf(res + resl, "; ");
          resl += 2;
        }
      }
      if (i != outsl - 1) {
        sprintf(res + resl, "\n");
        resl += 1;
      }
    }
  }
  res[resl] = '\0';

  char cte[2048] = {0};
  {
    char ct[2048] = {0};
    bold_text(ct);
    eescape(cte, ct);
  }
  char rese[2048] = {0};
  eescape(rese, res);
  // fprintf(stdout, "\n%s\n",  rese);

  char req[2048] = {0};
  if (*audioPath) {
    char s[] = 
  "curl localhost:8765 -X -POST -d '{"
    "\"action\": \"%s\","
    "\"version\": 6,"
    "\"params\": {"
      "\"note\": {"
        "\"deckName\": \"Kill Myself\","
        "\"modelName\": \"Kms\","
        "\"fields\": {"
          "\"Deutsch\": \"%s\","
          "\"English\": \"%s\","
          "\"Audio\": \"\","
          "\"Pos\": \"\""
        "},"
        "\"options\": {"
          "\"allowDuplicate\": false,"
          "\"duplicateScope\": \"deck\","
          "\"duplicateScopeOptions\": {"
            "\"deckName\": \"Sentence Mining\","
            "\"checkChildren\": false,"
            "\"checkAllModels\": false"
          "}"
        "},"
        "\"tags\": [\"audio\"],"
        "\"audio\": [{"
          "\"filename\": \"%s\","
          "\"path\": \"%s\","
          "\"fields\": [\"Audio\"]"
        "}]"
      "}"
    "}"
  "}'"; // Action[gui/nogui] Text Words AudioFname AudioPath
    sprintf(req, s, action, cte, rese, get_last_fname(audioPath, strlen(audioPath)), audioPath);
  } else {
    char s[] = 
  "curl localhost:8765 -X -POST -d '{"
    "\"action\": \"%s\","
    "\"version\": 6,"
    "\"params\": {"
      "\"note\": {"
        "\"deckName\": \"Kill Myself\","
        "\"modelName\": \"Kms\","
        "\"fields\": {"
          "\"Deutsch\": \"%s\","
          "\"English\": \"%s\","
          "\"Audio\": \"\","
          "\"Pos\": \"\""
        "},"
        "\"options\": {"
          "\"allowDuplicate\": false,"
          "\"duplicateScope\": \"deck\","
          "\"duplicateScopeOptions\": {"
            "\"deckName\": \"Sentence Mining\","
            "\"checkChildren\": false,"
            "\"checkAllModels\": false"
          "}"
        "}"
      "}"
    "}"
  "}'"; // Action[gui/nogui] Text Words
    sprintf(req, s, action, cte, rese);
  }
  fprintf(stdout, "\n%s\n", req);
  if (system(req)) {
    if (system("plant \"Could not connect to anki!\"")) {
      if (system("herbe \"Could not connect to anki!\""));
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 4) {
    help();
  }
  setlocale(LC_ALL, "");

  //log_file = fopen("/tmp/armee-log", "w");
  log_file = stderr;
  set_logging_level(2);
  set_logging_string("Armee");

  s = setup_server_connection(log_file, "armee", "sarmale");

  text = strdup(argv[1]);
  textl = strlen(text);
  if (argc == 3) {
    audioPath = strdup(argv[2]);
    apathl = strlen(audioPath);
  }
  
  initscr();
  curs_set(0);
  cbreak();
  nodelay(curscr, 1);
  noecho();
  clear();

  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = rscr;
  sigaction(SIGWINCH, &sa, NULL);

  setup_windows();
  //update();

  char ch;
  RUNNING = 2;
  while (RUNNING == 2 && (ch = wgetch(ma.w)) != 'q') {
    handle_input(ch);
    highlight_text();
    update_panels();
    doupdate();
  }

  endwin();
  unload_dict();

  if (RUNNING < 2) {
    to_anki();
  }
  shutdown_server_connection(s);

  fclose(log_file);
  
  return 0;
}
