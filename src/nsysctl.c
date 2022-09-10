/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018-2021 Alfonso Sabato Siciliano
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

#include <sys/types.h>
#include <sys/sysctl.h>

#include <inttypes.h>
#include <libutil.h>
#include <libxo/xo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysctlmibinfo2.h>
#include <unistd.h>

#include "opaque.h"
#include "special_value.h"

#define VERSION "2.1"
#define MAXSIZELINE 255
#define IS_LEAF(node)	(node->children == NULL || SLIST_EMPTY(node->children))

struct ctl_flag {
    unsigned int flag_bit;
    const char *flag_name;
};

#define NUM_CTLFLAGS 22
#ifndef CTLFLAG_NEEDGIANT
#define CTLFLAG_NEEDGIANT 0x00000800
#endif
static const struct ctl_flag ctl_flags[NUM_CTLFLAGS] = {
    { CTLFLAG_RD,        "RD"       },
    { CTLFLAG_WR,        "WR"       },
    { CTLFLAG_RW,        "RW"       },
    { CTLFLAG_DORMANT,   "DORMANT"  },
    { CTLFLAG_ANYBODY,   "ANYBODY"  },
    { CTLFLAG_SECURE,    "SECURE"   },
    { CTLFLAG_PRISON,    "PRISON"   },
    { CTLFLAG_DYN,       "DYN"      },
    { CTLFLAG_SKIP,      "SKIP"     },
    { CTLMASK_SECURE,    "SECURE"   },
    { CTLFLAG_TUN,       "TUN"      },
    { CTLFLAG_RDTUN,     "RDTUN"    },
    { CTLFLAG_RWTUN,     "RWTUN"    },
    { CTLFLAG_MPSAFE,    "MPSAFE"   },
    { CTLFLAG_VNET,      "VNET"     },
    { CTLFLAG_DYING,     "DYING"    },
    { CTLFLAG_CAPRD,     "CAPRD"    },
    { CTLFLAG_CAPWR,     "CAPWR"    },
    { CTLFLAG_STATS,     "STATS"    },
    { CTLFLAG_NOFETCH,   "NOFETCH"  },
    { CTLFLAG_CAPRW,     "CAPRW"    },
    { CTLFLAG_NEEDGIANT, "NEEDGIANT"}
};

struct ctl_type {
    char *name;
    size_t size;
};

static const struct ctl_type ctl_types[CTLTYPE+1] = {
    { "ZEROUNUSED",       0                     },
    { "node",             0                     },
    { "integer",          sizeof(int)           },
    { "string",           0                     },
    { "int64_t",          sizeof(int64_t)       },
    { "opaque",           0                     },
    { "unsigned integer", sizeof(unsigned int)  },
    { "long integer",     sizeof(long int)      },
    { "unsigned long",    sizeof(unsigned long) },
    { "uint64_t",         sizeof(uint64_t)      },
    { "uint8_t",          sizeof(uint8_t)       },
    { "uint16_t",         sizeof(uint16_t)      },
    { "int8_t",           sizeof(int8_t)        },
    { "int16_t",          sizeof(int16_t)       },
    { "int32_t",          sizeof(int32_t)       },
    { "uint32_t",         sizeof(uint32_t)      }
};

void usage(void);
int parse_line_or_argv(char *arg);
int display_tree(struct sysctlmif_object *root);
int visit_object(struct sysctlmif_object *object, char *newvalue, bool toggle,
    bool *printed);
int display_basic_value(struct sysctlmif_object *object, void *value, size_t valuesize);
int set_basic_value(struct sysctlmif_object *object, char *input);

bool aflag, bflag, dflag, Fflag, fflag, hflag, Gflag, gflag, Hflag;
bool Iflag, iflag, kflag, lflag, Nflag, nflag, Oflag, oflag, pflag;
bool qflag, rflag, Sflag, Tflag, tflag, Vflag, Wflag, xflag, zflag;
char *sep, *rflagstr;
unsigned int Bflagsize;

void usage()
{

    printf("usage: nsysctl [--libxo options [-r tagroot]] [-DdeFGgHIilnOpqTtvWz]\n");
    printf("               [-N | -h [b | o | x]] [-B bufsize] [-f filename] [-s sep]\n");
    printf("               name[=value[,value]] ...\n");
    printf("       nsysctl [--libxo options [-r tagroot]] [-DdeFGgHIklnOpqSTtvWz]\n");
    printf("               [-N | -Vh [b | o | x]] [-B bufsize] [-s sep] -a\n");
}

