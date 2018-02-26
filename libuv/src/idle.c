#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

void wait_process(uv_idle_t* handle) {
  static uint64_t counter = 0;
  counter += 1;

  if (counter >= 10e6) {
    uv_idle_stop(handle);
  }
}

int main() {
  uv_loop_t* loop = malloc(sizeof(uv_loop_t));
  uv_idle_t idle;
  
  uv_loop_init(loop);
  uv_idle_init(loop, &idle);
  uv_idle_start(&idle, wait_process);

  printf("Idling...\n");
  uv_run(loop, UV_RUN_DEFAULT);

  uv_loop_close(loop);
  free(loop);

  return 0;
}
