nsysctl 1.2.1
=============

The **nsysctl** utility is a [/sbin/sysctl](https://man.freebsd.org/sysctl/8) 
clone to get or set the [FreeBSD](http://www.freebsd.org) kernel state, 
supporting [libxo](http://juniper.github.io/libxo/libxo-manual.html) and extra 
options; it depends on [sysctlmibinfo](https://gitlab.com/alfix/sysctlmibinfo) 
and [sysctlinfo](https://gitlab.com/alfix/sysctlinfo). 
Port: [sysutils/nsysctl](https://www.freshports.org/sysutils/nsysctl) and 
**[TUTORIAL](https://alfonsosiciliano.gitlab.io/posts/2019-02-19-nsysctl-tutorial.html)**.  


<u>Features:</u>
 * output is explicitly indicated by the options,
 * new options to show the properties of a state,
 * the options are not mutually exclusive,
 * output via libxo in human and machine readable formats,
 * isolated code to manage "opaque values" (opaque.c),
 * some string value is parsed to show structured output (special\_value.c),
 * use sysctlinfo kmod if loaded.


**TODO**

 * [ ] I can not test opaque S,efi\_map\_header (machdep.efi\_map) on my laptop
 * [ ] Add libnv
 * [ ] Add opt -k (avoid SKIP objects)
 * [ ] Rename opt: -m -> -0 (zero)
 * [ ] Delete sysctlinfo\_helper.h/c
 * [ ] Complete nsysctl.8
 
