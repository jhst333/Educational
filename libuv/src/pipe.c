#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <uv.h>

typedef struct {
  uv_write_t request;
  uv_buf_t buffer;
} uv_write_req_t;

typedef struct {
  uv_pipe_t stdin;
  uv_pipe_t stdout;
  uv_pipe_t file;
} uv_pipes_t;

void write_data(uv_stream_t* _destination, size_t _size, const uv_buf_t* _buf) {
  uv_write_t* request = NULL;
  request = (uv_write_t*) malloc(sizeof(uv_write_t));

  uv_write(request, _destination, _buf, 1, NULL);
}

void read_stdin(uv_stream_t* _stream, ssize_t _nread, const uv_buf_t* _buf) {
  uv_pipes_t* pipes = NULL;
  pipes = (uv_pipes_t*)_stream->data;
  
  if (_nread < 0) {
    if (_nread == UV_EOF) {
      uv_close((uv_handle_t*)&pipes->stdin, NULL);
      uv_close((uv_handle_t*)&pipes->stdout, NULL);
      uv_close((uv_handle_t*)&pipes->file, NULL);
    }
  }
  else if (_nread > 0) {
    write_data((uv_stream_t*)&pipes->stdout, _nread, _buf);
    write_data((uv_stream_t*)&pipes->file, _nread, _buf);
  }

  if (_buf->base) {
    free(_buf->base);
  }
}

void alloc_iov(uv_handle_t* _handle, size_t _size, uv_buf_t* _buf) {
  (*_buf) = uv_buf_init(malloc(_size), _size);
}

int main(int argc, char* argv[]) {
  uv_loop_t* default_loop = NULL;
  uv_pipes_t* pipes = NULL;
  uv_fs_t open_request;

  default_loop = uv_default_loop();
  if (!default_loop) {
    fprintf(stderr, "Unable to allocate uv_loop_t");
    exit(-1);
  }

  pipes = malloc(sizeof(uv_pipes_t));

  uv_pipe_init(default_loop, &pipes->stdin, 0);
  pipes->stdin.data = pipes;
  
  uv_pipe_init(default_loop, &pipes->stdout, 0);
  uv_pipe_init(default_loop, &pipes->file, 0);

  uv_pipe_open(&pipes->stdin, 0);
  uv_pipe_open(&pipes->stdout, 1);

  int32_t fd = uv_fs_open(default_loop, &open_request, argv[1],
			  O_CREAT | O_RDWR, 0644, NULL);
  uv_pipe_init(default_loop, &pipes->file, 0);
  uv_pipe_open(&pipes->file, fd);

  uv_read_start((uv_stream_t*)&pipes->stdin, alloc_iov, read_stdin);
  uv_run(default_loop, UV_RUN_DEFAULT);

  uv_fs_req_cleanup(&open_request);
  
  free(pipes);
}
