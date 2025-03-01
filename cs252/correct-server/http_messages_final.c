#include "http_messages.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Return the reason string for a particular status code.
 */
const char *status_reason(int status) {
  switch (status) {
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 307: return "Temporary Redirect";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Time-out";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Payload Too Large";
    case 414: return "URI Too Long";
    case 415: return "Unsupported Media Type";
    case 416: return "Range Not Satisfiable";
    case 417: return "Expectation Failed";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    case 505: return "HTTP Version Not Supported";
    default:  return "Unknown Status";
  }
} /* status_reason() */

/*
 * Create the actual response string to be sent over the socket.
 * Allocates a string that contains the response to send back.
 */
char *response_string(http_response *response) {
  // Calculate the total length needed
  size_t total_length = strlen(response->http_version) + 1 +  // HTTP version and space
                        3 + 1 +  // Status code and space
                        strlen(response->reason_phrase) + 2;  // Reason phrase and \r\n

  for (int i = 0; i < response->num_headers; i++) {
    total_length += strlen(response->headers[i].key) + 2 +  // Key and ": "
                    strlen(response->headers[i].value) + 2; // Value and \r\n
  }

  total_length += 2; // Additional \r\n after headers

  if (response->message_body) {
    total_length += strlen(response->message_body);
  }

  char *resp_str = malloc(total_length + 1);
  if (!resp_str) {
    return NULL;
  }

  // Build the response string
  sprintf(resp_str, "%s %d %s\r\n", response->http_version, response->status_code, response->reason_phrase);
  for (int i = 0; i < response->num_headers; i++) {
    strcat(resp_str, response->headers[i].key);
    strcat(resp_str, ": ");
    strcat(resp_str, response->headers[i].value);
    strcat(resp_str, "\r\n");
  }
  strcat(resp_str, "\r\n");
  if (response->message_body) {
    strcat(resp_str, response->message_body);
  }

  return resp_str;
} /* response_string() */

/*
 * Parse an HTTP request and populate fields in an http_request struct.
 * buf: pointer to an HTTP request (string)
 * request: pointer to an http_request struct to fill out
 * Return 0 on success
 * Return -1 on error
 */
int parse_request(const char *buf, http_request *request) {
  if (!buf || !request) {
    return -1;
  }

  char *buf_copy = strdup(buf);
  if (!buf_copy) {
    return -1;
  }

  char *saveptr;
  char *line = strtok_r(buf_copy, "\r\n", &saveptr);
  if (!line) {
    free(buf_copy);
    return -1;
  }

  // Parse request line
  char *method = strtok(line, " ");
  char *uri = strtok(NULL, " ");
  char *version = strtok(NULL, " ");

  if (!method || !uri || !version) {
    free(buf_copy);
    return -1;
  }

  request->method = strdup(method);
  request->request_uri = strdup(uri);
  request->http_version = strdup(version);

  // Parse headers
  int headers_capacity = 10;
  request->headers = malloc(sizeof(header) * headers_capacity);
  request->num_headers = 0;

  while ((line = strtok_r(NULL, "\r\n", &saveptr)) && strlen(line) > 0) {
    char *colon = strstr(line, ": ");
    if (!colon) {
      // Malformed header
      free(buf_copy);
      return -1;
    }
    *colon = '\0';
    char *key = line;
    char *value = colon + 2;

    if (request->num_headers >= headers_capacity) {
      headers_capacity *= 2;
      request->headers = realloc(request->headers, sizeof(header) * headers_capacity);
      if (!request->headers) {
        free(buf_copy);
        return -1;
      }
    }

    request->headers[request->num_headers].key = strdup(key);
    request->headers[request->num_headers].value = strdup(value);
    request->num_headers++;
  }

  free(buf_copy);

  // For simplicity, we assume there is no message body
  request->message_body = NULL;
  request->query = NULL;

  return 0;
} /* parse_request() */

/*
 * Print the request to stdout, useful for debugging.
 */
void print_request(http_request *request) {
  printf("\\\\==////REQ\\\\\\\\==////\n");

  printf("Method: {%s}\n", request->method);
  printf("Request URI: {%s}\n", request->request_uri);
  printf("Query string: {%s}\n", request->query ? request->query : "");
  printf("HTTP Version: {%s}\n", request->http_version);

  printf("Headers: \n");
  for (int i = 0; i < request->num_headers; i++) {
    printf("field-name: %s; field-value: %s\n",
           request->headers[i].key, request->headers[i].value);
  }

  printf("Message body length: %ld\n", request->message_body ? strlen(request->message_body) : 0);
  if (request->message_body) {
    printf("%s\n", request->message_body);
  }

  printf("//==\\\\\\\\REQ////==\\\\\n");
} /* print_request() */

