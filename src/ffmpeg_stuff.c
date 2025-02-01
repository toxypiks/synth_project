#include "ffmpeg_stuff.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int ffmpeg_start_rendering(size_t width, size_t height, size_t fps)
{
  int pipefd[2];

  if (pipe(pipefd) < 0) {
    fprintf(stderr, "ERROR: could not create pipe: %s\n", strerror(errno));
    return -1;
  }

  pid_t child = fork();
  if (child < 0) {
    fprintf(stderr, "ERROR: could not fork a child: %s\n", strerror(errno));
    return -1;
  }
  if (child == 0) {
    if (dup2(pipefd[READ_END], STDIN_FILENO) < 0) {
      fprintf(stderr, "ERROR: could not reopen read end of pipe as stdin: %s\n", strerror(errno));
      return -1;
    }
    close(pipefd[WRITE_END]);

    char resolution[64];
    snprintf(resolution, sizeof(resolution), "%zux%zu", width, height);

    char framerate[64];
    snprintf(framerate, sizeof(framerate), "%zu", fps);

    int ret = execlp("ffmpeg",
                     "ffmpeg",
                     "-loglevel", "verbose",
                     "-y",
                     "-f", "rawvideo",
                     "-pix_fmt", "rgba",
                     "-s", resolution,
                     "-r", fps,
                     "-an",
                     "-i", "-",
                     "-c:v", "libx264",

                     "../output_video/output_video_simple_example.mp4",
                     NULL
      );
    if (ret < 0) {
      fprintf(stderr, "ERROR: could not run ffmpeg as a child process: %s\n", strerror(errno));
      return -1;
    }
    assert(0 && "unreachable");
  }
  close(pipefd[READ_END]);

  return pipefd[WRITE_END];
}

void ffmpeg_end_rendering(int pipe)
{
  close(pipe);
  wait(NULL);
}
