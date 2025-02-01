#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <raylib.h>

#define READ_END 0
#define WRITE_END 1

#define WIDTH 800
#define HEIGHT 600
#define FPS 30
// hack that returns the value of the argument of macro as a string
// STR(FPS) would return 60 now
#define STR2(x) #x
#define STR(x) STR2(x)

uint32_t pixels[WIDTH*HEIGHT];

int main (void)
{
  int pipefd[2];

  if (pipe(pipefd) < 0) {
    fprintf(stderr, "ERROR: could not create pipe: %s\n", strerror(errno));
    return 1;
  }

  pid_t child = fork();
  if (child < 0) {
    fprintf(stderr, "ERROR: could not fork a child: %s\n", strerror(errno));
    return 1;
  }
  if (child == 0) {
    // replace standard input of ffmpeg with the read end of the pipe (pipefd[0])
    // dup2(pipefd[READ_END], STDIN_FILENO);
    if (dup2(pipefd[READ_END], STDIN_FILENO) < 0) {
      fprintf(stderr, "ERROR: could not reopen read end of pipe as stdin: %s\n", strerror(errno));
      return 1;
    }
    close(pipefd[WRITE_END]);
    int ret = execlp("ffmpeg",
                     "ffmpeg",
                     "-loglevel", "verbose",
                     "-y",
                     "-f", "rawvideo",
                     "-pix_fmt", "rgba",
                     "-s", STR(WIDTH) "x" STR(HEIGHT),
                     "-r", STR(FPS),
                     "-an",
                     "-i", "-",
                     "-c:v", "libx264",

                     "../output_video/output_video_simple_example.mp4",
                     NULL
      );
    if (ret < 0) {
      fprintf(stderr, "ERROR: could not run ffmpeg as a child process: %s\n", strerror(errno));
      return 1;
    }
    assert(0 && "unreachable");
  }
  close(pipefd[READ_END]);

  size_t duration = 10;
  float x = WIDTH/2;
  float y = HEIGHT/2;
  float r = HEIGHT/8;
  float dx = 200;
  float dy = 200;
  float dt = 1.0f/FPS;

  InitWindow(WIDTH, HEIGHT, "Raylib + FFmpeg");
  SetTargetFPS(60);

  RenderTexture2D screen = LoadRenderTexture(WIDTH, HEIGHT);

  for (size_t i = 0; i < FPS*duration && !WindowShouldClose(); ++i) {
    BeginDrawing();
    float nx = x + dx*dt;
    if(0 < nx - r && nx + r < WIDTH) {
      x = nx;
    } else {
      dx = -dx;
    }
    float ny = y += dy*dt;
    if(0 < ny -r && ny + r < HEIGHT) {
      y = ny;
    } else {
      dy = -dy;
    }

    BeginDrawing();
        BeginTextureMode(screen);
            ClearBackground(*(Color*)(uint32_t[1]){0xFF181818});
            DrawCircle(x, y, r, *(Color*)(uint32_t[1]){0xFF0000FF});
        EndTextureMode();
        DrawTexture(screen.texture, 0, 0, WHITE);
    EndDrawing();

    Image image = LoadImageFromTexture(screen.texture);
    write(pipefd[WRITE_END], image.data, sizeof(uint32_t)*WIDTH*HEIGHT);
    UnloadImage(image);
  }
  CloseWindow();

  close(pipefd[WRITE_END]);

  //waiting for child being finished with execution
  wait(NULL);
  printf("Done rendering the video\n");
  return 0;
}
