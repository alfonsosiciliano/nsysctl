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
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/sysctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysctlmibinfo.h>

/* <sys/sysctl.h> lacks these identifiers */

/* Top-level identifiers */
#define CTL_SYSCTLMIB    	0

/* CTL_SYSCTLMIB identifiers */
#define MIB_OBJECTNAME   	1
#define MIB_NEXTOID      	2
#define MIB_NAME2OID     	3
#define MIB_OBJECTFMT    	4
#define MIB_OBJECTDESCR  	5
#define MIB_OBJECTLABEL  	6

/* Internal use */

static int
sysctlmif_internal_subtree(struct sysctlmif_object *obj, unsigned int flags,
    unsigned int depth)
{
	int error = 0;
	struct sysctlmif_object *child;

	obj->children = sysctlmif_grouplist(obj->id, obj->idlevel, flags, 1);
	if (obj->children == NULL) {
		return (-1);
	}
	SLIST_REMOVE_HEAD(obj->children, object_link);

	if (depth < 1) {
		return (0);
	}

	SLIST_FOREACH(child, obj->children, object_link) {
		error = sysctlmif_internal_subtree(child, flags, depth - 1);
		if (error != 0) {
			return (error);
		}
	}

	return (error);
}

/* API implementation */

int
sysctlmif_nametoid(const char *name, size_t namelen, int *id, size_t *idlevel)
{
	int mib[2];
	int error = 0;

	mib[0] = CTL_SYSCTLMIB;
	mib[1] = MIB_NAME2OID;

	*idlevel *= sizeof(int);
	error = sysctl(mib, 2, id, idlevel, name, namelen);
	*idlevel /= sizeof(int);

	return (error);
}

int
sysctlmif_desc(int *id, size_t idlevel, char *desc, size_t *desclen)
{
	int mib[CTL_MAXNAME + 2];
	int error = 0;

	mib[0] = CTL_SYSCTLMIB;
	mib[1] = MIB_OBJECTDESCR;
	memcpy(mib + 2, id, idlevel * sizeof(int));

	error = sysctl(mib, idlevel + 2, (void *)desc, desclen, NULL, 0);

	return (error);
}

int
sysctlmif_name(int *id, size_t idlevel, char *name, size_t *namelen)
{
	int mib[CTL_MAXNAME + 2];
	int error = 0;

	mib[0] = CTL_SYSCTLMIB;
	mib[1] = MIB_OBJECTNAME;
	memcpy(mib + 2, id, idlevel * sizeof(int));

	error = sysctl(mib, idlevel + 2, (void *)name, namelen, NULL, 0);

	return (error);
}

int
sysctlmif_label(int *id, size_t idlevel, char *label, size_t *labellen)
{
	int mib[CTL_MAXNAME + 2];
	int error = 0;

	mib[0] = CTL_SYSCTLMIB;
	mib[1] = MIB_OBJECTLABEL;
	memcpy(mib + 2, id, idlevel * sizeof(int));

	error = sysctl(mib, idlevel + 2, (void *)label, labellen, NULL, 0);

	return (error);
}

int
sysctlmif_info(int *id, size_t idlevel, void *info, size_t *infosize)
{
	int mib[CTL_MAXNAME + 2];
	int error = 0;

	mib[0] = CTL_SYSCTLMIB;
	mib[1] = MIB_OBJECTFMT;
	memcpy(mib + 2, id, idlevel*sizeof(int));

	error = sysctl(mib, idlevel + 2, info, infosize, NULL, 0);

	return (error);
}

int
sysctlmif_nextleaf(int *id, size_t idlevel, int *idnext, size_t *idnextlevel)
{
	int mib[CTL_MAXNAME + 2];
	int error = 0;
	size_t tmp_nextlevel;

	mib[0] = CTL_SYSCTLMIB;
	mib[1] = MIB_NEXTOID;
	memcpy(mib + 2, id, idlevel*sizeof(int));

	tmp_nextlevel = *idnextlevel * sizeof(int);

	error = sysctl(mib, idlevel + 2, idnext, &tmp_nextlevel, NULL, 0);
	if (error == 0) {
		*idnextlevel = tmp_nextlevel / sizeof(int);
	}

	return (error);
}

