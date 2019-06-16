nsysctl 1.0
=============

The **nsysctl** utility is a [/sbin/sysctl](https://man.freebsd.org/sysctl/8) 
clone for [FreeBSD](http://www.freebsd.org), depending on 
[sysctlmibinfo](https://gitlab.com/alfix/sysctlmibinfo), 
to get or set the kernel state supporting 
[libxo](http://juniper.github.io/libxo/libxo-manual.html) 
and extra options; 
port: [sysutils/nsysctl](https://www.freshports.org/sysutils/nsysctl) and 
**[TUTORIAL](https://alfix.gitlab.io/bsd/2019/02/19/nsysctl-tutorial.html)**.  


<u>Features:</u>
 * sysctlmibinfo provides a simple API to the sysctl MIB,
 * output via libxo in human and machine readable formats,
 * isolated code to manage "opaque values" (opaque.c),
 * some string value is splitted to show structured output (special\_value.c),
 * output is explicitly indicated by the options,
 * the options are not mutually exclusive,
 * new options to show the properties of a state.



**TODO**

 * [X] ~~fix: -B~~
 * [X] ~~fix: -h~~
 * [X] ~~fix: "AND" flags~~
 * [X] ~~fix: -g with [x|o]~~
 * [X] ~~fix: mutually exclusive options~~
 * [X] ~~fix: vflag ans Vflag efficiency~~
 * [X] ~~clean code: delete macros to get and set a value~~
 * [ ] I can not test opaque S,efi\_map\_header (machdep.efi\_map) on my laptop
 
