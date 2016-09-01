CC=gcc

ifdef DEBUG
	CFLAGS += -g
endif

.PHONY: all

all: sh

clean:
	rm -rf sh *.dSYM
