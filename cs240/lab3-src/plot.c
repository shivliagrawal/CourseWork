
#include <stdio.h>
#include <stdlib.h>

#include "rpn.h"

#define MAXCOLS 80
#define MAXROWS 40

char plot[MAXROWS][MAXCOLS];

void clearPlot()
{
  for (int i = 0; i < MAXROWS; i++) {
    for (int j = 0; j < MAXCOLS; j++) {
      plot[i][j] = ' ';
    }
  }
}

void printPlot()
{
  for (int i = 0; i < MAXROWS; i++) {
    for (int j = 0; j < MAXCOLS; j++) {
      printf("%c", plot[i][j]);
    }
    printf("\n");
  }
}

void plotXY(int x, int y, char c) {
  if ( x <0 || x>=MAXCOLS || y < 0 || y >=MAXROWS) return;
  plot[MAXROWS - y - 1][x]=c;
}

void createPlot( char * funcFile, double minX, double maxX) {
  int nvals = MAXCOLS;
  double yy[MAXCOLS];
  double xx[MAXCOLS];

  clearPlot();

    for (int i = 0; i < MAXCOLS; i++) {
        double x = minX + (maxX-minX) * i / MAXCOLS; // X-value
        double y = rpn_eval(funcFile,x); // Y-value
        xx[i] = x;
        yy[i] = y;
    }
    double minY = 100;
    double maxY = 0;

    for (int j = 0; j < MAXCOLS; j++) {
        if (yy[j] < minY) {
            minY = yy[j];
        }
        if (yy[j] > maxY) {
            maxY = yy[j];
        }
    }

  // Evaluate function and store in vector yy

  //Compute maximum and minimum y in vector yy
  
  //Plot x axis
  int u = (int) ((-1.0 * minY) * MAXROWS / (maxY - minY));
  for (int k = 0; k < MAXCOLS; k++) {
           plotXY(k, u, '_');
  }
  //Plot y axis
  double a1 = -1 * minX;
    double a2 = a1 * MAXCOLS;
    double a3 = a2 / (maxX - minX);
    int v = (int) a3;
    for (int i = 0; i < MAXROWS; i++) {
            plotXY(v, i, '|');
    }


  // Plot function. Use scaling.
  for (int i = 0; i < MAXCOLS; i++) {
        int x = i; // X plotted value
        int y = (int) ((yy[i] - minY) * MAXROWS / (maxY - minY)); // Y plotted value
        plotXY(x, y, '*');
  }

  // minX is plotted at column 0 and maxX is plotted ar MAXCOLS-1
  // minY is plotted at row 0 and maxY is plotted at MAXROWS-1

  printPlot();

}

int main(int argc, char ** argv)
{
  printf("RPN Plotter.\n");
  
  if (argc < 4) {
    printf("Usage: plot func-file xmin xmax\n");
    exit(1);
  }

  // Get arguments
  double xmin = atof(argv[2]);
  double xmax = atof(argv[3]);

  
  createPlot(argv[1], xmin, xmax);
}
