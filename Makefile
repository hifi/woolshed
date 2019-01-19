DEBUG    = 1

REV     ?= $(shell git rev-parse --short @{0})
CFLAGS  ?= -std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter -Wl,--export-dynamic -DREV=\"$(REV)\" -Isrc
CC       = gcc

ifdef DEBUG
CFLAGS  += -ggdb -DDEBUG
else
CFLAGS  += -O2
endif

WOOLSHED           ?= woolshed
WOOLSHED_SRC        = $(wildcard src/*.c)
WOOLSHED_HDR        = $(wildcard src/*.h)

STDCLIB            ?= StdCLib.so
STDCLIB_SRC         = $(wildcard lib/StdCLib/*.c)
STDCLIB_HDR         = $(wildcard lib/StdCLib/*.h) src/debug.h

INTERFACELIB       ?= InterfaceLib.so
INTERFACELIB_SRC    = $(wildcard lib/InterfaceLib/*.c)
INTERFACELIB_HDR    = $(wildcard lib/InterfaceLib/*.h) src/debug.h

MATHLIB            ?= MathLib.so
MATHLIB_SRC         = $(wildcard lib/MathLib/*.c)
MATHLIB_HDR         = $(wildcard lib/MathLib/*.h) src/debug.h

all: $(WOOLSHED) $(STDCLIB) $(INTERFACELIB) $(MATHLIB)

$(WOOLSHED): $(WOOLSHED_SRC) $(WOOLSHED_HDR)
	$(CC) $(CFLAGS) -o $@ $(WOOLSHED_SRC) -lm -ldl

$(STDCLIB): $(STDCLIB_SRC) $(STDCLIB_HDR)
	$(CC) -shared $(CFLAGS) -o $@ $(STDCLIB_SRC)

$(INTERFACELIB): $(INTERFACELIB_SRC) $(INTERFACELIB_HDR)
	$(CC) -shared $(CFLAGS) $(shell sdl2-config --cflags) -o $@ $(INTERFACELIB_SRC) $(shell sdl2-config --libs) -lSDL2_gfx

$(MATHLIB): $(MATHLIB_SRC) $(MATHLIB_HDR)
	$(CC) -shared $(CFLAGS) -o $@ $(MATHLIB_SRC)

.PHONY: clean
clean:
	$(RM) $(WOOLSHED) $(STDCLIB) $(INTERFACELIB) $(MATHLIB)
