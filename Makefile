CC=gcc

CFLAGS += -Wall -Werror -Wno-unused-variable
ifdef DEBUG
	CFLAGS += -g
endif

.PHONY: all

all: sh

clean:
	rm -rf sh *.dSYM
