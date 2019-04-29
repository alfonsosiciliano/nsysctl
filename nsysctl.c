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

#define MAXSIZELINE 255
#define IS_LEAF(node)	(node->children == NULL || SLIST_EMPTY(node->children))

struct ctl_flag {
    unsigned int flag_bit;
    const char *flag_name;
};

#define NUM_CTLFLAGS 21

static const struct ctl_flag ctl_flags[NUM_CTLFLAGS] = {
    { CTLFLAG_RD, "RD" },
    { CTLFLAG_WR, "WR" },
    { CTLFLAG_RW, "RW" },
    { CTLFLAG_DORMANT, "DORMANT" },
    { CTLFLAG_ANYBODY, "ANYBODY" },
    { CTLFLAG_SECURE, "SECURE" },
    { CTLFLAG_PRISON, "PRISON" },
    { CTLFLAG_DYN, "DYN" },
    { CTLFLAG_SKIP, "SKIP" },
    { CTLMASK_SECURE, "SECURE" },
    { CTLFLAG_TUN, "TUN" },
    { CTLFLAG_RDTUN, "RDTUN" },
    { CTLFLAG_RWTUN, "RWTUN" },
    { CTLFLAG_MPSAFE, "MPSAFE" },
    { CTLFLAG_VNET, "VNET" },
    { CTLFLAG_DYING, "DYING" },
    { CTLFLAG_CAPRD, "CAPRD" },
    { CTLFLAG_CAPWR, "CAPWR" },
    { CTLFLAG_STATS, "STATS" },
    { CTLFLAG_NOFETCH, "NOFETCH" },
    { CTLFLAG_CAPRW, "CAPRW" }
};

struct ctl_type {
    char *name;
    size_t size;
    bool sign;
    char *fmt;
};

static const struct ctl_type ctl_types[CTLTYPE+1] = {
    { "ZEROUNUSED", 0, false , "error"},
    { "node", 0, false, "error"},
    { "integer", sizeof(int), true, "%d" },
    { "string", 0, false, "%s" },
    { "int64_t", sizeof(int64_t), true, "%ld" },
    { "opaque", 0, false, "error" },
    { "unsigned integer", sizeof(unsigned int), false, "%u" },
    { "long integer", sizeof(long int), true, "%ld" },
    { "unsigned long", sizeof(unsigned long), false, "%lu" },
    { "uint64_t", sizeof(uint64_t), false, "%lu" },
    { "uint8_t", sizeof(uint8_t), false, "%u" },
    { "uint16_t", sizeof(uint16_t), false, "%u" },
    { "int8_t", sizeof(int8_t), true, "%d" },
    { "int16_t", sizeof(int16_t), true, "%d" },
    { "int32_t", sizeof(int32_t), true, "%d" },
    { "uint32_t", sizeof(uint32_t), false, "%u" }
};


void usage(void);
int parse_line_or_argv(char *arg);
int display_tree(struct sysctlmif_object *object, char *newvalue);
int display_basic_type(struct sysctlmif_object *object, void *value, size_t valuesize);
int set_basic_value(struct sysctlmif_object *object, char *input);

bool aflag, bflag, dflag, Fflag, fflag, hflag, Gflag, gflag, Iflag;
bool iflag, lflag, Nflag, nflag, oflag, pflag, qflag, rflag, Sflag;
bool Tflag, tflag, Vflag, vflag, Wflag, xflag, yflag;
char *sep, *rflagstr;
unsigned int Bflagsize;

void usage()
{
    printf("usage:\n");
    printf("\tnsysctl [--libxo=opts [-r tagname]] [-DdFGgIilNpqTt[V|v[h[b|o|x]]]Wy]\n");
    printf("\t\t[-e sep] [-B <bufsize>] [-f filename] name[=value[,value]] ...\n");
    printf("\tnsysctl [--libxo=opts [-r tagname]] [-DdFGgIlNpqSTt[V|v[h[b|o|x]]]Wy]\n");
    printf("\t\t[-e sep] [-B <bufsize>] -A|a|X\n");
}

