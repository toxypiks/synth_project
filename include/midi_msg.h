#ifndef MIDI_MSG_H_
#define MIDI_MSG_H_
#include <stdbool.h>
#include <stddef.h>

typedef struct MidiMsg {
    float freq;
    float vel;
    bool is_on;
    size_t time_stamp;
} MidiMsg;

void set_midi_msg(void* midi_msg_new_raw, void* midi_msg_raw);

#endif // MIDI_MSG_H_
