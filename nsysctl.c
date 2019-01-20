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

#include "nsysctl.h"

#define IS_LEAF(node)    (node->children == NULL || SLIST_EMPTY(node->children))

void usage();
int filter_level_one(struct sysctlmif_object *);
int set_value(struct sysctlmif_object *, char *);
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

int main(int argc, char *argv[argc])
{
    int ch, error, rootid[SYSCTLMIF_IDMAXLEVEL];
    size_t rootidlevel = SYSCTLMIF_IDMAXLEVEL;
    struct sysctlmif_object *root;
    struct sysctlmif_object_list *rootslist = NULL;
    char *tofree, *rootname, *parsestring;

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

    if (argc > 0) /* the roots are given in input */
    {
	aflag = 0; /* set to 0 for display_tree() */
	argc = 0;
	while (argv[argc])
	{
	    parsestring = strdup(argv[argc]);
	    tofree = rootname = strsep(&parsestring, "=");
	    if (sysctlmif_nametoid(rootname, strlen(rootname) +1,
				   rootid, &rootidlevel) != 0) {
		if (!iflag)
		    error++;

		if (!iflag && !qflag)
		    printf("sysctl: unknown oid \'%s\'\n", rootname);

		free(tofree);
		argc++;
		continue;
	    }

	    if (strlen(rootname) == strlen(argv[argc])) {/* only "name" */
		root = sysctlmif_tree(rootid, rootidlevel,
				      SYSCTLMIF_FALL, SYSCTLMIF_MAXDEPTH);
		display_tree(root);
		sysctlmif_freetree(root);
	    }
	    else { /* a value is given*/
		root = sysctlmif_object(rootid, rootidlevel,
					SYSCTLMIF_FNAME | SYSCTLMIF_FTYPE);
		set_value(root, parsestring);
		sysctlmif_freeobject(root);
	    }

	    free(tofree);
	    argc++;
	}
    }
    
    else if (aflag) { /* the roots are objects with level 1 */
	rootslist = sysctlmif_filterlist(filter_level_one, SYSCTLMIF_FALL);
	xo_open_list("tree");
	while (!SLIST_EMPTY(rootslist)) {
	    root = SLIST_FIRST(rootslist);
	    root = sysctlmif_tree(root->id, root->idlevel,
				  SYSCTLMIF_FALL, SYSCTLMIF_MAXDEPTH);
	    display_tree(root);
	    SLIST_REMOVE_HEAD(rootslist, object_link);
	    sysctlmif_freetree(root);
	}
	xo_close_list("tree");
    }
    
    else /* no roots and no -a */
	usage();

    if (Mflag)
	xo_close_container("MIB");

    return (error);
}


void usage()
{
    printf("usage: nsysctl [-AadeFhiIlMmNnoqSTtWXxy] [ -B <bufsize> ] " \
	   "[-f filename] name[=value] ...\n");
    printf("       nsysctl [-AadeFhIlMmNnoqSTtWXxy] [ -B <bufsize> ] -a\n");
    printf("       nsysctl --libxo <libxo_options> [above options]\n");
}


int filter_level_one(struct sysctlmif_object *object)
{
    return object->idlevel == 1 ? 0 : -1;
}


/* Preorder visit */
void display_tree(struct sysctlmif_object *object)
{
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
	    else /* print value */
		if (IS_LEAF(object)) {
		    /* don't show opaque with -a opt */
		    if (object->type == CTLTYPE_OPAQUE) {
			display_opaque_value(object, hflag,
					     oflag, xflag);
		    }
		    /*sysctl.* leaves have node type,sysctl.name2id integer*/
		    else if ((object->type != CTLTYPE_NODE) &&
			     (strcmp(object->name,
				     "sysctl.name2oid") != 0)) {
			display_basic_type(object);
		    }
		}
	}
	xo_emit("{L:\n}");
    }


    struct sysctlmif_object *child;

    if (object->children != NULL) {
	if (!SLIST_EMPTY(object->children)) {
	    if (Iflag)
		xo_open_container("children");

	    SLIST_FOREACH(child, object->children, object_link)
		display_tree(child);
	    if (Iflag)
		xo_close_container("children");
	}
    }

    xo_close_instance("object");
}


void display_basic_type(struct sysctlmif_object *object)
{
    size_t value_size = 0;
    void *value;

    // BUG --libxo=xml => segmentation fault
    //if(strcmp(object->name,"debug.witness.fullgraph") ==0)
    //return;

    sysctl(object->id, object->idlevel, NULL, &value_size, NULL, 0);
    if ((value = malloc(value_size)) == NULL) {
	printf("%s: Cannot get value MALLOC\n", object->name);
	return;
    }
    memset(value, 0, value_size);

    if (sysctl(object->id, object->idlevel, value, &value_size, NULL, 0) < 0) {
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


int set_value(struct sysctlmif_object *object, char *input)
{
    int error = 0;

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
	xo_emit("{:input/%d}", *((int *)input));
	break;
    case CTLTYPE_LONG:
	xo_emit("{:input/%ld}", *((long *)input));
	break;
    case CTLTYPE_S8:
	xo_emit("{:input/%d}", *((int8_t *)input));
	break;
    case CTLTYPE_S16:
	xo_emit("{:input/%d}", *((int16_t *)input));
	break;
    case CTLTYPE_S32:
	xo_emit("{:input/%d}", *((int32_t *)input));
	break;
    case CTLTYPE_S64:
	xo_emit("{:input/%ld}", *((int64_t *)input));
	break;
    case CTLTYPE_UINT:
	xo_emit("{:input/%u}", *((u_int *)input));
	break;
    case CTLTYPE_ULONG:
	xo_emit("{:input/%lu}", *((u_long *)input));
	break;
    case CTLTYPE_U8:
	xo_emit("{:input/%u}", *((uint8_t *)input));
	break;
    case CTLTYPE_U16:
	xo_emit("{:input/%u}", *((uint16_t *)input));
	break;
    case CTLTYPE_U32:
	xo_emit("{:input/%u}", *((uint32_t *)input));
	break;
    case CTLTYPE_U64:
	xo_emit("{:input/%lu}", *((uint64_t *)input));
	break;
    default:
	xo_warnx("Unknown type");
	error = 1;
	break;
    }

    return (error);
}
