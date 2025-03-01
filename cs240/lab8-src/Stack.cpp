// Stack Implementation

// Used for cout << "string"
#include <iostream>

using namespace std;

#include "Stack.h"

// Constructor
Stack::Stack(int maxStack) {
	this->maxStack = maxStack;
	stack = new double[maxStack];
	top = 0;
}

// Push a value into the stack. 
// Return false if max is reached.
bool
Stack::push(double value) {
	if (top == maxStack) {
        return false;
	}
	stack[top]=value;
  	top++;
  	return true;
}


// Pop a value from the stack. 
// Return false if stack is empty
bool
Stack::pop(double & value) {
	if (top == 0) { return false;
	} top--;
	// Copy top of stack to variable value // passed by reference
	value = stack[top];
	return true;
}


// Return number of values in the stack
int
Stack::getTop() const {
	return top;
}	


// Return max number of values in stack
int
Stack::getMaxStack() const {
       	return maxStack;
}

// Prints the stack contents void
void
Stack::print() const {
       	cout << "Stack:" << endl;
   	if (top==0) {
		cout << "Empty" << endl;
       	}
	for (int i = 0; i < top; i++) {
		cout << i << ":" <<  stack[i] << endl;
	}
   	cout << "------" << endl;
}
// Destructor
Stack::~Stack() {
	delete [] stack;
}
