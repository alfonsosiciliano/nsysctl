nsysctl 0.1.1
=============

Legal notice: [FreeBSD](http://www.freebsd.org)&copy; is a registered trademark 
of the [FreeBSD Foundation](https://www.freebsdfoundation.org).  

**nsysctl** is a FreeBSD [/sbin/sysctl](https://man.freebsd.org/sysctl/8) 
clone: to get or set the kernel state with [libxo](https://wiki.freebsd.org/LibXo), 
[sysctlmibinfo](https://gitlab.com/alfix/sysctlmibinfo) 
and extra options, 
port: [sysutils/nsysctl](https://www.freshports.org/sysutils/nsysctl) and 
**[TUTORIAL](http://alfix.gitlab.io/bsd/2019/02/19/nsysctl-tutorial.html)**.  


<u>nsysctl improvements:</u> 

 * **sysctlmibinfo** provides a simple API to the sysctl MIB
 * all code about "opaque values" is in **opaque.c**
 * output via **libxo** in human and machine readable formats
 * **output is explicitly indicated** by the options
 * Options:
   * new **-D** show all properties
   * update **-e** specific _sep_ as a separator
   * new **-F** show flags
   * new **-I** show internal nodes
   * new **-l** show label
   * new **-r** show _tag-root_ with libxo
   * new **-m** show format string
   * delete **-n** simply do not use -N
   * updated **-N** force to show name
   * new **-p** show [_property-name_]: _property-value_
   * new **-S** show magic nodes with -a
   * new **-V** display value is "showable", otherwise hide the state
   * new **-v** force to show value
   * new **-y** show id


**TODO Version 0.2**

 * [ ] set array name=value,value,value...
 * [ ] failure: add xo\_warn and xo\_err messages
 * [ ] test/fix: machdep.efi\_map - S,efi\_map\_header
 * [ ] improve -f filename with whitespaces
 * [X] ~~"sysctl -a[o]" != "nsysctl -aNv[o]" debug.fail\_point.test\_trigger\_fail\_point it is a sysctl problem~~
 * [X] ~~"sysctl -a[o]" != "nsysctl -aNv[o]" hw.dri.0.bufs it is a sysctl problem~~
 * [X] ~~kern.file and hw.dri.0.vblank "change value\_size FIX" see nsysctl.c display\_tree()~~
 * [X] ~~nsysctl -aNv hw.dri.0.info.i915\_drpc\_info sometimes doesn't print "change\_value fix" (on/RC1)~~
 * [X] ~~libxo=xml -aD segmentation fault, to fix: xo\_set\_flags(NULL, XOF\_FLUSH) but new problem:~~
 * [X] ~~problem-xof-flush.c [PR236935](https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=236935)~~
 * [X] ~~libxo=xml segmentation fault with huge string, problem-xo-huge-string.c 
       [PR236937](https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=236937)~~
 * [X] ~~delete oid-only-value.txt, problem-xo-huge-string.c and problem-xof-flush.c after PR commit~~


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

