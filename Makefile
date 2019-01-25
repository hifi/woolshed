DEBUG    = 1

REV     ?= $(shell git rev-parse --short @{0})
CFLAGS  ?= -std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter -Wl,--export-dynamic -DREV=\"$(REV)\" -Isrc
CXXFLAGS  ?= -fPIC -DREV=\"$(REV)\" -Isrc -I/usr/include/x86_64-linux-gnu/qt5 -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets
CC       = gcc
CXX      = g++

ifdef DEBUG
CFLAGS  += -ggdb -DDEBUG
CXXFLAGS  += -ggdb -DDEBUG
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
INTERFACELIB_SRC    = $(wildcard lib/InterfaceLib/*.c lib/InterfaceLib/*.cpp)
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
	$(CXX) -shared $(CXXFLAGS) -o $@ $(INTERFACELIB_SRC) -lQt5Core -lQt5Widgets

$(MATHLIB): $(MATHLIB_SRC) $(MATHLIB_HDR)
	$(CC) -shared $(CFLAGS) -o $@ $(MATHLIB_SRC)

.PHONY: clean
clean:
	$(RM) $(WOOLSHED) $(STDCLIB) $(INTERFACELIB) $(MATHLIB)
