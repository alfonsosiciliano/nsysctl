nsysctl 1.2.1
=============

The **nsysctl** utility is a [/sbin/sysctl](https://man.freebsd.org/sysctl/8) 
clone to get or set the [FreeBSD](http://www.freebsd.org) kernel state, 
supporting [libxo](http://juniper.github.io/libxo/libxo-manual.html) and extra 
options; it depends on [sysctlmibinfo](https://gitlab.com/alfix/sysctlmibinfo) 
and [sysctlinfo](https://gitlab.com/alfix/sysctlinfo).

**Features**

 * output is explicitly indicated by the options,
 * new options to show the properties of a parameter,
 * the options are not mutually exclusive,
 * output via libxo in human and machine readable formats,
 * isolated code to manage "opaque values" (opaque.c),
 * some string value is parsed to show structured output (special\_value.c),
 * use sysctlinfo kmod if loaded.

**Installation**

To install the port [sysutils/nsysctl](https://www.freshports.org/sysutils/nsysctl)

    # cd /usr/ports/deskutils/nsysctl/ && make install clean

To add the package:

    # pkg install nsysctl

**Documentation**

 * Manual Page:
   [manual-nsysctl.html](https://alfonsosiciliano.gitlab.io/posts/2019-02-23-manual-nsysctl.html)
   or [man.freebsd.org/nsysctl/8](https://man.freebsd.org/nsysctl/8) (could be outdated)
 * Tutorial:
   ["Step-by-step Tutorial"](https://alfonsosiciliano.gitlab.io/posts/2019-02-19-nsysctl-tutorial.html)

**TODO**

 * [ ] I can not test opaque S,efi\_map\_header (machdep.efi\_map) on my laptop
 * [ ] Add libnv
 * [ ] Add -k opt: show SKIP objects
 * [ ] Add NEEDGIANT flag for the -G opt
 * [ ] Rename opt: -m -> -0 (zero)
 * [ ] Delete sysctlinfo\_helper.h/c
 * [ ] Delete libsysctlmibinfo
 * [ ] Add libsysctlmibinfo2
 * [ ] Update manual
 
