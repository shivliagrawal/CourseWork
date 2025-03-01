
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LinkedList.h"
//
// Initialize a linked list
//
void llist_init(LinkedList * list) {
        list->head = NULL;
}

//
// It prints the elements in the list in the form:
// 4, 6, 2, 3, 8,7
//
void llist_print(LinkedList * list) {
        ListNode * e;

        if (list->head == NULL) {
                printf("{EMPTY}\n");
                return;
        }

        printf("{");

        e = list->head;
        while (e != NULL) {
                printf("%d", e->value);
                e = e->next;
                if (e!=NULL) {
                        printf(", ");
                }
        }
        printf("}\n");
}

//
// Appends a new node with this value at the beginning of the list
//
void llist_add(LinkedList * list, int value) {
        // Create a new node
        ListNode * n = (ListNode *) malloc(sizeof(ListNode));
        n->value = value;

        // Add at the beginning of the list
        n->next = list->head;
        list->head = n;
}

//
// Returns true if the value exists in the list.
//
int llist_exists(LinkedList * list, int value) {
        ListNode* e;
	if (list->head == NULL) {
		return 0;
	}

        e = list->head;
        while (e != NULL) {
                if (e->value == value) {
                        return 1;
                }
                e = e->next;
        }
        return 0;
}

//
// It removes the entry with that value in the list.
//
int llist_remove(LinkedList * list, int value) {
        ListNode* e;
        ListNode* p;

	if (list->head == NULL) {
                return 0;
        }

        e = list->head;
        p = NULL;
        while (e != NULL) {
                if (e->value == value) {
                       if (p == NULL) {
                                list->head = e->next;
                       } else {
                       p->next = e->next;
                       }
                       free(e);
                       return 1;
                }
                p = e;
                e = e->next; 
        }
        return 0;
}

//
// It stores in *value the value that correspond to the ith entry.
// It returns 1 if success or 0 if there is no ith entry.
//
int llist_get_ith(LinkedList * list, int ith, int * value) {
        ListNode* e;
        e = list->head;
        if (ith == 0) {
                if (e != NULL) {
                        *value = e->value;
                        return 1;
                }
                return 0;
        }
        for (int i = 0; i < ith; i++) {
                e = e->next;
                if (e == NULL) {
                        return 0;
                }
        }
        *value = e->value;
        return 1;
}

//
// It removes the ith entry from the list.
// It returns 1 if success or 0 if there is no ith entry.
//
int llist_remove_ith(LinkedList * list, int ith) {
        ListNode* e;
        ListNode* p;
        e = list->head;
        p = NULL;
        if (ith == 0) {
                if (e != NULL) {
                        list->head = e->next;
                        return 1;
                }
                return 0;
        }
        for (int i = 0; i < ith; i++) {
                p = e;
                e = e->next;
                if (e == NULL) {
                        return 0;
                }
        }
        p->next = e->next;
        free(e);
        return 1;
}

//
// It returns the number of elements in the list.
//
int llist_number_elements(LinkedList * list) {
        int n = 0;
        ListNode* e;
        e = list->head;
        while (e != NULL) {
                n++;
                e = e->next;
        }
        return n;
}


//
// It saves the list in a file called file_name. The format of the
// file is as follows:
//
// value1\n
// value2\n
// ...
//
int llist_save(LinkedList * list, char * file_name) {
        FILE *f = fopen(file_name, "w");
        if (f == NULL) {
               return 0; 
        }
        ListNode* e;
        e = list->head;
        while (e != NULL) {
                fprintf(f, "%d\n", e->value);
                e = e->next;
        }
        fclose(f);
        return 0;
}

//
// It reads the list from the file_name indicated. If the list already has entries, 
// it will clear the entries.
//
int llist_read(LinkedList * list, char * file_name) {
        FILE *f = fopen(file_name, "r");
        if (f == NULL) {
                return 0;
        }

        llist_clear(list);
        char* c = (char*)malloc(10 * sizeof(char));
        int a;
        while (fgets(c, 10, f)) {
                a = atoi(c);
                llist_add(list, a);
        }
        fclose(f);
        return 1;
}


//
// It sorts the list. The parameter ascending determines if the
// order si ascending (1) or descending(0).
//
void llist_sort(LinkedList * list, int ascending) {
        int z = llist_number_elements(list);
        if (ascending) {
                ListNode* e;
                ListNode* n;
                e = list->head;
                n = e->next;
                for (int i = 1; i < z; i++) {
                        for (int j = 0; j < z - i; j++) {
                                if (e->value > n->value) {
                                        int t = e->value;
                                        e->value = n->value;
                                        n->value = t;
                                }
                                e = e->next;
                                n = e->next;
                        }
                        e = list->head;
                        n = e->next;
                }
        } else {
                ListNode* e;
                ListNode* n;
                e = list->head;
                n = e->next;
                for (int i = 1; i < z; i++) {
                        for (int j = 0; j < z - i; j++) {
                                if (e->value < n->value) {
                                        int t = e->value;
                                        e->value = n->value;
                                        n->value = t;
                                }
                                e = e->next;
                                n = e->next;
                        }
                        e = list->head;
                        n = e->next;
                }
        }
}

//
// It removes the first entry in the list and puts value in *value.
// It also frees memory allocated for the node
//
int llist_remove_first(LinkedList * list, int * value) {
        ListNode* e;

        e = list->head;
        if (e != NULL) {
                *value = e->value;
                list->head = e->next;
                free(e);
                return 1;
        }
        return 0;
}

//
// It removes the last entry in the list and puts value in *value/
// It also frees memory allocated for node.
//
int llist_remove_last(LinkedList * list, int *value) {
        ListNode* e;
        ListNode* p;
        e = list->head;
        p = NULL;
        if (e != NULL) {
                while (e->next != NULL) {
                        p = e;
                        e = e->next;
                }
                if (p != NULL) {
                        *value = e->value;
                        p->next = NULL;
                        free(e);
                        return 1;
                } else {
                        *value = e->value;
                        list->head = NULL;
                        free(e);
                        return 1;
                }
        }
        return 0;
}

//
// Insert a value at the beginning of the list.
// There is no check if the value exists. The entry is added
// at the beginning of the list.
//
void llist_insert_first(LinkedList * list, int value) {
        ListNode* e = (ListNode*)malloc(sizeof(ListNode));
        e->value = value;
        e->next = list->head;
        list->head = e;
}

//
// Insert a value at the end of the list.
// There is no check if the name already exists. The entry is added
// at the end of the list.
//
void llist_insert_last(LinkedList * list, int value) {
        ListNode* e = (ListNode*)malloc(sizeof(ListNode));
        e->value = value;
        e->next = NULL;
        ListNode* f;
        f = list->head;
        if (f != NULL) {
                while (f->next != NULL) {
                        f = f->next;
                }
                f->next = e;
        } else {
                llist_insert_first(list, value);
        }
}

//
// Clear all elements in the list and free the nodes
//
void llist_clear(LinkedList *list) {
        ListNode* e;
        ListNode* p;
        p = list->head;
        if (p != NULL) {
                list->head = NULL;
                while (p != NULL) {
                        e = p;
                        p = p->next;
                        free(e);
                }
        }
}