int main(int argc, char *argv[argc])
{
    int ch, error;
    struct sysctlmif_object *topobject = NULL;
    struct sysctlmif_list *mib;
    char *filename, line[MAXSIZELINE];
    FILE *fp;
    
    sep = ": ";
    error = 0;
    Bflagsize = 0;
    aflag = bflag = dflag = Fflag = fflag = Gflag = gflag = Hflag = hflag = false;
    Iflag = iflag = kflag = lflag = Nflag = nflag = Oflag = oflag = pflag = false;
    qflag = rflag = Sflag = Tflag = tflag = Vflag = Wflag = xflag = zflag = false;

    atexit(xo_finish_atexit);
    xo_set_flags(NULL, XOF_UNITS | XOF_FLUSH);
    if ((argc = xo_parse_args(argc, argv)) < 0)
	exit(EXIT_FAILURE);

    if(kld_isloaded("sysctlinfo") == 0)
        xo_errx(1, "\'sysctlinfo\' kmod unloaded");

    while ((ch = getopt(argc, argv, "AaB:bDdeFf:GgHhiIklmNnOopqr:Ss:TtVvWwXxyz")) != -1) {
	switch (ch) {
	case 'A': aflag = true; oflag = true; break;
	case 'a': aflag = true; break;
	case 'B': Bflagsize = (unsigned int) strtoull(optarg, NULL, 10); break;
	case 'b': bflag = true; break;
	case 'd': dflag = true; break;
	case 'D': dflag = Fflag = Gflag = Hflag = lflag = Oflag = tflag = true;
	    break;
	case 'e': sep = "="; break;
	case 'F': Fflag = true; break;
	case 'f': fflag = true; filename = optarg; break;
	case 'G': Gflag = true; break;
	case 'g': gflag = true; break;
	case 'H': Hflag = true; break;
	case 'h': hflag = true; break;
	case 'I': Iflag = true; break;
	case 'i': iflag = true; break;
	case 'k': kflag = true; break;
	case 'l': lflag = true; break;
	case 'm': Sflag = true; break; /* compatibility <= 1.2.1 */
	case 'N': Nflag = true; break;
	case 'n': nflag = true; break;
	case 'O': Oflag = true; break;
	case 'o': oflag = true; break;
	case 'p': pflag = true; break;
	case 'q': qflag = true; break;
	case 'r': rflag = true; rflagstr = optarg; break;
	case 'S': Sflag = true; break;
	case 's': sep = optarg; break;
	case 'T': Tflag = true; break;
	case 't': tflag = true; break;
	case 'V': Vflag = true; break;
	case 'v': printf("nsysctl %s\n", VERSION); return(0);
	case 'W': Wflag = true; break;
	case 'w': /* compatibility, ignored */ break;
	case 'X': aflag = true; xflag = true; break;
	case 'x': xflag = true; break;
	case 'y': Oflag = true; break;  /* compatibility <= 1.2.1 */
	case 'z': zflag = true; break;
	default:
	    usage();
	    return (1);
	}
    }
    argc -= optind;
    argv += optind;
    
    if(Nflag && Vflag)
	xo_errx(1, "-N and -V are mutually exclusive");

    if( Nflag && (bflag || xflag || oflag || Vflag || hflag))
	xo_errx(1, "-N and [-Vhbox] are mutually exclusive");

    if((bflag && oflag) || (bflag && xflag) || (oflag && xflag))
	xo_errx(1, "-[b|o|x] are mutually exclusive");

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
	    error += parse_line_or_argv(line);
	}
    }

    if (argc > 0) { /* objects in input */
	aflag = false; /* important for display_tree() */
	argc = 0;
	while (argv[argc]) {
	    error += parse_line_or_argv(argv[argc]);
	    argc++;
	}
    }
    else if (aflag) { /* -a flag and no object in input */
	xo_open_list("tree");
	if((mib = sysctlmif_mib()) == NULL)
	    xo_err(1, "cannot build the MIB-tree");

	if (!Sflag) {
		topobject = SLIST_FIRST(mib);
		topobject = SLIST_NEXT(topobject, object_link);
	}
	/* objests have level 1 */
	SLIST_FOREACH_FROM(topobject, mib, object_link)
	    error += display_tree(topobject);

	sysctlmif_freemib(mib);
	xo_close_list("tree");
    }
    else if (!fflag){ /* no object, no -a, no -f */
	usage();
	error++;
    }

    if (rflag)
	xo_close_container(rflagstr);

    return (error);
}

