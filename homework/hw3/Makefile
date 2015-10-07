#
# WARNING
#
# You should NOT edit this file, to change compiler flags, defines, etc.
# The hw3 autograder will replace this Makefile with a fresh copy.
#
SRCS=mm_alloc.c mm_test.c
EXECUTABLES=mm_test

CC=gcc
CFLAGS=-g -Wall
LDFLAGS=

OBJS=$(SRCS:.c=.o)

all: $(EXECUTABLES)

$(EXECUTABLES): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(EXECUTABLES) $(OBJS)
