/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 *  Alfonso Sabato Siciliano added sysctlmibinfo2 and libxo features,
 *      Original: https://cgit.freebsd.org/src/tree/sbin/sysctl/sysctl.c
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

#include <sys/param.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/vmmeter.h>
#include <dev/evdev/input.h>

#ifdef __amd64__
#include <sys/efi.h>
#include <x86/metadata.h>
#endif

#if defined(__amd64__) || defined(__i386__)
#include <machine/pc/bios.h>
#endif

#include <assert.h>     //assert
#include <stdbool.h>
#include <stdlib.h>     //free
#include <string.h>     //strdup
#include <unistd.h>     //getpagesize

#include <libxo/xo.h>
#include <sysctlmibinfo2.h>

/* Func declarations */
static int S_clockinfo(void *value, size_t value_size, bool hflag);
static int S_input_id(void *value, size_t value_size, bool hflag);
static int S_loadavg(void *value, size_t value_size, bool hflag);
static int S_timeval(void *value, size_t value_size, bool hflag);
static int S_vmtotal(void *value, size_t value_size, bool hflag);
#ifdef __amd64__
static int S_efi_map(void *value, size_t value_size, bool hflag);
#endif
#if defined(__amd64__) || defined(__i386__)
static int S_bios_smap_xattr(void *value, size_t value_size, bool hflag);
#endif
int strIKtoi(const char *str, char **endptrp, const char *fmt);

bool is_opaque_defined(struct sysctlmif_object *object)
{
    if ( strcmp(object->fmt, "S,clockinfo") == 0 ||
#if defined(__amd64__) || defined(__i386__)
	strcmp(object->fmt, "S,bios_smap_xattr") == 0 ||
#endif
#ifdef __amd64__
	strcmp(object->fmt, "S,efi_map_header") == 0 ||
#endif
	 strcmp(object->fmt, "S,loadavg") == 0  ||
	 strcmp(object->fmt, "S,timeval") == 0  ||
	 strcmp(object->fmt, "S,input_id") == 0 ||
	 strcmp(object->fmt, "S,vmtotal") == 0)
	return true;

    return false;
}

int 
display_opaque_value(struct sysctlmif_object *object, void *value,
		     size_t value_size, bool hflag, bool oflag, bool xflag)
{
	int error = 0;
	int i;
	int to = value_size;
	
	xo_open_container("value");
	
	if (strcmp(object->fmt, "S,clockinfo") == 0) {
		error += S_clockinfo(value, value_size, hflag);
	} else if (strcmp(object->fmt, "S,timeval") == 0) {
		error += S_timeval(value, value_size, hflag);
	} else if (strcmp(object->fmt, "S,loadavg") == 0) {
		error += S_loadavg(value, value_size, hflag);
	} else if (strcmp(object->fmt, "S,vmtotal") == 0) {
		error += S_vmtotal(value, value_size, hflag);
	} else if (strcmp(object->fmt, "S,input_id") == 0) {
		error += S_input_id(value, value_size, hflag);
	}
#ifdef __amd64__
	else if (strcmp(object->fmt, "S,efi_map_header") == 0) {
		error += S_efi_map(value, value_size, hflag);
	}
#endif
#if defined(__amd64__) || defined(__i386__)
	else if (strcmp(object->fmt, "S,bios_smap_xattr") == 0) {
		error += S_bios_smap_xattr(value, value_size, hflag);
	}
#endif
	else if (oflag || xflag) {
	    	xo_open_container(object->fmt + 2);
		xo_emit("{Lc:Format}{:format/%s}", object->fmt);
		xo_emit("{P: }{Lc:Length}{:lenght/%lu}", value_size);
		xo_emit("{P: }{Lc:Dump}0x" /*{:dump/%16x}...", value*/);

		if(oflag)
		    to = 16 < value_size ? 16 : value_size;
		
		for (i = 0; i < to; i++) {
			xo_emit("{:dump/%02x}", ((unsigned char*)value)[i]);
		}
		if (oflag && (value_size >= 8)) {
			xo_emit("{L:...}");
		}

		xo_close_container(object->fmt + 2);
	}

	xo_close_container("value");

	return error;
}


