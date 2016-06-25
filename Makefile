CFLAGS=-Wall -std=c++11 -ggdb

.PHONY: all clean cleanall

all: mini-shell

mini-shell: mini-shell.c
	echo -e '\0033\0143'
	g++ $(CFLAGS) -o mini-shell mini-shell.c

clean:
	rm -f mini-shell

cleanall:
	rm -f mini-shell
