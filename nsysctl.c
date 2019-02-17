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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysctlmibinfo.h>
#include <unistd.h>

#include "opaque.h"
#include "special_value.h"

#define IS_LEAF(node)	(node->children == NULL || SLIST_EMPTY(node->children))

void usage(void);
int parse_line_or_argv(char *arg);
int display_tree(struct sysctlmif_object *object);
int display_basic_type(struct sysctlmif_object *object, void *value, size_t valuesize);
int set_basic_value(struct sysctlmif_object *object, char *input);

bool aflag, bflag, dflag, Fflag, fflag, hflag, Iflag, iflag, lflag, mflag;
bool Nflag, nflag, oflag, pflag, qflag, rflag, Sflag, Tflag, tflag, Vflag;
bool vflag, Wflag, xflag, yflag;
char *sep, *rflagstr;
unsigned int Bflagsize;

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

void usage()
{
    printf("usage:\n");
    printf("\tnsysctl [--libxo=opts [-r tagname]] [-DdFIilmNpqTt[-V|v[h[b|o|x]]]Wy]\n");
    printf("\t\t[-e sep] [-B <bufsize>] [-f filename] name[=value] ...\n");
    printf("\tnsysctl [--libxo=opts [-r tagname]] [-DdFIlmNpqSTt[-V|v[h[b|o|x]]]Wy]\n");
    printf("\t\t[-e sep] [-B <bufsize>] -A|a|X\n");
}

int main(int argc, char *argv[argc])
{
    int ch, error;
    struct sysctlmif_object *root, *nodelevel1;
    int idroot[1] = {0};
    size_t idrootlevel = 0;
    
    sep = ": ";
    error = 0;
    Bflagsize = 0;
    aflag = bflag = dflag = Fflag = fflag = hflag = Iflag = iflag = false;
    lflag = mflag = Nflag = nflag = oflag = pflag = qflag = rflag = false;
    Sflag = Tflag = tflag = Vflag = vflag = Wflag = xflag = yflag = false;

    atexit(xo_finish_atexit);

    xo_set_flags(NULL, XOF_UNITS/*XOF_FLUSH*/);
    argc = xo_parse_args(argc, argv);
    if (argc < 0)
	exit(EXIT_FAILURE);

    while ((ch = getopt(argc, argv, "AabDde:FhiIlmNnopqr:STtVvWXxy")) != -1) {
	switch (ch) {
	case 'A': aflag = true; oflag = true; break;
	case 'a': aflag = true; break;
	case 'B': Bflagsize = (unsigned int) strtoull(optarg, NULL, 10);
	    	  break;
	case 'b': bflag = true; break;
	case 'd': dflag = true; break;
	case 'D': dflag = Fflag = lflag = mflag = true;
	    	  Nflag = tflag = vflag = yflag = true;
		  break;
	case 'e': sep = optarg; break;
	case 'F': Fflag = true; break;
	case 'f': fflag = true; break;
	case 'h': hflag = true; break;
	case 'I': Iflag = true; break;
	case 'i': iflag = true; break;
	case 'l': lflag = true; break;
	case 'm': mflag = true; break;
	case 'N': Nflag = true; break;
	case 'n': nflag = true; break;
	case 'o': oflag = true; break;
	case 'p': pflag = true; break;
	case 'q': qflag = true; break;
	case 'r': rflag = true; rflagstr = optarg; break;
	case 'S': Sflag = true; break;
	case 'T': Tflag = true; break;
	case 't': tflag = true; break;
	case 'V': Vflag = true; break;
	case 'v': vflag = true; break;
	case 'W': Wflag = true; break;
	case 'w': /* compatibility, ignored */ break;
	case 'X': aflag = true; xflag = true; break;
	case 'x': xflag = true; break;
	case 'y': yflag = true; break;
	default:
	    usage();
	    return (1);
	}
    }
    argc -= optind;
    argv += optind;

    if (rflag)
	xo_open_container(rflagstr);

    if (argc > 0) { /* the roots are given in input */
	aflag = 0; /* set to 0 for display_tree() */
	argc = 0;
	while (argv[argc]) {
	    error += parse_line_or_argv(argv[argc]);
	    argc++;
	}
    }
    else if (aflag) { /* the roots are objects with level 1 */
	xo_open_list("tree");
	root = sysctlmif_tree(idroot, idrootlevel, SYSCTLMIF_FALL,
			      SYSCTLMIF_MAXDEPTH);

	SLIST_FOREACH(nodelevel1, root->children, object_link)
	    error += display_tree(nodelevel1);

	sysctlmif_freetree(root);
	xo_close_list("tree");
    }
    else { /* no roots and no -a */
	usage();
	error++;
    }

    if (rflag)
	xo_close_container(rflagstr);

    
    return (error);
}

