/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

//#include <sys/types.h>
#include <sys/param.h>
#include <sys/efi.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/vmmeter.h>

#include <assert.h>     //assert
#include <err.h>        //warnx
#include <errno.h>      //errno
#include <machine/pc/bios.h>
#include <stdbool.h>
#include <stdio.h>      //printf
#include <stdlib.h>     //free
#include <string.h>     //strdup
#include <unistd.h>     //getpagesize
#include <x86/metadata.h>

#include <libxo/xo.h>
#include <sysctlmibinfo.h>

/* Func declarations */
static int S_clockinfo(struct sysctlmif_object *object, bool hflag);
static int S_loadavg(struct sysctlmif_object *object, bool hflag);
static int S_timeval(struct sysctlmif_object *object, bool hflag);
static int S_vmtotal(struct sysctlmif_object *object, bool hflag);
#ifdef __amd64__
static int S_efi_map(struct sysctlmif_object *object, bool hflag);
#endif
#if defined(__amd64__) || defined(__i386__)
static int S_bios_smap_xattr(struct sysctlmif_object *object, bool hflag);
#endif
static int strIKtoi(const char *str, char **endptrp, const char *fmt);

bool is_opaque_defined(struct sysctlmif_object *object)
{
    if ( strcmp(object->fmt, "S,clockinfo") == 0 ||
	 strcmp(object->fmt, "S,timeval") == 0 ||
	 strcmp(object->fmt, "S,loadavg") == 0 ||
	 strcmp(object->fmt, "S,vmtotal") == 0 ||
#ifdef __amd64__
	strcmp(object->fmt, "S,efi_map_header") == 0 ||
#endif
#if defined(__amd64__) || defined(__i386__)
	strcmp(object->fmt, "S,bios_smap_xattr") == 0
#endif
	)
	return true;

    return false;
}

void display_opaque_value(struct sysctlmif_object *object, bool hflag, bool oflag, bool xflag)
{
	unsigned char opaquevalue[BUFSIZ * 500];

	bzero(opaquevalue, BUFSIZ * 500);
	size_t sizevalue = BUFSIZ *500;

	xo_open_container("value");
	
	if (strcmp(object->fmt, "S,clockinfo") == 0) {
	    S_clockinfo(object, hflag);
	} else if (strcmp(object->fmt, "S,timeval") == 0) {
		S_timeval(object, hflag);
	} else if (strcmp(object->fmt, "S,loadavg") == 0) {
		S_loadavg(object, hflag);
	} else if (strcmp(object->fmt, "S,vmtotal") == 0) {
		S_vmtotal(object, hflag);
	}
#ifdef __amd64__
	/*else if (strcmp(object->fmt, "S,efi_map_header") == 0) {
		S_efi_map(object, hflag);
		}*/
#endif
#if defined(__amd64__) || defined(__i386__)
	/*else if (strcmp(object->fmt, "S,bios_smap_xattr") == 0) {
		S_bios_smap_xattr(object, hflag);
		}*/
#endif
	else if (oflag || xflag) {
	    	xo_open_container(object->fmt + 2);
		xo_emit("{Lc:Format}{:format/%s}", object->fmt);
		sysctl(object->id, object->idlevel, opaquevalue, &sizevalue, NULL, 0);
		xo_emit("{P: }{Lc:Length}{:lenght/%lu}", sizevalue);
		xo_emit("{P: }{Lc:Dump}0x" /*{:dump/%16x}...",opaquevalue*/);

		int i;
		int to = sizevalue;
		if(oflag)
		    to = 16 < sizevalue ? 16 : sizevalue;
		
		for (i = 0; i < to; i++) {
			xo_emit("{:dump/%02x}", opaquevalue[i]);
		}
		if (oflag && (sizevalue >= 8)) {
			xo_emit("{L:...}");
		}

		xo_close_container(object->fmt + 2);
	}

	xo_close_container("value");
}