int
sysctlmif_nextnode(int *id, size_t idlevel, int *idnext, size_t *idnextlevel)
{
	int error = 0;
	size_t i, minlevel;
	size_t tmp_nextlevel;

	/* it could be "id = idnext", then: */
	int previd[CTL_MAXNAME];
	size_t prevlevel = idlevel;

	memcpy(previd, id, idlevel * sizeof(int));

	tmp_nextlevel = *idnextlevel * sizeof(int);
	error = sysctlmif_nextleaf(id, idlevel, idnext, &tmp_nextlevel);
	if (error != 0) {
		return (error);
	}
	*idnextlevel = tmp_nextlevel;

	/*
	 * avoid:   id 5.6 -> next 5.6.4.8.2
	 * we want: id 5.6 -> next 5.6.4    (just 1 level)
	 */
	if (*idnextlevel > prevlevel) {
		*idnextlevel = prevlevel + 1;
	}

	minlevel = *idnextlevel < prevlevel ? *idnextlevel : prevlevel;

	for (i = 0; i < minlevel; i++) {
		if (previd[i] != idnext[i]) {
			*idnextlevel = i+1;
			break;
		}
	}

	return (error);
}

struct sysctlmif_object *
sysctlmif_object(int *id, size_t idlevel, unsigned int flags)
{
	struct sysctlmif_object *obj = NULL;
	size_t size = 0;
	void *tmpinfo = NULL;

	obj = malloc(sizeof(struct sysctlmif_object));
	if (obj == NULL) {
		return (NULL);
	}
	memset(obj, 0, sizeof(struct sysctlmif_object));

	/* id and idlevel are always set */
	obj->id = malloc(idlevel * sizeof(int));
	if (obj->id == NULL) {
		sysctlmif_freeobject(obj);
		return (NULL);
	}
	memcpy(obj->id, id, idlevel * sizeof(int));
	obj->idlevel = idlevel;

	if (flags & SYSCTLMIF_FNAME) {
		/* kernel returns false positive */
		if (SYSCTLMIF_NAMELEN(id, idlevel, &size) == 0) {
			if ((obj->name = malloc(size)) == NULL) {
				sysctlmif_freeobject(obj);
				return (NULL);
			}
			memset(obj->name, 0, size);
			if (sysctlmif_name(id, idlevel, obj->name,
			    &size) != 0) {
				obj->name = NULL;
			}
		}
	}

	if (flags & SYSCTLMIF_FDESC) {
		size = 0;
		/* entry without descr could return "\0" or NULL */
		if (SYSCTLMIF_DESCLEN(id, idlevel, &size) == 0) {
			if ((obj->desc = malloc(size)) == NULL) {
				sysctlmif_freeobject(obj);
				return (NULL);
			}
			memset(obj->desc, 0, size);
			if (sysctlmif_desc(id, idlevel, obj->desc,
			    &size) != 0) {
				obj->desc = NULL;
			}
		}
	}

	if (flags & SYSCTLMIF_FLABEL) {
		size = 0;
		if (SYSCTLMIF_LABELLEN(id, idlevel, &size) == 0) {
			if ((obj->label = malloc(size)) == NULL) {
				sysctlmif_freeobject(obj);
				return (NULL);
			}
			memset(obj->label, 0, size);
			if (sysctlmif_label(id, idlevel, obj->label,
			    &size) != 0) {
				obj->label = NULL;
			}
		}
	}

	if ((flags & SYSCTLMIF_FFLAGS) || (flags & SYSCTLMIF_FFMT) ||
	    (flags & SYSCTLMIF_FTYPE)) {
		size = 0;
		/* get info size because fmt is variable */
		if (sysctlmif_info(id, idlevel, NULL, &size) == 0) {
			tmpinfo = malloc(size);
			if (tmpinfo == NULL) {
				sysctlmif_freeobject(obj);
				return (NULL);
			}
			memset(tmpinfo, 0, size);

			if (sysctlmif_info(id, idlevel, tmpinfo, &size) < 0) {
				sysctlmif_freeobject(obj);
				free(tmpinfo);
				return (NULL);
			}

			if (flags & SYSCTLMIF_FFMT) {
				obj->fmt = strndup(SYSCTLMIF_INFOFMT(tmpinfo),
					size - sizeof(uint32_t));
				if (obj->fmt == NULL) {
					sysctlmif_freeobject(obj);
					return (NULL);
				}
			}

			if (flags & SYSCTLMIF_FFLAGS) {
				obj->flags = SYSCTLMIF_INFOFLAGS(tmpinfo);
			}

			if (flags & SYSCTLMIF_FTYPE) {
				obj->type = SYSCTLMIF_INFOTYPE(tmpinfo);
			}

			free(tmpinfo);
		}
	}

	return (obj);
}

