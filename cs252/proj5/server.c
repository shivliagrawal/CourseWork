#include "server.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include "http_messages.h"
#include "misc.h"
#include "handlers.h"
#include "socket.h"

extern volatile sig_atomic_t server_running;

char g_user_pass[MAX_USR_PWD_LEN];

/* Function prototypes */

char *return_user_pwd_string(void);
void handle(socket_t *sock);
void free_http_request(http_request *request);
void free_http_response(http_response *response);
char *base64_encode(const unsigned char *data, size_t input_length);
http_response *handle_request(const http_request *request);
void handle_error(socket_t *sock, int status_code, const char *reason_phrase);

/*
 * Function to send an HTTP error response to the client.
 */

void handle_error(socket_t *sock, int status_code, const char *reason_phrase) {
  http_response response = {0};
  response.http_version = strdup("HTTP/1.1");
  response.status_code = status_code;
  response.reason_phrase = strdup(reason_phrase);
  response.num_headers = 1;
  response.headers = malloc(sizeof(header));
  response.headers[0].key = strdup("Connection");
  response.headers[0].value = strdup("close");
  response.message_body = strdup(reason_phrase);

  char *response_str = response_string(&response);
  socket_write_string(sock, response_str);

  free(response_str);
  free_http_response(&response);
} /* handle_error() */

/*
 * Return a string in a format <user>:<password>
 * either from auth.txt or from your implementation.
 */

char *return_user_pwd_string(void) {
  FILE *fp = NULL;
  char *line = NULL;
  size_t len = 0;

  fp = fopen("./auth.txt", "r");
  if (fp == NULL) {
    perror("couldn't read auth.txt");
    exit(-1);
  }

  if (getline(&line, &len, fp) == -1) {
    perror("couldn't read auth.txt");
    exit(-1);
  }

  size_t pass_len = strlen(line);
  if (pass_len > 0 && line[pass_len - 1] == '\n') {
    line[pass_len - 1] = '\0';
  }

  strncpy(g_user_pass, line, MAX_USR_PWD_LEN - 1);
  g_user_pass[MAX_USR_PWD_LEN - 1] = '\0';

  free(line);
  fclose(fp);

  return g_user_pass;
} /* return_user_pwd_string() */

/*
 * Accept connections one at a time and handle them.
 */

void run_linear_server(acceptor *accepto) {
  while (server_running) {
    socket_t *sock = accept_connection(accepto);
    if (!sock) {
      break;
    }
    handle(sock);
    close_socket(sock);
  }
} /* run_linear_server() */

/*
 * Accept connections, creating a different child process to handle each one.
 */

void run_forking_server(acceptor *accepto) {
  while (server_running) {
    socket_t *sock = accept_connection(accepto);
    if (!sock) {
      break;
    }

    pid_t pid = fork();
    if (pid < 0) {
      perror("Unable to fork");
    } else if (pid == 0) {
      handle(sock);
      close_socket(sock);
      exit(EXIT_SUCCESS);
    } else {
      close_socket(sock);
    }
  }
} /* run_forking_server() */

/*
 * Thread function to handle connections.
 */

void *handle_threaded(void *arg) {
  socket_t *sock = (socket_t *)arg;
  handle(sock);
  close_socket(sock);
  return NULL;
} /* handle_threaded() */

/*
 * Accept connections, creating a new thread to handle each one.
 */

void run_threaded_server(acceptor *accepto) {
  while (server_running) {
    socket_t *sock = accept_connection(accepto);
    if (!sock) {
      break;
    }
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, handle_threaded, (void *)sock);
    if (ret != 0) {
      perror("pthread_create failed");
      handle_error(sock, 500, "Internal Server Error");
      close_socket(sock);
      continue;
    }
    pthread_detach(thread);
  }
} /* run_threaded_server() */

/*
 * Thread pool function to handle connections.
 */

void *thread_pool_handle(void *arg) {
  acceptor *accepto = (acceptor *)arg;
  while (server_running) {
    socket_t *sock = accept_connection(accepto);
    if (!sock) {
      break;
    }
    handle(sock);
    close_socket(sock);
  }
  return NULL;
} /* thread_pool_handle() */

