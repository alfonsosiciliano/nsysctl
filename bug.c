/*
 * % cc bug.c -lxo -o bug
 * % ./bug
 * <OK>
 * % ./bug --libxo=text
 * <OK>
 * % ./bug --libxo=xml,pretty
 * <valuelen>34952</valuelen>
 * Segmentation fault (core dumped)
 */

#include <sys/types.h>
#include <sys/sysctl.h>

#include <libxo/xo.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    int id[CTL_MAXNAME];
    size_t idlevel= CTL_MAXNAME;
    void *value;
    size_t valuelen;

    atexit(xo_finish_atexit);

    xo_set_flags(NULL, XOF_FLUSH);
    argc = xo_parse_args(argc, argv);
    if (argc < 0)
	exit(EXIT_FAILURE);
    
    sysctlnametomib("debug.witness.fullgraph", id, &idlevel);

    sysctl(id,idlevel,NULL,&valuelen,NULL,0);
    value=malloc(valuelen);
    sysctl(id,idlevel,value,&valuelen,NULL,0);

    xo_emit("{:valuelen/%lu}", valuelen);
    xo_emit("{:value/%s}", (char *)value);
    xo_emit("{:valuelen/%lu}", valuelen);
    xo_emit("{L:\n}");
    
    return 0;
}
