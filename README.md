nsysctl 1.2.1
=============

The **nsysctl** utility can get and set the [FreeBSD](http://www.freebsd.org)
kernel state at runtime, supporting
[libxo](http://juniper.github.io/libxo/libxo-manual.html) and a lof of options;
it depends on [sysctlmibinfo](https://gitlab.com/alfix/sysctlmibinfo) and
[sysctlinfo](https://gitlab.com/alfix/sysctlinfo)
([sysctlmibinfo2](https://gitlab.com/alfix/sysctlmibinfo2) in the future).

**Features**

 * get or set the system state at runtime,
 * handle an object up to CTL\_MAXNAME levels,
 * print the right object also with an empty level name,
 * options to show the properties of a parameter,
 * output explicitly indicated by the options,
 * options not mutually exclusive,
 * output via libxo in human and machine readable formats,
 * isolated code to manage opaque values,
 * some string value is parsed to show structured output,
 * avoid non-primitive data types hardcode via libnv,
 * debug without recompiling the kernel with SYSCTL\_DEBUG.

**Installation**

To install the port [sysutils/nsysctl](https://www.freshports.org/sysutils/nsysctl):

    # cd /usr/ports/sysutils/nsysctl/ && make install clean

To add the package:

    # pkg install nsysctl

**Documentation**

 * Manual Page:
   [manual nsysctl](https://alfonsosiciliano.gitlab.io/posts/2019-02-23-manual-nsysctl.html)
   or [man.freebsd.org/nsysctl/8](https://man.freebsd.org/nsysctl/8) (could be outdated)
 * Tutorial:
   ["Step-by-step Tutorial"](https://alfonsosiciliano.gitlab.io/posts/2019-02-19-nsysctl-tutorial.html)

**TODO**

 * [ ] Test: opaque S,efi\_map\_header (machdep.efi\_map)

