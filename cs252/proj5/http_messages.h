#ifndef HTTP_MESSAGES_H
#define HTTP_MESSAGES_H

const char *status_reason(int status);

typedef struct {
  char *key;
  char *value;
} header;

typedef struct {
  char *method;
  char *request_uri;
  char *http_version;
  int num_headers;
  header *headers;
  char *message_body;
  char *query;
} http_request;

typedef struct {
  char *http_version;
  int status_code;
  char *reason_phrase;
  int num_headers;
  header *headers;
  char *message_body;
} http_response;

char *response_string(http_response *response);
int parse_request(const char *buf, http_request *request);

void print_request(http_request *request);

#endif // HTTP_MESSAGES_H
