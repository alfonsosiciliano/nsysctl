nsysctl 0.2
=============

Legal notice: [FreeBSD](http://www.freebsd.org)&copy; is a registered trademark 
of the [FreeBSD Foundation](https://www.freebsdfoundation.org).  

**nsysctl** is a FreeBSD [/sbin/sysctl](https://man.freebsd.org/sysctl/8) 
clone: to get or set the kernel state with [libxo](https://wiki.freebsd.org/LibXo), 
[sysctlmibinfo](https://gitlab.com/alfix/sysctlmibinfo) 
and extra options, 
port: [sysutils/nsysctl](https://www.freshports.org/sysutils/nsysctl) and 
**[TUTORIAL](http://alfix.gitlab.io/bsd/2019/02/19/nsysctl-tutorial.html)**.  


<u>Features:</u>
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


**TODO Version 0.3**

 * [ ] add: array 'set': name=value,value,value...
 * [ ] add: on failure: xo\_warn and xo\_err messages
 * [ ] add: whitespace support '-f filename'
 * [ ] fix: machdep.efi\_map - S,efi\_map\_header


~~**TODO Version 0.2**~~

 * [X] ~~fix: "sysctl -a[o]" != "nsysctl -aNv[o]" debug.fail\_point.test\_trigger\_fail\_point (sysctl problem)~~
 * [X] ~~fix: "sysctl -a[o]" != "nsysctl -aNv[o]" hw.dri.0.bufs (sysctl problem)~~
 * [X] ~~fix: kern.file and hw.dri.0.vblank (see "change value\_size FIX" nsysctl.c display\_tree())~~
 * [X] ~~fix: hw.dri.0.info.i915\_drpc\_info sometimes doesn't print ("change\_value fix" (on/RC1))~~
 * [X] ~~fix: libxo=xml -aD segmentation fault, to fix: xo\_set\_flags(NULL, XOF\_FLUSH) but new problem:~~
 * [X] ~~fix: problem-xof-flush.c [PR236935](https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=236935)~~
 * [X] ~~fix: libxo=xml segmentation: problem-xo-huge-string.c, [PR236937](https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=236937)~~
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
 * [X] ~~add: option: -f filename~~
 * [X] ~~add: option: -b~~
 * [X] ~~add: option: -B bufsize~~
 * [X] ~~fix: 'set' output: name=value~~
 * [X] ~~fix: on failure return '1'~~
 * [X] ~~fix: libxo=xml debug.witness.fullgraph, segmentation fault (special\_value.c)~~
 * [X] ~~fix: libxo=text debug.fail\_point  (utf8)~~

