CC=gcc

CFLAGS += -Wall -Werror -Wno-unused-variable -Wno-unused-but-set-variable
ifdef DEBUG
	CFLAGS += -g
endif

.PHONY: all

all: sh

sh: parsing.c utils.c

clean:
	rm -rf sh *.dSYM
