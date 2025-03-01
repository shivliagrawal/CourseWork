all: mytest

problem=mysort.c

mytest: mytest.c $(problem)
	gcc -std=gnu99 -g -o mytest mytest.c $(problem)

clean:
	rm -f *.o *.out *.txt mytest none
