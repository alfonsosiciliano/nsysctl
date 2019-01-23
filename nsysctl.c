/*-
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

#include <sys/queue.h>

#include <libxo/xo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysctlmibinfo.h>
#include <unistd.h>

#include "opaque.h"

#define IS_LEAF(node)    (node->children == NULL || SLIST_EMPTY(node->children))

void usage();
int set_basic_value(struct sysctlmif_object *, char *);
int parse_line_or_argv(char*);
void display_tree(struct sysctlmif_object *);
void display_basic_type(struct sysctlmif_object *);

static const char *ctl_typename[CTLTYPE+1] =
{
    [CTLTYPE_INT] = "integer",
    [CTLTYPE_UINT] = "unsigned integer",
    [CTLTYPE_LONG] = "long integer",
    [CTLTYPE_ULONG] = "unsigned long",
    [CTLTYPE_U8] = "uint8_t",
    [CTLTYPE_U16] = "uint16_t",
    [CTLTYPE_U32] = "uint32_t",
    [CTLTYPE_U64] = "uint64_t",
    [CTLTYPE_S8] = "int8_t",
    [CTLTYPE_S16] = "int16_t",
    [CTLTYPE_S32] = "int32_t",
    [CTLTYPE_S64] = "int64_t",
    [CTLTYPE_NODE] = "node",
    [CTLTYPE_STRING] = "string",
    [CTLTYPE_OPAQUE] = "opaque",
};

int aflag, bflag, Bflag, dflag, eflag, Fflag, fflag, hflag, Iflag;
int iflag, lflag, Mflag, mflag, Nflag, nflag, oflag, qflag, Sflag;
int Tflag, tflag, Wflag, xflag, yflag;

void usage()
{
    printf("usage:\n");
    printf("\tnsysctl [-AadeFhiIlMmNnoqSTtWXxy] [-B <bufsize>] [-f filename] name[=value] ...\n");
    printf("\tnsysctl [-AadeFhIlMmNnoqSTtWXxy] [-B <bufsize>] -a\n");
    printf("\tnsysctl --libxo=<opts> [-AadeFhiIlMmNnoqSTtWXxy] [-B <bufsize>] [-f filename] name[=value] ...\n");
    printf("\tnsysctl --libxo=<opts> [-AadeFhIlMmNnoqSTtWXxy] [-B <bufsize>] -a\n");
}

/*
 * this func will be merged with set_basic_value() in version 1.0 
 */
void display_basic_type(struct sysctlmif_object *object)
{
    size_t value_size = 0;
    void *value;

    // BUG --libxo=xml => segmentation fault
    if(strcmp(object->name,"debug.witness.fullgraph") ==0)
    	return;

    sysctl(object->id, object->idlevel, NULL, &value_size, NULL, 0);
    if ((value = malloc(value_size)) == NULL) {
	printf("%s: Cannot get value MALLOC\n", object->name);
	return;
    }
    memset(value, 0, value_size);

    if (sysctl(object->id, object->idlevel, value, &value_size, NULL, 0) != 0) {
	xo_warn("cannot get value");
	return;
    }

    switch (object->type) {
    case CTLTYPE_INT:
	xo_emit("{:value/%d}", *((int *)value));
	break;
    case CTLTYPE_LONG:
	xo_emit("{:value/%ld}", *((long *)value));
	break;
    case CTLTYPE_S8:
	xo_emit("{:value/%d}", *((int8_t *)value));
	break;
    case CTLTYPE_S16:
	xo_emit("{:value/%d}", *((int16_t *)value));
	break;
    case CTLTYPE_S32:
	xo_emit("{:value/%d}", *((int32_t *)value));
	break;
    case CTLTYPE_S64:
	xo_emit("{:value/%ld}", *((int64_t *)value));
	break;
    case CTLTYPE_UINT:
	xo_emit("{:value/%u}", *((u_int *)value));
	break;
    case CTLTYPE_ULONG:
	xo_emit("{:value/%lu}", *((u_long *)value));
	break;
    case CTLTYPE_U8:
	xo_emit("{:value/%u}", *((uint8_t *)value));
	break;
    case CTLTYPE_U16:
	xo_emit("{:value/%u}", *((uint16_t *)value));
	break;
    case CTLTYPE_U32:
	xo_emit("{:value/%u}", *((uint32_t *)value));
	break;
    case CTLTYPE_U64:
	xo_emit("{:value/%lu}", *((uint64_t *)value));
	break;
    case CTLTYPE_NODE:
	xo_emit("{:value/%s}", "--- TYPE NODE ---");
	break;
    case CTLTYPE_STRING:
	xo_emit("{:value/%s}", (char *)value);
	break;
    default:
	printf("%s, Error bad type!\n", object->name);
    }

    free(value);
}


