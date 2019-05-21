# Any copyright is dedicated to the Public Domain.
# <http://creativecommons.org/publicdomain/zero/1.0/>
#
# Written by Alfonso S. Siciliano https://alfix.gitlab.io

OUTPUT= nsysctl
SOURCES= nsysctl.c opaque.c special_value.c
OBJECTS= ${SOURCES:.c=.o}
CCFLAGS= -I/usr/local/include -Wall -g
LDFLAGS= -L/usr/local/lib -lsysctlmibinfo -lxo
SBINDIR= /usr/local/sbin

MAN= ${OUTPUT}.8
GZIP= gzip -cn
MANDIR= /usr/local/man/man8

RM= rm -f
INSTALL= install -o root -g wheel

all : ${OUTPUT}

clean:
	${RM} ${OUTPUT} *.o *~ *.core ${MAN}.gz

${OUTPUT}: ${OBJECTS}
	${CC} ${LDFLAGS} ${OBJECTS} -o ${.PREFIX}

.c.o:
	${CC} ${CCFLAGS} -c ${.IMPSRC} -o ${.TARGET}

install:
	${INSTALL} -s -m 555 ${OUTPUT} ${SBINDIR}/${OUTPUT}
	${GZIP} ${MAN} > ${MAN}.gz
	${INSTALL} -m 444 ${MAN}.gz ${MANDIR}/

unistall:
	${RM} ${SBINDIR}/${OUTPUT}
	${RM} ${MANDIR}/${MAN}.gz
