#ifndef MSG_HANDLER_H_
#define MSG_HANDLER_H_

#include "lf_queue.h"
#include <string.h>

typedef struct Key2Fct {
    char* key;
    void (*fct)(void*, void*);
    void* datastruct;
} Key2Fct;

typedef struct MsgHdl{
    Key2Fct key2fct[5];
    size_t nkeys;
} MsgHdl;

int msg_hdl_add_key2fct(MsgHdl* msg_hdl, char* key, void (*fct)(void*, void*), void* datastruct);

void msg_hdling(MsgHdl* msg_hdl, lf_queue_bss_state* msg_queue);
void set_float_value(void* new_value_raw, void* value_raw);
void set_bool_value(void* new_value_raw, void* value_raw);

#endif // MSG_HANDLER_H_
