#include <stdio.h>
#include <jack/ringbuffer.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "oscillator.h"
#include "ray_out_buffer.h"
#include "jack_stuff.h"
#include "ui_stuff.h"
#include "ffmpeg_stuff.h"
#include "thread_stuff.h"
#include "model_gen_signal.h"
#include "adsr.h"
#include "lf_queue.h"
#include "msg_handler.h"

int main(void) {

    FfmpegStuff ffmpeg_stuff = {0};
    ffmpeg_stuff.enable = false;
    ffmpeg_stuff.fps = 60;

    size_t window_factor = 80;
    size_t screen_width = (16*window_factor);
    size_t screen_height = (9*window_factor);

    int ret = ffmpeg_start_rendering(&ffmpeg_stuff, screen_width, screen_height);
    if (ret != 0) {
        return -1;
    }

    // adsr view Ui stuff and model
    float adsr_height = 0.0f;
    float adsr_length = 0.0f;

    JackStuff* jack_stuff = create_jack_stuff("SineWaveWithJack", 192000);
    ThreadStuff* thread_stuff = create_thread_stuff(jack_stuff);

    // start model thread
    pthread_t model_gen_signal_thread;
    pthread_create(&model_gen_signal_thread, NULL, model_gen_signal_thread_fct, (void*) thread_stuff);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screen_width, screen_height, "sine_wave");
    SetTargetFPS(ffmpeg_stuff.fps);
    RayOutBuffer ray_out_buffer = create_ray_out_buffer(10000);

    UiStuff* ui_stuff = create_ui_stuff(screen_width, screen_height);
    LayoutStack ls = {0};

    bool is_play_pressed = false;
    bool is_reset_pressed = false;

    MsgHdl msg_hdl = {0};

    msg_hdl_add_key2fct(&msg_hdl, "adsr_height", set_float_value, (void*)&adsr_height);
    msg_hdl_add_key2fct(&msg_hdl, "adsr_length", set_float_value, (void*)&adsr_length);

    while(!WindowShouldClose()) {
        msg_hdling(&msg_hdl, &thread_stuff->raylib_msg_queue);

        size_t num_bytes = jack_ringbuffer_read_space(jack_stuff->ringbuffer_audio);
        // TODO ~Setter for text ->better update for ui_stuff
        // TODO Seperate value for label from actual parameter for change frequency
        ui_stuff->text.freq = 50.0 + 1000.0 * ui_stuff->slider_freq.scroll;
        ui_stuff->text.vol = 1.0 * ui_stuff->slider_vol.scroll;

        ADSR* adsr_msg = malloc(sizeof(ADSR));
        adsr_msg->attack = ui_stuff->adsr.attack.scroll;
        adsr_msg->decay = ui_stuff->adsr.decay.scroll;
        adsr_msg->sustain = ui_stuff->adsr.sustain.scroll;
        adsr_msg->release = ui_stuff->adsr.release.scroll;
        int ret_adsr = lf_queue_push(&thread_stuff->model_msg_queue, "adsr", adsr_msg);

        // send messages through msg_queue
        float *vol_msg = malloc(sizeof(float));
        *vol_msg = ui_stuff->text.vol;
        int ret_vol = lf_queue_push(&thread_stuff->model_msg_queue, "vol", vol_msg);

        float *freq_msg = malloc(sizeof(float));
        *freq_msg = ui_stuff->text.freq;
        int ret_freq = lf_queue_push(&thread_stuff->model_msg_queue, "freq", freq_msg);

        bool *is_play_pressed_msg = malloc(sizeof(bool));
        *is_play_pressed_msg = is_play_pressed;
        int ret_is_play_pressed = lf_queue_push(&thread_stuff->model_msg_queue, "is_play_pressed", is_play_pressed_msg);


        if(jack_stuff->ringbuffer_video){
            float output_buffer[1024];
            size_t num_bytes = jack_ringbuffer_read_space(jack_stuff->ringbuffer_video);
            if(num_bytes >= (1024* sizeof(float))) {

                jack_ringbuffer_read(jack_stuff->ringbuffer_video, (char*)output_buffer, 1024 * sizeof(float));
            } else {
                for ( int i = 0; i < 1024; i++)
                {
                    output_buffer[i] = 0.0;
                }
            }
            copy_to_ray_out_buffer(&ray_out_buffer, output_buffer, 1024);
        }

        float w = GetRenderWidth();
        float h = GetRenderHeight();

        BeginDrawing();
        // TODO check record toggle on
        BeginTextureMode(ui_stuff->screen);
        ClearBackground(BLACK);

        layout_stack_push(&ls, LO_VERT, ui_rect(0, 0, w, h), 3, 0);
        layout_stack_push(&ls, LO_HORZ, layout_stack_slot(&ls), 3, 0);
        start_button_widget(layout_stack_slot(&ls), PINK, &is_play_pressed);
        reset_button_widget(layout_stack_slot(&ls), PINK, &is_reset_pressed);
        text_widget(layout_stack_slot(&ls), &ui_stuff->text);
        layout_stack_pop(&ls);
        signal_widget(layout_stack_slot(&ls), &ray_out_buffer, BLUE);
        layout_stack_push(&ls, LO_HORZ, layout_stack_slot(&ls), 3, 0);
        slider_widget(layout_stack_slot(&ls), &ui_stuff->slider_vol);
        adsr_widget(layout_stack_slot(&ls), &ui_stuff->adsr, adsr_height, adsr_length);
        slider_widget(layout_stack_slot(&ls), &ui_stuff->slider_freq);
        layout_stack_pop(&ls);
        layout_stack_pop(&ls);
        EndTextureMode();

        Vector2 pos_rect = {0,0};
        Rectangle flip_rect = {0, 0, screen_width, -1 * (int)screen_height};

        DrawTextureRec(ui_stuff->screen.texture,
        flip_rect,
        pos_rect,
        WHITE);

        EndDrawing();
        assert(ls.count == 0);

        Image image = LoadImageFromTexture(ui_stuff->screen.texture);
        ffmpeg_send_frame(&ffmpeg_stuff, image.data, screen_width, screen_height);
        // TODO check if screen_width and screen_height can change <- window resizable

        UnloadImage(image);

        if(is_reset_pressed) {
            ui_stuff->adsr.attack.scroll = 0.05f;
            ui_stuff->adsr.decay.scroll = 0.25f;
            ui_stuff->adsr.sustain.scroll = 0.5f;
            ui_stuff->adsr.release.scroll = 0.2;
        }
    }
    CloseWindow();
    ffmpeg_end_rendering(&ffmpeg_stuff);

    thread_stuff->is_running = false;
    pthread_join(model_gen_signal_thread, NULL);

    ui_stuff_clear(ui_stuff);
    jack_stuff_clear(jack_stuff);
    return 0;
}
