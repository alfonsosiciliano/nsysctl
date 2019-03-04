/*
 * % cc -g problem-xo-huge-string.c -lxo -o problem-xo-huge-string
 * % ./problem-xo-huge-string
 * <OK>
 * % ./problem-xo-huge-string.c --libxo=text
 * <OK>
 * % ./problem-xo-huge-string.c --libxo=xml,pretty
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

    valuelen = 0;
    if((sysctl(id,idlevel,NULL,&valuelen,NULL,0)) != 0) {
	    printf("sysctl '1' error\n");
	    return 1;
    }
    if((value=malloc(valuelen)) == NULL) {
	    printf("malloc error\n");
	    return 1;
    }
    if((sysctl(id,idlevel,value,&valuelen,NULL,0)) != 0) {
	    printf("sysctl '2' error\n");
	    return 1;
    }

    xo_emit("{:valuelen/%lu}", valuelen);
    xo_emit("{:value/%s}", (char *)value);
    xo_emit("{:valuelen/%lu}", valuelen);
    xo_emit("{L:\n}");
    
    return 0;
}

/*
alfix@fbsd:~/nsysctl/% lldb -- ./bug --libxo=xml,pretty
(lldb) target create "./bug"
Current executable set to './bug' (x86_64).
(lldb) settings set -- target.run-args  "--libxo=xml,pretty"
(lldb) run
Process 1797 launching
Process 1797 launched: '/home/alfix/nsysctl/bug' (x86_64)
<valuelen>35285</valuelen>
Process 1797 stopped
* thread #1, name = 'bug', stop reason = signal SIGSEGV: invalid address (fault address: 0x8006f1000)
    frame #0: 0x000000080043ca30 libc.so.7`memcpy at memmove.S:306
   303 	END(memmove)
   304 	#else
   305 	ENTRY(memcpy)
-> 306 		MEMMOVE erms=0 overlap=1 begin=MEMMOVE_BEGIN end=MEMMOVE_END
   307 	END(memcpy)
   308 	#endif
(lldb) bt
* thread #1, name = 'bug', stop reason = signal SIGSEGV: invalid address (fault address: 0x8006f1000)
  * frame #0: 0x000000080043ca30 libc.so.7`memcpy at memmove.S:306
    frame #1: 0x000000080025f8f2 libxo.so.0`xo_do_format_field [inlined] xo_buf_escape(xop=<unavailable>, xbp=0x0000000800243b20, str=<unavailable>, len=<unavailable>, flags=<unavailable>) at libxo.c:911
    frame #2: 0x000000080025f8bf libxo.so.0`xo_do_format_field [inlined] xo_format_string(xop=<unavailable>, xbp=0x0000000800243b20, flags=<unavailable>) at libxo.c:2963
    frame #3: 0x000000080025f853 libxo.so.0`xo_do_format_field(xop=<unavailable>, xbp=0x0000000800243b20, fmt="%s", flen=<unavailable>, flags=<unavailable>) at libxo.c:3499
    frame #4: 0x000000080025d5d7 libxo.so.0`xo_format_value [inlined] xo_simple_field(xop=<unavailable>, encode_only=0, value=0x0000000000000000, vlen=0, fmt=<unavailable>, flen=<unavailable>, flags=<unavailable>) at libxo.c:3810
    frame #5: 0x000000080025d5a4 libxo.so.0`xo_format_value(xop=0x0000000800243ad8, name="value/%s}", nlen=<unavailable>, value=0x0000000000000000, vlen=0, fmt="", flen=2, encoding=0x0000000000000000, elen=0, flags=0) at libxo.c:4423
    frame #6: 0x00000008002589d4 libxo.so.0`xo_do_emit_fields(xop=0x0000000800243ad8, fields=0x00007fffffffe3c0, max_fields=5, fmt=<unavailable>) at libxo.c:6342
    frame #7: 0x0000000800257683 libxo.so.0`xo_do_emit(xop=0x0000000800243ad8, flags=<unavailable>, fmt="{:value/%s}") at libxo.c:6523
    frame #8: 0x00000008002578d8 libxo.so.0`xo_emit(fmt="{:value/%s}") at libxo.c:6594
    frame #9: 0x00000000002014f8 bug`main(argc=1, argv=0x00007fffffffe840) at bug.c:48
    frame #10: 0x000000000020111b bug`_start(ap=<unavailable>, cleanup=<unavailable>) at crt1.c:76
(lldb) quit
Quitting LLDB will kill one or more processes. Do you really want to proceed: [Y/n] Y
alfix@fbsd:~/nsysctl/%

*/