/*
 * Accept connections, drawing from a thread pool with num_threads to handle the
 * connections.
 */

void run_thread_pool_server(acceptor *accepto, int num_threads) {
  pthread_t thread_pool[num_threads];
  for (int i = 0; i < num_threads; i++) {
    pthread_create(&thread_pool[i], NULL, thread_pool_handle, (void *)accepto);
  }
  for (int i = 0; i < num_threads; i++) {
    pthread_join(thread_pool[i], NULL);
  }
} /* run_thread_pool_server() */

/*
 * Base64 encoding function.
 */

char *base64_encode(const unsigned char *data, size_t input_length) {
  const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int mod_table[] = {0, 2, 1};

  size_t output_length = 4 * ((input_length + 2) / 3);
  char *encoded_data = malloc(output_length + 1);
  if (encoded_data == NULL) return NULL;

  for (size_t i = 0, j = 0; i < input_length;) {
    uint32_t octet_a = i < input_length ? data[i++] : 0;
    uint32_t octet_b = i < input_length ? data[i++] : 0;
    uint32_t octet_c = i < input_length ? data[i++] : 0;
    uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

    encoded_data[j++] = encoding_table[(triple >> 18) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 12) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 6) & 0x3F];
    encoded_data[j++] = encoding_table[triple & 0x3F];
  }

  for (int i = 0; i < mod_table[input_length % 3]; i++)
    encoded_data[output_length - 1 - i] = '=';

  encoded_data[output_length] = '\0';
  return encoded_data;
} /* base64_encode() */

/*
 * Handle an incoming connection on the passed socket.
 */