int main(int argc, char *argv[argc])
{
    int ch, error;
    struct sysctlmif_object *root, *nodelevel1;
    int idroot[1] = {0};
    size_t idrootlevel = 0;
    char *filename, line[MAXSIZELINE];
    FILE *fp;
    
    sep = ": ";
    error = 0;
    Bflagsize = 0;
    aflag = bflag = dflag = Fflag = fflag = Gflag = gflag = hflag = Iflag = false;
    iflag = lflag = Nflag = nflag = oflag = pflag = qflag = rflag = false;
    Sflag = Tflag = tflag = Vflag = vflag = Wflag = xflag = yflag = false;

    atexit(xo_finish_atexit);

    xo_set_flags(NULL, XOF_UNITS | XOF_FLUSH);
    argc = xo_parse_args(argc, argv);
    if (argc < 0)
	exit(EXIT_FAILURE);

    while ((ch = getopt(argc, argv, "AabDde:Ff:GghiIlNnopqr:STtVvWXxy")) != -1) {
	switch (ch) {
	case 'A': aflag = true; oflag = true; break;
	case 'a': aflag = true; break;
	case 'B': Bflagsize = (unsigned int) strtoull(optarg, NULL, 10);
	    break;
	case 'b': bflag = true; break;
	case 'd': dflag = true; break;
	case 'D': dflag = Fflag = lflag = Gflag = gflag = true;
	    Nflag = tflag = vflag = yflag = true;
	    break;
	case 'e': sep = optarg; break;
	case 'F': Fflag = true; break;
	case 'f': fflag = true; filename = optarg; break;
	case 'G': Gflag = true; break;
	case 'g': gflag = true; break;
	case 'h': hflag = true; break;
	case 'I': Iflag = true; break;
	case 'i': iflag = true; break;
	case 'l': lflag = true; break;
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

    if(fflag)
    {
	fp = fopen(filename, "r");
	if(fp == NULL)
	    xo_err(1, "cannot open: %s", filename);
	while(fgets(line,MAXSIZELINE,fp) != NULL) {
	    if(line[0] == '#' || line[0] == '\n')
		continue;
	    if(strchr(line, '\n') != NULL)
		strchr(line, '\n')[0] = '\0';
	    parse_line_or_argv(line);
	}
    }

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
	if(root == NULL)
	    xo_err(1, "cannot build the MIB-tree");

	SLIST_FOREACH(nodelevel1, root->children, object_link)
	    error += display_tree(nodelevel1, NULL);

	sysctlmif_freetree(root);
	xo_close_list("tree");
    }
    else if (!fflag){ /* no roots, no -a and no -f*/
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
	if(node == NULL)
	    xo_err(1, "cannot build the tree of '%s'", nodename);
	
	error = display_tree(node, NULL);
	sysctlmif_freetree(node);
    }
    else { /* nodename=value */
	/* FALL for fmt 'A' and for display_tree */
	node = sysctlmif_object(id, idlevel,
				SYSCTLMIF_FALL/*SYSCTLMIF_FNAME | SYSCTLMIF_FTYPE*/ );
	if(node == NULL)
	    xo_err(1, "cannot build the node to set '%s'", nodename);
	
	if(!IS_LEAF(node)) {
	    xo_warnx("oid \'%s\' isn't a leaf node",node->name);
	    error++;
	}
	else /* here node is a leaf */
	    error += display_tree(node, parsestring);
	
	sysctlmif_freeobject(node);
    }

    free(tofree);

    return error;
}

/* Preorder visit */
int display_tree(struct sysctlmif_object *object, char *newvalue)
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

    if(vflag || Vflag) // XXX && showable == true
    {
	if(Bflagsize > 0) {
	    value_size = Bflagsize;
	} 
	else {
	    /* XXX add an error check for this sysctl() */
	    sysctl(object->id, object->idlevel, NULL, &value_size, NULL, 0);
	    /*
	     * change value_size between 2 sysctl calls (e.g., kern.file,
	     * hw.dri.0.vblank and hw.dri.0.info.i915_drpc_info)
	     * /sbin/sysctl.c solution:
	     * j = 0;
	     * i = sysctl(oid, nlen, 0, &j, 0, 0);
	     * j += j; / * we want to be sure :-) * /
	     */
	    value_size += value_size;
	}
	
	if ((value = malloc(value_size)) == NULL) {
	    xo_err(1, "allocation memory to get the value of '%s'", object->name);
	    showable = false;
	}
	memset(value, 0, value_size);

	error =sysctl(object->id, object->idlevel, value, &value_size, NULL, 0);
	if (error != 0 || value_size == 0 || !IS_LEAF(object)) {
	    if(Vflag)
		showable = false;
	    showvalue = false;
	    /* XXX free(value) */
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
		if (i > 0)
		    xo_emit("{L:.}");
		snprintf(idlevelstr, sizeof(idlevelstr), "level%d", i+1);
		xo_emit_field(NULL, idlevelstr, (oflag || xflag) ? "%x" : "%d", NULL, object->id[i]);
	    }
	    xo_close_container("id");
	    showsep=true;
	}

#define XOEMITPROP(propname,content,value) do {		\
	    if(showsep)					\
		xo_emit("{L:/%s}",sep);			\
	    if (pflag)					\
		xo_emit("{L:[" propname "]: }");	\
	    xo_emit(content,value);			\
	    showsep = true;				\
	} while(0)

	if (Nflag)
	    XOEMITPROP("NAME","{:name/%s}", object->name);
	
	if (lflag) /* entry without label could return "\0" or NULL */
	    XOEMITPROP("LABEL","{:label/%s}", object->label == NULL ? "" : object->label);

	if (dflag) /* entry without descr could return "\0" or NULL */
	    XOEMITPROP("DESCRIPTION","{:description/%s}", object->desc == NULL ? "" : object->desc);
	
	if (tflag)
	    XOEMITPROP("TYPE","{:type/%s}", ctl_types[object->type].name);
	
	if (Fflag)
	    XOEMITPROP("FORMAT STRING","{:format/%s}", object->fmt);

	if (gflag)
	    XOEMITPROP("FLAGS","{:flags/%x}", object->flags);
	
	if (Gflag) {
	    if(showsep)
		xo_emit("{L:/%s}",sep);
	    if (pflag)
		xo_emit("{L:[TRUE-FLAGS]:}");
	    xo_open_container("true-flags");
	    for(i=0; i < NUM_CTLFLAGS; i++) {
		if(object->flags & ctl_flags[i].flag_bit)
		    xo_emit("{Lw:}{:flag/%s}",ctl_flags[i].flag_name);
	    }
	    xo_close_container("true-flags");
	    showsep = true;
	}

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
		error += display_opaque_value(object, value, value_size, hflag, oflag, xflag);
	    else if ( object->id[0] != 0)
		error += display_basic_type(object, value, value_size);

	    free(value);
	    showsep = true;
	}

	if(newvalue != NULL)
	    set_basic_value(object, newvalue);

	if(showsep)
	    xo_emit("{L:\n}");

    } /* end showable */

    /* visit children */
    if (!IS_LEAF(object)) {
	if (Iflag)
	    xo_open_container("children");

	SLIST_FOREACH(child, object->children, object_link)
	    error += display_tree(child, NULL);
	
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
    int i, error = 0, j;
    unsigned char *hexvalue;
    
    if (bflag) {
	for (i = 0; i < value_size; i++) {
	    xo_emit("{:raw/%c}", ((unsigned char*)(value))[i]);
	}
	return error;
    }
    
#define GTVL(typevar) do {						\
	for (i=0; i< value_size / sizeof( typevar); i++) {		\
	    if (i > 0)							\
		xo_emit("{Pw:}");					\
	    if(xflag) {							\
		hexvalue = (unsigned char *)  &((typevar *)value)[i];	\
		if(((typevar *)value)[i] !=0)				\
		    xo_emit("{L:0x}");					\
		else							\
		    xo_emit("{L:00}");					\
		for(j = ctl_types[object->type].size -1;j>=0; j--)	\
		    xo_emit("{:dump/%02x}", hexvalue[j]);		\
	    }								\
	    else							\
		xo_emit_field("", "value", ctl_types[object->type].fmt,	\
			      NULL, ((typevar *)value)[i] );		\
	}								\
    } while(0)
    
    switch (object->type) {
    case CTLTYPE_NODE:
	xo_emit("{:value/%s}", "--- TYPE NODE ---");
	break;
    case CTLTYPE_STRING:
	/* XXX 'if' can be deleted after the 'change value_size' fix*/
	if( ((char*)value)[value_size]!='\0')
	    ((char*)value)[value_size]='\0';
	xo_emit("{:value/%s}", (char *)value);
	break;
    case CTLTYPE_INT:   GTVL(int);      break;
    case CTLTYPE_LONG: 	GTVL(long);     break;
    case CTLTYPE_S8:	GTVL(int8_t);	break;
    case CTLTYPE_S16:	GTVL(int16_t);	break;
    case CTLTYPE_S32:	GTVL(int32_t);	break;
    case CTLTYPE_S64:	GTVL(int64_t);	break;
    case CTLTYPE_UINT:	GTVL(u_int);	break;
    case CTLTYPE_ULONG:	GTVL(u_long);	break;
    case CTLTYPE_U8:	GTVL(uint8_t);	break;
    case CTLTYPE_U16:	GTVL(uint16_t);	break;
    case CTLTYPE_U32:	GTVL(uint32_t);	break;
    case CTLTYPE_U64:	GTVL(uint64_t);	break;
    default:
	printf("%s, Error bad type!\n", object->name);
	error++;
    }

    return error;
}


