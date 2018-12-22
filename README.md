README
======

nsysctl is a sysctl(8) rewrite adding --libxo and -FIlMmSy options.   
It depends on http://gitlab.com/alfix/libsysctl (nsysctl was just the 
"main()" to test libstsctl).   

nsysctl is handier than sysctl: libsysctl provides a simple API 
avoiding to use undocumented kernel sysctl-mib-tree API, 
currently has less lines then sysctl.c, 
all code about "opaque values" is in opaque.c.  

TODO  

 * implement: nsysctl -bBf value=NUMBER
 * fix: nsysctl libxo=xml debug.witness.fullgraph segmentation fault
 * add: showbasictype.c
 * testing: stress test
 * doc: nsysctl.8