int parse_line_or_argv(char *arg)
{
    char *name, *valuestr;
    int error = 0;
    struct sysctlmif_object *node;
    bool printed;

    name = arg;
    valuestr = strchr(arg, '=');
    if(valuestr != NULL) {
	*valuestr='\0';
	valuestr++;
    }

    node = sysctlmif_treebyname(name);
    if (node == NULL) {
	if (errno == ENOMEM) { /* malloc error */
	    xo_err(1, "cannot build the tree of '%s'", name);
	} else { /* nodename does not exist */
	    if (!iflag)
		error++;
	    if (!iflag && !qflag)
		xo_warnx("unknow \'%s\' oid", name);
	}
    }
    else if (valuestr == NULL) { /* only nodename */
	if (zflag && IS_LEAF(node)) { /* leaf */
	    error = visit_object(node, NULL, true, &printed);
	    if (printed)
	        xo_close_instance("object");
	} else /* internal node */
	    error = display_tree(node);
    }
    else { /* nodename=value */
	if (!IS_LEAF(node)) {
	    xo_warnx("oid \'%s\' isn't a leaf node", node->name);
	    error++;
	} else { /* leaf */
	    error = visit_object(node, valuestr, false, &printed);
	    if (printed)
	        xo_close_instance("object");
        }
    }

    sysctlmif_freetree(node);

    return error;
}


int display_tree(struct sysctlmif_object *root)
{
    struct sysctlmif_object *child;
    int error = 0;
    bool printed;
    
    if ((error = visit_object(root, NULL, false, &printed)) != 0)
    	return error;

    if (!IS_LEAF(root)) {
	if (Iflag && printed)
	    xo_open_container("children");

	SLIST_FOREACH(child, root->children, object_link)
	    error += display_tree(child);
	
	if (Iflag && printed)
	    xo_close_container("children");
    }
    
    if (printed)
    	xo_close_instance("object");
    
    return error;
}



