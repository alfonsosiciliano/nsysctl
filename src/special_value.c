/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019-2021 Alfonso Sabato Siciliano
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

/* 'Special values' are strings parsed for libxo output */

#include <libxo/xo.h>
#include <string.h>

#include "special_value.h"

static int vm_phys_free(void *value, size_t value_size);
static int debug_witness_fullgraph(void *value, size_t value_size);
static int kern_conftxt(void *value, size_t value_size);

/*
 * char *start, *next, sep;
 * start = value;
 * while (parse_string(start, &next, &value[value_size], sep)) {
 *	printf("%s\n", start);
 *	...
 *	start = next;
 * }
 */
bool parse_string(char *start, char **next, char *endbuffer, char sep)
{
    int i = 0;

    if(start == endbuffer)
	return false;

    while( start[i] != sep && start[i] != '\0' ) {
	i++;
    }

    start[i]='\0';    
    *next = &(start[i]);
    
    if(*next != endbuffer)
	*next = &(start[i+1]);

    return true;
}

static bool find_int(char *start, char **end, int *intvalue)
{
    int j=0,i=0;
    char *e;

    while (true) {
	if(start[i] == '\n' || start[i] == '\0' || start[i] == EOF)
	    return false;
	if(start[i] >= '0' && start[i] <= '9')
	    break;
	i++;
    }
    *intvalue = (int)strtoll(&start[i], NULL, 10);

    e = &start[i];
    while (true) {
	if(e[j] == '\n' || e[j] == '\0' || e[j] == EOF || e[j] < '0' || e[j] > '9')
	    break;
	j++;
    }
    *end = &e[j];

    return true;
}


/* API implementation */

bool is_special_value(struct sysctlmif_object *object)
{
    bool special = false;

    special = ( strcmp(object->name,"vm.phys_free") == 0 ||
		strcmp(object->name,"kern.conftxt") == 0 ||
		strcmp(object->name,"debug.witness.fullgraph") == 0);

    return special;
}

int display_special_value(struct sysctlmif_object *object, void* value,
			  size_t value_size)
{
    int error = 0;

    if( strcmp(object->name,"vm.phys_free") == 0)
	error += vm_phys_free(value, value_size);
    else if(strcmp(object->name, "kern.conftxt") == 0)
	error += kern_conftxt(value, value_size);
    else if(strcmp(object->name, "debug.witness.fullgraph") == 0)
	error += debug_witness_fullgraph(value, value_size);
    else
	error++;

    return error;
}

static int kern_conftxt(void *value, size_t value_size)
{
    int error = 0;
    char *line = value, *next, *n, *v;

    xo_open_container("value");
    while(parse_string(line, &next, &value[value_size], '\n')) {
	if(line[0] != '\0') {
	    xo_open_container("configuration");
	    n= line;
	    parse_string(n, &v, next, '\t');
	    xo_emit("{:name/%s}{L:\t}", n);
	    xo_emit("{:value/%s}{L:\n}", v);
	    xo_close_container("configuration");
	    }
	line = next;
    }
    xo_close_container("value");

    return  error;
}

static int vm_phys_free(void* value, size_t value_size)
{
    int num_domain, num_list, num_pool, tmp, error;
    char *start, *end;
    char *line = NULL, *next;
    char poolstr[15];

    num_domain = error = 0;
    xo_emit("{L:\n}");
    xo_open_container("value");
    line = value;
    while (parse_string(line, &next, &(value[value_size]), '\n')) {
	// DOMAIN X :
	if( strstr(line, "DOMAIN") != NULL) {
	    if(num_domain > 0)
		xo_close_container("domain");
	    xo_open_container("domain");
	    xo_emit("{L:DOMAIN }{:num-domain/%d}{Lc:}{L:\n}", num_domain);
	    num_domain++;
	    // '\n'
	    line=next;
	    parse_string(line, &next, &(value[value_size]), '\n');
	    xo_emit("{L:\n}");
	    num_list = 0;
	}

	// FREE LIST Y :
	if( strstr(line, "FREE LIST") != NULL) {
	    xo_open_container("free-list");
	    if(num_list > 0) //not \n by domain
		xo_emit("{L:\n}");
	    xo_emit("{L:FREE LIST }{:num-list/%d}{Lc:}{L:\n}", num_list);
	    num_list++;
	    // '\n'
	    line=next;
	    parse_string(line, &next, &(value[value_size]),'\n');
	    xo_emit("{L:\n}");
	    
	    //  ORDER (SIZE)  |  NUMBER
	    line=next;
	    parse_string(line, &next, &(value[value_size]), '\n');
	    xo_emit_field("L", line, NULL, NULL);
	    xo_emit("{L:\n}");
	    //                |  POOL 0  |  POOL 1 ....
	    line=next;
	    parse_string(line, &next, &(value[value_size]), '\n');
	    xo_emit_field("L", line, NULL, NULL);
	    xo_emit("{L:\n}");
	    // --            -- --      -- --      -- ...
	    line=next;  
	    parse_string(line, &next, &(value[value_size]), '\n');
	    xo_emit_field("L", line, NULL, NULL);
	    xo_emit("{L:\n}");

	    // Rows
	    line=next;
	    parse_string(line, &next, &(value[value_size]), '\n');
	    start = line;
	    while(find_int(start, &end, &tmp)) {
		xo_open_container("order");
		xo_emit("{:num-order/%4d}{Lw:}", tmp);
		start = end;
		find_int(start, &end, &tmp);
		xo_emit("{L:(}{:size/%6d}{U:K}{L:)}", tmp);
		start = end;
		num_pool=0;
		while(find_int(start, &end, &tmp)) { // Pools cols
		    xo_emit("{Lw: }{L:|}");
		    snprintf(poolstr, sizeof(poolstr), "pool%d", num_pool);
		    xo_emit_field("V",poolstr,"%8d", NULL, tmp);
		    start = end;
		    num_pool++;
		}
		xo_emit("{L:\n}");
		xo_close_container("order");
		line = next;
		parse_string(line, &next, &(value[value_size]), '\n');
		start=line;
		}
	    xo_close_container("free-list");
	}//if "LIST FREE"	
	line=next;
    }// while line
    xo_close_container("domain");
    xo_close_container("value");
    
    return error;
}

static int debug_witness_fullgraph(void *value, size_t value_size)
{
    char *line, *next;

    xo_open_container("value");
    line = value;
    xo_emit("{L:\n}");
    while(parse_string(line, &next, &value[value_size], '\n')) {
	if(line[0] != '\0') {
	    xo_open_container("property");
	    parse_string(line, &next, next, ',');
	    xo_emit("{:name/%s}{L:,}", line);
	    line=next;
	    parse_string(line, &next, &value[value_size], '\n');
	    xo_emit("{:value/%s}{L:\n}", line);
	    xo_close_container("property");
	}

	line = next;
    }
    xo_close_container("value");

    return 0;
}

