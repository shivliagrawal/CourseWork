#include "handlers.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include "misc.h"
#include "http_messages.h"

/*
 * Handle the HTTP request and generate an appropriate HTTP response.
 */

http_response *handle_request(const http_request *request) {
  http_response *resp = malloc(sizeof(*resp));
  if (!resp) {
    return NULL;
  }
  memset(resp, 0, sizeof(*resp));

  resp->http_version = strdup(request->http_version);
  if (!resp->http_version) {
    free(resp);
    return NULL;
  }

  // Define the Document Root
  const char *document_root = "http-root-dir/htdocs";

  // Get the request URI
  const char *uri = request->request_uri;

  // Handle the case where request URI is empty or "/"
  if (strcmp(uri, "/") == 0) {
    uri = "/index.html";
  }

  // Prevent directory traversal attacks
  if (strstr(uri, "..") != NULL) {
    // Return 403 Forbidden
    resp->status_code = 403;
    resp->reason_phrase = strdup("Forbidden");
    resp->num_headers = 2;
    resp->headers = malloc(sizeof(header) * 2);
    resp->headers[0].key = strdup("Connection");
    resp->headers[0].value = strdup("close");
    resp->headers[1].key = strdup("Content-Type");
    resp->headers[1].value = strdup("text/plain");
    resp->message_body = strdup("403 Forbidden");
    return resp;
  }

  // Construct the full file path
  char filepath[PATH_MAX];
  snprintf(filepath, sizeof(filepath), "%s%s", document_root, uri);

  // Debug: Print the file path being accessed
  printf("Serving file: %s\n", filepath);

  // Check if the file exists and is accessible
  struct stat sb;
  if (stat(filepath, &sb) == -1) {
    // File not found
    resp->status_code = 404;
    resp->reason_phrase = strdup("Not Found");
    resp->num_headers = 2;
    resp->headers = malloc(sizeof(header) * 2);
    resp->headers[0].key = strdup("Connection");
    resp->headers[0].value = strdup("close");
    resp->headers[1].key = strdup("Content-Type");
    resp->headers[1].value = strdup("text/plain");
    resp->message_body = strdup("404 Not Found");
    return resp;
  }

  // Check for read permissions
  if (access(filepath, R_OK) != 0) {
    // No read permissions
    resp->status_code = 403;
    resp->reason_phrase = strdup("Forbidden");
    resp->num_headers = 2;
    resp->headers = malloc(sizeof(header) * 2);
    resp->headers[0].key = strdup("Connection");
    resp->headers[0].value = strdup("close");
    resp->headers[1].key = strdup("Content-Type");
    resp->headers[1].value = strdup("text/plain");
    resp->message_body = strdup("403 Forbidden");
    return resp;
  }

  // Open the file
  int fd = open(filepath, O_RDONLY);
  if (fd == -1) {
    // Error opening file
    resp->status_code = 500;
    resp->reason_phrase = strdup("Internal Server Error");
    resp->num_headers = 2;
    resp->headers = malloc(sizeof(header) * 2);
    resp->headers[0].key = strdup("Connection");
    resp->headers[0].value = strdup("close");
    resp->headers[1].key = strdup("Content-Type");
    resp->headers[1].value = strdup("text/plain");
    resp->message_body = strdup("500 Internal Server Error");
    return resp;
  }

  // Get the file size
  off_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  // Read the file contents
  char *file_contents = malloc(file_size + 1);
  if (!file_contents) {
    close(fd);
    resp->status_code = 500;
    resp->reason_phrase = strdup("Internal Server Error");
    resp->num_headers = 2;
    resp->headers = malloc(sizeof(header) * 2);
    resp->headers[0].key = strdup("Connection");
    resp->headers[0].value = strdup("close");
    resp->headers[1].key = strdup("Content-Type");
    resp->headers[1].value = strdup("text/plain");
    resp->message_body = strdup("500 Internal Server Error");
    return resp;
  }

  ssize_t bytes_read = read(fd, file_contents, file_size);
  if (bytes_read != file_size) {
    free(file_contents);
    close(fd);
    resp->status_code = 500;
    resp->reason_phrase = strdup("Internal Server Error");
    resp->num_headers = 2;
    resp->headers = malloc(sizeof(header) * 2);
    resp->headers[0].key = strdup("Connection");
    resp->headers[0].value = strdup("close");
    resp->headers[1].key = strdup("Content-Type");
    resp->headers[1].value = strdup("text/plain");
    resp->message_body = strdup("500 Internal Server Error");
    return resp;
  }
  file_contents[file_size] = '\0';
  close(fd);

  // Set up the response
  resp->status_code = 200;
  resp->reason_phrase = strdup("OK");

  // Set headers
  resp->num_headers = 3;
  resp->headers = malloc(sizeof(header) * resp->num_headers);
  resp->headers[0].key = strdup("Connection");
  resp->headers[0].value = strdup("close");

  // Content-Type
  char *content_type = get_content_type(filepath);
  if (!content_type) {
    content_type = strdup("application/octet-stream");
  }
  resp->headers[1].key = strdup("Content-Type");
  resp->headers[1].value = content_type;

  // Content-Length
  char content_length_str[32];
  snprintf(content_length_str, sizeof(content_length_str), "%ld", (long)file_size);
  resp->headers[2].key = strdup("Content-Length");
  resp->headers[2].value = strdup(content_length_str);

  // Message body
  resp->message_body = file_contents;

  return resp;
} /* handle_request() */
