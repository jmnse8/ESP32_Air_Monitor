#include <stdio.h>
#include <string.h>

#include "context.h"

static char *NODE_CONTEXT = "2/3";

void context_set_node_ctx(char *c){
    if (c != NULL) {
        NODE_CONTEXT = malloc(strlen(c) + 1); // +1 for the null terminator
        strcpy(NODE_CONTEXT, c);
    }
}

char *context_get_node_ctx(){
    return NODE_CONTEXT;
}
