#include "misc.h"

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>   // For PATH_MAX
#include <errno.h>

/*
 * Returns the Content-Type value that should be used for a given filename.
 * The returned string must be free()'d by the caller.
 */
char *get_content_type(char *filename) {
    // Ensure filename is valid and prevent command injection
    if (!filename || strchr(filename, '\'') || strchr(filename, '`') || strchr(filename, ';')) {
        return strdup("application/octet-stream");
    }

    // Extract file extension
    char *ext = strrchr(filename, '.');
    if (ext != NULL) {
        ext++; // Move past the dot character
    }

    // Define common MIME types
    if (ext) {
        if (strcasecmp(ext, "html") == 0 || strcasecmp(ext, "htm") == 0) {
            return strdup("text/html");
        } else if (strcasecmp(ext, "css") == 0) {
            return strdup("text/css");
        } else if (strcasecmp(ext, "js") == 0) {
            return strdup("application/javascript");
        } else if (strcasecmp(ext, "json") == 0) {
            return strdup("application/json");
        } else if (strcasecmp(ext, "png") == 0) {
            return strdup("image/png");
        } else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) {
            return strdup("image/jpeg");
        } else if (strcasecmp(ext, "gif") == 0) {
            return strdup("image/gif");
        } else if (strcasecmp(ext, "txt") == 0) {
            return strdup("text/plain");
        } else if (strcasecmp(ext, "svg") == 0) {
            return strdup("image/svg+xml");
        } else if (strcasecmp(ext, "ico") == 0) {
            return strdup("image/vnd.microsoft.icon");
        }
    }

    // Fallback to 'file' command for unknown types
    char command[PATH_MAX + 50];
    snprintf(command, sizeof(command), "file -bi '%s'", filename);

    // Open a pipe to read the output of the command
    FILE *fp = popen(command, "r");
    if (!fp) {
        fprintf(stderr, "Failed to run command: %s\n", strerror(errno));
        return strdup("application/octet-stream");
    }

    char buf[256];
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        pclose(fp);
        return strdup("application/octet-stream");
    }
    pclose(fp);

    // Remove any trailing whitespace
    size_t len = strlen(buf);
    while (len > 0 && isspace((unsigned char)buf[len - 1])) {
        buf[len - 1] = '\0';
        len--;
    }

    // Extract MIME type before any semicolon (to remove charset info)
    char *semicolon = strchr(buf, ';');
    if (semicolon) {
        *semicolon = '\0';
    }

    // Duplicate the MIME type string
    return strdup(buf);
} /* get_content_type() */

/*
 * Generate an HTML string which can be used for the message_body of a
 * response. files must have at least num_files files correctly set.
 * The returned string must be free()'d by the caller.
 */
char *generate_dir_listing(char *dir_name, int num_files, file_link *files) {

#define DIR_LIST_INITIAL_BUF_SIZE (1024)
#define HEADER_FORMAT "<!doctype html>\n" \
    "<html>\n<head>\n<title>%s</title>\n</head>\n" \
    "<body>\n<h1>Index of %s</h1>\n" \
    "<ul>\n"
#define LIST_ITEM_FORMAT "<li><a href=\"%s\">%s</a></li>\n"
#define FOOTER_FORMAT "</ul>\n</body>\n</html>\n"

  int buffer_size = DIR_LIST_INITIAL_BUF_SIZE;
  char *buffer = malloc(sizeof(char) * buffer_size);
  if (!buffer) {
    return NULL;
  }

  int written = snprintf(buffer, buffer_size, HEADER_FORMAT, dir_name, dir_name);
  if (written >= buffer_size) {
    buffer_size = written + 1;
    buffer = realloc(buffer, buffer_size);
    if (!buffer) {
      return NULL;
    }
    snprintf(buffer, buffer_size, HEADER_FORMAT, dir_name, dir_name);
  }

  for (int i = 0; i < num_files; i++) {
    // Calculate the required length for the list item
    int item_length = snprintf(NULL, 0, LIST_ITEM_FORMAT, files[i].url, files[i].display_name);
    // Check if buffer needs to be resized
    if (strlen(buffer) + item_length + 1 > buffer_size) {
      buffer_size += item_length + DIR_LIST_INITIAL_BUF_SIZE;
      buffer = realloc(buffer, buffer_size);
      if (!buffer) {
        return NULL;
      }
    }
    // Append the list item to the buffer
    snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), LIST_ITEM_FORMAT, files[i].url, files[i].display_name);
  }

  // Append the footer
  if (strlen(buffer) + strlen(FOOTER_FORMAT) + 1 > buffer_size) {
    buffer_size += strlen(FOOTER_FORMAT) + 1;
    buffer = realloc(buffer, buffer_size);
    if (!buffer) {
      return NULL;
    }
  }
  strcat(buffer, FOOTER_FORMAT);

  return buffer;
} /* generate_dir_listing() */


