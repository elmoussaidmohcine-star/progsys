CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu11 -g
PROGS   = mypipe majuscules pipeline pipeline_simple pipeline_args

all: $(PROGS)

pipeline_simple: pipeline.c utils.h
	$(CC) $(CFLAGS) -DSIMPLER_VERSION -o $@ $<

%: %.c utils.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)

.PHONY: all clean
