# $FreeBSD$

PORTNAME=	nsysctl
PORTVERSION=	0.1
CATEGORIES=	sysutils
MASTER_SITES=

MAINTAINER=	alfix86@gmail.com
COMMENT=	Sysctl utility clone with libxo and extra options

LICENSE=	BSD2CLAUSE
LICENSE_FILE=	${WRKSRC}/LICENSE

USE_GITLAB=	yes

GL_ACCOUNT=	alfix
GL_COMMIT=	460a8eb2fd94d34364470ecf3edc70fd7ef3722c

PLIST_FILES=	bin/nsysctl \
		man/man8/${PORTNAME}.8.gz

do-install:
	${INSTALL_PROGRAM} ${WRKSRC}/${PORTNAME} ${STAGEDIR}${PREFIX}/bin
	${INSTALL_MAN} ${WRKSRC}/${PORTNAME}.8 ${STAGEDIR}${MAN8PREFIX}/man/man8/${PORTNAME}.8

.include <bsd.port.mk>
