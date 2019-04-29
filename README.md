nsysctl 0.9
=============

Legal notice: [FreeBSD](http://www.freebsd.org)&copy; is a registered trademark 
of the [FreeBSD Foundation](https://www.freebsdfoundation.org).  

The **nsysctl** utility is a FreeBSD [/sbin/sysctl](https://man.freebsd.org/sysctl/8) 
clone, depending on [sysctlmibinfo](https://gitlab.com/alfix/sysctlmibinfo), 
to get or set the kernel state supporting 
[libxo](http://juniper.github.io/libxo/libxo-manual.html) 
and extra options; 
port: [sysutils/nsysctl](https://www.freshports.org/sysutils/nsysctl) and 
**[TUTORIAL](http://alfix.gitlab.io/bsd/2019/02/19/nsysctl-tutorial.html)**.  


<u>Features:</u>
 * **sysctlmibinfo** provides a simple API to the sysctl MIB,
 * output via **libxo** in human and machine readable formats,
 * isolated code to manage "opaque values" (_opaque.c_),
 * some string value is splitted to show structured output (_special\_value.c_),
 * **output is explicitly indicated** by the options,
 * the options are not mutually exclusive,
 * new options to show the properties of a state.



**TODO**

 * [ ] testing
 * [ ] test: opaque S,efi\_map\_header (e.g. machdep.efi\_map)
 * [ ] clean code


~~**TODO Version 0.9**~~

 * [X] ~~change: option -m -> -F~~
 * [X] ~~change: option -F -> -g~~
 * [X] ~~add: option -G~~
 * [X] ~~add: array set: name=value,value,value...~~
 * [X] ~~add: xo\_warn and xo\_err messages~~
 * [X] ~~add: opaque S,input\_id (e.g. kern.evdev.input.0.id)~~
 * [X] ~~fix: -x with numeric array~~
 * [X] ~~fix: -x if num == 0 print 00... else 0x... (compatibility)~~
 * [X] ~~fix: don't show label "null"~~


~~**TODO Version 0.2**~~

 * [X] ~~fix: "sysctl -a[o]" != "nsysctl -aNv[o]" debug.fail\_point.test\_trigger\_fail\_point (sysctl problem)~~
 * [X] ~~fix: "sysctl -a[o]" != "nsysctl -aNv[o]" hw.dri.0.bufs (sysctl problem)~~
 * [X] ~~fix: kern.file and hw.dri.0.vblank (see "change value\_size FIX" nsysctl.c display\_tree())~~
 * [X] ~~fix: hw.dri.0.info.i915\_drpc\_info sometimes doesn't print ("change\_value fix" (on/RC1))~~
 * [X] ~~fix: libxo=xml -aD segmentation fault, to fix: xo\_set\_flags(NULL, XOF\_FLUSH) but new problem:~~
 * [X] ~~fix: problem-xof-flush.c [PR236935](https://bugs.freebsd.org/236935)~~
 * [X] ~~fix: problem-xo-huge-string.c [PR236937](https://bugs.freebsd.org/236937)~~
 * [X] ~~delete oid-only-value.txt, problem-xo-huge-string.c and problem-xof-flush.c (after PRs commit)~~


~~**TODO Version 0.1.1**~~

* [X] ~~fix headerss in opaque.c for no i386/amd64~~
* [X] ~~delete sysctlmibinfo.h/c (dynamic linking)~~


~~**TODO Version 0.1**~~

 * [X] ~~add: 'set' for NUMERIC value: value=NUMBER~~
 * [X] ~~add: libxo support to opaque.c~~
 * [X] ~~add: output type NUMERIC - "format" 'A' (array)~~
 * [X] ~~add: output "format" 'K'~~
 * [X] ~~add: testing file comparetest.sh~~
 * [X] ~~add: manual page nsysctl.8~~
 * [X] ~~add: FreeBSD port~~
 * [X] ~~fix: output: name=value~~
 * [X] ~~fix: on failure return '1'~~
 * [X] ~~fix: libxo=xml debug.witness.fullgraph, segmentation fault (special\_value.c)~~
 * [X] ~~fix: libxo=text debug.fail\_point  (utf8)~~
