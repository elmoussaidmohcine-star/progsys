CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11 -g
PROGS   = buffer fork_pid lance segfault

all: $(PROGS)

%: %.c utils.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)

.PHONY: all clean
