README
======

nsysctl is a [sbin/sysctl(8)](https://www.freebsd.org/cgi/man.cgi?query=sysctl&sektion=8&manpath=FreeBSD+13-current) 
rewrite, it depends on http://gitlab.com/alfix/libsysctl   
(nsysctl was just the "main()" to test libsysctl).   

nsysctl is handier than sysctl: 

 * libsysctl provides a simple API avoiding to use undocumented kernel sysctl-mib-tree API, 
 * currently has splitted and less code than sysctl.c, 
 * all code about "opaque values" is in opaque.c, 
 * [--libxo](https://wiki.freebsd.org/LibXo) and -FIlMmSy options. 
 * useful code will be add to http://gitlab.com/alfix/sysctlview too.

TODO  

 * options: nsysctl -bB -f filename  value=NUMBER
 * fix: nsysctl libxo=xml debug.witness.fullgraph segmentation fault
 * testing: stress test
 * doc: nsysctl.8
 * FreeBSD port

