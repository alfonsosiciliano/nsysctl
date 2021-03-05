nsysctl 2.0
===========

The **nsysctl** utility can get and set the [FreeBSD](http://www.freebsd.org)
kernel state at runtime, it depends on
[sysctlmibinfo2](https://gitlab.com/alfix/sysctlmibinfo2) and (implicitly)
[sysctlinfo](https://gitlab.com/alfix/sysctlinfo).

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

 * Manual Page: *coming soon new manual*
   (old [manual](https://alfonsosiciliano.gitlab.io/posts/2019-02-23-manual-nsysctl.html)
   \<= 1.2.1)
 * Tutorial: *coming soon new tutorial*
   (old [Tutorial](https://alfonsosiciliano.gitlab.io/posts/2019-02-19-nsysctl-tutorial.html)
   \<= 1.2.1)

**TODO**

 * [ ] Test: opaque S,efi\_map\_header (machdep.efi\_map)

