REV     ?= $(shell git rev-parse --short @{0})
CFLAGS  ?= -g3 -std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter -DREV=\"$(REV)\"
TARGET  ?= peftool
CC       = gcc

ifdef DEBUG
CFLAGS  += -ggdb
else
CFLAGS  += -O2
endif

SRC     = $(wildcard src/*.c)
HDR     = $(wildcard src/*.h)

all: $(TARGET) StdCLib.so InterfaceLib.so MathLib.so

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $@ $(SRC) -lm -ldl

StdCLib.so: lib/stdclib.c
	$(CC) -shared $(CFLAGS) -o $@ lib/stdclib.c

InterfaceLib.so: lib/interfacelib.c
	$(CC) -shared $(CFLAGS) -o $@ lib/interfacelib.c

MathLib.so: lib/mathlib.c
	$(CC) -shared $(CFLAGS) -o $@ lib/mathlib.c

.PHONY: clean
clean:
	$(RM) $(TARGET) StdCLib.so InterfaceLib.so MathLib.so
