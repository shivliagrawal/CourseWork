#include "handlers.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>

#include "misc.h"
#include "http_messages.h"

/*
 * Forward declarations of helper functions.
 */

http_response *handle_file_request(const http_request *request, const char *file_root);
http_response *handle_directory_listing(const char *directory_path, const char *request_uri);

/*
 * Compare function for sorting directory entries in ASCII order (case-sensitive).
 */

static int compare_entries_ascii(const void *a, const void *b) {
  const struct dirent *const *entry_a = (const struct dirent *const *)a;
  const struct dirent *const *entry_b = (const struct dirent *const *)b;
  return strcmp((*entry_a)->d_name, (*entry_b)->d_name);
} /* compare_entries_ascii() */

/*
 * Determine if a request should serve icons from outside htdocs.
 * If the request path starts with /icons/, return 1.
 */

static int is_icon_request(const char *uri) {
  return (strncmp(uri, "/icons/", 7) == 0);
} /* is_icon_request() */

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

  const char *document_root = "http-root-dir/htdocs";
  const char *icons_root = "http-root-dir/icons";
  const char *uri = request->request_uri;

  if (strcmp(uri, "/") == 0) {
    uri = "/index.html";
  }

  if (strstr(uri, "..") != NULL) {
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

  printf("Handling request for URI: %s\n", uri);

  if (is_icon_request(uri)) {
    const char *icon_name = uri + strlen("/icons/");
    printf("Serving icon: %s\n", icon_name);

    free(resp->http_version);
    free(resp);
    http_request icon_request = *request;
    icon_request.request_uri = strdup(icon_name);

    http_response *icon_response = handle_file_request(&icon_request, icons_root);
    free(icon_request.request_uri);
    return icon_response;
  }

  char filepath[PATH_MAX];
  if (snprintf(filepath, sizeof(filepath), "%s%s", document_root, uri) >= (int)sizeof(filepath)) {
    resp->status_code = 414;
    resp->reason_phrase = strdup("Request-URI Too Large");
    resp->num_headers = 2;
    resp->headers = malloc(sizeof(header) * 2);
    resp->headers[0].key = strdup("Connection");
    resp->headers[0].value = strdup("close");
    resp->headers[1].key = strdup("Content-Type");
    resp->headers[1].value = strdup("text/plain");
    resp->message_body = strdup("414 Request-URI Too Large");
    return resp;
  }

  printf("Serving file: %s\n", filepath);

  struct stat sb;
  if (stat(filepath, &sb) == -1) {
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

  if (S_ISDIR(sb.st_mode)) {
    char uri_slash[PATH_MAX];
    if (uri[strlen(uri) - 1] != '/') {
      if (snprintf(uri_slash, sizeof(uri_slash), "%s/", uri) >= (int)sizeof(uri_slash)) {
        resp->status_code = 414;
        resp->reason_phrase = strdup("Request-URI Too Large");
        resp->num_headers = 2;
        resp->headers = malloc(sizeof(header) * 2);
        resp->headers[0].key = strdup("Connection");
        resp->headers[0].value = strdup("close");
        resp->headers[1].key = strdup("Content-Type");
        resp->headers[1].value = strdup("text/plain");
        resp->message_body = strdup("414 Request-URI Too Large");
        return resp;
      }
    } else {
      snprintf(uri_slash, sizeof(uri_slash), "%s", uri);
    }

    char indexpath[PATH_MAX];
    if (snprintf(indexpath, sizeof(indexpath), "%s/index.html", filepath) >= (int)sizeof(indexpath)) {
      resp->status_code = 414;
      resp->reason_phrase = strdup("Request-URI Too Large");
      resp->num_headers = 2;
      resp->headers = malloc(sizeof(header) * 2);
      resp->headers[0].key = strdup("Connection");
      resp->headers[0].value = strdup("close");
      resp->headers[1].key = strdup("Content-Type");
      resp->headers[1].value = strdup("text/plain");
      resp->message_body = strdup("414 Request-URI Too Large");
      return resp;
    }

    struct stat index_sb;
    if ((stat(indexpath, &index_sb) == 0) &&
        (S_ISREG(index_sb.st_mode))) {
      free(resp->http_version);
      free(resp);
      http_request index_request = *request;
      index_request.request_uri = uri_slash;
      return handle_file_request(&index_request, document_root);
    } else {
      free(resp->http_version);
      free(resp);
      return handle_directory_listing(filepath, uri_slash);
    }
  }

  free(resp->http_version);
  free(resp);
  return handle_file_request(request, document_root);
} /* handle_request() */

/*
 * Handle file requests separately. This function serves the requested file from the specified file_root.
 */

http_response *handle_file_request(const http_request *request, const char *file_root) {
  http_response *resp = malloc(sizeof(*resp));
  if (!resp) {
    return NULL;
  }
  memset(resp, 0, sizeof(*resp));

  const char *uri = request->request_uri;
  char filepath[PATH_MAX];
  if (snprintf(filepath, sizeof(filepath), "%s/%s", file_root, uri) >= (int)sizeof(filepath)) {
    resp->status_code = 414;
    resp->reason_phrase = strdup("Request-URI Too Large");
    resp->num_headers = 2;
    resp->headers = malloc(sizeof(header) * 2);
    resp->headers[0].key = strdup("Connection");
    resp->headers[0].value = strdup("close");
    resp->headers[1].key = strdup("Content-Type");
    resp->headers[1].value = strdup("text/plain");
    resp->message_body = strdup("414 Request-URI Too Large");
    return resp;
  }

  struct stat sb;
  if (stat(filepath, &sb) == -1) {
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

  if (access(filepath, R_OK) != 0) {
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

  int fd = open(filepath, O_RDONLY);
  if (fd == -1) {
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

  off_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

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
  if (bytes_read < 0) {
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
  file_contents[bytes_read] = '\0';
  close(fd);

  resp->status_code = 200;
  resp->reason_phrase = strdup("OK");
  resp->http_version = strdup("HTTP/1.1");

  resp->num_headers = 3;
  resp->headers = malloc(sizeof(header) * resp->num_headers);
  resp->headers[0].key = strdup("Connection");
  resp->headers[0].value = strdup("close");

  char *content_type = get_content_type(filepath);
  if (!content_type) {
    content_type = strdup("application/octet-stream");
  }
  resp->headers[1].key = strdup("Content-Type");
  resp->headers[1].value = content_type;

  char content_length_str[32];
  snprintf(content_length_str, sizeof(content_length_str), "%ld", (long)bytes_read);
  resp->headers[2].key = strdup("Content-Length");
  resp->headers[2].value = strdup(content_length_str);

  resp->message_body = file_contents;
  return resp;
} /* handle_file_request() */

/*
 * Generate an HTTP response listing the contents of a directory.
 */

http_response *handle_directory_listing(const char *directory_path, const char *request_uri) {
  DIR *dir = opendir(directory_path);
  if (!dir) {
    http_response *resp = malloc(sizeof(*resp));
    if (!resp) return NULL;
    memset(resp, 0, sizeof(*resp));
    resp->status_code = 500;
    resp->http_version = strdup("HTTP/1.1");
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

  struct dirent **entries = NULL;
  int count = 0;
  int capacity = 10;
  entries = malloc(sizeof(struct dirent *) * capacity);
  if (!entries) {
    closedir(dir);
    return NULL;
  }

  while (1) {
    errno = 0;
    struct dirent *entry = readdir(dir);
    if (!entry) {
      if (errno != 0) {
        for (int i = 0; i < count; i++) {
          free(entries[i]);
        }
        free(entries);
        closedir(dir);
        http_response *resp = malloc(sizeof(*resp));
        if (!resp) return NULL;
        memset(resp, 0, sizeof(*resp));
        resp->status_code = 500;
        resp->http_version = strdup("HTTP/1.1");
        resp->reason_phrase = strdup("Internal Server Error");
        resp->num_headers = 2;
        resp->headers = malloc(sizeof(header) * 2);
        resp->headers[0].key = strdup("Connection");
        resp->headers[0].value = strdup("close");
        resp->headers[1].key = strdup("Content-Type");
        resp->headers[1].value = strdup("text/plain");
        resp->message_body = strdup("500 Internal Server Error");
        return resp;
      } else {
        break;
      }
    }

    if ((strcmp(entry->d_name, ".") == 0) ||
	(strcmp(entry->d_name, "..") == 0)) {
      continue;
    }

    if (entry->d_name[0] == '.') {
      continue;
    }

    struct dirent *entry_copy = malloc(sizeof(struct dirent));
    if (!entry_copy) {
      for (int i = 0; i < count; i++) {
        free(entries[i]);
      }
      free(entries);
      closedir(dir);
      return NULL;
    }
    memcpy(entry_copy, entry, sizeof(struct dirent));

    if (count == capacity) {
      capacity *= 2;
      struct dirent **new_entries = realloc(entries, sizeof(struct dirent *) * capacity);
      if (!new_entries) {
        for (int i = 0; i < count; i++) {
          free(entries[i]);
        }
        free(entries);
        closedir(dir);
        return NULL;
      }
      entries = new_entries;
    }
    entries[count++] = entry_copy;
  }
  closedir(dir);

  qsort(entries, count, sizeof(struct dirent *), compare_entries_ascii);

  size_t capacity_html = 1024;
  size_t used_html = 0;
  char *body = malloc(capacity_html);
  if (!body) {
    for (int i = 0; i < count; i++) {
      free(entries[i]);
    }
    free(entries);
    return NULL;
  }

  snprintf(body, capacity_html,
      "<html>\n"
      "<head><title>Index of %s</title></head>\n"
      "<body>\n"
      "<h1>Index of %s</h1>\n"
      "<table>\n"
      "<tr><th>Name</th><th>Last modified</th><th>Size</th></tr>\n"
      "<tr><th colspan=\"3\"><hr></th></tr>\n",
      request_uri, request_uri
  );
  used_html = strlen(body);

  if (strcmp(request_uri, "/") != 0) {
    char parent_uri[PATH_MAX];
    strcpy(parent_uri, request_uri);
    size_t uri_len = strlen(parent_uri);
    if (uri_len > 1 && parent_uri[uri_len - 1] == '/') {
      parent_uri[uri_len - 1] = '\0';
    }
    char *last_slash = strrchr(parent_uri, '/');
    if (last_slash && last_slash != parent_uri) {
      *last_slash = '\0';
    } else {
      strcpy(parent_uri, "/");
    }

    size_t needed = strlen(parent_uri) + 256;
    if (used_html + needed >= capacity_html) {
      capacity_html = used_html + needed + 1;
      char *temp = realloc(body, capacity_html);
      if (!temp) {
        for (int i = 0; i < count; i++) {
          free(entries[i]);
        }
        free(entries);
        free(body);
        return NULL;
      }
      body = temp;
    }

    used_html += snprintf(body + used_html, capacity_html - used_html,
        "<tr><td valign=\"top\"><img src=\"/icons/back.gif\" alt=\"[PARENTDIR]\"> <a href=\"%s\">Parent Directory</a></td><td>&nbsp;</td><td align=\"right\">-</td></tr>\n", parent_uri
    );
  }

  for (int i = 0; i < count; i++) {
    struct dirent *entry = entries[i];
    char full_path[PATH_MAX];
    if (snprintf(full_path, PATH_MAX, "%s/%s", directory_path, entry->d_name) >= (int)PATH_MAX) {
      free(entry);
      continue;
    }

    struct stat entry_stat;
    if (stat(full_path, &entry_stat) == -1) {
      free(entry);
      continue;
    }

    char *icon = "text.gif";
    char *slash = "";
    if (S_ISDIR(entry_stat.st_mode)) {
      icon = "folder.gif";
      slash = "/";
    } else {
      char *ext = strrchr(entry->d_name, '.');
      if (ext) {
        if (strcasecmp(ext, ".gif") == 0 || strcasecmp(ext, ".png") == 0 ||
            strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0 ||
            strcasecmp(ext, ".svg") == 0) {
          icon = "image.gif";
        }
      }
    }

    char time_str[64];
    struct tm *tm_info = localtime(&entry_stat.st_mtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);

    char size_str[32];
    if (S_ISDIR(entry_stat.st_mode)) {
      snprintf(size_str, sizeof(size_str), "%s", "-");
    } else {
      snprintf(size_str, sizeof(size_str), "%ld", (long)entry_stat.st_size);
    }

    char item_url[PATH_MAX * 2];
    if (strcmp(request_uri, "/") == 0) {
      if (snprintf(item_url, sizeof(item_url), "/%s%s", entry->d_name, slash) >= (int)sizeof(item_url)) {
        free(entry);
        continue;
      }
    } else {
      if (snprintf(item_url, sizeof(item_url), "%s%s%s", request_uri, entry->d_name, slash) >= (int)sizeof(item_url)) {
        free(entry);
        continue;
      }
    }

    size_t needed = strlen(entry->d_name) * 2 + 512;
    if (used_html + needed >= capacity_html) {
      capacity_html = used_html + needed + 1;
      char *temp = realloc(body, capacity_html);
      if (!temp) {
        for (int j = 0; j < count; j++) {
          if (j != i) {
            free(entries[j]);
          }
        }
        free(entries);
        free(body);
        return NULL;
      }
      body = temp;
    }

    char display_name[PATH_MAX];
    snprintf(display_name, sizeof(display_name), "%s%s", entry->d_name, slash);

    used_html += snprintf(body + used_html, capacity_html - used_html,
        "<tr><td valign=\"top\"><img src=\"/icons/%s\" alt=\"[ICO]\"> <a href=\"%s\">%s</a></td>"
        "<td align=\"right\">%s</td>"
        "<td align=\"right\">%s</td></tr>\n",
        icon, item_url, display_name, time_str, size_str
    );

    free(entry);
  }

  if (used_html + 256 >= capacity_html) {
    capacity_html = used_html + 256 + 1;
    char *temp = realloc(body, capacity_html);
    if (!temp) {
      free(body);
      return NULL;
    }
    body = temp;
  }

  used_html += snprintf(body + used_html, capacity_html - used_html,
      "<tr><th colspan=\"3\"><hr></th></tr>\n"
      "</table>\n"
      "<address>CS252 webserver</address>\n"
      "</body></html>\n"
  );

  free(entries);

  http_response *resp = malloc(sizeof(*resp));
  if (!resp) {
    free(body);
    return NULL;
  }
  memset(resp, 0, sizeof(*resp));

  resp->http_version = strdup("HTTP/1.1");
  resp->status_code = 200;
  resp->reason_phrase = strdup("OK");
  resp->num_headers = 2;
  resp->headers = malloc(sizeof(header) * resp->num_headers);
  resp->headers[0].key = strdup("Connection");
  resp->headers[0].value = strdup("close");
  resp->headers[1].key = strdup("Content-Type");
  resp->headers[1].value = strdup("text/html");
  resp->message_body = body;

  return resp;
} /* handle_directory_listing() */

