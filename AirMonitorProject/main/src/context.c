#include <stdio.h>
#include <string.h>

#include "context.h"

static char *NODE_CONTEXT = "2/3";
static int NODE_ONOFF_STATUS = CONTEXT_ON;

int NODE_STATUS = NODE_STATE_SIGUP_DEVICE2TB_STATE;

void context_set_node_ctx(char *c){
    if (c != NULL) {
        NODE_CONTEXT = malloc(strlen(c) + 1); // +1 for the null terminator
        strcpy(NODE_CONTEXT, c);
    }
}

char *context_get_node_ctx(){
    return NODE_CONTEXT;
}

int context_it_is_i(char * ctx){
    return strncmp(NODE_CONTEXT, ctx, strlen(NODE_CONTEXT));
}

int context_get_onoff(){
    return NODE_ONOFF_STATUS;
}

void context_set_onoff(int onoff){
    NODE_ONOFF_STATUS = onoff;
}


