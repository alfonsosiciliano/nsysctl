README
======

Legal notice: [FreeBSD](http://www.freebsd.org)&copy; is a registered trademark of the [FreeBSD Foundation](https://www.freebsdfoundation.org).  

**nsysctl 0.1** is a FreeBSD [/sbin/sysctl](https://man.freebsd.org/sysctl/8) 
clone: to get or set kernel state with [libxo](https://wiki.freebsd.org/LibXo), 
[sysctlmibinfo](https://wiki.freebsd.org/AlfonsoSiciliano/sysctlmibinfo) 
and extra options, [TUTORIAL](http://alfix.gitlab.io/bsd/2019/02/19/nsysctl-tutorial.html).  
**This software is unstable and under heavy development**  
```
usage:
	nsysctl [--libxo=opts [-r tagname]] [-DdFIilmNpqTt[-V|v[h[b|o|x]]]Wy]
		[-e sep] [-B <bufsize>] [-f filename] name[=value] ...
	nsysctl [--libxo=opts [-r tagname]] [-DdFIlmNpqSTt[-V|v[h[b|o|x]]]Wy]
		[-e sep] [-B <bufsize>] -A|a|X
```

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
 * [ ] libxo=xml -aD segmentation fault, to fix: xo\_set\_flags(NULL, XOF\_UNITS -> XOF\_FLUSH) and split xo\_emit()
 * [ ] nsysctl -aNtVo | grep opaque: vm.vmtotal and machdep.smap
 * [ ] libxo=xml segmentation fault with huge string, example: _bug.c_
 * [ ] improve -f filename with whitespaces
 * [ ] "sysctl -a" == "nsysctl -aNv" debug.fail\_point.test\_trigger\_fail\_point 
 * [ ] "sysctl -a" == "nsysctl -aNv" hw.dri.0.vblank
 * [ ] "sysctl -a" == "nsysctl -aNv" hw.dri.0.info.i915\_drpc\_info sometimes doesn't print
 * [ ] "sysctl -ao" == "nsysctl -aNvo" kern.file
 * [ ] "sysctl -ao" == "nsysctl -aNvo" debug.fail\_point.test\_trigger\_fail\_point
 * [ ] "sysctl -ao" == "nsysctl -aNvo" hw.dri.0.vblank


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