void
sysctlmif_freeobject(struct sysctlmif_object *object)
{
	if (object == NULL) {
		return;
	}

	free(object->id);
	free(object->name);
	free(object->desc);
	free(object->label);
	free(object->fmt);
	free(object);
	object = NULL;
}

struct sysctlmif_object_list *
sysctlmif_filterlist(sysctlmif_filterfunc_t *filterfunc, unsigned int flags)
{
	int id[CTL_MAXNAME], idnext[CTL_MAXNAME];
	size_t idlevel, idnextlevel;
	struct sysctlmif_object_list *list = NULL;
	struct sysctlmif_object *last, *new;

	list = malloc(sizeof(struct sysctlmif_object_list));
	if (list == NULL) {
		return (NULL);
	}

	SLIST_INIT(list);

	id[0] = 0;
	idlevel = 1;

	for (;;) {
		if ((new = sysctlmif_object(id, idlevel, flags)) == NULL) {
			sysctlmif_freelist(list);
			return (NULL);
		}

		if ((filterfunc == NULL) || (filterfunc(new) == 0)) {
			if (SLIST_EMPTY(list)) {
				SLIST_INSERT_HEAD(list, new, object_link);
			} else {
				SLIST_INSERT_AFTER(last, new, object_link);
			}

			last = new;
		}

		idnextlevel = CTL_MAXNAME;
		if (sysctlmif_nextnode(id, idlevel, idnext, &idnextlevel) < 0) {
			break;
		}
		memcpy(id, idnext, idnextlevel * sizeof(int));
		idlevel = idnextlevel;
	}

	return (list);
}

struct sysctlmif_object_list *
sysctlmif_grouplist(int *idstart, size_t idstartlen, unsigned int flags,
    unsigned int depth)
{
	int id[CTL_MAXNAME], idnext[CTL_MAXNAME];
	size_t idlevel, idnextlevel;
	struct sysctlmif_object_list *list = NULL;
	struct sysctlmif_object *last, *new;
	size_t i;

	list = malloc(sizeof(struct sysctlmif_object_list));
	if (list == NULL) {
		return (NULL);
	}

	SLIST_INIT(list);

	memcpy(id, idstart, idstartlen * sizeof(int));
	idlevel = idstartlen;

	if ((new = sysctlmif_object(id, idlevel, flags)) == NULL) {
		free(list);
		return (NULL);
	}

	SLIST_INSERT_HEAD(list, new, object_link);

	last = new;

	for (;;) {
		idnextlevel = CTL_MAXNAME;
		if (sysctlmif_nextnode(id, idlevel, idnext, &idnextlevel) < 0) {
			break;
		}
		memcpy(id, idnext, idnextlevel * sizeof(int));
		idlevel = idnextlevel;

		if (idlevel - idstartlen > depth) {
			continue;
		}

		if (idlevel < idstartlen) {
			break;
		}

		for (i = 0; i < idstartlen; i++) {
			if (id[i] != idstart[i]) {
				return (list);
			}
		}

		new = sysctlmif_object(id, idlevel, flags);
		if (new == NULL) {
			sysctlmif_freelist(list);
			return (NULL);
		}
		SLIST_INSERT_AFTER(last, new, object_link);
		last = new;
	}

	return (list);
}

void
sysctlmif_freelist(struct sysctlmif_object_list *list)
{
	struct sysctlmif_object *obj;

	if (list == NULL) {
		return;
	}

	while (!SLIST_EMPTY(list)) {
		obj = SLIST_FIRST(list);
		SLIST_REMOVE_HEAD(list, object_link);
		sysctlmif_freeobject(obj);
	}

	free(list);
	list = NULL;
}

struct sysctlmif_object *
sysctlmif_tree(int *id, size_t idlevel, unsigned int flags,
    unsigned int max_depth)
{
	int error;
	struct sysctlmif_object *root = NULL;

	if ((root = sysctlmif_object(id, idlevel, flags)) == NULL) {
		return (NULL);
	}

	if (max_depth < 1) {
		return (root);
	}

	error = sysctlmif_internal_subtree(root, flags, max_depth - 1);
	if (error != 0) {
		sysctlmif_freetree(root);
	}

	return (root);
}

/* postorder visit */
void
sysctlmif_freetree(struct sysctlmif_object *node)
{
	struct sysctlmif_object *child;

	if (node == NULL) {
		return;
	}

	if (node->children != NULL) {
		while (!SLIST_EMPTY(node->children)) {
			child = SLIST_FIRST(node->children);
			SLIST_REMOVE_HEAD(node->children, object_link);
			sysctlmif_freetree(child);
		}
	}

	sysctlmif_freeobject(node);
}
