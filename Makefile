CFILES = tsfix.c util.c
HFILES = util.h argboiler.h
LFLAGS = -largtable2 -lcurses
CFLAGS = -O3
tsfix: $(CFILES) $(HFILES) Makefile
	gcc $(CLFAGS) $(CFILES) $(LFLAGS) -o $@
