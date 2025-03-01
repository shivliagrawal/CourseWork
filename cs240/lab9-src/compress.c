//File for functions focused on compression/encoding
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

// Writes the number of unique characters to the file as a single byte
// Then, for each character in the node array, write the character (as a byte)
// and the frequency of that character (size = sizeof(unsigned int))
//Format: [nChars] [char1 ascii code(char)] [char1 frequency(int)] [char2 ascii code] [char2 frequency] ...
void writeCharFrequenciesinFile(FILE * out_file, NodeArray * node_array){
	fwrite(&node_array->nChars, sizeof(int), 1, out_file);
        for (int i = 0; i < node_array->nChars; i++) {
            fwrite(&node_array->nodes[i]->val, sizeof(char), 1, out_file);
            fwrite(&node_array->nodes[i]->frequency, sizeof(unsigned int), 1, out_file);
        }
	//TODO: write your implementation here
}

// Adds 0 bits to the end of the final byte if the encoded document
// ends with a partial byte (cannot write single bits to the file!)
void padBuffer(FILE * out_file){
	//TODO: write your implementation here
}

// Add the next bit to the bit buffer. If a full byte is buffered, 
// write it to the file (cannot write individual bits to a file!)
// Reset nBits and the bitBuffer if the buffer is written to the file
void writeBitBuffer(int bit, FILE * out_file){
	//TODO: write your implementation here
}


// Given a code string, it takes each "bit" representing character and
// writes it to the bit buffer for the output file
void writeCodeToFile (char * code, FILE * out_file){
	int i = 0;
        char c = 0x0;
        while (code[i] != '\0') {
            if (code[i] == '1') {
                unsigned mask = (1 << (7 - (i % 8)));
                c = c | mask;
            }
            i++;
            if (i % 8 == 0) {
                fwrite(&c, sizeof(char), 1, out_file);
                c = 0x0;
            }
        }
        if (i % 8 != 0) {
            fwrite(&c, sizeof(char), 1, out_file);
        }
	//TODO: write your implementation here
}


// Primary compression method
// (1) stores frequencies in the output file
// (2) rewrites the input file using the Huffman Code in the huffman_tree
void createCompressedFile(char* input_file, char * output_file, NodeArray * node_array, NodeArray * huffman_tree){
	nBits = 0;
        bit_buffer = 0;
        FILE * input = fopen(input_file, "r");
        FILE * output = fopen(output_file, "w");
        writeCharFrequenciesinFile(output, node_array);
        char c;
        char * s = (char*)malloc(5000);
        while ((c = fgetc(input)) != -1) {
            strcat(s, char_to_code[c]);
        }
        writeCodeToFile(s, output);
        fclose(input);
        fclose(output);
	//TODO: write your implementation here
}
