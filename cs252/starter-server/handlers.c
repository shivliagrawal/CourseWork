#include "handlers.h"

#include <string.h>

/*
 * Allocate an http_response struct and populate it appropriately.
 * Create a response for regular requests serving static files.
 * If an error occurs, return NULL.
 */

http_response *handle_htdocs(const http_request *request) {
  return NULL;

  /*
  http_response *resp = malloc(sizeof(*resp));
  if (!resp) {
    return NULL;
  }
  memset(resp, 0, sizeof(*resp));

  resp.http_version = request->http_version;

  // TODO: Get the request URL, verify the file exists, and serve it

  return resp;
  */
} /* handle_htdocs() */

/*
 * Allocate an http_response struct and populate it appropriately.
 * Create a response for cgi-bin requests by running the cg-bin executable.
 * If an error occurs, return NULL.
 */

http_response *handle_cgi_bin(const http_request *request) {
  return NULL;

  /*
  http_response *resp = malloc(sizeof(*resp));
  if (!resp) {
    return NULL;
  }
  memset(resp, 0, sizeof(*resp));

  resp.http_version = request->http_version;

  // TODO: Find the executable and get its output

  return resp;
  */
} /* handle_cgi_bin() */

