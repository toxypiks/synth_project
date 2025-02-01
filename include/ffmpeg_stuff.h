#ifndef FFMPEG_H_
#define FFMPEG_H_

#include <stddef.h>
#include <raylib.h>

typedef struct FfmpegStuff {
  RenderTexture2D screen;
  int fps;
  int pipe;
} FfmpegStuff;

#define READ_END 0
#define WRITE_END 1

int ffmpeg_start_rendering(size_t width, size_t height, size_t fps);
void ffmpeg_end_rendering(int pipe);
void ffmpeg_send_frame(int pipe, void *data, size_t width, size_t height);

#endif // FFMPEG_H