static int
S_clockinfo(void *value, size_t value_size, bool hflag)
{
	struct clockinfo *ci = (struct clockinfo*)value;
	char *hfield = hflag ? "h" : NULL;

	if (value_size != sizeof(*ci)) {
		xo_warnx("S_clockinfo %zu != %zu", value_size, sizeof(*ci));
		return (1);
	}
       
	xo_open_container("clockinfo");

	xo_emit("{L:{ }");
	xo_emit("{Lw:hz =}");
	xo_emit_field(hfield, "hz", "%d", NULL, ci->hz);
	xo_emit(", {Lw:tick =}");
	xo_emit_field(hfield, "tick", "%d", NULL, ci->tick);
	xo_emit(", {Lw:profhz =}");
	xo_emit_field(hfield, "profhz", "%d", NULL, ci->profhz);
	xo_emit(", {Lw:stathz =}");
	xo_emit_field(hfield, "stathz", "%d", NULL, ci->stathz);
	xo_emit("{N: }}");

	xo_close_container("clockinfo");

	return (0);
}


static int
S_loadavg(void *value, size_t value_size, bool hflag)
{
	struct loadavg *tv = (struct loadavg*)value;
	char *hfield = /*hflag ? "h" :*/ NULL;
	/* libxo 'h' modifier does not affect the size and treatment of %f */

	if (value_size != sizeof(*tv)) {
		xo_warnx("S_loadavg %zu != %zu", value_size, sizeof(*tv));
		return (1);
	}

#define TV_FSCALE(idx)    ((double)tv->ldavg[idx]/(double)tv->fscale)

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
S_input_id(void *value, size_t value_size, bool hflag)
{
    struct input_id *id = (struct input_id *)value;
    /*libxo 'h' modifier does not affect the size and treatment of %f */
    char *hfield = /*hflag ? "h" :*/ NULL;
	 	 
    if (value_size != sizeof(*id)) {
	xo_warnx("S_input_id %zu != %zu", value_size, sizeof(struct input_id));
	return (1);
    }

    xo_open_container("input_id");
    xo_emit("{L:{ bustype = 0x}");
    xo_emit_field(hfield, "bustype", "%.4x", NULL, id->bustype);
    xo_emit("{L:, vendor = 0x}");
    xo_emit_field(hfield, "vendor", "%.4x", NULL, id->vendor);
    xo_emit("{L:, product = 0x}");
    xo_emit_field(hfield, "product", "%.4x", NULL, id->product);
    xo_emit("{L:, version = 0x}");
    xo_emit_field(hfield, "version", "%.4x", NULL, id->version);
    xo_emit("{N: }}");
    xo_close_container("input_id");
    
    return (0);
}

static int
S_timeval(void *value, size_t value_size, bool hflag)
{
	struct timeval *tv = (struct timeval*)value;
	time_t tv_sec;
	char *p1;
	char *hfield = hflag ? "h,hn-decimal" : NULL;

	if (value_size != sizeof(*tv)) {
		xo_warnx("S_timeval %zu != %zu", value_size, sizeof(*tv));
		return (1);
	}
	xo_open_container("timeval");

	xo_emit("{Lw:{ sec =}");
	xo_emit_field(hfield, "sec", "%jd", NULL, (intmax_t)tv->tv_sec);
	xo_emit(", {Lw:usec =}");
	xo_emit_field(hfield, "usec", "%ld", NULL, tv->tv_usec);
	xo_emit("{Nw:}}");

	tv_sec = tv->tv_sec;
	p1 = strdup(ctime(&tv_sec));
	p1[strlen(p1) -1] = '\0';
	xo_emit("{P: }{:date/%s}", p1);
	free(p1);

	xo_close_container("timeval");

	return (0);
}


static int
S_vmtotal(void *value, size_t value_size, bool hflag)
{
	struct vmtotal *v = (struct vmtotal*)value;
	int pageKilo;
	char *hfield = hflag ? "h,hn-decimal" : NULL;

	if (value_size != sizeof(struct vmtotal)) {
		xo_warnx("S_vmtotal %zu != %zu", value_size, sizeof(*v));
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
	xo_emit_field(hfield, "runq", "%d", NULL, v->t_rq);
	xo_emit(" {Lwc:Disk Wait}");
	xo_emit_field(hfield, "disk-wait", "%d", NULL, v->t_dw);
	xo_emit(" {Lwc:Page Wait}");
	xo_emit_field(hfield, "page-wait", "%d", NULL, v->t_pw);
	xo_emit(" {Lwc:Sleep}");
	xo_emit_field(hfield, "sleep", "%d", NULL, v->t_sl);
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
	    pg2k(v->t_vm), pg2k(v->t_avm));
	XOVM("real-memory", "Real Memory", "\t\t",
	    pg2k(v->t_rm), pg2k(v->t_arm));
	XOVM("shared-virtual-memory", "Shared Virtual Memory", "\t",
	    pg2k(v->t_vmshr), pg2k(v->t_avmshr));
	XOVM("shared-real-memory", "Shared Real Memory", "\t",
	    pg2k(v->t_rmshr), pg2k(v->t_armshr));

	xo_emit("{Lwc:Free Memory}{P:	}");
	xo_emit_field(hfield, "free-memory", "%ju", NULL, pg2k(v->t_free));
	xo_emit("{U:K}");

	xo_close_container("vmtotal");

	return (0);
}


#ifdef __amd64__
static int
S_efi_map(void *value, size_t value_size, bool hflag)
{
	struct efi_map_header *efihdr;
	struct efi_md *map;
	const char *type;
	size_t efisz;
	int ndesc, i;

	static const char * const types[] = {
		[EFI_MD_TYPE_NULL] =	"Reserved",
		[EFI_MD_TYPE_CODE] =	"LoaderCode",
		[EFI_MD_TYPE_DATA] =	"LoaderData",
		[EFI_MD_TYPE_BS_CODE] =	"BootServicesCode",
		[EFI_MD_TYPE_BS_DATA] =	"BootServicesData",
		[EFI_MD_TYPE_RT_CODE] =	"RuntimeServicesCode",
		[EFI_MD_TYPE_RT_DATA] =	"RuntimeServicesData",
		[EFI_MD_TYPE_FREE] =	"ConventionalMemory",
		[EFI_MD_TYPE_BAD] =	"UnusableMemory",
		[EFI_MD_TYPE_RECLAIM] =	"ACPIReclaimMemory",
		[EFI_MD_TYPE_FIRMWARE] = "ACPIMemoryNVS",
		[EFI_MD_TYPE_IOMEM] =	"MemoryMappedIO",
		[EFI_MD_TYPE_IOPORT] =	"MemoryMappedIOPortSpace",
		[EFI_MD_TYPE_PALCODE] =	"PalCode",
		[EFI_MD_TYPE_PERSISTENT] = "PersistentMemory",
	};

	/*
	 * Memory map data provided by UEFI via the GetMemoryMap
	 * Boot Services API.
	 */
	if (value_size < sizeof(*efihdr)) {
		xo_warnx("S_efi_map length less than header");
		return (1);
	}
	efihdr = value;
	efisz = (sizeof(struct efi_map_header) + 0xf) & ~0xf;
	map = (struct efi_md *)((uint8_t *)efihdr + efisz);

	if (efihdr->descriptor_size == 0)
		return (0);
	if (value_size != efisz + efihdr->memory_size) {
		xo_warnx("S_efi_map length mismatch %zu vs %zu", value_size, efisz +
		    efihdr->memory_size);
		return (1);
	}
	ndesc = efihdr->memory_size / efihdr->descriptor_size;

	xo_emit("{L:\n}{L:/%23s}{L:/%12s}{L:/%12s}{L:/%8s}{L:/%4s}",
	    "Type", "Physical", "Virtual", "#Pages", "Attr");

	for (i = 0; i < ndesc; i++,
	    map = efi_next_descriptor(map, efihdr->descriptor_size)) {
		xo_open_container("md");
		type = NULL;
		if (map->md_type < nitems(types))
			type = types[map->md_type];
		if (type == NULL)
			type = "<INVALID>";
		xo_emit("{L:\n}{:types/%23s}{L: }{:md_phys/%012jx}",
			type, (uintmax_t)map->md_phys);
		xo_emit("{L: }{:md_virt/%12p}{L: }{:md_pages/%08jx}{L: }",
			map->md_virt, (uintmax_t)map->md_pages);
		xo_open_container("md_attrs");
		if (map->md_attr & EFI_MD_ATTR_UC)
			xo_emit("{:md_attr/%s}{L: }","UC");
		if (map->md_attr & EFI_MD_ATTR_WC)
			xo_emit("{:md_attr/%s}{L: }","WC");
		if (map->md_attr & EFI_MD_ATTR_WT)
			xo_emit("{:md_attr/%s}{L: }","WT");
		if (map->md_attr & EFI_MD_ATTR_WB)
			xo_emit("{:md_attr/%s}{L: }","WB");
		if (map->md_attr & EFI_MD_ATTR_UCE)
			xo_emit("{:md_attr/%s}{L: }","UCE");
		if (map->md_attr & EFI_MD_ATTR_WP)
			xo_emit("{:md_attr/%s}{L: }","WP");
		if (map->md_attr & EFI_MD_ATTR_RP)
			xo_emit("{:md_attr/%s}{L: }","RP");
		if (map->md_attr & EFI_MD_ATTR_XP)
			xo_emit("{:md_attr/%s}{L: }","XP");
		if (map->md_attr & EFI_MD_ATTR_RT)
			xo_emit("{:md_attr/%s}","RUNTIME");
		xo_close_container("md_attrs");
		xo_close_container("md");
	}
	return (0);
}
#endif

#if defined(__amd64__) || defined(__i386__)
static int
S_bios_smap_xattr(void* value, size_t value_size, bool hflag)
{
	struct bios_smap_xattr *smap, *end;

	if (value_size % sizeof(*smap) != 0) {
		xo_warnx("S_bios_smap_xattr %zu is not a multiple of %zu", 
			 value_size, sizeof(*smap));
		return (1);
	}

	end = (struct bios_smap_xattr *)((char *)value + value_size);
	for (smap = value; smap < end; smap++) {
		xo_open_container("SMAP");
		xo_emit("{L:\n}");
		xo_emit("{L:SMAP }");
		xo_emit("{L:type=}{V:type/%02x}{L:, }", smap->type);
		xo_emit("{L:xattr=}{V:xattr/%02x}{L:, }", smap->xattr);
		xo_emit("{L:base=}{V:base/%016jx}{L:, }",(uintmax_t)smap->base);
		xo_emit("{L:len=}{V:len/%016jx}", (uintmax_t)smap->length);
		xo_close_container("SMAP");
	}

	return (0);
}
#endif


/* strIK_to_int() and display_IK_value() could be in kelvin.c */
int
strIK_to_int(const char *str, int *kelvin, const char *fmt)
{
	float temp;
	size_t len;
	const char *p;
	char *endptrp;
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
		temp = strtof(str, &endptrp);
		if ((endptrp != str) && (endptrp == p) && (errno == 0)) {
			if (*p == 'F') {
				temp = (temp - 32) * 5 / 9;
			}
			endptrp = NULL;
			if (*p != 'K') {
				temp += 273.15;
			}
			for (i = 0; i < prec; i++) {
				temp *= 10.0;
			}
			*kelvin = ((int)(temp + 0.5));
			return 0;
		}
	} else {
		/* No unit specified -> treat it as a raw number */
		*kelvin = (int)strtol(str, &endptrp, 10);
		if ((endptrp != str) && (endptrp == p) && (errno == 0)) {
			endptrp = NULL;
			return (0);
		}
	}

	return (1);
}

int
display_IK_value(struct sysctlmif_object *obj, void *value, size_t value_size,
		 bool hflag)
{
    int i, prec = 1, intvalue = *((int*)value);
    float base;
    
    if (obj->fmt[2] != '\0')
	prec = obj->fmt[2] - '0';
    base = 1.0;
    for (i = 0; i < prec; i++)
	base *= 10.0;

    xo_emit(hflag ? "{h:value/%f}{U:C}" : "{:value/%f}{U:C}", 
	    (float)intvalue / base - 273.15);

    return 0;
}
