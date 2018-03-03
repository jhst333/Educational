#include <stdlib.h>
#include <stdio.h>
#include <uv.h>

#define MAX_BUFFER 1024

struct uv_data_s {
  uv_loop_t* main_loop;
  uv_buf_t buffer;
};
typedef struct uv_data_s uv_data_t;

void on_read(uv_fs_t* _handle) {
  uv_data_t* data;

  data = (uv_data_t*)_handle->data;
  
  if (_handle->result < 0) {
    fprintf(stderr, "%s\n", uv_strerror((int)_handle->result));
  }
  else if (_handle->result == 0) {
    uv_fs_t request;

    uv_fs_close(data->main_loop, &request, _handle->result, NULL);
  }
  else {
    fprintf(stdout, "%s", data->buffer.base);
  }
}

void on_open(uv_fs_t* _handle) {
  uv_data_t* data;

  data = (uv_data_t*)_handle->data;

  if (_handle->result >= 0) {
    uv_fs_t request;
    request.data = _handle->data;

    uv_fs_read(data->main_loop, &request, _handle->result,
	       &data->buffer, 1, -1, on_read);
  }
  else {
    fprintf(stderr, "%s\n", uv_strerror((int)_handle->result));
  }
}

int main(int argc, char* argv[]) {
  uv_data_t data;
  uv_fs_t request;
  char buffer[MAX_BUFFER];

  data.main_loop = malloc(sizeof(uv_loop_t));
  data.buffer = uv_buf_init(buffer, MAX_BUFFER);
  request.data = &data;

  uv_loop_init(data.main_loop);
  uv_fs_open(data.main_loop, &request, argv[1], O_RDONLY, 0, on_open);
  uv_run(data.main_loop, UV_RUN_DEFAULT);

  uv_fs_req_cleanup(&request);
  
  return 0;
}