static int
S_clockinfo(struct sysctlmif_object *object, bool hflag)
{
	size_t ci_size = sizeof(struct clockinfo);
	struct clockinfo ci;
	char *hfield = hflag ? "h" : NULL;

	if (sysctl(object->id, object->idlevel, (void *)&ci, &ci_size, NULL,
	    0) < 0) {
		xo_warnx("Impossible get clockinfo");
		return (1);
	}

	xo_open_container("clockinfo");

	xo_emit("{L:{ }");
	xo_emit("{Lw:hz =}");
	xo_emit_field(hfield, "hz", "%d", NULL, ci.hz);
	xo_emit(", {Lw:tick =}");
	xo_emit_field(hfield, "tick", "%d", NULL, ci.tick);
	xo_emit(", {Lw:profhz =}");
	xo_emit_field(hfield, "profhz", "%d", NULL, ci.profhz);
	xo_emit(", {Lw:stathz =}");
	xo_emit_field(hfield, "stathz", "%d", NULL, ci.stathz);
	xo_emit("{N: }}");

	xo_close_container("clockinfo");

	return (0);
}


static int
S_loadavg(struct sysctlmif_object *object, bool hflag)
{
	struct loadavg tv;
	size_t tv_size = sizeof(struct loadavg);
	char *hfield = /*hflag ? "h" :*/ NULL;

	/*libxo 'h' modifier does not affect the size and treatment of %f */

	if (sysctl(object->id, object->idlevel, (void *)&tv, &tv_size, NULL,
	    0) < 0) {
		xo_warnx("Impossible get loadavg");
		return (1);
	}

#define TV_FSCALE(idx)    ((double)tv.ldavg[idx]/(double)tv.fscale)

	xo_open_container("loadavg");
	xo_emit("{L:{ }");
	xo_emit_field(hfield, "ldavg0", "%.2f", NULL, TV_FSCALE(0));
	xo_emit("{Lw:}");
	xo_emit_field(hfield, "ldavg1", "%.2f", NULL, TV_FSCALE(1));
	xo_emit("{Lw:}");
	xo_emit_field(hfield, "ldavg2", "%.2f", NULL, TV_FSCALE(2));
	xo_emit("{N: }}");
	xo_close_container("loadavg");

	return (0);
}


static int
S_timeval(struct sysctlmif_object *object, bool hflag)
{
	struct timeval tv;
	size_t tv_size = sizeof(struct timeval);
	time_t tv_sec;
	char *p1;
	char *hfield = hflag ? "h,hn-decimal" : NULL;

	if (sysctl(object->id, object->idlevel, (void *)&tv, &tv_size, NULL,
	    0) < 0) {
		xo_warnx("Impossible get timeval");
		return (1);
	}

	xo_open_container("timeval");

	xo_emit("{Lw:{ sec =}");
	xo_emit_field(hfield, "sec", "%jd", NULL, (intmax_t)tv.tv_sec);
	xo_emit(", {Lw:usec =}");
	xo_emit_field(hfield, "usec", "%ld", NULL, tv.tv_usec);
	xo_emit("{Nw:}}");

	tv_sec = tv.tv_sec;
	p1 = strdup(ctime(&tv_sec));
	p1[strlen(p1) -1] = '\0';
	xo_emit("{P: }{:date/%s}", p1);
	free(p1);

	xo_close_container("timeval");

	return (0);
}


