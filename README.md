nsysctl 2.1.1
=============

The **nsysctl** utility can get and set the [FreeBSD](http://www.freebsd.org)
kernel state at runtime.

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

 * Manual [nsysctl(8)](https://alfonsosiciliano.gitlab.io/posts/2021-03-07-manual-nsysctl-2.html).
 * [Tutorial](https://alfonsosiciliano.gitlab.io/posts/2021-03-08-tutorial-nsysctl-2.html).

**TODO**

 * [ ] Improve: -f (trim like sysctl)

