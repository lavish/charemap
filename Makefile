# charemap - play with substitution ciphers
# See LICENSE file for copyright and license details.

SRC = charemap.c
VERSION = 0.1
CFLAGS = -std=c99 -pedantic -Wall -O2 -DVERSION=\"${VERSION}\"
CC = gcc

charemap: charemap.c options
	${CC} ${CFLAGS} -o $@ $<

options:
	@echo charemap build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CC       = ${CC}"

clean:
	@echo cleaning
	@rm -f charemap charemap-${VERSION}.tar.gz *.txt

dist: clean
	@echo creating dist tarball
	mkdir -p charemap-${VERSION}
	@cp -R README LICENSE Makefile languages ${SRC} charemap-${VERSION}
	@tar -cf charemap-${VERSION}.tar charemap-${VERSION}
	@gzip charemap-${VERSION}.tar
	rm -rf charemap-${VERSION}

.PHONY: options clean

