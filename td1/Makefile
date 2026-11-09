CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11 -g
PROGS   = echo write_number read read_number write_struct read_struct

all: $(PROGS)

%: %.c utils.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)

.PHONY: all clean
