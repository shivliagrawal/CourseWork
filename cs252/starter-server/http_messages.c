#include "http_messages.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Return the reason string for a particular status code. You might find this
 * helpful when implementing response_string().
 */

const char *status_reason(int status) {
  switch (status) {
  case 100:
    return "Continue";
  case 101:
    return "Switching Protocols";
  case 200:
    return "OK";
  case 201:
    return "Created";
  case 202:
    return "Accepted";
  case 203:
    return "Non-Authoritative Information";
  case 204:
    return "No Content";
  case 205:
    return "Reset Content";
  case 206:
    return "Partial Content";
  case 300:
    return "Multiple Choices";
  case 301:
    return "Moved Permanently";
  case 302:
    return "Found";
  case 303:
    return "See Other";
  case 304:
    return "Not Modified";
  case 305:
    return "Use Proxy";
  case 307:
    return  "Temporary Redirect";
  case 400:
    return "Bad Request";
  case 401:
    return "Unauthorized";
  case 402:
    return "Payment Required";
  case 403:
    return "Forbidden";
  case 404:
    return "Not Found";
  case 405:
    return "Method Not Allowed";
  case 406:
    return "Not Acceptable";
  case 407:
    return "Proxy Authentication Required";
  case 408:
    return "Request Time-out";
  case 409:
    return "Conflict";
  case 410:
    return "Gone";
  case 411:
    return "Length Required";
  case 412:
    return "Precondition Failed";
  case 413:
    return "Request Entity Too Large";
  case 414:
    return "Request-URI Too Large";
  case 415:
    return  "Unsupported Media Type";
  case 416:
    return "Requested range not satisfiable";
  case 417:
    return "Expectation Failed";
  case 500:
    return "Internal Server Error";
  case 501:
    return "Not Implemented";
  case 502:
    return "Bad Gateway";
  case 503:
    return "Service Unavailable";
  case 504:
    return "Gateway Time-out";
  case 505:
    return "HTTP Version Not Supported";
  default:
    return "Unknown status";
  }
} /* status_reason() */

/*
 * Create the actual response string to be sent over the socket.
 * Allocates a string that contains the response to send back.
 */

char *response_string(http_response *response) {
  // TODO: Replace this code to correctly create the HTTP response
  // using the struct

  char *incorrect = " 200 OK\r\n"
    "Connection: close\r\n"
    "Content-Length: 12\r\n"
    "\r\n"
    "Hello CS252!\r\n";

  char *to_string = malloc(sizeof(char) * (strlen(incorrect) +
                                           strlen(response->http_version) +
                                           1));
  strcpy(to_string, response->http_version);
  strcat(to_string, incorrect);
  return to_string;
} /* response_string() */

/*
 * Parse an http request and populates fields in an http_request struct
 * buf: pointer to an http request (string)
 * request: pointer to an http_request struct to fill out
 * Return 0 on success
 * Return -1 on error
 */

int parse_request(const char *buf, http_request *request) {
  if (!buf) {
    return -1;
  }

  // TODO: Replace this code to parse the request from buf and
  // fill out the http_request accordingly

  request->method = "GET";
  request->request_uri = "/";
  request->query = "";
  request->http_version = "HTTP/1.1";
  request->num_headers = 0;
  request->message_body = "";

  return 0;
} /* parse_request() */

/*
 * Print the request to stdout, useful for debugging.
 */

void print_request(http_request *request) {
  // Magic string to help with autograder

  printf("\\\\==////REQ\\\\\\\\==////\n");

  printf("Method: {%s}\n", request->method);
  printf("Request URI: {%s}\n", request->request_uri);
  printf("Query string: {%s}\n", request->query);
  printf("HTTP Version: {%s}\n", request->http_version);

  printf("Headers: \n");
  for (int i = 0; i < request->num_headers; i++) {
    printf("field-name: %s; field-value: %s\n",
           request->headers[i].key, request->headers[i].value);
  }

  printf("Message body length: %ld\n", strlen(request->message_body));
  printf("%s\n", request->message_body);

  // Magic string to help with autograder

  printf("//==\\\\\\\\REQ////==\\\\\n");
} /* print_request() */
