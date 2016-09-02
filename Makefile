CC=gcc

CFLAGS += -Wall
ifdef DEBUG
	CFLAGS += -g
endif

.PHONY: all

all: sh

clean:
	rm -rf sh *.dSYM
