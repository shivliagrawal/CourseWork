//File for functions focused on building a Huffman Tree
#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"

void clearNodeArray(NodeArray * node_array){
        for (int i = 0; i < node_array->nChars; i++) {
                free(node_array->nodes[i]);
        }
        free(node_array);
}

// Checks to see if nChars == capacity in the node_array
// If so, the size of the node_array is doubled
void resizeArrayIfNeeded(NodeArray * node_array){
        //TODO: write your implementation here
        if (node_array->capacity == node_array->nChars) {
            // Node * arr = (Node*)malloc((2 * node_array->capacity) * sizeof(Node));
            // for (int i = 0; i < node_array->capacity; i++) {
            //     arr[i] = node_array->nodes[i];
            // }
            // free(node_array->nodes);
            // node_array->nodes = arr;
            // arr = NULL;
            // node_array->capacity *= 2;

            *node_array->nodes = (Node*)realloc(node_array->nodes, (2 * node_array->capacity) * sizeof(Node));
            node_array->capacity *= 2;
        }
}

// First calls resizeArrayIfNeeded
// Then, inserts a new node at the end of the node array for the new character
// All values for the node are initialized, and the nChars count is updated
void insertInNodeArray(int c, int freq, NodeArray * node_array){
        //TODO: write your implementation here
        Node * node = (Node*)malloc(sizeof(Node));
        node->left = NULL;
        node->right = NULL;
        node->val = c;
        node->frequency = freq;
        node_array->nodes[node_array->nChars] = node;
        node_array->nChars++;
}

// Node comparison function
// Compares two nodes by frequency
//
// Returns the difference a->freq - b->freq if it is non-zero
// Else, returns the difference a->val - b->val
int compNodes (const void * a, const void * b){
        //TODO: write your implementation here
        Node * aNode = (Node*) a;
        Node * bNode = (Node*) b;
        if (aNode->frequency != bNode->frequency) {
                return aNode->frequency - bNode->frequency;
        } else {
                return aNode->val - bNode->val;
        }
}

// Ascending sorting function for the nodes of the node array
// Sorts in ascending order by frequency
// (uses minimum ascii value as a tie breaker)
// Hint: use the function compNodes to simplify your function
void sortNodeArray(NodeArray * node_array){
        //TODO: write your implementation here
        int n = node_array->nChars;
        for (int i = 1; i < n; i++) {
            for (int j = 0; j < n - i; j++) {
                if (compNodes(node_array->nodes[j], node_array->nodes[j + 1]) > 0) {
                        Node * e = node_array->nodes[j];
                        node_array->nodes[j] = node_array->nodes[j + 1];
                        node_array->nodes[j + 1] = e;
                }
            }
        }
}

// Allocates, initializes, and returns a node array
NodeArray * initializeNodeArray(int * ascii_freq, int nChars){
        //TODO: write your implementation here
        NodeArray * nodeArray = (NodeArray*)malloc(sizeof(NodeArray));
        nodeArray->nodes = (Node**)malloc(512 * sizeof(Node*));
        nodeArray->nChars = 0;
        nodeArray->capacity = 512;
        for (int i = 0; i < 256; i++) {
                nodeArray->nodes[i] = NULL;
        }
        for (int i = 0; i < 256; i++) {
            if (ascii_freq[i] != 0) {
                insertInNodeArray(i, ascii_freq[i], nodeArray);
            }
        }
        sortNodeArray(nodeArray);
        return nodeArray;
}

// Opens the specified file and counts the frequency of each ascii character in it
// Returns a node array corresponding to the frequencies
NodeArray * createHistogramFromFile(char * input_file, int * ascii_freq){
        //TODO: write your implementation here
        FILE * fd = fopen(input_file, "r");
        if (fd == NULL) {
            return NULL;
        }
        for (int i = 0; i < 256; i++) {
            ascii_freq[i] = 0;
        }
        int c;
        while ((c = fgetc(fd)) != -1) {
            ascii_freq[c]++;
        }
        fclose(fd);
        NodeArray * node_array = initializeNodeArray(ascii_freq, 256);
        return node_array;
}



// Prints the node array in the format "[character], [frequency]\n"
void printNodeArray(NodeArray * node_array){
        for (int i=0; i < node_array->nChars; i++){
                // Node * n = (Node*)((char*)node_array->nodes + i*sizeof(Node));
                Node * n = node_array->nodes[i];
                printf("%c, %u\n", n->val, n->frequency);
        }
}

// Returns the minimum node of the node_array,
// assuming that the array has been sorted
// If the array is empty, returns NULL
Node * getMin(NodeArray * node_array){
        if (node_array->nChars > 0)
                return (Node*)node_array->nodes;
        else return NULL;
}

//Selects the minimum of two nodes as the next child based on frequency
//Character value is used as a tie breaker if needed
Node* selectChild(Node* n1, Node* n2){
        //TODO: write your implementation here
        int k = compNodes(n1, n2);
        if (k > 0) {
                return n2;
        } else {
                return n1;
        }
}

//Initializes a parent node by assigning its children
//Also sets its frequency to the sum of its children's frequencies
//and sets its value to the minimum value of its children
void initParent(Node* parent, Node* left, Node* right){
        parent->frequency = left->frequency + right->frequency;
        parent->left = left;
        parent->right = right;
        if(right->val < left->val) parent->val = right->val;
        else parent->val = left->val;
}


