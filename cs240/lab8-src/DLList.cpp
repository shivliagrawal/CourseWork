#include "DLList.h"
#include <stdio.h>

/**
 * @brief      Constructor for the DLList.
 */
DLList::DLList()
{
	/** @todo Write a constructor for a dllist. Check slides! **/
	// Create Sentinel node. This node does not store any data // but simplifies the list implementation.
	head = new DLNode();
	head->next = head;
	head->prev = head;

}

/**
 * @brief      Destructor for the DLList.
 */
DLList::~DLList()
{

}

/**
 * @brief      Print the DLList line by line.
 */
void DLList::print()
{
	/*Print this list line by line **/
	DLNode *e;
	e= head->next;
	while(e != head) {
		printf("%d\n",e->data);
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
	DLNode * e;
	int n = 0;
        e = head->next;
        while (e != head) {
            n++;                                        
            e = e->next;
        }
	int a[n];
        e = head->next;
        for (int i = 0; i < n; i++) {
            a[i] = e->data;
            e = e->next;                                                      
        }
        for (int i = 1; i < n; i++) {
            for (int j = 0; j < n - i; j++) {
                if (a[j] > a[j + 1]) {
                    int temp = a[j];
                    a[j] = a[j + 1];                                        
                    a[j + 1] = temp;
                }
            }
        }
        for (int i = 0; i < n; i++) {
            printf("%d\n", a[i]);                                     
        }
}

/**
 * @brief      Add to the front of the list.
 *
 * @param[in]  data  Item to add to front.
 */
void DLList::insertFront(int data)
{
	/** Insert to the front of the list **/
	DLNode * e = new DLNode(); 
	e->data = data;
	//Add at the beginning 
	e->next = head->next; 
	e->prev = head;
       	e->next->prev = e; 
	head->next = e;
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
	/** Remove the last thing **/
	DLNode *e;
	e = head->prev;
	if (e != head) {
		head->prev = e->prev;
		e->prev->next = head;
		data = e->data;
		delete e;
		return true;
	}
	return false;
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
	/** Implementing this function **/
        DLNode * a = head->next;
        DLNode * b = list.head->next;
        while (a != head) {
            while (b != list.head) {
                if (a->data == b->data) {
                    break;
                }
                b = b->next;                                                  
            }                                                                  
            if (b == list.head) {                                       
                diff->insertFront(a->data);
            }
            a = a->next;
            b = list.head->next;
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
	/**  intersection **/
        DLNode *e;
        DLNode *f;
        e = this->head->next;
        f = list.head->next;

        while(e != this->head) {
                while(f != list.head) {
                        if (e->data == f->data) {
                                DLNode * x;
				x = inter->head->prev;
				DLNode * n = new DLNode();
				n->data = e->data;
				x->next = n;
				n->prev = x;
				n->next = inter->head;
				inter->head->prev = n;
				break;
                        }
                        f = f->next;
		
		}
		f = list.head->next;
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
}
