#include <stdio.h>
#include <unistd.h>

#include "my_malloc.h"

//TODO: From stack overflow
void DumpHex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}

void examine()
{
	header * h = freelist_root;
	while (h != NULL){
		printNode(h);
		h = h -> next;
	}
}

int main()
{

	int * arr[5];

	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	
	printf("Current %p\n", sbrk(0x0));
	arr[0] = (int *) my_malloc(16*sizeof(int));
	examine();
	printf("Current %p\n", sbrk(0x0));
	arr[1] = (int *) my_malloc(16*sizeof(int));
	examine();
	printf("Current %p\n", sbrk(0x0));
	arr[2] = (int *) my_malloc(16*sizeof(int));
	examine();
	printf("Current %p\n", sbrk(0x0));
	arr[3] = (int *) my_malloc(16*sizeof(int));
	examine();
	printf("Current %p\n", sbrk(0x0));

	for (int i = 0; i < 4; i++){
		printf("PTRS: %p\n", arr[i]);
	}

	#pragma clang diagnostic pop

	printf("\n\n\t\tGOOD FOR NOW\n\n");
	for (int i = 0; i < 4; i++){
		my_free(arr[i]);
		examine();
		printf("\nBREAK\n\n");
	}

	printf("\n\n\t\tGOOD FOR NOW\n\n");


	return 0;
}
