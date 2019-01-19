README
======

**nsysctl** is a [sysctl](https://www.freebsd.org/cgi/man.cgi?query=sysctl&sektion=8&manpath=FreeBSD+13-current) 
utility clone, it depends on [libsysctl](http://gitlab.com/alfix/libsysctl) (renamed sysctlmibinfo(3)).   

**nsysctl** improvements: 

 * handy: sysctlmibinfo(3) provides a simple API to the kernel sysctl-mib, 
 * all code about "opaque values" is in opaque.c, 
 * output via [libxo(3)](https://wiki.freebsd.org/LibXo) in human and machine readable formats,
 * new options [-FIlMmSy],
 * useful code will be added to [sysctlview](http://gitlab.com/alfix/sysctlview), too.

Version 1.0 TODO  

 * [ ] options: nsysctl value=NUMBER
 * [ ] options: nsysctl -f filename
 * [ ] options: nsysctl -b
 * [ ] options: nsysctl -B
 * [ ] fix: nsysctl libxo=xml debug.witness.fullgraph  (segmentation fault)
 * [ ] fix: nsysctl libxo=text debug.fail_point  (utf8)
 * [X] ~~testing: add regression comparetest.sh~~
 * [X] ~~manual page: add nsysctl.8~~
 * [ ] manual page: add new options to nsysctl.8
 * [ ] FreeBSD port

