
// String Implementation
// IMPORTANT: Do not use any of the functions in the string C runtime library
// Example. Do not use strcpy, strcmp, etc. Implement your own

// IMPORTANT: See the MyString.h file for a description of
// what each method needs to do.

#include <stdio.h>
#include "MyString.h"

// My own implementation of strlen
int
MyString::slength(const char *s) const
{
	int cnt=0;
	while (*s != '\0') {
		cnt++;
		s++;
	}
      
  return cnt;
}

// Initialize _s. Allocate memory for _s and copy s into _s
void
MyString::initialize(const char * s)
{
	int len = slength(s);
	// Allocate memory for _s.
	_s = new char[len + 1];
	// Copy s into _s
	for(int i=0; i<len; i++) {
		_s[i] = s[i];
	}
	_s[len] = '\0';
}

// Create a MyString from a C string
MyString::MyString(const char * s)
{
	initialize(s);

}

// Create a MyString from a copy of another string
MyString::MyString(const MyString &s)
{

  	initialize(s._s);
}

// Create a MyString with an empty string
MyString::MyString()
{
  _s = new char[1];
  *_s = 0;

}

// Assignment operator. Without this operator the assignment is
// just a shallow copy that will copy the pointer _s. If the original _s
// goes away then the assigned _s will be a dangling reference.
MyString &
MyString::operator = (const MyString & other) {
  if (this != &other) // protect against invalid self-assignment
  {
    // deallocate old memory
    delete [] _s;

    // Initialize _s with the "other" object.
    initialize(other._s);

    // by convention, always return *this
    return *this;
  }
}

// Obtain a substring of at most n chars starting at location i
// if i is larger than the length of the string return an empty string.
MyString
MyString::substring(int i, int n)
{
  // Add implementation here

  // Make sure that i is not beyond the end of string
	int c = slength(_s);
	if(i >= c) {
		MyString s;
		return s;
	}
	int cnt=0;

  // Allocate memory for substring
	for(int j=i; j<c; j++) {
		if(cnt == n) {
			break;
		}
		cnt++;
	}
	char *arr = new char[cnt+1];

	MyString sub(arr);

  // Copy characters of substring
  	for(int x=0; x<cnt; x++) {
		sub._s[x] = _s[i+x];
	}
	sub._s[cnt] = '\0';

  // Return substring
 	 return sub;
}

// Remove at most n chars starting at location i
void
MyString::remove(int i, int n)
{
  // Add implementation here

  // If i is beyond the end of string return
 	int c = slength(_s);
        if(i >= c) {
                return;
        }

  // If i+n is beyond the end trunc string
  	if(i+n >= c) {
		_s[i] = '\0';
	}	

  // Remove characters
 	else {
		for(int a=0; a<n; a++) {
			_s[i+a] = _s[i+n+a];
		}
		for(int b=i+n; b<c; b++) {
			_s[b] = _s[b+n];
		}
	}
}

// Return true if strings "this" and s are equal
bool
MyString::operator == (const MyString & s)
{
  // Add implementation here
  	int len = s.slength(s._s);
	int len1 = slength(_s);

	if (len != len1) {
		return false;
	}
	else {
		for(int i=0; i<len; i++) {
			if(s._s[i] != _s[i]) {
			       return false;
			}
		}
	}	
 	return true;
}


// Return true if strings "this" and s are not equal
bool
MyString::operator != (const MyString &s)
{
  // Add implementation here
  	int len = s.slength(s._s);
        int len1 = slength(_s);
	if(len != len1) {
		return true;
	}
	else {
		for(int i=0; i<len; i++) {
		       	if(s._s[i] != _s[i]) {
				return true;
                        }
                }
        }
        return false;
}

// Return true if string "this" and s is less or equal
bool
MyString::operator <= (const MyString &s)
{
  // Add implementation here
  	int len = s.slength(s._s);
        int len1 = slength(_s);
	int c=0;
	if(len1 <= len) {
		for(int i=0; i<len1; i++) {
			if(_s[i] <= s._s[i]) {
				return true;
			} else {
				return false;
			}
		}
	}
	if(len1 > len) {
                for(int i=0; i<len; i++) {
                        if(_s[i] < s._s[i]) {
                                return true;
                        } if (_s[i] == s._s[i]) {
				c++;
			}
			if (c == len) {
				return false;

			}
                }
        }
  	return false;
}

// Return true if string "this" is greater than s
bool
MyString::operator > (const MyString &s)
{
  // Add implementation here

  return false;

}

// Return character at position i.  Return '\0' if out of bounds.
char
MyString::operator [] (int i)
{
  int len = slength(_s);
  if(i > len-1) {
	  return '\0';
  }
  else {
	  return _s[i];
  }
  return ' ';
}

// Return C representation of string
const char *
MyString::cStr()
{
  // Add implementation here
  return _s;
}

// Get string length of this string.
int
MyString::length() const
{
  // Add implementation here
  return slength(_s);
}

// Destructor. Deallocate the space used by _s
MyString::~MyString()
{
  // Add implementation here
  delete [] _s;
}

// Concatanate two strings (non member method)
MyString operator + (const MyString &s1, const MyString &s2)
{
  // Add implementation here

  // Allocate memory for the concatenated string

  // Add s1

  // Add s2
	int len = s1.slength(s1._s) + s2.slength(s2._s);
	char *s = new char[len+1];
	for (int i = 0; i < s1.slength(s1._s); i++) {
		s[i] = s1._s[i];
	}
	int a = s1.slength(s1._s);
	for (int i = 0; i < s2.slength(s2._s); i++) {
		s[a + i] = s2._s[i];
	}
	s[len] = 0;
	MyString ms(s);
	return ms;
}

