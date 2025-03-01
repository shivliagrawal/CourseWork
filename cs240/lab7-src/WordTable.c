#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "WordTable.h"

// Initializes a word table
void wtable_init(WordTable * wtable)
{
        // Allocate and initialize space for the table
        wtable->nWords = 0;
        wtable->maxWords = 10;
        wtable->wordArray = (WordInfo *) malloc(wtable->maxWords * sizeof(WordInfo));
        for (int i = 0; i < wtable->maxWords; i++) {
                llist_init(&wtable->wordArray[i].positions);
        }
}

// Add word to the tableand position. Position is added to the corresponding linked list.
void wtable_add(WordTable * wtable, char * word, int position)
{
        // Find first word if it exists
        for (int i = 0; i < wtable->nWords; i++) {
                if (strcmp(wtable->wordArray[i].word, word) == 0) {
                        // Found word. Add position in the list of positions
                        llist_insert_last(&wtable->wordArray[i].positions, position);
                        return;
                }
        }

        // Word not found.

        // Make sure that the array has space.
        // Expand the wordArray here.
        if (wtable->nWords == wtable->maxWords) {
                WordInfo* wi = (WordInfo*)malloc((2 * wtable->maxWords) * sizeof(WordInfo));
                for (int i = 0; i < wtable->maxWords; i++) {
                    wi[i] = wtable->wordArray[i];
                }
                free(wtable->wordArray);
                wtable->wordArray = wi;
                wi = NULL;
                wtable->maxWords *= 2;
        }
        // Add new word and position
        wtable->wordArray[wtable->nWords].word = strdup(word);
        llist_insert_last(&wtable->wordArray[wtable->nWords].positions, position); 
        wtable->nWords++;
}

// Print contents of the table.
void wtable_print(WordTable * wtable, FILE * fd)
{
        fprintf(fd, "------- WORD TABLE -------\n");
        // Print words
        for (int i = 0; i < wtable->nWords; i++) {
                fprintf(fd, "%d: %s: ", i, wtable->wordArray[i].word);
                llist_print(&wtable->wordArray[i].positions);
        }
}
//
// Get positions where the word occurs
LinkedList * wtable_getPositions(WordTable * wtable, char * word)
{
        for (int i = 0; i < wtable->nWords; i++) {
            if (strcmp(wtable->wordArray[i].word, word) == 0) {
                return &wtable->wordArray[i].positions;
            }
        }
        return NULL;
}

//
// Separates the string into words
//

#define MAXWORD 200
char word[MAXWORD];
int wordLength;
int wordCount;
int charCount;
int wordPos;

// It returns the next word from stdin.
// If there are no more more words it returns NULL.
// A word is a sequence of alphabetical characters.
static char * nextword(FILE * fd) {
        char c;
        memset(word, 0, sizeof(word));
        c = fgetc(fd);
        charCount++;
        while (c != EOF && ((!(c >= 65 && c <= 90) && !(c >= 97 && c <= 122)) || c == ' ' || c == '\n' || c == '\t' || c == '\r')) {
                c = fgetc(fd);
                charCount++;
        }
        wordPos = charCount - 1;
        wordLength = 0;
        while (c != EOF) {
            if ((!(c >= 65 && c <= 90) && !(c >= 97 && c <= 122)) || c == ' ' || c == '\n' || c == '\t' || c == '\r') {
                wordCount++;
                word[wordLength + 1] = '\0';
                return word;
            }
            word[wordLength] = c;
            wordLength++;
            c = fgetc(fd);
            charCount++;
        }
        return NULL;
}

// Convert string to lower case
void toLower(char *c) {
        while (*c) {
            unsigned int a = *(unsigned char*)c;
            if (a >= 65 && a <= 90) {
                char r = a + 32;
                *c = r;
            }
            c++;
        }
}


// Read a file and obtain words and positions of the words and save them in table.
int wtable_createFromFile(WordTable * wtable, char * fileName, int verbose) {
        FILE* fd = fopen(fileName, "r");
        if (fd == NULL) {
            return 0;
        }
        char* w;
        w = nextword(fd);
        toLower(w);
        if (verbose == 1) {
            int i = 0;
            while (w != NULL) {
                printf("%d: word=%s, pos=%d\n", i, w, wordPos);
                wtable_add(wtable, w, wordPos);
                i++;
                w = nextword(fd);
                if (w != NULL) {
                    toLower(w);
                }
            }
            return 0;
        } else {
            int i = 0;
            while (w != NULL) {
                wtable_add(wtable, w, wordPos);
                i++;
                w = nextword(fd);
                if (w != NULL) {
                    toLower(w);
                }
            }
            return 0;
        }
        return 0;
}

// Sort table in alphabetical order.
void wtable_sort(WordTable * wtable)
{
        int n = wtable->nWords;
        for (int i = 1; i < n; i++) {
            for (int j = 0; j < n - i; j++) {
                if (strcmp(wtable->wordArray[j].word, wtable->wordArray[j + 1].word) > 0) {
                    WordInfo w = wtable->wordArray[j];
                    wtable->wordArray[j] = wtable->wordArray[j + 1];
                    wtable->wordArray[j + 1] = w;
                }
            }
        }
}

// Print all segments of text in fileName that contain word.
// at most 200 character. Use fseek to position file pointer.
// Type "man fseek" for more info. 
int wtable_textSegments(WordTable * wtable, char * word, char * fileName){
        FILE* fd = fopen(fileName, "r");
        if (fd == NULL) {
            return 1;
        }
        LinkedList* positions = wtable_getPositions(wtable, word);
        printf("===== Segments for word \"%s\" in book \"%s\" =====\n", word, fileName);
        ListNode* e;
        e = positions->head;
        while (e != NULL) {
            char* buffer = malloc(201 * sizeof(char));
            printf("---------- pos=%d-----\n", e->value);
            fseek(fd, e->value, SEEK_SET);
            fread(buffer, 1, 200, fd);
            printf("......");
            printf("%s", buffer);
            printf("......\n");
            e = e->next;
        }
return 0;
}
