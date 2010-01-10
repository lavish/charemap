# program name
PROJECT  = charemap

# charemap version
VERSION  = 0.5

# flags
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -march=native -DVERSION=\"${VERSION}\" -O3
CFLAGS  += -mfpmath=sse # x86 only, remove it if you are on a different arch.
CPPFLAGS = $(shell pkg-config glib-2.0 --cflags)
LDLIBS   = $(shell pkg-config glib-2.0 --libs)

# compiler and linker
CC       = gcc
