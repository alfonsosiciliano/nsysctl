#	PUBLIC DOMAIN - NO WARRANTY
#
#	Alfonso S. Siciliano http://alfix.gitlab.io

CC = cc

# lib
LIB_SRC =  libsysctl.c opaque.c
LIB_OBJ = ${LIB_SRC:.c=.o}

CCFLAGS = -Wall # -g -O0
LDFLAGS = -lxo 
INCLUDEDIR = ./
EXAMPLES = nsysctl hello list_example tree_example object_example wrap_example name_example 
SOURCES = ${EXAMPLES:=.c}
OBJECTS = ${SOURCES:.c=.o}

all : ${LIB_OBJ} ${EXAMPLES}

clean:
	rm -f ${EXAMPLES} *.o *~

${EXAMPLES}: ${OBJECTS}
	${CC} ${LDFLAGS} ${LIB_OBJ} ${.TARGET}.o -o ${.PREFIX}

.PATH.c : ./test
.c.o:
	${CC} -I${INCLUDEDIR} ${CCFLAGS} -c ${.IMPSRC} -o ${.TARGET}
