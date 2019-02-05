# PUBLIC DOMAIN - NO WARRANTY
# written by Alfonso S. Siciliano

PROG=	nsysctl
SRCS=	nsysctl.c opaque.c

MAN=	nsysctl.8

CFLAGS=		-I/usr/local/include -Wall -g
LDFLAGS=	-lxo -L/usr/local/lib -lsysctlmibinfo

CLEANFILES=	*~ *.core

.include <bsd.prog.mk>
