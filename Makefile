CC=gcc

CFLAGS += -std=gnu99 -Wall -Werror
ifdef DEBUG
	CFLAGS += -g
endif

.PHONY: all

all: sh

sh: parsing.c history.c utils.c

clean:
	rm -rf sh *.dSYM
