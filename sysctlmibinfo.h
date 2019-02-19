/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2018-2019 Alfonso Sabato Siciliano
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef _SYSCTLMIBINFO_H_
#define _SYSCTLMIBINFO_H_

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/sysctl.h>

#define SYSCTLMIF_VERSION        1
#define SYSCTLMIF_MAXIDLEVEL     CTL_MAXNAME
#define SYSCTLMIF_MAXDEPTH       (CTL_MAXNAME - 1)

/*
 * functions to wrap 'undocumented kern_sysctl.c API',
 *    return: 0 for success, negative value for failure.
 */

int
sysctlmif_nametoid(const char *name, size_t namelen, int *id, size_t *idlevel);

int sysctlmif_name(int *id, size_t idlevel, char *name, size_t *namelen);
int sysctlmif_desc(int *id, size_t idlevel, char *desc, size_t *desclen);
int sysctlmif_label(int *id, size_t idlevel, char *label, size_t *labellen);

#define SYSCTLMIF_NAMELEN(id, idlevel, size) \
	sysctlmif_name(id, idlevel, NULL, size)
#define SYSCTLMIF_DESCLEN(id, idlevel, size) \
	sysctlmif_desc(id, idlevel, NULL, size)
#define SYSCTLMIF_LABELLEN(id, idlevel, size) \
	sysctlmif_label(id, idlevel, NULL, size)

int sysctlmif_info(int *id, size_t idlevel, void *info, size_t *infolen);

#define SYSCTLMIF_INFOKIND(info)        (*((unsigned int *)info))
#define SYSCTLMIF_INFOTYPE(info)        (*((unsigned int *)info) & CTLTYPE)
#define SYSCTLMIF_INFOFLAGS(info)       (*((unsigned int *)info) & 0xfffffff0)
#define SYSCTLMIF_INFOFMT(info)         ((char *)info + sizeof(unsigned int))

/* kernel returns only next leaf, next node requires extra computation */
int
sysctlmif_nextnode(int *id, size_t idlevel, int *idnext, size_t *idnextlevel);
int
sysctlmif_nextleaf(int *id, size_t idlevel, int *idnext, size_t *idnextlevel);

/*
 * functions related to "struct sysctlmif_object"
 *    params: id and idlevel to identify a mib entry
 *    return: NULL for failure, pointer to allocated memory for success.
 */

/* 'struct sysctlmif_object': userspace mib entry definition */

SLIST_HEAD(sysctlmif_object_list, sysctlmif_object);

struct sysctlmif_object {
	SLIST_ENTRY(sysctlmif_object) object_link;
	int *id;
	size_t idlevel; /* between 1 and SYSCTLMIF_MAXIDLEVEL */
	char *name;
	char *desc;
	char *label;    /* aggregation label */
	uint8_t type;   /* defined in <sys/sysctl.h> */
	uint32_t flags; /* defined in <sys/sysctl.h> */
	char *fmt;      /* format string */
	                /* children is set by sysctlmif_tree() */
	struct sysctlmif_object_list *children;
};

/*
 * OR_FLAGS: object fields to set,
 * .id and .idlevel are always set
 * .children is default for sysctlmif_tree()
 */
#define SYSCTLMIF_FNAME  	0x01    /* .name  */
#define SYSCTLMIF_FDESC  	0x02    /* .desc  */
#define SYSCTLMIF_FLABEL 	0x04    /* .label */
#define SYSCTLMIF_FTYPE  	0x08    /* .type  */
#define SYSCTLMIF_FFLAGS 	0x10    /* .flags */
#define SYSCTLMIF_FFMT   	0x20    /* .fmt   */
#define SYSCTLMIF_FALL   	        /*  all   */    \
		(SYSCTLMIF_FNAME    | SYSCTLMIF_FDESC   \
		| SYSCTLMIF_FLABEL | SYSCTLMIF_FTYPE    \
		| SYSCTLMIF_FFLAGS | SYSCTLMIF_FFMT)

/* object functions  */

struct sysctlmif_object *
sysctlmif_object(int *id, size_t idlevel, unsigned int flags);

void
sysctlmif_freeobject(struct sysctlmif_object *object);

/* list functions  */

typedef int sysctlmif_filterfunc_t (struct sysctlmif_object *object);

struct sysctlmif_object_list *
sysctlmif_filterlist(sysctlmif_filterfunc_t *filterfunc, unsigned int flags);

#define SYSCTLMIF_LIST(flags)     sysctlmif_filterlist(NULL, flags)

struct sysctlmif_object_list *
sysctlmif_grouplist(int *id, size_t idlevel, unsigned int flags,
    unsigned int max_depth);

void
sysctlmif_freelist(struct sysctlmif_object_list *list);

/* tree fuctions  */

struct sysctlmif_object *
sysctlmif_tree(int *id, size_t idlevel, unsigned int flags,
    unsigned int max_depth);

void
sysctlmif_freetree(struct sysctlmif_object *object_root);

#endif /* _SYSCTLMIBINFO_H_ */
