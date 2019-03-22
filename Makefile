# PUBLIC DOMAIN - NO WARRANTY
# written by wiki.freebsd.org/AlfonsoSiciliano

PROG=	nsysctl
SRCS=	nsysctl.c opaque.c special_value.c
MAN=	${PROG}.8

CFLAGS=		-I/usr/local/include -Wall -g
LDFLAGS=	-L/usr/local/lib -lsysctlmibinfo -lxo
MK_DEBUG_FILES= no

PREFIX?=        /usr/local
MANDIR=		/man/man
DESTDIR=	${PREFIX}
BINDIR=		/bin

CLEANFILES=	*~ *.core

RM = rm -f
unistall:
	${RM} ${DESTDIR}${BINDIR}/${PROG}
	${RM} ${PREFIX}${MANDIR}8/${MAN}.gz

.include <bsd.prog.mk>
