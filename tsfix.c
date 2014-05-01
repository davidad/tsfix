#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <curses.h>
#include "util.h"

#define ARGBOILER(ARG) \
  ARG(ARG_FIL1,input_filename,NULL,NULL,"the input MPEG-TS file","") \
  ARG(ARG_LIT0,hexdump,"x","hexdump","just dump the input to stdout in 188-byte lines",0)

#include "argboiler.h"

static char* ts;
static int ts_n;
static const int L = 188;
static int status = 0;
#define ST_OF_X 0x01
#define ST_OF_Y 0x02

static void finish(int sig) {
  endwin();
  exit(0);
}

void do_hexdump(void) {
  int i;
  for(i=0;i<ts_n;i++) {
    printf("%02hhx",ts[i]);
    if(!((i+1)%L)) putchar('\n');
  }
}

void show_slice(int fl, int fx, int y, int x) {
  char s[3];
  int i,j,k;
  status &= ~ST_OF_X;
  for(k=fl*L, i=j=0; j<=y && k<ts_n; (i=++k%L) || j++) {
    if((!fx || i>fx) && 2*(i-fx)+1<x) {
      snprintf(s,3,"%02hhx",ts[k]);
      mvaddnstr(j,2*(i-fx),s,2);
    } else if(fx==i) {
      mvaddnstr(j,0," <",2);
    } else if(x/2==(i-fx)) {
      mvaddnstr(j,x-1&~1,"> ",2);
      status |= ST_OF_X;
    }
  }
  if(k<ts_n) status |=  ST_OF_Y;
  else       status &= ~ST_OF_Y;
}

void handler(int c) {
  static int fl=0, fx=0;
  int y,x;
  int j,k;
  getmaxyx(stdscr,y,x);
  switch(c) {
    case 'j':
    case KEY_DOWN:
      if(status & ST_OF_Y) fl++;
      goto reshow;
    case 'k':
    case KEY_UP:
      if(fl) fl--;
      goto reshow;
    case 'l':
    case KEY_RIGHT:
      if(status & ST_OF_X) fx++;
      goto reshow;
    case 'h':
    case KEY_LEFT:
      if(fx) fx--;
      goto reshow;
    case 'U'+(1-'A'):
    case KEY_PPAGE:
      fl-=(y/2);
      if(fl<0) fl=0;
      goto reshow;
    case 'D'+(1-'A'):
    case KEY_NPAGE:
      fl+=(y/2);
      if(fl>ts_n/L) fl=ts_n/L;
      goto reshow;
    case '-':
    case KEY_RESIZE:
    reshow:
      show_slice(fl,fx,y,x);
      break;
    case 'q':
      finish(0);
      break;
  }
  refresh();
}

static void setup_curses(void) {
  signal(SIGINT,finish);
  initscr(); noecho(); nonl(); cbreak(); keypad(stdscr,TRUE);
}

void curses_mainloop(void (*f)(int)) {
    int c = KEY_RESIZE;
    setup_curses();
    while(true) {
      f(c);
      c = getch();
    }
  
}

int main(int argc, char** argv) {
  args_t args[1]; parse_args(argc,argv,args);
  int i;
  ts = open_mmapped_file_read(args->input_filename,&ts_n);
  if(args->hexdump) {
    do_hexdump();
  } else {
    curses_mainloop(handler);
  }
}
