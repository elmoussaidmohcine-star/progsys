CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11 -g
PROGS   = filetype client serveur alias alias_open alias_dup myls chat_serveur chat_client

all: $(PROGS) donnees.txt

%: %.c utils.h transfer.h
	$(CC) $(CFLAGS) -o $@ $<

donnees.txt:
	printf 'abcdefgh' > $@

clean:
	rm -f $(PROGS)

.PHONY: all clean
