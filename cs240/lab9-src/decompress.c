//File for functions focused on decompression/decoding
#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"

// Read and return a single int from the file
int readIntFromFile(FILE * in_file){
	int n;
        fread(&n, sizeof(int), 1, in_file);
        return n;
	//TODO: write your implementation here
}


// Read and return a single char from the file
char readCharFromFile(FILE * in_file){
	return (char)fgetc(in_file);
}

// Read and return the number of unique characters in the file
// Obtain all character frequencies of the encoded file and store them in ascii_freq
int loadFrequenciesFromFile(FILE * in_file, int* ascii_freq){
	int nChars;
        fread(&nChars, sizeof(int), 1, in_file);
        char c;
        int freq;
        for (int i = 1; i <= nChars; i++) {
                fread(&c, sizeof(char), 1, in_file);
                fread(&freq, sizeof(unsigned int), 1, in_file);
                ascii_freq[c] = freq;
        }
        return nChars;
	//TODO: write your implementation here
}


// Allocates, initializes, and returns a node array from the file
// (1) Load all frequency information from the header of the specified file
// (2) Builds a node array from the file frequencies
NodeArray * buildNodeArrayFromFile(FILE* in_file){
	unsigned ascii_freq[256] = {0};
        int nChars = loadFrequenciesFromFile(in_file, ascii_freq);
        NodeArray * node_array = initializeNodeArray(ascii_freq, nChars);
        sortNodeArray(node_array);
        return node_array;
	//TODO: write your implementation here
}

// Decodes the input file using the huffman tree
// Write the decoded file into the output file
// Assumes that the next byte to read is the first byte to decode (just past the header)
void writeUncompressedFile(FILE* in_file, FILE* out_file, NodeArray* huffman_tree){
	int nChars = 0;
        char c;
        Node * node = getRootHuffmanTree(huffman_tree);
        int freq = node->frequency;
        while (1) {
                c = readCharFromFile(in_file);
                for (int i = 0; i < 8; i++) {
                        unsigned char mask = 1 << (7 - i);
                        int bit = (c & mask) >> (7 - i);
                        if (bit == 1) {
                                node = node->left;
                        } else if (bit == 0) {
                                node = node->right;
                        }

                        if (node->left == NULL && node->right == NULL) {
                                fwrite(&node->val, sizeof(char), 1, out_file);
                                nChars++;
                                node = getRootHuffmanTree(huffman_tree);
                        }
                        if (nChars == freq) {
                                return;
                        }
                }
        }
	//TODO: write your implementation here
}

// Primary decoding function: 
// (1) Open the input file and load all frequency information
// (2) Build the Huffman Tree for those frequencies
// (3) Traverse the Huffman Tree bit-by-bit and write each decoded
// character to the output file
// (4) Free the node arrays and close the files
void uncompress(char * input_file, char* output_file){
	FILE * input = fopen(input_file, "rb");
        if (input == NULL) {
                printf("Could not open file %s\n", input_file);
                exit(1);
        }
        FILE * output = fopen(output_file, "w");
        if (output == NULL) {
                printf("Could not open file %s\n", output_file);
                exit(1);
        }
        NodeArray * nodeArray = buildNodeArrayFromFile(input);
        NodeArray * huffmanTree = buildHuffmanTree(nodeArray);
        writeUncompressedFile(input, output, huffmanTree);
        fclose(output);
        fclose(input);
	//TODO: write your implementation here
}
