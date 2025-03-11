#ifndef MIDI_MSG_H_
#define MIDI_MSG_H_
#include <stdbool.h>

typedef struct MidiMsg {
    float freq;
    float vel;
    bool is_on;
    size_t time_stamp;
} MidiMsg;

#endif // MIDI_MSG_H_