void handle(socket_t *sock) {
  char buf[4096];
  int read_bytes = socket_read(sock, buf, sizeof(buf) - 1);

  if (read_bytes <= 0) return;
  buf[read_bytes] = '\0';

  if (strstr(buf, "\r\n\r\n") == NULL) {
    http_response response = {0};
    response.http_version = strdup("HTTP/1.1");
    response.status_code = 400;
    response.reason_phrase = strdup("Bad Request");
    response.num_headers = 1;
    response.headers = malloc(sizeof(header));
    response.headers[0].key = strdup("Connection");
    response.headers[0].value = strdup("close");
    response.message_body = strdup("400 Bad Request");

    char *response_str = response_string(&response);
    socket_write_string(sock, response_str);
    free(response_str);
    free_http_response(&response);
    return;
  }

  http_request request = {0};
  if (parse_request(buf, &request) != 0) {
    http_response response = {0};
    response.http_version = strdup("HTTP/1.1");
    response.status_code = 400;
    response.reason_phrase = strdup("Bad Request");
    response.num_headers = 1;
    response.headers = malloc(sizeof(header));
    response.headers[0].key = strdup("Connection");
    response.headers[0].value = strdup("close");
    response.message_body = strdup("400 Bad Request");

    char *response_str = response_string(&response);
    socket_write_string(sock, response_str);
    free(response_str);
    free_http_response(&response);
    free_http_request(&request);
    return;
  }

  print_request(&request);

  if ((strcmp(request.http_version, "HTTP/1.1") != 0) &&
      (strcmp(request.http_version, "HTTP/1.0") != 0)) {
    http_response response = {0};
    response.http_version = strdup("HTTP/1.1");
    response.status_code = 505;
    response.reason_phrase = strdup("HTTP Version Not Supported");
    response.num_headers = 1;
    response.headers = malloc(sizeof(header));
    response.headers[0].key = strdup("Connection");
    response.headers[0].value = strdup("close");
    response.message_body = strdup("505 HTTP Version Not Supported");

    char *response_str = response_string(&response);
    socket_write_string(sock, response_str);
    free(response_str);
    free_http_response(&response);
    free_http_request(&request);
    return;
  }

  if (strcmp(request.method, "GET") != 0) {
    http_response response = {0};
    response.http_version = strdup("HTTP/1.1");
    response.status_code = 405;
    response.reason_phrase = strdup("Method Not Allowed");
    response.num_headers = 1;
    response.headers = malloc(sizeof(header));
    response.headers[0].key = strdup("Connection");
    response.headers[0].value = strdup("close");
    response.message_body = strdup("405 Method Not Allowed");

    char *response_str = response_string(&response);
    socket_write_string(sock, response_str);
    free(response_str);
    free_http_response(&response);
    free_http_request(&request);
    return;
  }

  int authorized = 0;
  char *auth_header_value = NULL;
  for (int i = 0; i < request.num_headers; i++) {
    if (strcasecmp(request.headers[i].key, "Authorization") == 0) {
      auth_header_value = request.headers[i].value;
      break;
    }
  }

  if (auth_header_value) {
    char expected_auth[256];
    char *user_pass = return_user_pwd_string();
    char *encoded_credentials = base64_encode((const unsigned char *)user_pass, strlen(user_pass));
    if (encoded_credentials) {
      snprintf(expected_auth, sizeof(expected_auth), "Basic %s", encoded_credentials);
      if (strcmp(auth_header_value, expected_auth) == 0) {
        authorized = 1;
      }
      free(encoded_credentials);
    }
  }

  if (!authorized) {
    http_response response = {0};
    response.http_version = strdup("HTTP/1.1");
    response.status_code = 401;
    response.reason_phrase = strdup("Unauthorized");
    response.num_headers = 2;
    response.headers = malloc(sizeof(header) * 2);
    response.headers[0].key = strdup("Connection");
    response.headers[0].value = strdup("close");
    response.headers[1].key = strdup("WWW-Authenticate");
    response.headers[1].value = strdup("Basic realm=\"myhttpd-cs252\"");
    response.message_body = strdup("401 Unauthorized");

    char *response_str = response_string(&response);
    socket_write_string(sock, response_str);
    free(response_str);
    free_http_response(&response);
    free_http_request(&request);
    return;
  }

  http_response *response = handle_request(&request);
  if (!response) {
    http_response response_500 = {0};
    response_500.http_version = strdup("HTTP/1.1");
    response_500.status_code = 500;
    response_500.reason_phrase = strdup("Internal Server Error");
    response_500.num_headers = 1;
    response_500.headers = malloc(sizeof(header));
    response_500.headers[0].key = strdup("Connection");
    response_500.headers[0].value = strdup("close");
    response_500.message_body = strdup("500 Internal Server Error");

    char *response_str = response_string(&response_500);
    socket_write_string(sock, response_str);
    free(response_str);
    free_http_response(&response_500);
    free_http_request(&request);
    return;
  }

  char *response_str = response_string(response);
  socket_write_string(sock, response_str);
  free(response_str);
  free_http_response(response);
  free(response);
  free_http_request(&request);
} /* handle() */

/*
 * Functions to free http_request structures
 */

void free_http_request(http_request *request) {
  if (request) {
    if (request->method) free(request->method);
    if (request->request_uri) free(request->request_uri);
    if (request->http_version) free(request->http_version);
    if (request->headers) {
      for (int i = 0; i < request->num_headers; i++) {
        if (request->headers[i].key) free(request->headers[i].key);
        if (request->headers[i].value) free(request->headers[i].value);
      }
      free(request->headers);
    }
    if (request->message_body) free(request->message_body);
    if (request->query) free(request->query);
  }
} /* free_http_request() */

/*
 * Functions to free http_response structures
 */

void free_http_response(http_response *response) {
  if (response) {
    if (response->http_version) free(response->http_version);
    if (response->reason_phrase) free(response->reason_phrase);
    if (response->headers) {
      for (int i = 0; i < response->num_headers; i++) {
        if (response->headers[i].key) free(response->headers[i].key);
        if (response->headers[i].value) free(response->headers[i].value);
      }
      free(response->headers);
    }
    if (response->message_body) free(response->message_body);
  }
} /* free_http_response() */
