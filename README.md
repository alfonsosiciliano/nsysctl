README
======

[Note: [FreeBSD](http://www.freebsd.org)&copy; is a registered trademark of the [FreeBSD Foundation](https://www.freebsdfoundation.org)]  


**nsysctl** is a FreeBSD [sysctl](https://www.freebsd.org/cgi/man.cgi?query=sysctl&sektion=8&manpath=FreeBSD+13-current) 
utility clone, it depends on [sysctlmibinfo](http://gitlab.com/alfix/sysctlmibinfo) library.   


**nsysctl** improvements: 

 * handy: [sysctlmibinfo](http://gitlab.com/alfix/sysctlmibinfo) provides a simple API to the kernel sysctl-mib, 
 * all code about "opaque values" is in opaque.c, 
 * output via [libxo(3)](https://wiki.freebsd.org/LibXo) in human and machine readable formats,
 * new options [-FIlMmSy],
 * useful code will be added to [sysctlview](http://gitlab.com/alfix/sysctlview), too.

TODO VErsion 0.1

 * [X] ~~Update lib API libsysctl -> sysctlmibinfo~~

TODO Version 0.2

 * [X] ~~options: nsysctl value=NUMBER~~
 * [ ] options: nsysctl -f filename
 * [ ] options: nsysctl -b
 * [ ] options: nsysctl -B
 * [ ] failure: add xo\_warn and xo\_err messages
 * [ ] failure: return 1
 * [ ] libxo: fix nsysctl libxo=xml debug.witness.fullgraph  (segmentation fault)
 * [ ] libxo: fix nsysctl libxo=text debug.fail_point  (utf8)
 * [X] ~~testing: add regression comparetest.sh~~
 * [X] ~~manual page: add nsysctl.8~~
 * [ ] manual page: add new options to nsysctl.8
 * [X] ~~create FreeBSD port directory~~

TODO Version 0.3

 * [ ] improve xml output for name=value
 * [ ] merge set_value() with display\_basic\_value()
 * [ ] libxo: xo-ish opaque.c
 * [ ] FreeBSD port: submitt a PR



