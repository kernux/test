#
# Makefile for chkrootkit
# (C) 1997-2007 Nelson Murilo, Pangeia Informatica, AMS Foundation and others.
#

CC       = gcc
LDFLAGS  = -static
CFLAGS	 = -O2 -Wall

SRCS   = chklastlog.c chkwtmp.c ifpromisc.c chkproc.c chkdirs.c strings.c

OBJS   = chklastlog.o chkwtmp.o ifpromisc.o chkproc.o chkdirs.o strings-static.o

all:
	@echo '*** stopping make sense ***'
	@exec make sense

sense: chklastlog chkwtmp ifpromisc chkproc chkdirs strings-static chkutmp

chklastlog:   chklastlog.c
	${CC} ${CFLAGS} -o $@ chklastlog.c
	@strip $@

chkwtmp:   chkwtmp.c
	${CC} ${CFLAGS} -o $@ chkwtmp.c
	@strip $@

ifpromisc:   ifpromisc.c
	${CC} ${CFLAGS} ${LDFLAGS}  -D_FILE_OFFSET_BITS=64 -o $@ ifpromisc.c
	@strip $@

chkproc:   chkproc.c
	${CC} ${LDFLAGS} -o $@ chkproc.c
	@strip $@

chkdirs:   chkdirs.c
	${CC} ${LDFLAGS} -o $@ chkdirs.c
	@strip $@

check_wtmpx:   check_wtmpx.c
	${CC} ${LDFLAGS} -o $@ check_wtmpx.c
	@strip $@

chkutmp:   chkutmp.c
	${CC} ${LDFLAGS} -o $@ chkutmp.c
	@strip $@


strings-static:   strings.c
	${CC} ${LDFLAGS} -o $@ strings.c
	@strip $@

clean:
	rm -f ${OBJS} core chklastlog chkwtmp ifpromisc chkproc chkdirs strings-static chkutmp
