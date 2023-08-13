.PHONY: all clean dist install uninstall
.SUFFIXES: .c .o

include config.mk

LDADD = -lSDL2

BIN = mandelbrot

OBJS = ${SRCS:.c=.o}

SRCS = test.c pixelmap.c

HDRS = graphics.h

all: ${BIN}

clean:
	rm -f ${BIN} ${OBJS}

dist: clean
	mkdir -p ${PACKAGE}-${VERSION}
	cp -Rf Makefile config.mk ${SRCS} ${HDRS} ${PACKAGE}-${VERSION}
	tar -cf ${PACKAGE}-${VERSION}.tar ${PACKAGE}-${VERSION}
	gzip ${PACKAGE}-${VERSION}.tar
	rm -rf ${PACKAGE}-${VERSION}

install: all

uninstall:

${BIN}: ${OBJS}
	${CC} ${LDFLAGS} -o $@ $^ ${LDADD}

.c.o:
	${CC} ${CFLAGS} -c -o $@ $<