int visit_object(struct sysctlmif_object *object, char *newvalue, bool toggle,
    bool *printed)
{
    bool showsep = false, showvalue, hashandler;
    int i, error = 0;
    size_t value_size = 0;
    void *value;
    char *oid = NULL, oidlevel[100]; /* MAX_INT in char digits */
    
    *printed = false;

    if (Wflag && !((object->flags & CTLFLAG_WR) && !(object->flags & CTLFLAG_STATS)))
	return error;

    if (Tflag && !(object->flags & CTLFLAG_TUN))
	return error;

    if (!Iflag && (!IS_LEAF(object)))
	return error;

    if (aflag && !kflag && (object->flags & CTLFLAG_SKIP))
	return error;

    showvalue = (!Nflag || Vflag) && IS_LEAF(object);

    if (showvalue && !Vflag && aflag && 
      (object->type == CTLTYPE_OPAQUE || object->type == CTLTYPE_NODE) &&
       !xflag && !oflag && !is_opaque_defined(object))
	  return error;

    if(showvalue && !Vflag && aflag && !IS_LEAF(object))
	return error;

    value = NULL;
    if (showvalue || toggle)
    {
	if (Bflagsize > 0) {
	    value_size = Bflagsize;
	} else {
	    sysctl(object->id, object->idlevel, NULL, &value_size, NULL, 0);
	    // value_size change with 2 sysctl calls (e.g., hw.dri.0.vblank,
	    // kern.file and hw.dri.0.info.i915_drpc_info), /sbin/sysctl.c solution:
	    value_size += value_size;
	}

	if ((value = malloc(value_size)) == NULL) {
	    xo_err(1, "allocation memory to get the value of '%s'", object->name);
	    return error;
	}
	memset(value, 0, value_size);

	error =sysctl(object->id, object->idlevel, value, &value_size, NULL, 0);
	if (error != 0 || (value_size == 0 && object->type == CTLTYPE_OPAQUE)) {
	    free(value);
	    // avoid the double free() down, otherwise clang 14.0.5 frees
	    // random object->* members causing segmentation fault
	    value = NULL;
	    if (!Vflag && aflag)
		return error;
	    else
	    	showvalue = false;
	}
    }

    /* print object */
    *printed = true;
    xo_open_instance("object");

#define XOEMITPROP(propname, content, propvalue) do {	\
	if(showsep)					\
	    xo_emit("{L:/%s}",sep);			\
	if (pflag)					\
	    xo_emit("{L:[" propname "]: }");		\
	xo_emit(content, propvalue);			\
	showsep = true;					\
    } while(0)

    if (Oflag)
    {
	for (i = 0; i < object->idlevel; i++)
	{
	    snprintf(oidlevel, sizeof(oidlevel), xflag ? "%x." : "%d.", object->id[i]);
	    oid = realloc((void*)oid, strlen(oidlevel) + 2); /* check NULL */
	    if (i == 0)
	        (oid[0] = '\0');
	    memcpy(oid + strlen(oid), &(oidlevel[0]), strlen(oidlevel)+1);
	}
	oid[strlen(oid) - 1] = '\0';
	XOEMITPROP("OID","{:OID/%s}", oid);
	free(oid);
    }

    if (!nflag)
	XOEMITPROP("NAME","{:name/%s}", object->name);

    if (lflag) /* entry without label could return "\0" or NULL */
	XOEMITPROP("LABEL","{:label/%s}", 
		   object->label == NULL ? "" : object->label);

    if (dflag) /* entry without descr could return "\0" or NULL */
	XOEMITPROP("DESCRIPTION","{:description/%s}", 
		   object->desc == NULL ? "" : object->desc);

    if (tflag)
	XOEMITPROP("TYPE","{:type/%s}", ctl_types[object->type].name);

    if (Fflag)
	XOEMITPROP("FORMAT","{:format/%s}", object->fmt);

    if (gflag)
	XOEMITPROP("FLAGS-RAW", xflag ? "{:flags-raw/%x}" : "{:flags-raw/%u}", 
		   object->flags);

    if (Gflag) {
	if(showsep)
	    xo_emit("{L:/%s}",sep);
	if (pflag)
	    xo_emit("{L:[FLAGS]:}");
	xo_open_container("flags");
	for(i=0; i < NUM_CTLFLAGS; i++) {
	    if( (object->flags & ctl_flags[i].flag_bit) == ctl_flags[i].flag_bit)
		xo_emit("{Lw:}{:flag/%s}",ctl_flags[i].flag_name);
	}
	xo_close_container("flags");
	showsep = true;
    }

    if (Hflag) {
	if (sysctlmif_hashandler(object->id, object->idlevel, &hashandler) !=0)
		xo_err(1, "cannot get handler %s", object->name);
	XOEMITPROP("HANDLER","{:handler/%s}", hashandler ? "Defined" : "Undefined");
    }

    if (showvalue && (!Nflag|| Vflag))
    {
	if(showsep)
	    xo_emit("{L:/%s}",sep);
	if (pflag)
	    xo_emit("{L:[VALUE]: }");

	if (is_special_value(object))
	    error += display_special_value(object,value,value_size);
	else if (object->type == CTLTYPE_OPAQUE || object->type == CTLTYPE_NODE)
	    error += display_opaque_value(object, value, value_size, tflag,
	        hflag, oflag, xflag);
	else if ( object->id[0] != 0)
	    error += display_basic_value(object, value, value_size);

	showsep = true;
    }

    if (toggle) {
	if (object->type == CTLTYPE_STRING) {
	    xo_emit("{L:\n}");
	    xo_warnx("'%s' cannot toggle a string", object->name);
	    error++;
	} else {
	    newvalue = "1";
	    for (i=0; i < value_size; i++) {
		if (((int8_t*)value)[i] != 0) {
		    newvalue = "0";
		    break;
		}
	    }
	}
    }

    if (value != NULL)
	free(value);

    if(newvalue != NULL)
	if(set_basic_value(object, newvalue) != 0)
	    showsep = false;

    if(showsep)
	xo_emit("{L:\n}");

    /* caller has to call: xo_close_instance("object") */

    return error;
}


