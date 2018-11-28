#	PUBLIC DOMAIN - NO WARRANTY
#
#	Alfonso S. Siciliano http://alfix.gitlab.io

CC = cc

CCFLAGS = -I/usr/local/include -Wall -g -O0 # -I../libsysctl
LDFLAGS = -lxo -L/usr/local/lib -lsysctl # ../libsysctl/libsysctl.a
OUTPUT = nsysctl 
SOURCES = nsysctl.c opaque.c
OBJECTS = ${SOURCES:.c=.o}

all : ${OUTPUT}

clean:
	rm -f ${OUTPUT} *.o *~ *.core

${OUTPUT}: ${OBJECTS}
	${CC} ${LDFLAGS} ${OBJECTS} -o ${.PREFIX}

.c.o:
	${CC} ${CCFLAGS} -c ${.IMPSRC} -o ${.TARGET}
