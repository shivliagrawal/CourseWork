#include "DLList.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


/**
 * @brief      Constructor for the DLList.
 */
DLList::DLList()
{
	/** @todo Write a constructor for a dllist. Check slides! **/
	head = new DLNode();
	nElements = 0;
	head->next = head;
	head->prev = head;
}

/**
 * @brief      Destructor for the DLList.
 */
DLList::~DLList()
{
	/** @todo Clean up your mess! **/
	DLNode* e = head->next;
	while (e != head) {
		DLNode* next = e->next;
		delete e;
		e = next;
	}
	delete head;
}

/**
 * @brief      Print the DLList line by line.
 */
void DLList::print()
{
	/** @todo Print this list line by line **/
	DLNode * e = head->next;
	while (e!= head) {
		std::cout << e->data << std::endl;
		e = e->next;
	}
}

/**
 * @brief      Sort and print the list.
 * 
 * This function should sort and print the list.
 * Note: the actual list is NOT to be modified.
 * In other words, it should only *print* in a
 * sorted order, not actually sort the list.
 */
void DLList::printSorted()
{
	/** @todo Print a sorted copy of this list **/
	if (nElements == 0) {
        return;
   	}

    	DLNode** node = new DLNode*[nElements];

    	DLNode* e = head->next;
    	int index = 0;
    	while (e != head) {
        	node[index++] = e;
        	e = e->next;
    	}

    	for (int i = 0; i < nElements - 1; ++i) {
        	for (int j = 0; j < nElements - 1 - i; ++j) {
            		if (node[j]->data > node[j + 1]->data) {
                	DLNode* temp = node[j];
                	node[j] = node[j + 1];
                	node[j + 1] = temp;
            		}	
        	}
    	}

    for (int i = 0; i < nElements; ++i) {
        std::cout << node[i]->data << std::endl;
    }

    delete[] node;
}

/**
 * @brief      Add to the front of the list.
 *
 * @param[in]  data  Item to add to front.
 */
void DLList::insertFront(int data)
{
	/** @todo Insert to the front of the list **/
	DLNode * e = new DLNode();
	e->data = data;

	e->next = head->next;
	e->prev = head;
	e->next->prev = e;
	head->next = e;

	nElements++;
}

/**
 * @brief      Removes & stores the last element.
 *
 * The last element is removed and stored in the
 * referenced variable data.
 * 
 * @param      data  Thing in which we are storing the data.
 *
 * @return     True upon successful removal.
 */
bool DLList::removeLast(int & data)
{
	/** @todo Remove the last thing **/
	if (nElements == 0) {
        	return false; 
    	}
	DLNode* last = head->prev;
	data = last->data;

    	last->prev->next = head;
   	head->prev = last->prev;

    	delete last;
	nElements--;

	return true;
}

/**
 * @brief      Difference of two lists.
 *
 * @param      list  Subtrahend.
 *
 * @return     Returns a pointer to the difference.
 */
DLList * DLList::difference(DLList & list)
{
	DLList * diff = new DLList();
	/** @todo Implement this function **/
	DLNode* e = this->head->next;
   	while (e != this->head) {
        
	DLNode* sub = list.head->next;
        bool found = false;
        while (sub != list.head) {
            if (e->data == sub->data) {
                found = true;
                break;
            }
            sub = sub->next;
        }

        if (!found) {
            diff->insertFront(e->data);
        }

        e = e->next;
    	}

	return diff;

}

/**
 * @brief      Returns a sublist of items in a range.
 *
 * @param[in]  start  First index.
 * @param[in]  end    Second index.
 *
 * @return     Elements between first and second index.
 */
DLList * DLList::getRange(int start, int end)
{
	DLList * range = new DLList();
	/** @todo getRange **/
	if (start < 0 || end >= nElements || start > end) {
        	return range;
    	}

    	DLNode* e = head->next;
    	int index = 0;

    	while (e != head) {
        	if (index >= start && index <= end) {
			DLNode* node = new DLNode();
            		node->data = e->data;

            		node->next = range->head;
           		node->prev = range->head->prev;
            		range->head->prev->next = node;
            		range->head->prev = node;
	    
	    		nElements++;
        	}
       		e = e->next;
        	index++;
    	}
	return range;
}

/**
 * @brief      Intersection of this list and another list.
 *
 * @param      list  The other list.
 *
 * @return     Elements list shares with this one.
 */
DLList * DLList::intersection(DLList & list)
{
	DLList * inter = new DLList();
	/** @todo intersection **/
	DLNode* e = this->head->next;
    	while (e != this->head) {
        	DLNode* f = list.head->next;
        	bool found = false;
        	while (f != list.head) {
            		if (e->data == f->data) {
                		found = true;
                		break;
            		}
            	f = f->next;
        	}

        	if (found) {
			DLNode* node = new DLNode();
            		node->data = e->data;
			node->next = inter->head;
            		node->prev = inter->head->prev;
            		inter->head->prev->next = node;
            		inter->head->prev = node;
			nElements++;
        	}

        	e = e->next;
    	}

	return  inter;
}

/**
 * @brief      Removes nodes in the start-end range.
 *
 * @param[in]  start  First node.
 * @param[in]  end    Second node.
 */
void DLList::removeRange(int start, int end)
{
	/** @todo Remove a range of elements **/
	if (start < 0 || end >= nElements || start > end) {
        	return;
    	}

    	DLNode* e = head->next;
    	int index = 0;

   	while (e != head && index < start) {
        	e = e->next;
        	index++;
    	}

    	DLNode* rStart = e;
    	DLNode* rEnd = NULL;

    	while (e != head && index <= end) {
        	rEnd = e;
        	e = e->next;
        	delete rEnd;
        	index++;
        	nElements--;
    	}	

    	if (rStart->prev != NULL && e != NULL) {
       		rStart->prev->next = e;
        	e->prev = rStart->prev;
    	}

}
