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

#include <string.h>
#include <libxo/xo.h>

#include "special_value.h"

static bool get_next_int(int);
static int vm_phys_free(void* value, size_t value_size);

bool is_special_value(struct sysctlmif_object *object)
{
    bool special = false;

    // XXX special = strcmp(object->name,"vm.phys_free") == 0;

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
    int num_domain, num_free_list, error;
    char *valuestr, *sentinel;
    char *domain = "DOMAIN";
    char *free_list = "FREE LIST";
    char *dashdash = "--";

    num_domain = error = 0;
    valuestr = (char*) value;

    //printf("%s\n", value);
    xo_open_container("value");
    while( (sentinel = strstr(valuestr, domain)) != NULL) {
	xo_open_container("Domain");
	xo_emit("{:num_domain/%d}",num_domain);
	num_free_list = 0;
	while( (sentinel = strstr(valuestr, free_list)) != NULL) {
	    xo_open_container("Free_List");
	    xo_emit("{:num_free_list/%d}", num_free_list);
	    xo_close_container("Free_List");
	    num_free_list++;
	    valuestr = sentinel;
	    if(valuestr != NULL)
		valuestr +=1;
	}
	xo_close_container("Domain");
	num_domain++;
	valuestr = sentinel;
	if(valuestr != NULL)
	    valuestr +=1;
    }
    xo_close_container("value");
    
    return error;
}