int display_basic_value(struct sysctlmif_object *object, void *value, size_t value_size)
{
    int i, error = 0, j;
    unsigned char *hexvalue;
    uintmax_t zero = 0;
    char *hfield = hflag ? "hn" : NULL;

    if (object->type == CTLTYPE_NODE) {
	xo_warnx("'%s' is a node", object->name);
	return 1;
    }

    if (bflag) {
	for (i = 0; i < value_size; i++) {
	    xo_emit("{:raw/%c}", ((unsigned char*)(value))[i]);
	}
	return 0;
    }
    
    if (object->type == CTLTYPE_STRING) {
	if( ((char*)value)[value_size]!='\0')
	    ((char*)value)[value_size]='\0';
	xo_emit("{:value/%s}", (char *)value);
	return 0;
    }

    for (i=0; i< value_size / ctl_types[object->type].size; i++) {
	if (i > 0)
	    xo_emit("{Pw:}");
	if(xflag) {
	    hexvalue = &value[i * ctl_types[object->type].size];
	    if(memcmp(hexvalue, &zero, ctl_types[object->type].size) == 0)
		xo_emit("{L:00}");
	    else
		xo_emit("{L:0x}");
	    for(j = ctl_types[object->type].size -1; j>=0; j--)
		xo_emit("{:dump/%02x}", hexvalue[j]);
	    
	    continue;
	}

	switch (object->type) {
	case CTLTYPE_INT:
	    if (strncmp(object->fmt, "IK", 2) == 0)
		error += display_IK_value(object, value, value_size, hflag);
	    else
		xo_emit_field(hfield, "value", "%d", NULL, ((int*)value)[i]);
	    break;
	case CTLTYPE_LONG:  
	    xo_emit_field(hfield, "value", "%ld", NULL, ((long*)value)[i]);    
	    break;
	case CTLTYPE_S8:    
	    xo_emit_field(hfield, "value", "%d",  NULL, ((int8_t*)value)[i]);  
	    break;
	case CTLTYPE_S16:   
	    xo_emit_field(hfield, "value", "%d",  NULL, ((int16_t*)value)[i]); 
	    break;
	case CTLTYPE_S32:   
	    xo_emit_field(hfield, "value", "%d",  NULL, ((int32_t*)value)[i]); 
	    break;
	case CTLTYPE_S64:   
	    xo_emit_field(hfield, "value", "%ld", NULL, ((int64_t*)value)[i]); 
	    break;
	case CTLTYPE_UINT:  
	    xo_emit_field(hfield, "value", "%u",  NULL, ((u_int*)value)[i]);   
	    break;
	case CTLTYPE_ULONG: 
	    xo_emit_field(hfield, "value", "%lu", NULL, ((u_long*)value)[i]);  
	    break;
	case CTLTYPE_U8:    
	    xo_emit_field(hfield, "value", "%u",  NULL, ((uint8_t*)value)[i]); 
	    break;
	case CTLTYPE_U16:   
	    xo_emit_field(hfield, "value", "%u",  NULL, ((uint16_t*)value)[i]);
	    break;
	case CTLTYPE_U32:   
	    xo_emit_field(hfield, "value", "%u",  NULL, ((uint32_t*)value)[i]);
	    break;
	case CTLTYPE_U64:   
	    xo_emit_field(hfield, "value", "%lu", NULL, ((uint64_t*)value)[i]);
	    break;
	default:
	    xo_warnx("'%s' unknown type", object->name);
	    error++;
	}
    }

    return error;
}


