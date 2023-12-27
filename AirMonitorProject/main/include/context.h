#ifndef __CONTEXT_H
#define __CONTEXT_H

enum {
    CONTEXT_ON,
    CONTEXT_OFF
};

/*
    Set node floor/room context
*/
void context_set_node_ctx(char *c);

/*
    Get node floor/room context
*/
char *context_get_node_ctx();

/*
    Check if node context == ctx
*/
int context_it_is_i(char * ctx);

/*
    Check if node is on or off
*/
int context_get_onoff();

/*
    Set onoff status to onoff
*/
void context_set_onoff(int onoff);
#endif