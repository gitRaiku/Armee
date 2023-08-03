#include <curses.h>
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <panel.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "../Libs/log.h"
#include "../Libs/socket.h"

FILE *__restrict log_file;

void help() {
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  ./armee \"<text>\" [path/to/audio]\n");
  fprintf(stderr, "\n");
  exit(1);
}

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
struct wp {
  WINDOW *w;
  PANEL *p;
};
struct wp ma, se;
WINDOW *ongelofelijk;

struct pointer {
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

uint32_t go_until(char *str, uint32_t strl, int pos) {
  uint32_t cmpos = 0;
  uint32_t chml = 0;
  uint32_t cdpos = 0;
  while (cdpos <= pos && cmpos < strl) {
    chml = runel(str + cmpos);
    cmpos += chml;
    ++cdpos;
  }
  if (cmpos == strl) {
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
  mvwchgat(ma.w, py + 1, px + 1, 1, (cp.sel ? A_UNDERLINE : 0) | A_REVERSE, 0, NULL);
  mvwchgat(ma.w, ey + 1, ex + 1, 1, (cp.sel ? A_UNDERLINE : 0) | A_REVERSE, 0, NULL);
  char a[1024];
  snprintf(a, sizeof(a), "chpl: %u; pos: %u; len: %u; sel: %u; po: %u; en: %u; py: %u; px: %u; ey: %u; ex: %u       ", chpl, cp.pos, cp.len, cp.sel, pointer, end, py, px, ey, ex);
  mvwaddstr(se.w, 2, 2, a);
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
  se.w = newwin(wy - nlines - 2, wx - 2, nlines + 2, 1);
  se.p = new_panel(se.w);
  box(se.w, 0, 0);
  draw_text();
  highlight_text();
  update_panels();
  doupdate();
}

void rscr() {
  endwin();
  refresh();
  clear();
  setup_windows();
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
  selection = text + st;
  selt = ed - st + 1;
}

void get_results() {
  // selection[0:selt]

}

void handle_input(char ch) {
  char a[1024];
  sprintf(a, "Presed %c    ", ch);
  mvwaddstr(se.w, 12, 1, a);
  if (cp.sel) {
    switch (ch) {
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
      case ' ':
        get_selection();
        selection[selt] = '\0';
        werase(se.w);
        box(se.w, 0, 0);
        mvwaddnstr(se.w, 15, 1, selection, selt);
        get_results();
        cp.sel = 0;
        cp.pos += cp.len;
        cp.len = 0;
        return;
    }
  } else {
    switch (ch) {
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
        return;
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
  set_logging_level(0);

  struct rsocket s = setup_server_connection(log_file, "armee", "sarmale");

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
  while ((ch = wgetch(ma.w)) != 'q') {
    handle_input(ch);
    highlight_text();
    update_panels();
    doupdate();
  }

  endwin();
  shutdown_server_connection(s);

  fclose(log_file);
  
  return 0;
}