int parse_line_or_argv(char *arg)
{
    char *tofree, *nodename, *parsestring;
    int error = 0;
    int id[SYSCTLMIF_MAXIDLEVEL];
    size_t idlevel = SYSCTLMIF_MAXIDLEVEL;
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
	node = sysctlmif_tree(id, idlevel, SYSCTLMIF_FALL, SYSCTLMIF_MAXDEPTH);
	error = display_tree(node);
	sysctlmif_freetree(node);
    }
    else { /* nodename=value */
	/* FALL for fmt 'A' */
	node = sysctlmif_object(id, idlevel, SYSCTLMIF_FNAME | SYSCTLMIF_FTYPE);
	if(!IS_LEAF(node)) {
	    xo_warnx("oid \'%s\' isn't a leaf node",node->name);
	    error++;
	}
	else
	    error += set_basic_value(node, parsestring);
		
	sysctlmif_freeobject(node);
    }

    free(tofree);

    return error;
}

/* Preorder visit */
int display_tree(struct sysctlmif_object *object)
{
    struct sysctlmif_object *child;
    bool showable = true, showsep = false, showvalue = true;
    int i, error = 0;
    char idlevelstr[7];
    size_t value_size = 0;
    void *value;

    if ((object->id[0] == 0) && !Sflag)
	showable = false;

    if (Wflag && !((object->flags & CTLFLAG_WR) && !(object->flags & CTLFLAG_STATS)))
	showable = false;

    if (Tflag && !(object->flags & CTLFLAG_TUN))
	showable = false;

    if (!Iflag && (!IS_LEAF(object)))
	showable = false;

    if(Vflag && (object->type == CTLTYPE_OPAQUE || object->type == CTLTYPE_NODE) && aflag && !xflag && !oflag && !is_opaque_defined(object))
 	showable = false;

    if(vflag || Vflag)
    {
	sysctl(object->id, object->idlevel, NULL, &value_size, NULL, 0);
	value_size += Bflagsize;
	if ((value = malloc(value_size)) == NULL) {
	    printf("%s: Cannot get value MALLOC\n", object->name);
	    showable = false;
	}
	memset(value, 0, value_size);

	error =sysctl(object->id, object->idlevel, value, &value_size, NULL, 0);
	if (error != 0 || value_size == 0 || !IS_LEAF(object)) {
	    if(Vflag)
		showable = false;
	    showvalue = false;
	}
    }
    
    if (showable)
    {
	xo_open_instance("object");

	if (yflag)
	{
	    xo_open_container("id");
	    if (pflag)
		xo_emit("{L:[ID]: }");
	    for (i = 0; i < object->idlevel; i++)
	    {
		snprintf(idlevelstr, sizeof(idlevelstr), "level%d", i+1);
		xo_emit_field(NULL, idlevelstr, "%x", NULL, object->id[i]);
		if (i+1 < object->idlevel)
		    xo_emit("{L:.}");
	    }
	    xo_close_container("id");
	    showsep=true;
	}

#define XOEMITPROP(propname,content,value) do {			\
	    if(showsep)						\
		xo_emit("{L:/%s}",sep);				\
	    if (pflag)						\
		xo_emit("{L:[" propname "]: }");		\
	    xo_emit(content,value);				\
	    showsep = true;					\
	} while(0)

	if (Nflag)
	    XOEMITPROP("NAME","{:name/%s}", object->name);
	
	if (lflag)
	    XOEMITPROP("LABEL","{:label/%s}", object->label);

	if (dflag) /* entry without descr could return "\0" or NULL */
	    XOEMITPROP("DESCRIPTION","{:description/%s}", object->desc == NULL ? "" : object->desc);
	
	if (tflag)
	    XOEMITPROP("TYPE","{:type/%s}", ctl_typename[object->type]);
	
	if (mflag)
	    XOEMITPROP("FORMAT STRING","{:format/%s}", object->fmt);

	if (Fflag)
	    XOEMITPROP("FLAGS","{:flags/%x}", object->flags);

	if(showvalue && (vflag || Vflag))
	{
	    if(showsep)
		xo_emit("{L:/%s}",sep);
	    if (pflag)
		xo_emit("{L:[VALUE]: }");

	    if (strncmp(object->fmt, "IK", 2) == 0)
		error += display_IK_value(object, value, value_size, hflag);
	    else if (is_special_value(object))
		error += display_special_value(object,value,value_size);
	    else if (object->type == CTLTYPE_OPAQUE || object->type == CTLTYPE_NODE)
		error += display_opaque_value(object, hflag, oflag, xflag);
	    else if ( object->id[0] != 0)
		error += display_basic_type(object, value, value_size);

	    //xo_emit("{L:appena}\n");

	    free(value);
	    showsep = true;
	}

	if(showsep)
		xo_emit("{L:\n}");

    } /* end showable */

    /* visit children */
    if (!IS_LEAF(object)) {
	if (Iflag)
	    xo_open_container("children");

	SLIST_FOREACH(child, object->children, object_link)
	    error += display_tree(child);
	
	if (Iflag)
	    xo_close_container("children");
    }

    if(showable)
	xo_close_instance("object");

    return error;
}


