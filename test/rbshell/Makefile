src = $(wildcard *.c)
obj = $(src:.c=.o)
bin = test

CC = gcc
CFLAGS = -pedantic -Wall -g -I../../src -I/usr/local/include
LDFLAGS = -L../.. -L/usr/local/lib -lrbtree

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
