# PUBLIC DOMAIN - NO WARRANTY
# written by Alfonso S. Siciliano

CC = cc
CCFLAGS = -I/usr/local/include -Wall -g
LDFLAGS = -lxo -L/usr/local/lib -lsysctl
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
