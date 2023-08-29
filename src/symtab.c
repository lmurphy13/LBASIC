/**
 * LBASIC Symbol Table Module
 * File: symtab.h
 * Author: Liam M. Murphy
 */

#include "symtab.h"

#include "error.h"

#include <stdlib.h>
#include <string.h>

binding_t *mk_binding() {
    binding_t *retval = (binding_t *)malloc(sizeof(binding_t));

    if (retval != NULL) {
        memset(retval, 0, sizeof(binding_t));
    } else {
        log_error("Unable to allocate binding_t");
    }

    return retval;
}
