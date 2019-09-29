/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Alfonso Sabato Siciliano
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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "sysctlinfo_helper.h"

/* Utils */

static struct sysctlmif_object *
util_sysctlinfo_object_allflags(int *id, size_t idlevel, 
	int idnext[CTL_MAXNAME], size_t *idnextlevel)
{
    struct sysctlmif_object *object = NULL;
    char *namep, *descp, *fmtp, *labelp, *buf = NULL;
    size_t buflen = 0, idlevel_unused;
    int prop[2], *idp_unused, *idnextp;
    unsigned int kind;
    
    prop[0] = CTL_SYSCTL;
    prop[1] = (idnext != NULL) ? ENTRYALLINFO_WITHNEXTNODE : ENTRYALLINFO;
    
    if(SYSCTLINFO(id, idlevel, prop, NULL, &buflen) != 0)
    	return NULL;
    if((buf = (char*)malloc(buflen)) == NULL)
    	return NULL;
    if(SYSCTLINFO(id, idlevel, prop, buf, &buflen) != 0)
    	return NULL;
    
    if(idnext != NULL) {
    	SYSCTLINFO_HELPER_ALLWITHNEXT(buf, idlevel_unused, idp_unused, namep,
			descp, kind, fmtp, labelp, *idnextlevel, idnextp);
	memcpy(idnext, idnextp, *idnextlevel * sizeof(int));
    } else {
    	SYSCTLINFO_HELPER_ALL(buf, idlevel_unused, idp_unused, namep, descp,
    			kind, fmtp, labelp);
    }
    
    if((object = malloc(sizeof(struct sysctlmif_object))) == NULL)
    	return NULL;
    memset(object, 0, sizeof(struct sysctlmif_object));
    
    object->idlevel = idlevel;
    if((object->id = (int*)malloc(sizeof(int) * idlevel)) == NULL)
    	return NULL;
    memcpy(object->id, id, sizeof(int) * idlevel);
    object->type =  kind & CTLTYPE;
    object->flags = kind & 0xfffffff0;
    object->name = strdup(namep);
    object->desc = strdup(descp);
    object->fmt = strdup(fmtp);
    object->label = strdup(labelp);
    
    free(buf);
    
    return object;
}

static struct sysctlmif_object *
buildTree(int id[CTL_MAXNAME], size_t idlevel, int idnext[CTL_MAXNAME],
	size_t *idnextlevel)
{
    struct sysctlmif_object *node, *last, *child;
    struct sysctlmif_object_list *list = NULL;
    
    node = util_sysctlinfo_object_allflags(id, idlevel, idnext, idnextlevel);
    if( (list = malloc(sizeof(struct sysctlmif_object_list))) == NULL){
	return (NULL);
    }
    node->children = list;
    SLIST_INIT(list);

    int childid[CTL_MAXNAME];
    size_t childidlevel;
    while(*idnextlevel > idlevel) {
        memcpy(childid, idnext, *idnextlevel * sizeof(int));
    	childidlevel = *idnextlevel;
    	child = buildTree(childid, childidlevel, idnext, idnextlevel);
    	if(child == NULL)
    		break;
    	if (SLIST_EMPTY(list)) {
		SLIST_INSERT_HEAD(list, child, object_link);
    	} else {
		SLIST_INSERT_AFTER(last, child, object_link);
    	}
    	last = child;
    }
    
    return node;
}

/* API */

int sysctlinfo_idbyname(char *name, int *id, size_t *idlevel)
{
    int prop[2] = {CTL_SYSCTL, ENTRYIDBYNAME};
    
    *idlevel *= sizeof(int);
    
    if (SYSCTLINFO_BYNAME(name, prop, id, idlevel) != 0)
	return -1;
    
    *idlevel /= sizeof(int);
    
    return 0;
}

struct sysctlmif_object *sysctlinfo_object_allflags(int *id, size_t idlevel)
{

    return util_sysctlinfo_object_allflags(id, idlevel, NULL, 0);
}

struct sysctlmif_object *
sysctlinfo_tree_allflags(int *idroot, size_t idlevelroot)
{
    struct sysctlmif_object *root, *last, *child;
    struct sysctlmif_object_list *list = NULL;
    int idchild[CTL_MAXNAME], idnext[CTL_MAXNAME], i;
    size_t idchildlevel, idnextlevel;
    
    if(idlevelroot == 0) {  /* all the MIB*/    
    	if((root = malloc(sizeof(struct sysctlmif_object))) == NULL)
    		return NULL;
    	memset(root, 0, sizeof(struct sysctlmif_object));
    	root->id = (int*)malloc(sizeof(int));
    	root->id[0] = 0;
    	idchild[0] = 0;
    	idchildlevel = 1;
    } else { /* a subtree */
    	root = util_sysctlinfo_object_allflags(idroot, idlevelroot, 
    					      idchild, &idchildlevel);
    	if(root == NULL)
    		return NULL;
    	if(idchildlevel <= idlevelroot)
    		return root;
    	for(i=0; i<idlevelroot; i++)
    		if(idroot[i] != idchild[i])
    			return root;
    }
    if( (list = malloc(sizeof(struct sysctlmif_object_list))) == NULL)
		return NULL;
    SLIST_INIT(list);
    root->children = list;
    
    while(true) {
    	child = buildTree(idchild, idchildlevel, idnext, &idnextlevel);
    	if(child == NULL)
    		return NULL;
    	
    	if (SLIST_EMPTY(list)) {
		SLIST_INSERT_HEAD(list, child, object_link);
    	} else {
		SLIST_INSERT_AFTER(last, child, object_link);
    	}
    	last = child;
    	
    	if(idnextlevel == 0 || idnextlevel <= idlevelroot)
    		break;
    	memcpy(idchild, idnext, idnextlevel * sizeof(int));
    	idchildlevel = idnextlevel;
    }

    return root;
}

