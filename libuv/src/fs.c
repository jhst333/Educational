#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define BUFFER_SIZE 1024

struct uv_data_t {
  uv_loop_t* main_loop;
  uv_buf_t iov;
};

void on_open(uv_fs_t* req) {
  ;
}

int main(int argc, char* argv[]) {
  struct uv_data_t data;
  uv_fs_t open_req;
  char* buffer = NULL;

  buffer = malloc(BUFFER_SIZE);
  data.main_loop = malloc(sizeof(uv_loop_t));
  data.iov = uv_buf_init(buffer, BUFFER_SIZE);
  open_req.data = &data;

  uv_loop_init(data.main_loop);
  uv_fs_open(data.main_loop, &open_req, argv[1], O_RDONLY, 0, on_open);
  uv_run(data.main_loop, UV_RUN_DEFAULT);

  uv_fs_req_cleanup(&open_req);
  free(data.main_loop);
  free(buffer);
}
