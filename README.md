nsysctl 1.0
=============

The **nsysctl** utility is a [/sbin/sysctl](https://man.freebsd.org/sysctl/8) 
clone to get or set the [FreeBSD](http://www.freebsd.org) kernel state, 
supporting [libxo](http://juniper.github.io/libxo/libxo-manual.html) and extra 
options; it depends on [sysctlmibinfo](https://gitlab.com/alfix/sysctlmibinfo). 
Port: [sysutils/nsysctl](https://www.freshports.org/sysutils/nsysctl) and 
**[TUTORIAL](https://alfix.gitlab.io/bsd/2019/02/19/nsysctl-tutorial.html)**.  


<u>Features:</u>
 * output is explicitly indicated by the options,
 * new options to show the properties of a state,
 * the options are not mutually exclusive,
 * output via libxo in human and machine readable formats,
 * isolated code to manage "opaque values" (opaque.c),
 * some string value is parsed to show structured output (special\_value.c).


**TODO**

 * [X] ~~fix: -B~~
 * [X] ~~fix: -h~~
 * [X] ~~fix: -G with "AND" flags~~
 * [X] ~~fix: -x with -y and -g~~
 * [X] ~~fix: vflag ans Vflag efficiency~~
 * [X] ~~fix: do not show \<children\> of a hidden node~~
 * [X] ~~add checking for mutually exclusive options~~
 * [X] ~~clean code: delete macros to get and set a value~~
 * [ ] I can not test opaque S,efi\_map\_header (machdep.efi\_map) on my laptop
 