int set_basic_value(struct sysctlmif_object *object, char *input)
{
    long long llivalue = strtoll(input, NULL, 10);
    unsigned long long ullvalue =strtoull(input, NULL, 10);
    int error = 0;
    size_t llsize, ullsize;
    int intvalue;
    long longvalue;
    int8_t  sint8value;
    int16_t sint16value;
    int32_t sint32value;
    int64_t sint64value;
    u_int uintvalue;
    u_long ulongvalue;
    uint8_t uint8value;
    uint16_t uint16value;
    uint32_t uint32value;
    uint64_t uint64value;
    
    llsize=sizeof(long long);
    ullsize=sizeof(unsigned long long);

    display_basic_type(object);

#define STVL(typedvalue, type, longinput, formatstr)	\
    do {						\
	typedvalue = (type)longinput;			\
	if(sysctl(object->id,object->idlevel,NULL,0,	\
		  & typedvalue,sizeof(type)) !=0)	\
	{						\
	    xo_emit("{L: -> }");			\
	    xo_emit_field("", "newvalue", formatstr,	\
			  NULL, typedvalue);		\
	    xo_emit("{L:\n}");				\
	}						\
	else						\
	    xo_warnx("cannot set new value %s",input);	\
    } while(0)
    
    switch (object->type) {
    case CTLTYPE_STRING:
	sysctl(object->id, object->idlevel, NULL, 0,
	       input, sizeof(input));
	break;
    case CTLTYPE_OPAQUE:
	xo_warnx("Cannot set an opaque input");
	error = 1;
	break;
    case CTLTYPE_NODE:
	xo_warnx("oid \'%s\' isn't a leaf node",object->name);
	error = 1;
	break;
    case CTLTYPE_INT:
	STVL(intvalue, int, llivalue, "%d");
	break;
    case CTLTYPE_LONG:
	STVL(longvalue, long, llivalue, "%ld");
	break;
    case CTLTYPE_S8:
	STVL(sint8value, int8_t, llivalue,"%d");
	break;
    case CTLTYPE_S16:
	STVL(sint16value, int16_t, llivalue,"%d");
	break;
    case CTLTYPE_S32:
	STVL(sint32value, int32_t, llivalue,"%d");
	break;
    case CTLTYPE_S64:
	STVL(sint64value, int64_t, llivalue,"%ld");
	break;
    case CTLTYPE_UINT:
	STVL(uintvalue, u_int, ullvalue,"%u");
	break;
    case CTLTYPE_ULONG:
	STVL(ulongvalue, u_long, ullvalue,"%lu");
	break;
    case CTLTYPE_U8:
	STVL(uint8value, uint8_t, ullvalue,"%u");
	break;
    case CTLTYPE_U16:
	STVL(uint16value, uint16_t, ullvalue,"%u");
	break;
    case CTLTYPE_U32:
	STVL(uint32value, uint32_t, ullvalue,"%u");
	break;
    case CTLTYPE_U64:
	STVL(uint64value, uint64_t, ullvalue,"%lu");
	break;
    default:
	xo_warnx("Unknown type");
	error = 1;
	break;
    }

    return (error);
}

int parse_line_or_argv(char *arg)
{
    char *tofree, *nodename, *parsestring;
    int error = 0;
    int id[SYSCTLMIF_IDMAXLEVEL];
    size_t idlevel = SYSCTLMIF_IDMAXLEVEL;
    struct sysctlmif_object *node;
    
    parsestring = strdup(arg);
    tofree = nodename = strsep(&parsestring, "=");
    
    if (sysctlmif_nametoid(nodename, strlen(nodename) +1,
			   id, &idlevel) != 0) {
	/* nodename doesn't exist*/
	if (!iflag)
	    error++;

	if (!iflag && !qflag)
	    xo_warnx("unknow \'%s\' oid", nodename);
    }
    else if (strlen(nodename) == strlen(arg)) { /* only nodename */
	node = sysctlmif_tree(id, idlevel,
			      SYSCTLMIF_FALL, SYSCTLMIF_MAXDEPTH);
	display_tree(node);
	sysctlmif_freetree(node);
    }
    else { /* nodename=value */
	node = sysctlmif_object(id, idlevel,
				SYSCTLMIF_FNAME | SYSCTLMIF_FTYPE);
	if(!IS_LEAF(node)) {
	    xo_warnx("oid \'%s\' isn't a leaf node",node->name);
	    error++;
	}
	else
	    set_basic_value(node, parsestring);
		
	sysctlmif_freeobject(node);
    }

    free(tofree);

    return error;
}

