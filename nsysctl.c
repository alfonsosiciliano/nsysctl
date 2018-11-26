#include <sys/queue.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <libxo/xo.h>

#include "libsysctl.h"


#define IS_LEAF(node) (node->childs == NULL || SLIST_EMPTY(node->childs))
#define GET_TYPE(node) (node->kind & CTLTYPE )

static const char *ctl_typename[CTLTYPE+1] = {
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

int Sflag, Iflag;
int aflag, dflag, eflag, nflag, oflag, xflag, hflag, tflag;
int yflag, kflag, lflag, Nflag;

void usage();
int filter_level_one(struct libsysctl_object*);
void parse_file(char *);
int parse_argv_or_line(char *);
void display_tree(struct libsysctl_object *);
void display_value(struct libsysctl_object*);
// delete: cc nsysctl.c libsysctl.c opaque.c -o nsysctl
void display_opaque_value(struct libsysctl_object*, int, int, int);

int main(int argc, char *argv[argc])
{
    int mib[CTL_MAXNAME];
    size_t miblevel;
    int ch;
    struct libsysctl_object *root;
    struct libsysctl_object_list *rootslist = NULL;

    atexit(xo_finish_atexit);
    xo_set_flags(NULL, XOF_UNITS);

    Sflag = Iflag = 0;
    aflag = dflag = eflag = hflag = nflag = oflag = xflag = tflag = 0;
    kflag = Nflag = 0;

    argc = xo_parse_args(argc, argv);
    if (argc < 0)
        exit(EXIT_FAILURE);
    
    while ((ch	= getopt(argc, argv, "adenNoxlklytSIh?")) != -1) {
	switch (ch) {
	case 'a':
	    aflag = 1;
	    break;
	case 'd':
	    dflag = 1;
	    break;
	case 'e':
	    eflag = 1;
	    break;
	case 'k':
	    kflag = 1;
	    break;
	case 'h':
	    hflag = 1;
	    break;
	case 'y':
	    yflag = 1;
	    break;
	case 'l':
	    lflag = 1;
	    break;
	case 'N':
	    Nflag=1;
	    break;
	case 'n':
	    nflag=1;
	    break;
	case 'o':
	    oflag=1;
	    break;
	case 'x':
	    xflag=1;
	    break;
	case 't':
	    tflag=1;
	    break;
	case 'S':
	    Sflag=1;
	    break;
	case 'I':
	    Iflag=1;
	    break;
	case '?':
	default:
	    usage();
	}
    }
    argc -= optind;
    argv += optind;

    mib[0] = Sflag ? 0 : 1;
    miblevel=1;

    /* get some "root tree" to pass to display_tree() */

    xo_open_container("MIB");
	
    if(argc > 0) // the roots are given in input
    {
	argc=0;
	while(argv[argc])
	{
	    parse_argv_or_line(argv[argc]);
	    argc++;
	}
    }
    else if (aflag) // ('-a' option) the roots are oids of level 1
    {
	rootslist = libsysctl_filterlist(filter_level_one, LIBSYSCTL_ALLFIELDS);
	xo_open_list("tree");
	while (!SLIST_EMPTY(rootslist))
	{
	    root = SLIST_FIRST(rootslist);
	    root = libsysctl_tree(root->id, root->idlen,
			       LIBSYSCTL_ALLFIELDS, LIBSYSCTL_MAXEDGES);
	    display_tree(root);
	    SLIST_REMOVE_HEAD(rootslist, object_link);
	    libsysctl_freetree(root);
	}
	xo_close_list("tree");
    }
    else // no roots
	usage();
    
    xo_close_container("MIB");
         
    return 0;
}


void usage()
{
    printf("Usage\n");
}


int filter_level_one(struct libsysctl_object* object)
{
    if(!Sflag && object->id[0] == 0)
	return -1;
	    
    if(object->idlen == 1)
	return 0;
    
    return -1;
}


void display_tree(struct libsysctl_object *object)
{   
    if(Iflag || IS_LEAF(object) )
    {
	xo_open_instance("object");

	if(!nflag)
	{
	    xo_emit("{:name/%s}",object->name);
	    if(!Nflag)
		eflag ? xo_emit("{L:=}"): xo_emit("{Pcw:}");
	}
	
	if(!Nflag)
	{
	    if(dflag) // printf description
		xo_emit("{:description/%s}",object->desc);
	    else if(tflag)
		xo_emit("{:type/%s}", ctl_typename[object->kind & CTLTYPE]);
	    else if(kflag)
		xo_emit("{:fmt/%x}",object->kind);
	    else if(lflag)
		xo_emit("{:label/%s}", object->label);
	    else if(yflag)
	    {
		xo_open_container("id");
		int i;
		for(i=0; i < object->idlen; i++)
		{
		    xo_emit("{:id/%x}", object->id[i]);
		    if(i+1<object->idlen)
			xo_emit("{L:.}");
		}
		xo_close_container("id");
	    }
	    else
		if(IS_LEAF(object))//print value
		    display_value(object);
	}
	xo_emit("{L:\n}");
    }
	
    struct libsysctl_object *child;

    if(object->childs != NULL)
	if(!SLIST_EMPTY(object->childs))
	{
	    if(Iflag)
		xo_open_container("childs");
	    SLIST_FOREACH(child, object->childs, object_link)
		display_tree(child);
	    if(Iflag)
		xo_close_container("childs");
	}

    xo_close_instance("object");
}


void display_value(struct libsysctl_object *object)
{
    unsigned int type = GET_TYPE(object);

    unsigned char value[BUFSIZ *100];
    size_t value_size=BUFSIZ *100;

    if((object->kind & CTLTYPE) == CTLTYPE_NODE)
	return;
    
    if(libsysctl_value(object->id,object->idlen,value,&value_size)<0)
    {
	if(!( (object->kind & CTLTYPE) & CTLTYPE_OPAQUE))
	{
	    printf("%s: Cannot get value\n",object->name);
	    return;
	}
    }
 
    switch(type)
    {
    case CTLTYPE_INT:
	xo_emit("{:value/%d}", *((int*)value) );
	break;
    case CTLTYPE_LONG:
	xo_emit("{:value/%ld}", *((long*)value) );
	break;
    case CTLTYPE_S8:
	xo_emit("{:value/%d}", *((int8_t*)value) );
	break;
    case CTLTYPE_S16:
	xo_emit("{:value/%d}", *((int16_t*)value) );
	break;
    case CTLTYPE_S32:
	xo_emit("{:value/%d}", *((int32_t*)value) );
	break;
    case CTLTYPE_S64:
	xo_emit("{:value/%ld}", *((int64_t*)value) );
	break;
    case CTLTYPE_UINT:
	xo_emit("{:value/%u}", *((u_int*)value) );
	break;
    case CTLTYPE_ULONG:
	xo_emit("{:value/%lu}", *((u_long*)value) );
	break;
    case CTLTYPE_U8:
	xo_emit("{:value/%u}", *((uint8_t*)value) );
	break;
    case CTLTYPE_U16:
	xo_emit("{:value/%u}", *((uint16_t*)value) );
	break;
    case CTLTYPE_U32:
	xo_emit("{:value/%u}", *((uint32_t*)value) );
	break;
    case CTLTYPE_U64:
	xo_emit("{:value/%lu}", *((uint64_t*)value) );
	break;
    case CTLTYPE_NODE:
	xo_emit("{:value/%s}", "--- TYPE NODE ---" );
	break;
    case CTLTYPE_STRING:
	xo_emit("{:value/%s}", (char*)value );
	break;
    case CTLTYPE_OPAQUE:
	display_opaque_value(object, hflag, oflag, xflag);
	break;
	/* #define	CTLTYPE_STRUCT	CTLTYPE_OPAQUE
	   case CTLTYPE_STRUCT:
	   break;
	*/
    default:
	printf("Error bad type!\n");
    }
}


int parse_argv_or_line(char* input)
{
    int id[CTL_MAXNAME];
    size_t idlevel= CTL_MAXNAME;
    char *tofree, *oid, *parsestring;
    struct libsysctl_object *object;

    parsestring = strdup(input);
    tofree = oid = strsep(&parsestring, "=");
    if(libsysctl_idbyname(oid, strlen(oid),
		       id, &idlevel) < 0)
    {
	printf("sysctl: unknown oid \'%s\'\n", oid);
	return 1;
    }

    if(strlen(oid) == strlen(input))
    {
	object = libsysctl_tree(id,idlevel,LIBSYSCTL_ALLFIELDS,LIBSYSCTL_MAXEDGES);
	display_tree(object);
	libsysctl_freetree(object);
    }
    else // a value is given
    {
 	object = libsysctl_object(id,idlevel,LIBSYSCTL_ALLFIELDS);
	switch(GET_TYPE(object))
	{
	case CTLTYPE_STRING:
	    libsysctl_newvalue(id,idlevel,parsestring,sizeof(parsestring));
	    break;
	case CTLTYPE_OPAQUE:
	    break;
	case CTLTYPE_NODE:
	    break;
	    /*
	case CTLTYPE_INT:
	    xo_emit("{:value/%d}", *((int*)value) );
	    break;
	case CTLTYPE_LONG:
	    xo_emit("{:value/%ld}", *((long*)value) );
	    break;
	case CTLTYPE_S8:
	    xo_emit("{:value/%d}", *((int8_t*)value) );
	    break;
	case CTLTYPE_S16:
	    xo_emit("{:value/%d}", *((int16_t*)value) );
	    break;
	case CTLTYPE_S32:
	    xo_emit("{:value/%d}", *((int32_t*)value) );
	    break;
	case CTLTYPE_S64:
	    xo_emit("{:value/%ld}", *((int64_t*)value) );
	    break;
	case CTLTYPE_UINT:
	    xo_emit("{:value/%u}", *((u_int*)value) );
	    break;
	case CTLTYPE_ULONG:
	    xo_emit("{:value/%lu}", *((u_long*)value) );
	    break;
	case CTLTYPE_U8:
	    xo_emit("{:value/%u}", *((uint8_t*)value) );
	    break;
	case CTLTYPE_U16:
	    xo_emit("{:value/%u}", *((uint16_t*)value) );
	    break;
	case CTLTYPE_U32:
	    xo_emit("{:value/%u}", *((uint32_t*)value) );
	    break;
	case CTLTYPE_U64:
	    xo_emit("{:value/%lu}", *((uint64_t*)value) );
	    break;
	    */
	default: // error
	    printf("Bad type to set\n");
	    break;	   
	}
    }

    free(tofree);

    return 0;
}

