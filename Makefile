# charemap - play with substitution ciphers
# See LICENSE file for copyright and license details.

PROJECT  = charemap
VERSION  = 0.5
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -march=native -DVERSION=\"${VERSION}\"
CPPFLAGS = $(shell pkg-config glib-2.0 --cflags)
LDLIBS   = $(shell pkg-config glib-2.0 --libs)
CC       = gcc
OBJ      = charemap.o decrypt.o utils.o
SRC	 = charemap.c decrypt.c utils.c charemap.h decrypt.h utils.h

${PROJECT}: options ${OBJ}
	$(CC) ${CFLAGS} ${CPPFLAGS} -o $(PROJECT) -lm $(OBJ) ${LDLIBS}

options:
	@echo charemap build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CC       = ${CC}"
	@echo

clean:
	@echo cleaning
	@rm -f charemap charemap-${VERSION}.tar.gz *.txt *.o *~

dist: clean
	@echo creating dist tarball
	mkdir -p charemap-${VERSION}
	@cp -R README LICENSE Makefile languages samples ciphers ${SRC} charemap-${VERSION}
	@tar --exclude=".svn" -cf charemap-${VERSION}.tar charemap-${VERSION}
	@gzip charemap-${VERSION}.tar
	rm -rf charemap-${VERSION}

.PHONY: options clean
