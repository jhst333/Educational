#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <uv.h>

#define DEFAULT_PORT 7000
#define MAX_BACKLOG 10

typedef struct {
  uv_loop_t* main_loop;
} uv_connection_t;

void on_close(uv_handle_t* _handle) {
  free(_handle);
}

void on_read(uv_stream_t* _client, ssize_t _nread, const uv_buf_t* _iov) {
  if (_nread > 0) {
    uv_write_t request;
    uv_write(&request, _client, _iov, 1, NULL);
  }
  else {
    if (_nread != UV_EOF) {
      fprintf(stderr, "%s\n", uv_strerror(_nread));
    }

    uv_close((uv_handle_t*) _client, on_close);
  }

  if (_iov->base) {
    free(_iov->base);
  }
}

void alloc_iov(uv_handle_t* _handle, size_t _size, uv_buf_t* _iov) {
  _iov->base = malloc(_size);
  _iov->len = _size;
  memset(_iov->base, '\0', _size);
}

void on_new_connection(uv_stream_t* _handle, int32_t _status) {
  uv_connection_t* connection = NULL;
  
  connection = (uv_connection_t*)_handle->data;
  
  if (_status < 0) {
    fprintf(stderr, "%s\n", uv_strerror(_status));
    return;
  }

  uv_tcp_t* client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
  uv_tcp_init(connection->main_loop, client);

  if (0 == uv_accept(_handle, (uv_stream_t*) client)) {
    uv_read_start((uv_stream_t*) client, alloc_iov, on_read);
  }
  else {
    uv_close((uv_handle_t*) client, on_close);
    free(client);
  }
}

int main(int argc, char* argv[]) {
  uv_connection_t* connection_info = NULL;
  struct sockaddr_in sin;
  uv_tcp_t server;

  connection_info = (uv_connection_t*) malloc(sizeof(uv_connection_t));
  connection_info->main_loop = (uv_loop_t*) malloc(sizeof(uv_loop_t));
  server.data = connection_info;

  uv_loop_init(connection_info->main_loop);
  uv_tcp_init(connection_info->main_loop, &server);
  uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &sin);
  uv_tcp_bind(&server, (const struct sockaddr*)&sin, 0);
  
  int32_t result = uv_listen((uv_stream_t*) &server, MAX_BACKLOG, on_new_connection);
  if (result) {
    fprintf(stderr, "%s\n", uv_strerror(result));
    return -1;
  }

  uv_run(connection_info->main_loop, UV_RUN_DEFAULT);
  
  uv_loop_close(connection_info->main_loop);
  free(connection_info->main_loop);
  free(connection_info);
  
  return 0;
}
