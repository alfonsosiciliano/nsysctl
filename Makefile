# PUBLIC DOMAIN - NO WARRANTY
# written by wiki.freebsd.org/AlfonsoSiciliano

PROG=	nsysctl
SRCS=	nsysctl.c opaque.c special_value.c

MAN=	nsysctl.8

CFLAGS=		-I/usr/local/include -Wall -g
LDFLAGS=	-lxo -L/usr/local/lib -lsysctlmibinfo

CLEANFILES=	*~ *.core

.include <bsd.prog.mk>