/*
 * this func will be merged with set_basic_value() in version 1.0 
 */
int display_basic_type(struct sysctlmif_object *object, void *value, size_t value_size)
{
    int i, error = 0;
    
    if (bflag) {
	for (i = 0; i < value_size; i++) {
	    xo_emit("{:raw/%c}", ((unsigned char*)(value))[i]);
	}
	return error;
    }

    if(xflag && object->type != CTLTYPE_STRING) {
	    xo_emit("{L:0x}");
	for (i = value_size-1; i >= 0; i--) {
	    xo_emit("{:dump/%02x}", ((unsigned char*)(value))[i]);
	}
	return error;
    }

#define GTVL(fmtstr, typevar) do {				\
	for (i=0; i< value_size / sizeof( typevar); i++) {	\
	    if (i != 0)						\
		xo_emit("{Pw:}");				\
	    xo_emit(fmtstr, ((typevar *)value)[i] );		\
	}							\
    } while(0)
    
    switch (object->type) {
    case CTLTYPE_INT:
	GTVL("{:value/%d}", int);
	break;
    case CTLTYPE_LONG:
	GTVL("{:value/%ld}", long);
	break;
    case CTLTYPE_S8:
	GTVL("{:value/%d}", int8_t);
	break;
    case CTLTYPE_S16:
	GTVL("{:value/%d}", int16_t);
	break;
    case CTLTYPE_S32:
	GTVL("{:value/%d}", int32_t);
	break;
    case CTLTYPE_S64:
	GTVL("{:value/%ld}", int64_t);
	break;
    case CTLTYPE_UINT:
	GTVL("{:value/%u}", u_int);
	break;
    case CTLTYPE_ULONG:
	GTVL("{:value/%lu}", u_long);
	break;
    case CTLTYPE_U8:
	GTVL("{:value/%u}", uint8_t);
	break;
    case CTLTYPE_U16:
	GTVL("{:value/%u}", uint16_t);
	break;
    case CTLTYPE_U32:
	GTVL("{:value/%u}", uint32_t);
	break;
    case CTLTYPE_U64:
	GTVL("{:value/%lu}", uint64_t);
	break;
    case CTLTYPE_NODE:
	xo_emit("{:value/%s}", "--- TYPE NODE ---");
	break;
    case CTLTYPE_STRING:
	if( ((char*)value)[value_size]!='\0')
	    ((char*)value)[value_size]='\0';
	xo_emit("{:value/%s}", (char *)value);
	break;
    default:
	printf("%s, Error bad type!\n", object->name);
	error++;
    }

    return error;
}


int set_basic_value(struct sysctlmif_object *object, char *input)
{
    long long llivalue = strtoll(input, NULL, 10);
    unsigned long long ullvalue =strtoull(input, NULL, 10);
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
    int error = 0;
    
    llsize=sizeof(long long);
    ullsize=sizeof(unsigned long long);
    
    switch (object->type) {
    case CTLTYPE_STRING:
	sysctl(object->id, object->idlevel, NULL, 0,
	       input, sizeof(input));
	break;
    case CTLTYPE_OPAQUE:
	xo_warnx("Cannot set an opaque input");
	error++;
	break;
    case CTLTYPE_NODE:
	xo_warnx("oid \'%s\' isn't a leaf node",object->name);
	error++;
	break;

#define STVL(typedvalue, type, longinput, formatstr)	\
    do {						\
	typedvalue = (type)longinput;			\
	if(sysctl(object->id,object->idlevel,NULL,0,	\
		  & typedvalue,sizeof(type)) == 0)	\
	{						\
	    xo_emit("{L: -> }");			\
	    xo_emit_field("", "newvalue", formatstr,	\
			  NULL, typedvalue);		\
	    xo_emit("{L:\n}");				\
	}						\
	else {						\
	    xo_warnx("cannot set new value %s",input);	\
	    error++;					\
	}						\
    } while(0)

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
	error++;
	break;
    }

    return (error);
}

