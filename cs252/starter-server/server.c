#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "handlers.h"
#include "http_messages.h"

char g_user_pass[MAX_USR_PWD_LEN];

/*
 * Return a string in a format <user>:<password> 
 * either from auth.txt or from your implememtation.
 */

char *return_user_pwd_string(void) {
  // Read from ./auth.txt. Don't change this. We will use it for testing
  FILE *fp = NULL;
  char *line = (char *)malloc(sizeof(char) * MAX_USR_PWD_LEN);
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

  sprintf(g_user_pass, "%s", line);

  free(line);

  return g_user_pass;
} /* return_user_pwd_string() */

/*
 * Accept connections one at a time and handle them.
 */

void run_linear_server(acceptor *acceptor) {
  while (1) {
    socket_t *sock = accept_connection(acceptor);
    handle(sock);
  }
} /* run_linear_server() */

/*
 * Accept connections, creating a different child process to handle each one.
 */

void run_forking_server(acceptor *acceptor) {
  // TODO: Add your code to accept and handle connections in child processes
} /* run_forking_server() */

/*
 * Accept connections, creating a new thread to handle each one.
 */

void run_threaded_server(acceptor *acceptor) {
  // TODO: Add your code to accept and handle connections in new threads
} /* run_threaded_server() */

/*
 * Accept connections, drawing from a thread pool with num_threads to handle the
 * connections.
 */

void run_thread_pool_server(acceptor *acceptor, int num_threads) {
  // TODO: Add your code to accept and handle connections in threads from a
  // thread pool
} /* run_thread_pool_server() */

/*
 * Handle an incoming connection on the passed socket.
 */

void handle(socket_t *sock) {
  http_request request;
  char *request_string = NULL;

  // TODO: Allocate request_string and use socket_read (from socket.c)
  // to read the request into a request_string as a null-terminated string

  // TODO: Implement the parse_request function in http_messages.c

  if (parse_request(request_string, &request) != 0) {
    // TODO: Handle malformed request (error 400)
  }
  print_request(&request);

  // TODO: Implement the handle_* functions in htdocs.c to handle the
  // different types of requests

  http_response *response = NULL;

  if (strncmp("/cgi-bin", request.request_uri, strlen("/cgi-bin")) == 0) {
    response = handle_cgi_bin(&request);
  }
  else {
    response = handle_htdocs(&request);
  }

  if (!response) {
    // TODO: Handle internal error (error 500)
    return;
  }

  // TODO: Implement the response_string function in http_messages.c

  char *to_string = response_string(response);

  printf("%s\n", to_string);
  socket_write_string(sock, to_string);

  free(to_string);
  to_string = NULL;

  free(response);
  response = NULL;

  free(request_string);
  request_string = NULL;

  close_socket(sock);
} /* handle() */
