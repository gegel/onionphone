BLOB = builtin.o

CC ?= cc
LD ?= ld

GENERIC_CFLAGS ?= -std=c99 -Wall -Wextra -pedantic

.PHONY: all clean test

