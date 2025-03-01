//File for the main function
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

void printHelp(){
	printf("Invalid arguments.\nType \"huffman [mode] [arguments]\" where mode and its arguments are one of the following:\n");
	printf("-p [src]: print the huffman tree of the file\n");
	printf("-pc [src]: print the huffman tree of an encoded file (using frequencies listed in its header)\n");
	printf("-pn [src]: print the node array of the file\n");
	printf("-e [src] [dest]: encode the source file and place the result into the specified destination\n");
	printf("-ep [src] [dest]: encode and print the resulting huffman tree\n");
	printf("-d [src] [dest]: decode the source file and place the result into the specified destination\n");
	printf("-dpc [src] [dest]: decode and print the resulting huffman tree\n");

	exit(1);
}

// Function for -pn option
void printNA(char* src){
	//Make frequency array
	unsigned ascii_freq[256] = {0};
	//Make the initial sorted node array
	NodeArray * node_array = createHistogramFromFile(src, ascii_freq);
	//Print the node array
	printNodeArray(node_array);
	//free memory
	clearNodeArray(node_array);
}

// Function for -p option
void printHT(char* src){
	unsigned ascii_freq[256] = {0};
        NodeArray * nodeArray = createHistogramFromFile(src, ascii_freq);
        buildHuffmanTree(nodeArray);
        printHuffmanTree(nodeArray);
	//TODO: write your implementation here
}

//Function for -e option
void encode(char* src, char* dest){
	unsigned ascii_freq[256] = {0};
        NodeArray * nodeArray = createHistogramFromFile(src, ascii_freq);
        NodeArray * huffmanTree = buildHuffmanTree(nodeArray);
        createCompressedFile(src, dest, nodeArray,  huffmanTree);
	//TODO: write your implementation here
}

//Function for -ep option
void encodeAndPrintHT(char* src, char* dest){
	unsigned ascii_freq[256] = {0};
        NodeArray * nodeArray = createHistogramFromFile(src, ascii_freq);
        NodeArray * huffmanTree = buildHuffmanTree(nodeArray);
        createCompressedFile(src, dest, nodeArray,  huffmanTree);
        printHuffmanTree(huffmanTree);

	//TODO: write your implementation here
}

//Function for -pc option
void printHTFromEncoded(char* src){
	unsigned ascii_freq[256] = {0};
        FILE * file = fopen(src, "rb");
        if (file == NULL) {
                printf("Could not open file %s\n", src);
                exit(1);
        }
        NodeArray * nodeArray = buildNodeArrayFromFile(file);
        NodeArray * huffmanTree = buildHuffmanTree(nodeArray);
        printHuffmanTree(huffmanTree);
	//TODO: write your implementation here
}

//Function for -d option TODO
void decode(char* src, char* dest){
	uncompress(src, dest);
}

//Function for -dpc option TODO
void decodeAndPrintHT(char* src, char* dest){
	uncompress(src, dest);
        FILE * file = fopen(src, "rb");
        if (file == NULL) {
                printf("Could not open file %s\n", src);
                exit(1);
        }
        NodeArray * nodeArray = buildNodeArrayFromFile(file);
        NodeArray * huffmanTree = buildHuffmanTree(nodeArray);
        printHuffmanTree(huffmanTree);
	//TODO: write your implementation here
}

//	p: print huffman of non-encoded file
//	pc: print huffman of encoded file (recontuct it from the header data)
//	pn: print node array of non-encoded file
//	e: encode infile into outfile
//		-ep: encode and print the huffman tree
//	d: decode coded_file into outfile
//		-dpc: decode and print the huffman tree
//
// huffman [-pna infile] [-ph uncoded_file] [-phc coded_file] [-e infile outfile] [-d coded_file outfile]
//
int main (int argc, char** argv){
	if(argc<3 || argc>4) printHelp();

	char* mode = argv[1];
	char* src = argv[2];

	//Check if the mode matches one of the 3 argument modes
	//TODO: Complete the main function below
	if(argc==3){
		if(!strcmp(mode, "-p")){
			printHT(src);
		} else if (!strcmp(mode, "-pc")) {
                        printHTFromEncoded(src);
                } else if (!strcmp(mode, "-pn")) {
                        printNA(src);
                } else {
                        printHelp();
                }
	}
	else{
		//Must have four arguments
		char* dest = argv[3];
		if(!strcmp(mode, "-d")){
			decode(src, dest);
		} else if (!strcmp(mode, "-e")) {
                        encode(src, dest);
                } else if (!strcmp(mode, "-ep")) {
                        encodeAndPrintHT(src, dest);
                } else if (!strcmp(mode, "-dpc")) {
                        decodeAndPrintHT(src, dest);
                } else {
                        printHelp();
                }
	}	
	return 0;
}

