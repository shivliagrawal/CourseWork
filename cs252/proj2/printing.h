
#include <stdbool.h>

#include "my_malloc.h"

#define RELATIVE_POINTERS (true)

typedef void (*printFormatter)(header* block);

void basic_print(header* block);
void print_list(header* block);
void print_pointer(void* p);
void print_object(header* block);
void print_status(header* block);
void freelist_print(printFormatter pf);
