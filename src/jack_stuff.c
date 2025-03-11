#include "jack_stuff.h"
#include <jack/midiport.h>
#include <stdio.h>
#include "midi_msg.h"
#include <math.h>
#include <string.h>

int process(jack_nframes_t nframes, void* jack_stuff_raw)
{
  // audio part
  JackStuff* jack_stuff = (JackStuff*)jack_stuff_raw;
  float* output_buffer = (float*)jack_port_get_buffer (jack_stuff->output_port, nframes);

  if(jack_stuff->ringbuffer_audio){ // is buffer even there?
    // see if theres enough data in buffer to read nframes out of it (num_bytes >nframes)
    size_t num_bytes = jack_ringbuffer_read_space (jack_stuff->ringbuffer_audio);
    if(num_bytes >= (nframes* sizeof(float))) {
      jack_ringbuffer_read(jack_stuff->ringbuffer_audio, (char*)output_buffer, nframes * sizeof(float));
    } else {
      for (int i = 0; i < (int) nframes; i++)
      {
        output_buffer[i] = 0.0;
      }
    }
  }

  // midi part
  void* jack_midi_in_buffer =  jack_port_get_buffer ( jack_stuff->midi_in_port, nframes);

  // get number of midi events
  int event_count = jack_midi_get_event_count(jack_midi_in_buffer);

  for(size_t i = 0; i < event_count; ++i) {
    jack_midi_event_t ev;
    int read = jack_midi_event_get(&ev, jack_midi_in_buffer, i);
    if(ev.size == 3) {
      jack_nframes_t current_time_frame = jack_last_frame_time(jack_stuff->client);
      ev.time += current_time_frame;
      // jack_midi_event -> MidiMsg
      // A: Status Byte -> NoteOn NoteOff, Slider, knobs, Modwheel, ...
      // B:ch -> [0-15]
      // C:key -> 0-127  => freq
      // D:velocity -> 0-127 => 0-1.0
      // [AAAABBBB][0CCCCCCC][0DDDDDDD]
      uint8_t status_byte = ev.buffer[0] >> 4;
      uint8_t channel = ev.buffer[0] & 0x0f;
      uint8_t param1 = ev.buffer[1] & 0x7f;
      uint8_t param2 = ev.buffer[2] & 0x7f;
      printf("Midi: statusbyte:%d channel: %d, p1: %d, p2: %d \n", status_byte, channel, param1, param2);
      if (status_byte == 8 || status_byte == 9){
        bool is_on = status_byte == 9 ? true : false;
        MidiMsg midi_msg = {
            .freq  = 440.0 * pow(2.0, (param1 - 69.0)/12.0),
            .vel   = param2/127.0,
            .is_on = is_on,
            .time_stamp = ev.time
        };
        printf("MidiMsg: freq: %f vel: %f is_on: %d \n",
               midi_msg.freq,
               midi_msg.vel,
               midi_msg.is_on
        );
        // queue push
        int ret_adsr = lf_queue_push(&jack_stuff->midi_msg_queue, "midi_msg", (void*)&midi_msg, sizeof(MidiMsg));
      }
    }
  }
  return 0;
}

JackStuff* create_jack_stuff(char* client_name,size_t buffer_size){
  JackStuff* jack_stuff = (JackStuff*)malloc(sizeof(JackStuff));

  jack_stuff->output_port = 0;
  jack_stuff->ringbuffer_audio = NULL;
  jack_stuff->ringbuffer_video = NULL;
  jack_stuff->client = NULL;

  memset(&jack_stuff->midi_msg_queue, 0, sizeof(lf_queue_bss_state));
  memset(jack_stuff->midi_msg_array, 0, 16*sizeof(lf_queue_bss_element));
  lf_queue_init(&jack_stuff->midi_msg_queue, jack_stuff->midi_msg_array, 16);


  jack_stuff->client = jack_client_open (client_name,
                                        JackNullOption,
                                        0,
                                        0 );

  jack_stuff->output_port = jack_port_register (jack_stuff->client,
                                    "output",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput,
                                    0 );

  jack_stuff->midi_in_port = jack_port_register (jack_stuff->client,
                                                  "midi_input",
                                                  JACK_DEFAULT_MIDI_TYPE,
                                                  JackPortIsInput,
                                                  0 );

  size_t my_size = buffer_size * sizeof(float);
  jack_stuff->ringbuffer_audio = jack_ringbuffer_create(my_size);
  jack_stuff->ringbuffer_video = jack_ringbuffer_create(my_size);

  jack_set_process_callback(jack_stuff->client, process, (void*)jack_stuff);
  //client.set_sample_rate(48000);
  jack_activate(jack_stuff->client);
  return jack_stuff;
}

void jack_stuff_clear(JackStuff* jack_stuff) {
  jack_deactivate(jack_stuff->client);
  jack_client_close(jack_stuff->client);
  jack_ringbuffer_free(jack_stuff->ringbuffer_audio);
  free(jack_stuff);
}
