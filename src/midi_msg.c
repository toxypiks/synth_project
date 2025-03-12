#include "midi_msg.h"
#include <string.h>

void set_midi_msg(void* midi_msg_new_raw, void* midi_msg_raw)
{
    MidiMsg* midi_msg = (MidiMsg*)midi_msg_raw;
    MidiMsg* midi_msg_new = (MidiMsg*)midi_msg_new_raw;
    memcpy(midi_msg, midi_msg_new, sizeof(MidiMsg));
}
