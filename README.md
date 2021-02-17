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
 * output is explicitly indicated by the options,
 * new options to show the properties of a parameter,
 * the options are not mutually exclusive,
 * output via libxo in human and machine readable formats,
 * isolated code to manage "opaque values" (opaque.c),
 * some string value is parsed to show structured output (special\_value.c),
 * handle an OID up to CTL\_MAXNAME levels,
 * print the right object also with a NULL-level name.

**Installation**

To install the port [sysutils/nsysctl](https://www.freshports.org/sysutils/nsysctl)

    # cd /usr/ports/sysutils/nsysctl/ && make install clean

To add the package:

    # pkg install nsysctl

**Documentation**

 * Manual Page:
   [manual-nsysctl.html](https://alfonsosiciliano.gitlab.io/posts/2019-02-23-manual-nsysctl.html)
   or [man.freebsd.org/nsysctl/8](https://man.freebsd.org/nsysctl/8) (could be outdated)
 * Tutorial:
   ["Step-by-step Tutorial"](https://alfonsosiciliano.gitlab.io/posts/2019-02-19-nsysctl-tutorial.html)

**TODO**

 * [ ] Test: opaque machdep.efi\_map (S,efi\_map\_header)
 * [ ] Add: opaque S,pagesizes (hw.pagesizes)
 * [ ] Add: libnv
 * [X] Add: -k opt: show SKIP objects with -a
 * [X] Add: -H opt: has handler? [un]defined
 * [X] Add: NEEDGIANT flag to -G
 * [ ] Delete: -g opt
 * [ ] Rename: -G -> -g
 * [ ] Change: -pg [TRUE-FLAGS] -> [FLAGS]
 * [X] Rename: opt: -m -> -S
 * [X] Fix: -S only with -a
 * [X] Delete: sysctlinfo\_helper.h/c
 * [X] Delete: libsysctlmibinfo
 * [X] Add: libsysctlmibinfo2
 * [X] Rename: opt: -y -> -O
 * [X] Change: -pO [ID] -> [OID]
 * [X] Change: xml \<id\>\<level1\>X\</level1\>\<level2\>Y\</level2\>\</id\> -> \<OID\>X.Y\</OID\>
 * [X] Rename: opt -e \<sep\> -> -s \<sep\>
 * [X] Add: opt -e (like sysctl)
 * [ ] Update manual
 * [X] Change: -pF [FORMAT-STRING] -> [FORMAT]
 * [X] Change: default show name (like sysctl)
 * [X] Add: -n hide name
 * [X] Change: default show value (like sysctl)
 * [X] Change: -N hide value (show name default like sysctl)
 * [X] Change: -v show value -> show version (show value default)
 * [X] Change: -V with -a show object with/without a readable value
 * [X] Delete: -A (obsolete, compatibility like sysctl)
 * [X] Delete: -X (obsolete, compatibility like sysctl)
 