int set_basic_value(struct sysctlmif_object *object, char *input)
{
    int error = 0, kelvin, i;
    void *newval = NULL;
    size_t newval_size = 0;
    char *start, *next, *input_m, *end;

    if (Tflag || Wflag) {
	xo_emit("{L:\n}");
	xo_warnx("Can't set variables when using -T or -W");
	return 1;
    }
    if (!(object->flags & CTLFLAG_WR)) {
	xo_emit("{L:\n}");
	if (object->flags & CTLFLAG_TUN) {
	    xo_warnx("oid '%s' is a read only tunable", object->name);
	    xo_warnx("Tunable values are set in /boot/loader.conf");
	} else
	    xo_warnx("oid '%s' is read only", object->name);
	return 1;
    }
    if(object->type == CTLTYPE_OPAQUE) {
	xo_emit("{L:\n}");
	xo_warnx("'%s' cannot set an opaque input", object->name);
	return 1;
    }
    if(object->type == CTLTYPE_NODE) {
	xo_emit("{L:\n}");
	xo_warnx("oid \'%s\' isn't a leaf node",object->name);
	return 1;
    }

    /* the object is settable */

    if(Bflagsize > 0) {
	newval_size = Bflagsize;
	newval = malloc(newval_size);
	if(newval == NULL){
	    xo_emit("{L:n}");	
	    xo_err(1, "malloc() to set '%s'", object->name);
	}
	memset(newval, 0 , newval_size);
    }					
    
    if(object->type == CTLTYPE_STRING) {
	if(Bflagsize > 0) {
	    strncpy(newval, input, Bflagsize);
	} else {
	    newval = input;
	    newval_size = strlen(input) + 1;
	}
    }
    else // numeric value
    {
	input_m = strdup(input);
	start = input_m;
	end = &input_m[strlen(input_m)+1];

	i=0;
	while(parse_string(start, &next, end, ',')) {
	    // some value is an array but fmt != A (e.g. kern.cp_times)
	    if(Bflagsize <= 0) {
		newval_size += ctl_types[object->type].size;
		newval = realloc(newval, ctl_types[object->type].size);
		if(newval == NULL){
		    xo_emit("{L:n}");
		    xo_err(1, "realloc() to set '%s'", object->name);
		}
	    }

	    switch (object->type) {
	    case CTLTYPE_INT:
		if (strncmp(object->fmt, "IK", 2) == 0) {
		    newval_size += ctl_types[object->type].size; // sizeof(int)
		    error += strIK_to_int(input, &kelvin, object->fmt);
		    ((int*)newval)[i] = kelvin;
		} else {
		    ((int *)newval)[i] = (int)strtoll(start, NULL, 0); 
		}
		break;
	    case CTLTYPE_LONG:  
		((long *)newval)[i] = (long)strtoll(start, NULL, 0);     
		break;
	    case CTLTYPE_S8:    
		((int8_t *)newval)[i] = (int8_t)strtoll(start, NULL, 0);   
		break;
	    case CTLTYPE_S16:   
		((int16_t *)newval)[i] = (int16_t)strtoll(start, NULL, 0);  
		break;
	    case CTLTYPE_S32:   
		((int32_t *)newval)[i] = (int32_t)strtoll(start, NULL, 0);  
		break;
	    case CTLTYPE_S64:   
		((int64_t *)newval)[i] = (int64_t)strtoimax(start, NULL, 0);  
		break;
	    case CTLTYPE_UINT:  
		((u_int *)newval)[i] = (u_int)strtoull(start, NULL, 0);   
		break;
	    case CTLTYPE_ULONG: 
		((u_long *)newval)[i] = (u_long)strtoull(start, NULL, 0);  
		break;
	    case CTLTYPE_U8:    
		((uint8_t *)newval)[i] = (uint8_t)strtoull(start, NULL, 0); 
		break;
	    case CTLTYPE_U16:   
		((uint16_t *)newval)[i] = (uint16_t)strtoull(start, NULL, 0);
		break;
	    case CTLTYPE_U32:   
		((uint32_t *)newval)[i] = (uint32_t)strtoull(start, NULL, 0);
		break;
	    case CTLTYPE_U64:   
		((uint64_t *)newval)[i] = (uint64_t)strtoumax(start, NULL, 0);
		break;
	    default:
		xo_emit("{L:\n}");
		xo_warnx("Unknown type '%s'", object->name);
		error++;
		break;
	    }// end switch
	    if(errno == EINVAL || errno == ERANGE) {
		error++;
		xo_warn_c(errno, " cannot set new value '%s'", input);
		break;
	    }
	    i++;
	    start = next;
	}// end while
	free(input_m);
    } // else numeric value

    if (error == 0) {
	if (newval_size == 0 && object->type != CTLTYPE_STRING) {
	    xo_emit("{L:\n}");
	    xo_warnx("empty numeric value");
	    error++;
	} 
	else if(sysctl(object->id, object->idlevel, NULL, 0, newval, newval_size)==0) {
	    if (!Nflag || Vflag)
		xo_emit("{L: -> }{:newvalue/%s}",input);
	} 
	else {
	    xo_emit("{L:\n}");
	    xo_warn_c(errno, " cannot set new value '%s'",input);
	    error++;
	}
    }

    if ((object->type != CTLTYPE_STRING || Bflagsize > 0) && newval != NULL)
	free(newval);

    return (error);
}
