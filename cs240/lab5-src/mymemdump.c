
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void mymemdump(FILE * fd, char * p , int len) {
    // Add your code here.
    // You may see p as an array.
    // p[0] will return the element 0 
    // p[1] will return the element 1 and so on
    
    int i;
    int j=0;
    int c;
    int x;
    if (len % 16 == 0) {
	    x = (len/16) - 1;
    } else {
	    x = (len/16);
    }
    

    while(j <= x) {
	    fprintf(fd, "0x%016lX: ", (unsigned long) &p[j*16]); // Print address of the beginning of p. You need to print it every 16 bytes
	    if(j == x && (len%16) != 0) {
		    int last = len % 16;
		    for (int k=0; k<last; k++) {
                            c = p[k + (16*j)]&0xFF;
                            fprintf(fd, "%02X ", c);
                    }
                    for(int l=0; l<(16 - last); l++) {
                            fprintf(fd, "   ");
                    }
		    fprintf(fd, " ");
		    for (i=0; i < last; i++) {
			    c = p[i + (16*j)]&0xFF;
			    fprintf(fd, "%c", (c>=32 && c<127)?c:'.');
		    }
           
		    fprintf(fd,"\n");

	    } else if(j != x || (len%16) == 0) {
	   	 for (i=0; i < 16; i++) {
			    c = p[i+(j*16)]&0xFF; // Get value at [p]. The &0xFF is to make sure you truncate to 8bits or one byte.

      		 	 // Print first byte as hexadecimal
		   	 fprintf(fd, "%02X ", c);
	 	 }
		 fprintf(fd, " ");
		 for (i=0; i < 16; i++) {
			    c = p[i + (16*j)]&0xFF;
                            fprintf(fd, "%c", (c>=32 && c<127)?c:'.');
                 }

                 fprintf(fd,"\n");
	    }

	// Print first byte as character. Only print characters >= 32 and < 127 that are the printable characters.
	    j++;
    }

}

