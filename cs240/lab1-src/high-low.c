#include <stdio.h>
//

int
main(int argc, char **argv) {
  printf("Welcome to the High Low game...\n");
  while (1) {
  printf("Think of a number between 1 and 100 and press <enter>\n");
	  int low=1;
	  int high=100;
	  int mid=50;
          while (high >= low) {
                  printf("Is it higher than %d? (y/n)\n", mid);
                  char ans;
                  scanf(" %c",&ans);
                  if (ans=='y') {
	                  low=mid+1;
	          }
                  else if (ans=='n') {
	                  high=mid-1;
	          }
		  else {
			  printf("Type y or n\n");
		  }
		  mid=(high+low)/2;
          }
  
          printf("\n>>>>>> The number is %d\n", mid+1);
	  printf("\nDo you want to continue playing (y/n)?");
	  char ans2;
	  scanf(" %c",&ans2);
	  if (ans2=='y') {
		  continue;
	  }
	  else if (ans2=='n') {
		  printf("Thanks for playing!!!\n");
		  break;
	  }
	  else {
		  printf("Type y or n\n");
	  }
  }
}

