
#include <stdio.h>
#include <stdlib.h>

// Return sum of the array
double sumArray (int n, double * array) {
        double sum = 0;

        double * p = array;
        double * pend = p+n;

        while (p < pend) {
            sum += *p;
            p++;
        }

        return sum;
}

// Return maximum element of array
double maxArray (int n, double * array) {
        double mx = 0;
        
        double *p = array;
        double *pend = p + n;

        mx = *p;
        p++;

        while (p < pend) {
            if (*p > mx) {
                mx = *p;
            }
            p++;
        }

        return mx;
}

// Return minimum element of array
double minArray (int n, double * array) {
        double mn = 0;
        
        double *p = array;
        double *pend = p + n;

        mn = *p;
        p++;

        while (p < pend) {
            if (*p < mn) {
                mn = *p;
            }
            p++;
        }

        return mn;
}

// Find the position int he array of the first element x 
// such that min<=x<=max or -1 if no element was found
int findArray (int n, double * array, double min, double max) {
        int index = 0;

        double *p = array;
        double *pend = p + n;

        while (p < pend) {
            if (*p >= min && *p <= max) {
                return index;
            }
            index++;
            p++;
        }

        return -1;
}


// Sort array without using [] operator. Use pointers 
// Hint: Use a pointer to the current and another to the next element
int sortArray (int n, double * array) {
        double *p = array;
        double t;
    
        for (int i = 0; i < n; i++) {
            for (int j = i + 1; j < n; j++) {
                if (*(p + j) < *(p + i)) { 
                    t = *(p + i);
                    *(p + i) = *(p + j);
                    *(p + j) = t;
                }
            }
        }
        return 0; 
}

// Print array
void printArray(int n, double * array) {
        double *p = array;
        double *pend = p + n;
        int i = 0;

        while (p < pend) {
            printf("%d:%f\n", i, *p);
            p++;
            i++;
        }
}