// Builds a Huffman Tree and associates characters to their code using a node_array
// Assumes that the node array has already been created and sorted
NodeArray * buildHuffmanTree(NodeArray * node_array){
        //TODO: Allocate another node array to serve as the Huffman Tree
        NodeArray * huffman_tree;
        //Declare indicies for both arrays
        int leaf_index=0, internal_index=0;
        //Declare the next available index to use for internal nodes
        int next_internal=0;

        //TODO: iterate through node_array and any created internal nodes to build the huffman tree:
        // (1) While any leaves or more than 1 non-leaves have not been turned into children, 
        //              select the minimum two nodes from either node array
        // (2) Use these nodes as the children left and right passed to initParent

        while (1) {
                Node * n1 = NULL;
                Node * n2 = NULL;

                int i = 0;
                int i1 = -1;
                for (i = 0; i < node_array->nChars; i++) {
                        Node * n = node_array->nodes[i];

                        if (n1 == NULL && !n->accounted) {
                                n1 = n;
                                i1 = i;
                        } else if (!n->accounted && n->frequency < n1->frequency) {
                                n1 = n;
                                i1 = i;
                        } else if (!n->accounted && n->frequency == n1->frequency && n->val < n1->val) {
                                n1 = n;
                                i1 = i;
                        }
                }
                n1->accounted = 1;

                int i2 = -1;
                for (i = 0; i < node_array->nChars; i++) {
                        Node * n = node_array->nodes[i];

                        if (n2 == NULL && !n->accounted) {
                                n2 = n;
                                i2 = i;
                        } else if (!n->accounted && n->frequency < n2->frequency) {
                                n2 = n;
                                i2 = i;
                        } else if (!n->accounted && n->frequency == n2->frequency && n->val < n2->val) {
                                n2 = n;
                                i2 = i;
                        }
                }
                if (n2 == NULL) {
                        break;
                }

                n2->accounted = 1;
                Node * m = (Node*)malloc(sizeof(Node));
                node_array->nodes[node_array->nChars] = m;
                node_array->nChars++;
                m->accounted = 0;
                m->left = n1;
                m->right = n2;
                if (n1->val < n2->val) {
                        m->val = n1->val;
                } else {
                        m->val = n2->val;
                }
                m->frequency = n1->frequency + n2->frequency;
        }
        huffman_tree = node_array;

        //Assign the codes for the leaves (characters) of the Huffman Tree and then return it
        codifyChars(huffman_tree);
        return huffman_tree;
}


//Returns the root node of the Huffman Tree
Node * getRootHuffmanTree(NodeArray * huffman_tree){
        return (Node*) (huffman_tree->nodes[huffman_tree->nChars - 1]);
}

// A leaf node has no children (so its left and right pointers are NULL)
//
// Returns 1 if the node is a leaf, 0 otherwise
int isLeafNode(Node * n){
        if (n->left == NULL && n->right == NULL) return 1;
        else return 0;
}

// Prints a huffman tree recursively from the root
// Prints a node as "[ascii value] = [ascii character]\t[bitstring]]\n"
// Note: The ascii value should be padded with 0's to be 3 digits
// Note2: The left subtree is assigned a 1 bit,
//              and the right subtree is assigned a 0 bit
void printTree(Node * root, int code[], int i){
        if (root == NULL) return;
        //TODO: write your implementation here
        code[i] = 1;
        printTree(root->left, code, i + 1);
        code[i] = 0;
        printTree(root->right, code, i + 1);
        if (root->left == NULL && root->right == NULL) {
                printf("%03d = %c\t", root->val, root->val);
                for (int j = 0; j < i; j++) {
                        if (code[j] == 1) {
                                printf("1");
                        } else {
                                printf("0");
                        }
                }
                printf("\n");
                return;
        }
}

// Traverses a Huffman Tree from the root to each node, and records each path
// As the code string for each character
//
// The left child of a node should have a 1 bit appended to their code
// The right child of a node should have a 0 bit appended to their code
void textToCode(Node * root, int code[], int i){
        if (root == NULL) return ;

        //Assign codes for the left subtree
        code[i] = 1;
        textToCode(root->left, code, i+1);

        //Assign codes for the right subtree
        code[i] = 0;
        textToCode(root->right, code, i+1);
        if (root->left == NULL && root->right == NULL) {
                char_to_code[root->val] = (char*)malloc(MAXCODELENGTH);
                for (int j = 0; j < i; j++) {
                        char_to_code[root->val][j] == '0' + code[j];
                }
                char_to_code[root->val][i] = '\0';
                return;
        }
        //TODO: write your implementation here to assign a code to the current node
        //Note: only leaves need to have a code assigned!
}

// Prints a huffman tree by calling printTree on the root node of the tree
void printHuffmanTree(NodeArray * huffman_tree){
        Node * root = getRootHuffmanTree(huffman_tree);

        int code[MAXCODELENGTH] = {0};
        printTree(root, code, 0);
}

// Calls textToCode on the root of the provided huffman tree
void codifyChars(NodeArray * huffman_tree){
        Node * root = getRootHuffmanTree(huffman_tree);

        int code[MAXCODELENGTH] = {0};
        textToCode(root, code, 0);
}