static int
S_vmtotal(struct sysctlmif_object *object, bool hflag)
{
	struct vmtotal v;
	size_t v_size = sizeof(struct vmtotal);
	char *hfield = hflag ? "h,hn-decimal" : NULL;
	int pageKilo;

	if (sysctl(object->id, object->idlevel, (void *)&v, &v_size, NULL,
	    0) < 0) {
		xo_warnx("Impossible get vmtotal");
		return (1);
	}

	pageKilo = getpagesize() / 1024;

	xo_open_container("vmtotal");

#define pg2k(a)    ((uintmax_t)(a) * pageKilo)

	xo_emit("\n{T:System wide totals computed every five seconds:"
	    " (values in kilobytes)}\n");
	xo_emit("{T:===============================================}\n");

	xo_open_container("processes");
	xo_emit("{Lc:Processes}{P:		}");
	xo_emit("{Lwc:(RUNQ}");
	xo_emit_field(hfield, "runq", "%d", NULL, v.t_rq);
	xo_emit(" {Lwc:Disk Wait}");
	xo_emit_field(hfield, "disk-wait", "%d", NULL, v.t_dw);
	xo_emit(" {Lw:Page Wait =}");
	xo_emit_field(hfield, "page-wait", "%d", NULL, v.t_pw);
	xo_emit(" {Lwc:Sleep}");
	xo_emit_field(hfield, "sleep", "%d", NULL, v.t_sl);
	xo_emit("{N:)}");
	xo_close_container("process");
	xo_emit("{L:\n}");

#define XOVM(contname, fieldname, padd, totalsize, activesize)		  \
	do {								  \
		xo_open_container(contname);				  \
		xo_emit("{Lc:/%s}{P:/%s}", fieldname, padd);		  \
		xo_emit("{Lcw:(Total}");				  \
		xo_emit_field(hfield, "total", "%ju", NULL, totalsize);	  \
		xo_emit("{U:K} {Lwc:Active}");				  \
		xo_emit_field(hfield, "active", "%ju", NULL, activesize); \
		xo_emit("{U:K}{N:)}");					  \
		xo_close_container(contname);				  \
		xo_emit("{L:\n}");					  \
	} while (0)

	XOVM("virtual-memory", "Virtual Memory", "\t\t",
	    pg2k(v.t_vm), pg2k(v.t_avm));
	XOVM("real-memory", "Real Memory", "\t\t",
	    pg2k(v.t_rm), pg2k(v.t_arm));
	XOVM("shared-virtual-memory", "Shared Virtual Memory", "\t",
	    pg2k(v.t_vmshr), pg2k(v.t_avmshr));
	XOVM("shared-real-memory", "Shared Real Memory", "\t",
	    pg2k(v.t_rmshr), pg2k(v.t_armshr));

	xo_emit("{Lwc:Free Memory}{P:	}");
	xo_emit_field(hfield, "free-memory", "%ju", NULL, pg2k(v.t_free));
	xo_emit("{U:K}");

	xo_close_container("vmtotal");

	return (0);
}


#ifdef __amd64__
static int
S_efi_map(struct sysctlmif_object *object, bool hflag)
{
	//value get from SYSCTLMIF_GETVALUE() remove p, l2 is useless
	void *p = NULL;
	size_t l2 = 0;
	//----------------
	struct efi_map_header *efihdr;
	struct efi_md *map;
	const char *type;
	size_t efisz;
	int ndesc, i;

	static const char *const types[] =
	{
		[EFI_MD_TYPE_NULL] = "Reserved",
		[EFI_MD_TYPE_CODE] = "LoaderCode",
		[EFI_MD_TYPE_DATA] = "LoaderData",
		[EFI_MD_TYPE_BS_CODE] = "BootServicesCode",
		[EFI_MD_TYPE_BS_DATA] = "BootServicesData",
		[EFI_MD_TYPE_RT_CODE] = "RuntimeServicesCode",
		[EFI_MD_TYPE_RT_DATA] = "RuntimeServicesData",
		[EFI_MD_TYPE_FREE] = "ConventionalMemory",
		[EFI_MD_TYPE_BAD] = "UnusableMemory",
		[EFI_MD_TYPE_RECLAIM] = "ACPIReclaimMemory",
		[EFI_MD_TYPE_FIRMWARE] = "ACPIMemoryNVS",
		[EFI_MD_TYPE_IOMEM] = "MemoryMappedIO",
		[EFI_MD_TYPE_IOPORT] = "MemoryMappedIOPortSpace",
		[EFI_MD_TYPE_PALCODE] = "PalCode",
		[EFI_MD_TYPE_PERSISTENT] = "PersistentMemory",
	};

	/*
	 * Memory map data provided by UEFI via the GetMemoryMap
	 * Boot Services API.
	 */
	if (l2 < sizeof(*efihdr)) {
		warnx("S_efi_map length less than header");
		return (1);
	}
	efihdr = p;
	efisz = (sizeof(struct efi_map_header) + 0xf) & ~0xf;
	map = (struct efi_md *)((uint8_t *)efihdr + efisz);

	if (efihdr->descriptor_size == 0) {
		return (0);
	}
	if (l2 != efisz + efihdr->memory_size) {
		warnx("S_efi_map length mismatch %zu vs %zu", l2, efisz +
		    efihdr->memory_size);
		return (1);
	}
	ndesc = efihdr->memory_size / efihdr->descriptor_size;

	printf("\n%23s %12s %12s %8s %4s",
	    "Type", "Physical", "Virtual", "#Pages", "Attr");

	for (i = 0; i < ndesc; i++,
	    map = efi_next_descriptor(map, efihdr->descriptor_size)) {
		type = NULL;
		if (map->md_type < nitems(types)) {
			type = types[map->md_type];
		}
		if (type == NULL) {
			type = "<INVALID>";
		}
		printf("\n%23s %012jx %12p %08jx ", type,
		    (uintmax_t)map->md_phys, map->md_virt,
		    (uintmax_t)map->md_pages);
		if (map->md_attr & EFI_MD_ATTR_UC) {
			printf("UC ");
		}
		if (map->md_attr & EFI_MD_ATTR_WC) {
			printf("WC ");
		}
		if (map->md_attr & EFI_MD_ATTR_WT) {
			printf("WT ");
		}
		if (map->md_attr & EFI_MD_ATTR_WB) {
			printf("WB ");
		}
		if (map->md_attr & EFI_MD_ATTR_UCE) {
			printf("UCE ");
		}
		if (map->md_attr & EFI_MD_ATTR_WP) {
			printf("WP ");
		}
		if (map->md_attr & EFI_MD_ATTR_RP) {
			printf("RP ");
		}
		if (map->md_attr & EFI_MD_ATTR_XP) {
			printf("XP ");
		}
		if (map->md_attr & EFI_MD_ATTR_RT) {
			printf("RUNTIME");
		}
	}
	return (0);
}


