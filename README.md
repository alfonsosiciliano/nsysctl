README
======

Legal notice: [FreeBSD](http://www.freebsd.org)&copy; is a registered trademark of the [FreeBSD Foundation](https://www.freebsdfoundation.org).  

**nsysctl 0.1** is a FreeBSD [/sbin/sysctl](https://man.freebsd.org/sysctl/8) 
clone: to get or set kernel state with [libxo](https://wiki.freebsd.org/LibXo), 
[sysctlmibinfo](https://gitlab.com/alfix/sysctlmibinfo) 
and extra options, [TUTORIAL](http://alfix.gitlab.io/bsd/2019/02/19/nsysctl-tutorial.html).  


**nsysctl** improvements: 

 * **sysctlmibinfo** provides a simple API to the sysctl MIB, 
 * all code about "opaque values" is in **opaque.c**, 
 * output via **libxo** in human and machine readable formats,
 * **output is explicitly indicated** by the options,
 * new option **-D** show all properties,
 * update option **-e** specific _sep_ as a separator,
 * new option **-F** show flags,
 * new option **-I** show internal nodes,
 * new option **-l** show label,
 * new option **-r** show _tag-root_ with libxo,
 * new option **-m** show format string,
 * delete option **-n** simply do not use -N,
 * updated option **-N** force to show name,
 * new option **-p** show [_property-name_]: _property-value_,
 * new option **-S** show magic nodes with -a,
 * new option **-V** display value is "showable", otherwise hide the state,
 * new option **-v** force to show value,
 * new option **-y** show id.


**TODO Version 0.2**

 * [ ] set array name=value,value,value...
 * [ ] failure: add xo\_warn and xo\_err messages
 * [ ] test/fix: machdep.efi\_map - S,efi\_map\_header
 * [ ] improve -f filename with whitespaces
 * [ ] "sysctl -a" == "nsysctl -aNv" debug.fail\_point.test\_trigger\_fail\_point 
 * [ ] "sysctl -a" == "nsysctl -aNv" hw.dri.0.vblank
 * [ ] "sysctl -a" == "nsysctl -aNv" hw.dri.0.info.i915\_drpc\_info sometimes doesn't print
 * [ ] "sysctl -ao" == "nsysctl -aNvo" kern.file
 * [ ] "sysctl -ao" == "nsysctl -aNvo" debug.fail\_point.test\_trigger\_fail\_point
 * [ ] "sysctl -ao" == "nsysctl -aNvo" hw.dri.0.vblank
 * [ ] libxo problems:
 * [ ] libxo=xml -aD segmentation fault [lldb below], to fix: xo\_set\_flags(NULL, XOF\_FLUSH) but new problem: 
 * [ ] problem-xof-flush.c
 * [ ] libxo=xml segmentation fault with huge string, problem-xo-huge-string.c


~~**TODO Version 0.1.1**~~

* [X] ~~fix includes in opaque.c for no i386/amd64~~
* [X] ~~delete sysctlmibinfo.h/c (dynamic linking)~~


~~**TODO Version 0.1**~~

 * [X] ~~options: nsysctl value=NUMBER~~
 * [X] ~~improve output for name=value~~
 * [X] ~~options: nsysctl -f filename~~
 * [X] ~~options: nsysctl -b~~
 * [X] ~~options: nsysctl -B~~
 * [X] ~~options: nsysctl -D~~
 * [X] ~~failure: return 1~~
 * [X] ~~libxo: fix nsysctl libxo=xml debug.witness.fullgraph,~~ segmentation fault, bug.c
 * [X] ~~libxo: fix nsysctl libxo=text debug.fail\_point  (utf8)~~
 * [X] ~~libxo: xo-ish opaque.c~~
 * [X] ~~output type: NUMBER - fmt 'A'~~
 * [X] ~~output fmt 'K'~~
 * [X] ~~testing: add regression comparetest.sh~~
 * [X] ~~manual page: nsysctl.8~~
 * [X] ~~create FreeBSD port directory~~


bug libxo=xml,pretty -aDI
```
lldb -- ./nsysctl --libxo=xml,pretty -r root -aDI
... stuff ...
... stuff ...
          <object>
            <id>
              <level1>1</level1>
              <level2>7ffffe0a</level2>
              <level3>7ffffe06</level3>
            </id>
Process 1463 stopped
* thread #1, name = 'nsysctl', stop reason = signal SIGSEGV: invalid address (fault address: 0x8016e450e)
    frame #0: 0x000000080025af56 libxo.so.0`xo_escape_xml(xbp=<unavailable>, len=11164, flags=0) at libxo.c:807
   804 		else if (attr && *cp == '"')
   805 		    sp = xo_xml_quot;
   806 		else {
-> 807 		    *ip = *cp;
   808 		    continue;
   809 		}
   810 	
(lldb) bt
* thread #1, name = 'nsysctl', stop reason = signal SIGSEGV: invalid address (fault address: 0x8016e450e)
  * frame #0: 0x000000080025af56 libxo.so.0`xo_escape_xml(xbp=<unavailable>, len=11164, flags=0) at libxo.c:807
    frame #1: 0x0000000800267932 libxo.so.0`xo_do_format_field [inlined] xo_buf_escape(xop=<unavailable>, xbp=0x000000080024cb20, str=<unavailable>, len=<unavailable>, flags=<unavailable>) at libxo.c:916
    frame #2: 0x00000008002678bf libxo.so.0`xo_do_format_field [inlined] xo_format_string(xop=<unavailable>, xbp=0x000000080024cb20, flags=<unavailable>) at libxo.c:2963
    frame #3: 0x0000000800267853 libxo.so.0`xo_do_format_field(xop=<unavailable>, xbp=0x000000080024cb20, fmt="%s", flen=<unavailable>, flags=<unavailable>) at libxo.c:3499
    frame #4: 0x00000008002655d7 libxo.so.0`xo_format_value [inlined] xo_simple_field(xop=<unavailable>, encode_only=0, value=0x0000000000000000, vlen=0, fmt=<unavailable>, flen=<unavailable>, flags=<unavailable>) at libxo.c:3810
    frame #5: 0x00000008002655a4 libxo.so.0`xo_format_value(xop=0x000000080024cad8, name="value/%s}", nlen=<unavailable>, value=0x0000000000000000, vlen=0, fmt="", flen=2, encoding=0x0000000000000000, elen=0, flags=0) at libxo.c:4423
    frame #6: 0x00000008002609d4 libxo.so.0`xo_do_emit_fields(xop=0x000000080024cad8, fields=0x00007fffffffdce0, max_fields=5, fmt=<unavailable>) at libxo.c:6342
    frame #7: 0x000000080025f683 libxo.so.0`xo_do_emit(xop=0x000000080024cad8, flags=<unavailable>, fmt="{:value/%s}") at libxo.c:6523
    frame #8: 0x000000080025f8d8 libxo.so.0`xo_emit(fmt="{:value/%s}") at libxo.c:6594
    frame #9: 0x0000000000204f06 nsysctl`display_basic_type(object=0x00000008007050c0, value=0x0000000800e9c000, value_size=11165) at nsysctl.c:450
    frame #10: 0x000000000020463b nsysctl`display_tree(object=0x00000008007050c0, newvalue=0x0000000000000000) at nsysctl.c:344
    frame #11: 0x000000000020470e nsysctl`display_tree(object=0x00000008006e3890, newvalue=0x0000000000000000) at nsysctl.c:364
    frame #12: 0x000000000020470e nsysctl`display_tree(object=0x00000008006e00f0, newvalue=0x0000000000000000) at nsysctl.c:364
    frame #13: 0x00000000002039fe nsysctl`main(argc=0, argv=0x00007fffffffe830) at nsysctl.c:179
    frame #14: 0x000000000020311b nsysctl`_start(ap=<unavailable>, cleanup=<unavailable>) at crt1.c:76
(lldb) quit
Quitting LLDB will kill one or more processes. Do you really want to proceed: [Y/n] Y
alfix@fbsd:~/nsysctl/% 
```
