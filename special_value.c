/*-
 * Copyright (c) 2019 Alfonso Sabato Siciliano
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * 'Special values' are values splitted for xo-output
 */

#include <stdbool.h>
#include <string.h>
#include <libxo/xo.h>

#include "special_value.h"

static int vm_phys_free(void* value, size_t value_size);

bool is_special_value(struct sysctlmif_object *object)
{
    bool special = false;

    special = strcmp(object->name,"vm.phys_free") == 0;

    return special;
}

int display_special_value(struct sysctlmif_object *object, void* value, size_t value_size)
{
    int error = 0;

    if( strcmp(object->name,"vm.phys_free") == 0)
	error += vm_phys_free(value, value_size);
    
    return error;
}

static int vm_phys_free(void* value, size_t value_size)
{
    int num_domain, num_list, num_pool, tmp, error;
    char *start, *end;
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    FILE *fp = fmemopen(value, value_size, "r");
    bool parselist;

    num_domain = error = 0;
    parselist = false;
    xo_emit("{L:\n}");
    xo_open_container("value");
    /*
     * XXX "domain" or "freelist" =>c xo_close_containes = <ZZZZZZZZZZZ>
     */
    while ((linelen = getline(&line, &linecap, fp)) > 0)
    {
	/* DOMAIN X :*/
	if( strstr(line, "DOMAIN") != NULL) {
	    if(num_domain > 0)
		xo_close_container("111cont");
	    xo_open_container("111cont");
	    xo_emit("{L:DOMAIN }{:num_domain/%d}{Lc:}{L:\n\n}", num_domain);
	    num_domain++;
	    num_list = 0;
	}

	/* FREE LIST Y :*/
	if( strstr(line, "FREE LIST") != NULL) {
	    if(num_list > 0)
		xo_close_container("222cont");
	    xo_open_container("222cont");
	    xo_emit("{L:FREE LIST }{:num_list/%d}{Lc:}{L:\n}", num_list);
	    num_list++;	    
	    free(line);
	    getline(&line, &linecap, fp);
	    xo_emit("{L:\n}");
	    free(line);
	    
	    /*  ORDER (SIZE)  |  NUMBER */
	    getline(&line, &linecap, fp);
	    xo_emit_field("L", line, NULL, NULL);
	    free(line);
	    /*                |  POOL 0  |  POOL 1 .... */
	    getline(&line, &linecap, fp);
	    xo_emit_field("L", line, NULL, NULL);
	    free(line);
	    /*--            -- --      -- --      -- ...*/
	    getline(&line, &linecap, fp);
	    xo_emit_field("L", line, NULL, NULL);
	    free(line);

	    /* Rows */
	    getline(&line, &linecap, fp);
	    xo_open_container("333cont");
	    xo_emit("{:num_ordermem/%d}{L:\n}", 5);
	    //xo_emit_field("V", "line", "%s", NULL, line);
	    xo_close_container("333cont");
	}

	free(line);
    }
    xo_close_container("222cont");
    xo_close_container("111cont");
    xo_close_container("value");

    return error;
}
