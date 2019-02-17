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

#include <sys/types.h>

#include <libxo/xo.h>
#include <stdbool.h>
#include <string.h>
#include <sysctlmibinfo.h>

static int vm_phys_free(void *value, size_t value_size);

/*
 * char value = "012\n3456\n789\0";
 * char *start, *end;
 * start = value;
 * while (find_line(start, &next, &value[strlen(value)+1])) {
 *	printf("%s\n", start);
 *	...
 *	start = next;
 * }
 */
static bool find_line(char *startline, char **endline, char *endbuffer)
{
    int i;
    char *e;

    if(startline == endbuffer)
	return false;

    e = startline;

    i=0;
    while( e[i] != '\n' && e[i] != '\0' ) {
	i++;
    }

    startline[i]='\0';    
    *endline = &(startline[i]);
    
    if(*endline != endbuffer)
	*endline = &(startline[i+1]);

    return true;
}


static bool find_int(char *start, char **end, int *intvalue)
{
    int j=0,i=0;
    char *s,*e;
    long long llvalue;

    s=start;

    while (true) {
	if(s[i] == '\n' || s[i] == '\0' || s[i] == EOF)
	    return false;
	if(s[i] >= '0' && s[i] <= '9')
	    break;
	i++;
    }

    e = &s[i];

    while (true) {
	if(e[j] == '\n' || e[j] == '\0' || e[j] == EOF || e[j] < '0' || e[j] > '9')
	    break;
	j++;
    }

    llvalue = strtoll(&s[i], NULL, 10);
    *intvalue = (int)llvalue;
    *end = &e[j];
    
    return true;
}

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
    char *line = NULL, *next;
    char poolstr[15]; /*"poolXXXXXXXXXX"*/

    num_domain = error = 0;
    xo_emit("{L:\n}");
    xo_open_container("value");
    line = value;
    while (find_line(line, &next, &(value[value_size]) )) {
	/* DOMAIN X : */
	if( strstr(line, "DOMAIN") != NULL) {
	    if(num_domain > 0)
		xo_close_container("domain");
	    xo_open_container("domain");
	    xo_emit("{L:DOMAIN }{:num_domain/%d}{Lc:}{L:\n}", num_domain);
	    num_domain++;
	    // '\n'
	    line=next;
	    find_line(line, &next, &(value[value_size]));
	    xo_emit("{L:\n}");
	    num_list = 0;
	}

	/* FREE LIST Y : */
	if( strstr(line, "FREE LIST") != NULL) {
	    xo_open_container("free-list");
	    xo_emit("{L:FREE LIST }{:num_list/%d}{Lc:}{L:\n}", num_list);
	    num_list++;
	    // '\n'
	    line=next;
	    find_line(line, &next, &(value[value_size]));
	    xo_emit("{L:\n}");
	    
	    //  ORDER (SIZE)  |  NUMBER
	    line=next;
	    find_line(line, &next, &(value[value_size]));
	    xo_emit_field("L", line, NULL, NULL);
	    xo_emit("{L:\n}");
	    //                |  POOL 0  |  POOL 1 ....
	    line=next;
	    find_line(line, &next, &(value[value_size]));
	    xo_emit_field("L", line, NULL, NULL);
	    xo_emit("{L:\n}");
	    // --            -- --      -- --      -- ...
	    line=next;  
	    find_line(line, &next, &(value[value_size]));
	    xo_emit_field("L", line, NULL, NULL);
	    xo_emit("{L:\n}");

	    // Rows
	    line=next;
	    find_line(line, &next, &(value[value_size]));
	    start = line;
	    while(find_int(start, &end, &tmp)) {
		xo_open_container("order");
		xo_emit("{:num_order/%4d}{Lw:}", tmp);
		start = end;
		find_int(start, &end, &tmp);
		xo_emit("{L:(}{:size/%6d}{U:K}{L:)}", tmp);
		start = end;
		num_pool=0;
		while(find_int(start, &end, &tmp)) { // Pools cols
		    //xo_emit("{L:|}{:pool/%5d}{L:|}", tmp);
		    xo_emit("{Lw: }{L:|}");
		    snprintf(poolstr, sizeof(poolstr), "pool%d", num_pool);
		    xo_emit_field("V",poolstr,"%8d", NULL, tmp);
		    start = end;
		    num_pool++;
		}
		xo_emit("{L:\n}");
		xo_close_container("order");
		line = next;
		find_line(line, &next, &(value[value_size]));
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
