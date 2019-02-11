README
======

Note: [FreeBSD](http://www.freebsd.org)&copy; is a registered trademark of the [FreeBSD Foundation](https://www.freebsdfoundation.org).  

**This software is unstable and under heavy development**  

**nsysctl** is a FreeBSD [sysctl](https://www.freebsd.org/cgi/man.cgi?query=sysctl&sektion=8&manpath=FreeBSD+13-current) 
utility clone, it depends on [sysctlmibinfo](https://wiki.freebsd.org/AlfonsoSiciliano/sysctlmibinfo) library.   

**nsysctl** improvements: 

 * [sysctlmibinfo(3)](http://gitlab.com/alfix/sysctlmibinfo) provides a simple API to the sysctl MIB, 
 * all code about "opaque values" is in opaque.c, 
 * output via [libxo(3)](https://wiki.freebsd.org/LibXo) in human and machine readable formats,
 * new option -D show all properties,
 * new option -F show flags,
 * new option -I show internal nodes,
 * new option -l show label,
 * new option -r show <tag-root> with libxo
 * new option -m show format string
 * new option -p show <property-name> _sep_ <property-value>
 * new option -S show magic nodes with -a,
 * new option -y show id.


TODO Version 0.1

 * [X] ~~options: nsysctl value=NUMBER~~
 * [ ] options: nsysctl -f filename
 * [X] ~~options: nsysctl -b~~
 * [X] ~~options: nsysctl -B~~
 * [X] ~~options: nsysctl -D~~
 * [ ] failure: add xo\_warn and xo\_err messages
 * [X] ~~failure: return 1~~
 * [ ] libxo: fix nsysctl libxo=xml debug.witness.fullgraph, segmentation fault, bug.c
 * [X] ~~libxo: fix nsysctl libxo=text debug.fail\_point  (utf8)~~
 * [ ] improve output for name=value
 * [ ] libxo: xo-ish opaque.c
 * [X] ~~testing: add regression comparetest.sh~~
 * [ ] manual page: nsysctl.8
 * [X] ~~create FreeBSD port directory~~
