#include "ffmpeg_stuff.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int ffmpeg_start_rendering(FfmpegStuff *ffmpeg_stuff, size_t width, size_t height)
{

  if (pipe(ffmpeg_stuff->pipe) < 0) {
    fprintf(stderr, "ERROR: could not create pipe: %s\n", strerror(errno));
    return 1;
  }

  pid_t child = fork();
  if (child < 0) {
    fprintf(stderr, "ERROR: could not fork a child: %s\n", strerror(errno));
    return 1;
  }
  if (child == 0) {
    if (dup2(ffmpeg_stuff->pipe[READ_END], STDIN_FILENO) < 0) {
      fprintf(stderr, "ERROR: could not reopen read end of pipe as stdin: %s\n", strerror(errno));
      return 1;
    }
    close(ffmpeg_stuff->pipe[WRITE_END]);
    ffmpeg_stuff->pipe[WRITE_END] = 0;

    char resolution[64];
    snprintf(resolution, sizeof(resolution), "%zux%zu", width, height);
    printf("%zux%zu\n", width, height);

    char framerate[64];
    snprintf(framerate, sizeof(framerate), "%zu", ffmpeg_stuff->fps);
    printf("%zu\n", ffmpeg_stuff->fps);

    int ret = execlp("ffmpeg",
                     "ffmpeg",
                     "-loglevel", "verbose",
                     "-y",
                     "-f", "rawvideo",
                     "-pix_fmt", "rgba",
                     "-s", resolution,
                     "-r", framerate,
                     "-an",
                     "-i", "-",
                     "-c:v", "libx264",
                     "../output_video/output_video_sine_wave.mp4",
                     NULL
      );
    if (ret < 0) {
      fprintf(stderr, "ERROR: could not run ffmpeg as a child process: %s\n", strerror(errno));
      return -1;
    }
    assert(0 && "unreachable");
  }
  int ret = close(ffmpeg_stuff->pipe[READ_END]);
  ffmpeg_stuff->pipe[READ_END] = 0;

  // raylib part
  ffmpeg_stuff->screen = LoadRenderTexture(width, height);
  return 0;
}

void ffmpeg_end_rendering(FfmpegStuff *ffmpeg_stuff)
{
  if(ffmpeg_stuff->pipe[READ_END] != 0) {
    close(ffmpeg_stuff->pipe[READ_END]);
  }
  if(ffmpeg_stuff->pipe[WRITE_END] != 0) {
    close(ffmpeg_stuff->pipe[WRITE_END]);
  }
  wait(NULL);
  printf("Done rendering the video\n");
}

void ffmpeg_send_frame(FfmpegStuff *ffmpeg_stuff, void *data, size_t width, size_t height)
{
    for (size_t y = height; y > 0; --y) {
        write(ffmpeg_stuff->pipe[WRITE_END], (uint32_t*)data + (y - 1)*width, sizeof(uint32_t)*width);
    }
}
