#ifndef FFMPEG_H_
#define FFMPEG_H_

#include <stddef.h>

#define READ_END 0
#define WRITE_END 1

int ffmpeg_start_rendering(size_t width, size_t height, size_t fps);
void ffmpeg_end_rendering(int pipe);

#endif // FFMPEG_H