#endif

#if defined(__amd64__) || defined(__i386__)
static int
S_bios_smap_xattr(struct sysctlmif_object *object, bool hflag)
{
	//value get from SYSCTLMIF_GETVALUE() remove p, l2 is useless
	void *p = NULL;
	size_t l2 = 0;
	//----------------
	struct bios_smap_xattr *smap, *end;

	if (l2 % sizeof(*smap) != 0) {
		warnx("S_bios_smap_xattr %zu is not a multiple of %zu", l2,
		    sizeof(*smap));
		return (1);
	}

	end = (struct bios_smap_xattr *)((char *)p + l2);
	for (smap = p; smap < end; smap++) {
		printf("\nSMAP type=%02x, xattr=%02x, base=%016jx, len=%016jx",
		    smap->type, smap->xattr, (uintmax_t)smap->base,
		    (uintmax_t)smap->length);
	}
	return (0);
}


#endif

static int
strIKtoi(const char *str, char **endptrp, const char *fmt)
{
	int kelv;
	float temp;
	size_t len;
	const char *p;
	int prec, i;

	assert(errno == 0);

	len = strlen(str);
	/* caller already checked this */
	assert(len > 0);

	/*
	 * A format of "IK" is in deciKelvin. A format of "IK3" is in
	 * milliKelvin. The single digit following IK is log10 of the
	 * multiplying factor to convert Kelvin into the untis of this sysctl,
	 * or the dividing factor to convert the sysctl value to Kelvin. Numbers
	 * larger than 6 will run into precision issues with 32-bit integers.
	 * Characters that aren't ASCII digits after the 'K' are ignored. No
	 * localization is present because this is an interface from the kernel
	 * to this program (eg not an end-user interface), so isdigit() isn't
	 * used here.
	 */
	if ((fmt[2] != '\0') && (fmt[2] >= '0') && (fmt[2] <= '9')) {
		prec = fmt[2] - '0';
	} else {
		prec = 1;
	}
	p = &str[len - 1];
	if ((*p == 'C') || (*p == 'F') || (*p == 'K')) {
		temp = strtof(str, endptrp);
		if ((*endptrp != str) && (*endptrp == p) && (errno == 0)) {
			if (*p == 'F') {
				temp = (temp - 32) * 5 / 9;
			}
			*endptrp = NULL;
			if (*p != 'K') {
				temp += 273.15;
			}
			for (i = 0; i < prec; i++) {
				temp *= 10.0;
			}
			return ((int)(temp + 0.5));
		}
	} else {
		/* No unit specified -> treat it as a raw number */
		kelv = (int)strtol(str, endptrp, 10);
		if ((*endptrp != str) && (*endptrp == p) && (errno == 0)) {
			*endptrp = NULL;
			return (kelv);
		}
	}

	errno = ERANGE;
	return (0);
}
