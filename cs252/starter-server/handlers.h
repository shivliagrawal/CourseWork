#ifndef HANDLERS_H
#define HANDLERS_H

#include "http_messages.h"

http_response *handle_htdocs(const http_request *request);
http_response *handle_cgi_bin(const http_request *request);

#endif // HANDLERS_H
