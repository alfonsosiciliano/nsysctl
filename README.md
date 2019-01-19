README
======

nsysctl is a [sbin/sysctl(8)](https://www.freebsd.org/cgi/man.cgi?query=sysctl&sektion=8&manpath=FreeBSD+13-current) 
rewrite, it depends on http://gitlab.com/alfix/libsysctl   
(nsysctl was just the "main()" to test libsysctl).   

nsysctl is handier than sysctl: 

 * libsysctl provides a simple API avoiding to use undocumented kernel sysctl-mib-tree API, 
 * all code about "opaque values" is in opaque.c, 
 * [--libxo](https://wiki.freebsd.org/LibXo) and -FIlMmSy options. 
 * useful code will be add to http://gitlab.com/alfix/sysctlview too.

TODO  

 * options: nsysctl value=NUMBER
 * options: nsysctl -f filename
 * options: nsysctl -b
 * options: nsysctl -B
 * fix: nsysctl libxo=xml debug.witness.fullgraph -> segmentation fault
 * fix: nsysctl libxo=text debug.fail_point
 * __auto/regression testing: comparetest.sh__
 * doc: nsysctl.8
 * FreeBSD port