/* Preorder visit */
void display_tree(struct sysctlmif_object *object)
{
    struct sysctlmif_object *child;
    int showable = 1;

    if ((object->id[0] == 0) && !Sflag)
	showable = 0;

    if (Wflag && !((object->flags & CTLFLAG_WR) && !(object->flags & CTLFLAG_STATS)))
	showable = 0;

    if (Tflag && !(object->flags & CTLFLAG_TUN))
	showable = 0;

    if (!Iflag && (!IS_LEAF(object)))
	showable = 0;

    if (showable)
    {
	xo_open_instance("object");

	if (!nflag)
	{
	    xo_emit("{:name/%s}", object->name);
	    if (!Nflag) {
		eflag ? xo_emit("{L:=}") : xo_emit("{Pcw:}");
	    }
	}

	if (!Nflag) {
	    if (dflag) /* entry without descr could return "\0" or NULL */
		xo_emit("{:description/%s}", object->desc == NULL ? "" : object->desc);
	    else if (tflag)
		xo_emit("{:type/%s}", ctl_typename[object->type]);
	    else if (Fflag)
		xo_emit("{:flags/%x}", object->flags);
	    else if (mflag)
		xo_emit("{:format/%s}", object->fmt);
	    else if (lflag)
		xo_emit("{:label/%s}", object->label);
	    else if (yflag)
	    {
		xo_open_container("id");
		int i;
		for (i = 0; i < object->idlevel; i++)
		{
		    xo_emit("{:id/%x}", object->id[i]);
		    if (i+1 < object->idlevel)
			xo_emit("{L:.}");
		}
		xo_close_container("id");
	    }
	    else {
		if (IS_LEAF(object)) {
		    if (object->type == CTLTYPE_OPAQUE)
			display_opaque_value(object, hflag,oflag, xflag);
		    /*sysctl.* leaves have node type,sysctl.name2id integer*/
		    else if ((object->type != CTLTYPE_NODE) &&
			     (object->id[0] != 0))
			display_basic_type(object);
		}
	    }
	}
	
	xo_emit("{L:\n}");
    }

    if (!IS_LEAF(object)) {
	if (Iflag)
	    xo_open_container("children");

	SLIST_FOREACH(child, object->children, object_link)
	    display_tree(child);
	
	if (Iflag)
	    xo_close_container("children");
    }

    xo_close_instance("object");
}

int main(int argc, char *argv[argc])
{
    int ch, error;
    struct sysctlmif_object *root, *nodelevel1;
    int idroot[1] = {0};
    size_t idrootlevel = 0;

    error = 0;

    aflag = bflag = Bflag = dflag = eflag = Fflag = fflag = 0;
    hflag = Iflag = iflag = lflag = Mflag = mflag = Nflag = 0;
    nflag = oflag = qflag = Sflag = Tflag = tflag = Wflag = 0;
    xflag = yflag = 0;

    atexit(xo_finish_atexit);

    xo_set_flags(NULL, XOF_UNITS);
    argc = xo_parse_args(argc, argv);
    if (argc < 0)
	exit(EXIT_FAILURE);

    while ((ch = getopt(argc, argv, "AadeFhiIlMmNnoqSTtWXxy")) != -1) {
	switch (ch) {
	case 'A':
	    aflag = 1;
	    oflag = 1;
	    break;
	case 'a':
	    aflag = 1;
	    break;
	case 'B':
	    Bflag = 1;
	    break;
	case 'b':
	    bflag = 1;
	    break;
	case 'd':
	    dflag = 1;
	    break;
	case 'e':
	    eflag = 1;
	    break;
	case 'F':
	    Fflag = 1;
	    break;
	case 'f':
	    fflag = 1;
	    break;
	case 'h':
	    hflag = 1;
	    break;
	case 'I':
	    Iflag = 1;
	    break;
	case 'i':
	    iflag = 1;
	    break;
	case 'l':
	    lflag = 1;
	    break;
	case 'M':
	    Mflag = 1;
	    break;
	case 'm':
	    mflag = 1;
	    break;
	case 'N':
	    Nflag = 1;
	    break;
	case 'n':
	    nflag = 1;
	    break;
	case 'o':
	    oflag = 1;
	    break;
	case 'q':
	    qflag = 1;
	    break;
	case 'S':
	    Sflag = 1;
	    break;
	case 'T':
	    Tflag = 1;
	    break;
	case 't':
	    tflag = 1;
	    break;
	case 'W':
	    Wflag = 1;
	    break;
	case 'w':
	    /* compatibility, ignored */
	    break;
	case 'X':
	    aflag = 1;
	    xflag = 1;
	    break;
	case 'x':
	    xflag = 1;
	    break;
	case 'y':
	    yflag = 1;
	    break;
	default:
	    usage();
	    return (1);
	}
    }
    argc -= optind;
    argv += optind;

    if (Mflag)
	xo_open_container("MIB");

    if (argc > 0) { /* the roots are given in input */
	aflag = 0; /* set to 0 for display_tree() */
	argc = 0;
	while (argv[argc]) {
	    parse_line_or_argv(argv[argc]);
	    argc++;
	}
    }
    else if (aflag) { /* the roots are objects with level 1 */
	xo_open_list("tree");
	root = sysctlmif_tree(idroot, idrootlevel, SYSCTLMIF_FALL,
			      SYSCTLMIF_MAXDEPTH);

	SLIST_FOREACH(nodelevel1, root->children, object_link)
	    display_tree(nodelevel1);

	sysctlmif_freetree(root);
	xo_close_list("tree");
    }
    else /* no roots and no -a */
	usage();

    if (Mflag)
	xo_close_container("MIB");

    return (error);
}
