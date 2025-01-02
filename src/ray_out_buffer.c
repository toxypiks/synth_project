#include <string.h>
#include <stdlib.h>
#include "ray_out_buffer.h"

RayOutBuffer create_ray_out_buffer(size_t size){
  RayOutBuffer ray_out_buffer = {
    .max_length = size,
    .global_frames_count = 0,
    .global_frames = (float*)malloc(size * sizeof(float))
  };
  return ray_out_buffer;
}
void clear_ray_out_buffer(RayOutBuffer buf){
  free(buf.global_frames);
}

void copy_to_ray_out_buffer(RayOutBuffer *buf, void *bufferData, unsigned int frames)
{
  size_t capacity = buf->max_length;
  if(frames <= capacity - buf->global_frames_count) {
    memcpy(buf->global_frames + buf->global_frames_count, bufferData, sizeof(float)*frames);
    buf->global_frames_count += frames;
  } else if (frames <= capacity) {
    memmove(buf->global_frames, buf->global_frames + frames, sizeof(float)*(capacity - frames));
    memcpy(buf->global_frames + (capacity - frames), bufferData, sizeof(float)*frames);
  } else {
    memcpy(buf->global_frames, bufferData, sizeof(float)* capacity);
    buf->global_frames_count = capacity;
  }
}
