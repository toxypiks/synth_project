#ifndef FFMPEG_H_
#define FFMPEG_H_

#include <stddef.h>
#include <stdbool.h>

typedef struct FfmpegStuff {
  int fps;
  int pipe[2];
  bool enable;
} FfmpegStuff;

#define READ_END 0
#define WRITE_END 1

int ffmpeg_start_rendering(FfmpegStuff *ffmpeg_stuff, size_t width, size_t height);
void ffmpeg_end_rendering(FfmpegStuff *ffmpeg_stuff);
void ffmpeg_send_frame(FfmpegStuff *ffmpeg_stuff, void *data, size_t width, size_t height);

#endif // FFMPEG_H