int set_basic_value(struct sysctlmif_object *object, char *input)
{
    int error = 0;
    void *newval = NULL;
    size_t newval_size = 0;
    char *start, *next, *input_m, *end;
    
    if (Tflag || Wflag) {
	xo_warnx("Can't set variables when using -T or -W");
	return error++;
    }
    if (!(object->flags & CTLFLAG_WR)) {
	if (object->flags & CTLFLAG_TUN) {
	    xo_warnx("oid '%s' is a read only tunable", object->name);
	    xo_warnx("Tunable values are set in /boot/loader.conf");
	} else
	    xo_warnx("oid '%s' is read only", object->name);
	return error++;
    }
	

    /* XXX add Bflag support for setting, too */
    
#define STVL(typevar) do {						\
	input_m = strdup(input);					\
	start = input_m;						\
	end	= &input_m[strlen(input_m)+1];				\
	int i = 0;							\
	while(parse_string(start, &next, end, ',')) {			\
	    /* XXX warn fmt != 'A' */					\
	    newval_size += ctl_types[object->type].size;		\
	    newval = realloc(newval, ctl_types[object->type].size);	\
	    if(newval == NULL)						\
		xo_err(1, "realloc memory to set '%s'", object->name);	\
	    ((typevar *)newval)[i] = ctl_types[object->type].sign ?	\
		(typevar)strtoll(start, NULL, 10) :			\
		(typevar)strtoull(start, NULL, 10);			\
	    i++;							\
	    start = next;						\
	}								\
	free(input_m);							\
    } while(0)
    
    switch (object->type) {
    case CTLTYPE_STRING:
	newval = input;
	newval_size = strlen(input) + 1;
	break;
    case CTLTYPE_OPAQUE:
	xo_warnx("'%s' cannot set an opaque input", object->name);
	error++;
	break;
    case CTLTYPE_NODE:
	xo_warnx("oid \'%s\' isn't a leaf node",object->name);
	error++;
	break;
    case CTLTYPE_INT:   STVL(int);      break;
    case CTLTYPE_LONG:  STVL(long);     break;
    case CTLTYPE_S8:    STVL(int8_t);   break;
    case CTLTYPE_S16:   STVL(int16_t);  break;
    case CTLTYPE_S32:   STVL(int32_t);  break;
    case CTLTYPE_S64:   STVL(int64_t);  break;
    case CTLTYPE_UINT:  STVL(u_int);    break;
    case CTLTYPE_ULONG:	STVL(u_long);   break;
    case CTLTYPE_U8:    STVL(uint8_t);  break;
    case CTLTYPE_U16:   STVL(uint16_t); break;
    case CTLTYPE_U32:   STVL(uint32_t); break;
    case CTLTYPE_U64:   STVL(uint64_t); break;
    default:
	xo_warnx("Unknown type '%s'", object->name);
	error++;
	break;
    }
    
    if(error == 0) {
	if (newval_size == 0 && object->type != CTLTYPE_STRING) {
	    xo_warnx("empty numeric value");
	    error++;
	}
	else if(sysctl(object->id, object->idlevel, NULL, 0, newval, newval_size)==0)
	{
	    if(vflag || Vflag)
		xo_emit("{L: -> }{:newvalue/%s}",input);
	}
	else 
	{
	    xo_warn_c(errno, "\ncannot set new value %s",input);
	    error++;
	}
    }
    
    if(object->type != CTLTYPE_STRING && newval != NULL)
	free(newval);

    return (error);
}

