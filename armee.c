#include <curses.h>
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <panel.h>
#include <signal.h>

void help() {
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  ./armee \"<text>\" [path/to/audio]\n");
  fprintf(stderr, "\n");
  exit(1);
}

int32_t wx, wy;
char *text;
uint32_t textl;
uint32_t textdl;
char *audioPath;
uint32_t apathl;
struct wp {
  WINDOW *w;
  PANEL *p;
};
struct wp ma, se;
WINDOW *ongelofelijk;

struct pointer {
  uint32_t cpos;
  int32_t clen;
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

uint32_t aaaaaaaaaaa = 0;
uint32_t get_char_len(char *ch, uint32_t chl) {
  mvwaddnstr(ongelofelijk, 0, 0, ch, chl);
  uint32_t cx, cy;
  getyx(ongelofelijk, cy, cx);
  return cx + cy - cy;
}

void draw_text(uint32_t chpl, uint32_t nlines) {
  int32_t i;
  uint32_t cmpos = 0;
  uint32_t lmpos = 0;
  uint32_t chml = 0;
  uint32_t chdl, cdpos;

  for(i = 0; i < nlines; ++i) {
    cdpos = 0;
    while (cdpos <= chpl && cmpos < textl) {
      chml = runel(text + cmpos);
      chdl = get_char_len(text + cmpos, chml);
      cmpos += chml;
      cdpos += chdl;
    }
    if (cmpos > textl) {
      mvwaddnstr(ma.w, i + 1, 1, text + lmpos, cmpos - lmpos);
      break;
    }
    cdpos -= chdl;
    cmpos -= chml;
    mvwaddnstr(ma.w, i + 1, 1, text + lmpos, cmpos - lmpos);
    lmpos = cmpos;
  }
}

void highlight_text() {

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
  getmaxyx(curscr, wy, wx);
  uint32_t chpl = wx - 4;
  {
    int32_t i;
    textdl = 0;
    uint32_t crl;
    for(i = 0; i < textl; ) {
      crl = runel(text + i);
      textdl += get_char_len(text + i, crl);
      i += crl;
    }
  }

  uint32_t nlines = ((textdl - 1) / chpl) + 1;
  fprintf(stdout, "%u %u %u\n", chpl, textdl, nlines);

  ma.w = newwin(nlines + 2, wx - 2, 0, 1);
  ma.p = new_panel(ma.w);
  box(ma.w, 0, 0);
  se.w = newwin(wy - nlines - 2, wx - 2, nlines + 2, 1);
  se.p = new_panel(se.w);
  box(se.w, 0, 0);
  draw_text(chpl, nlines);
  //highlight_text();
  update_panels();
  doupdate();
}

void rscr() {
  endwin();
  refresh();
  clear();
  setup_windows();
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 4) {
    help();
  }
  setlocale(LC_ALL, "");

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

  ongelofelijk = newwin(3, 5, 10, 10);

  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = rscr;
  sigaction(SIGWINCH, &sa, NULL);

  setup_windows();
  //update();

  char ch;
  while ((ch = wgetch(ma.w)) != 'q') {
    //handle_input(ch);
    //cpp = min(max(cpp, 0), cstrl - tsp - 1);
    //update();
    update_panels();
    doupdate();
  }

  endwin();
  
  return 0;
}
