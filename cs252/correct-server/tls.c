#include "tls.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string.h>

/*
 * Initialize OpenSSL library
 */

void init_openssl() {
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
}

/*
 * Cleanup OpenSSL library
 */

void cleanup_openssl() {
  EVP_cleanup();
}

/*
 * Create an SSL context
 */

SSL_CTX *create_context() {
  const SSL_METHOD *method;
  SSL_CTX *ctx;

  method = TLS_server_method();

  ctx = SSL_CTX_new(method);
  if (!ctx) {
    perror("Unable to create SSL context");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  return ctx;
}

/*
 * Configure an SSL context with certificate and private key
 */

void configure_context(SSL_CTX *ctx) {
  SSL_CTX_set_ecdh_auto(ctx, 1);

  // Load certificate into context
  if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  // Load private key into context
  if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
}

/*
 * Close and free a TLS socket
 */

int close_tls_socket(tls_socket *socket) {
  SSL_shutdown(socket->ssl);
  SSL_free(socket->ssl);
  close(socket->socket_fd);
  free(socket);
  return 0;
} /* close_tls_socket() */

/*
 * Read from a TLS socket
 */

int tls_read(tls_socket *socket, char *buf, size_t buf_len) {
  if (buf == NULL) {
    return -1;
  }
  int r = SSL_read(socket->ssl, buf, buf_len);
  if (r <= 0) {
    return -1;
  }
  return r;
} /* tls_read() */

/*
 * Write to a TLS socket
 */

int tls_write(tls_socket *socket, char *buf, size_t buf_len) {
  if (buf == NULL) {
    return -1;
  }
  int sent = SSL_write(socket->ssl, buf, buf_len);
  if (sent <= 0) {
    return -1;
  }
  return 0;
} /* tls_write() */

/*
 * Create a new TLS socket acceptor
 */

tls_acceptor *create_tls_acceptor(int port) {
  tls_acceptor *acceptor = malloc(sizeof(tls_acceptor));

  init_openssl();
  SSL_CTX *ctx = create_context();
  configure_context(ctx);

  acceptor->ssl_ctx = ctx;

  acceptor->addr.sin_family = AF_INET;
  acceptor->addr.sin_port = htons(port);
  acceptor->addr.sin_addr.s_addr = htonl(INADDR_ANY);

  acceptor->master_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (acceptor->master_socket < 0) {
    perror("Unable to create socket");
    exit(EXIT_FAILURE);
  }

  int optval = 1;
  setsockopt(acceptor->master_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  if (bind(acceptor->master_socket, (struct sockaddr *)&acceptor->addr, sizeof(acceptor->addr)) < 0) {
    perror("Unable to bind to socket");
    exit(EXIT_FAILURE);
  }

  if (listen(acceptor->master_socket, SOMAXCONN) < 0) {
    perror("Unable to listen on socket");
    exit(EXIT_FAILURE);
  }

  return acceptor;
} /* create_tls_acceptor() */

/*
 * Accept a new connection from the TLS socket acceptor
 */

tls_socket *accept_tls_connection(tls_acceptor *acceptor) {
  struct sockaddr_in addr;
  uint len = sizeof(addr);

  int client = accept(acceptor->master_socket, (struct sockaddr *)&addr, &len);
  if (client < 0) {
    perror("Unable to accept connection");
    return NULL;
  }

  SSL *ssl = SSL_new(acceptor->ssl_ctx);
  SSL_set_fd(ssl, client);

  if (SSL_accept(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
    close(client);
    SSL_free(ssl);
    return NULL;
  }

  tls_socket *socket = malloc(sizeof(tls_socket));
  socket->socket_fd = client;
  socket->addr = addr;
  socket->ssl = ssl;

  return socket;
} /* accept_tls_connection() */

/*
 * Close and free the passed TLS socket acceptor
 */

int close_tls_acceptor(tls_acceptor *acceptor) {
  close(acceptor->master_socket);
  SSL_CTX_free(acceptor->ssl_ctx);
  cleanup_openssl();
  free(acceptor);
  return 0;
} /* close_tls_acceptor() */

