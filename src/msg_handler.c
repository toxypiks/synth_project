#include "msg_handler.h"
#include "lf_queue.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// Adds key, fct and datastruct to msg_hdl
int msg_hdl_add_key2fct(MsgHdl* msg_hdl, char* key, void (*fct)(void*, void*), void* datastruct)
{
    if (msg_hdl->nkeys < 5) {
        msg_hdl->key2fct[msg_hdl->nkeys].key = key;
        msg_hdl->key2fct[msg_hdl->nkeys].fct = fct;
        msg_hdl->key2fct[msg_hdl->nkeys].datastruct = datastruct;
        msg_hdl->nkeys++;
    } else {
        return -1;
    }
    return 0;
}

// Extracts key and value from msg_queue and compares key of queue with key of msg_hdl
void msg_hdling(MsgHdl* msg_hdl, lf_queue_bss_state* msg_queue)
{
    char* key = NULL;
    void* value = NULL;

    while (true) {
        int ret = lf_queue_pop(msg_queue, (void**)&key, &value);
        if (ret == 0) return;
        for (size_t i = 0; i < msg_hdl->nkeys; ++i) {
            if (strcmp(key, msg_hdl->key2fct[i].key) == 0) {
                msg_hdl->key2fct[i].fct(value,
                msg_hdl->key2fct[i].datastruct);
            }
        }
        free(key);
        free(value);
        key = NULL;
        value = NULL;
    }
}

void set_float_value(void* new_value_raw, void* value_raw)
{
    float* new_value = (float*)new_value_raw;
    float* value = (float*)value_raw;
    *value = *new_value;
}

void set_bool_value(void* new_value_raw, void* value_raw)
{
    bool* new_value = (bool*)new_value_raw;
    bool* value = (bool*)value_raw;
    *value = *new_value;
}
