#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <Math.h>
#include "automata.h"

int main()
{
    setWindMatrix();
    SetProbabilities();

    InitGrid(0);
    clock_t start = clock();

    /*
    double sumExperiments[3][MAXTS];
    double resultExperiments[3][MAXTS];
    
    for(int i = 0; i < 3; i++){
        for(int k = 0; k < MAXTS; k++)
            sumExperiments[i][k] = 0.0;
        for (int test = 0; test < MAXTESTS; test++)
        {
            InitGrid(i);
            for (int ts = 0; ts < MAXTS; ts++){
                UpdateGrid();
                if(countBurnedCells() >= (COLS*ROWS)) printf("%f",countBurnedCells());
                sumExperiments[i][ts] += countBurnedCells();
            }
        }
        for (int j = 0; j < MAXTS; j++)
        {
            double mean = sumExperiments[i][j]/MAXTESTS;
            resultExperiments[i][j] = mean/(COLS*ROWS);
        }
    }

    char filename[30];
    sprintf(filename, "%s_%d_.csv", "output", 0);
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    fprintf(file, "index,veg1,veg2,veg3\n");

    for (int i = 0; i < MAXTS; i++)
    {
        fprintf(file, "%d,%f,%f,%f\n", i, resultExperiments[0][i], resultExperiments[1][i], resultExperiments[2][i]);
    }

    fclose(file);
    */

    double sumExperimentsLeft[3][MAXTS];
    double resultExperimentsLeft[3][MAXTS];
    double sumExperimentsRight[3][MAXTS];
    double resultExperimentsRight[3][MAXTS];

    for(int i = 0; i < 3; i++){
        for(int k = 0; k < MAXTS; k++){
            sumExperimentsLeft[i][k] = 0.0;
            sumExperimentsRight[i][k] = 0.0;
        }
            
        for (int test = 0; test < MAXTESTS; test++)
        {
            InitGrid(i+6);
            for (int ts = 0; ts < MAXTS; ts++){
                UpdateGrid();
                double left = 0, right = 0; 
                countBurnedCellsEachSide(&left,&right);
                sumExperimentsLeft[i][ts] += left;
                sumExperimentsRight[i][ts] += right;
            }
        }
        for (int j = 0; j < MAXTS; j++)
        {
            double meanL = sumExperimentsLeft[i][j]/MAXTESTS;
            double meanR = sumExperimentsRight[i][j]/MAXTESTS;
            resultExperimentsLeft[i][j] = meanL/(COLS*ROWS);
            resultExperimentsRight[i][j] = meanR/(COLS*ROWS);
        }
    }

    char filenameA[30];
    sprintf(filenameA, "%s_%d_.csv", "outputAlpha", 0);
    FILE *fileA = fopen(filenameA, "w");
    if (fileA == NULL) {
        perror("Error opening file");
        return 1;
    }

    fprintf(fileA, "index,Aveg1L,Aveg1R,Aveg2L,Aveg2R,Aveg3L,Aveg3R\n");

    for (int i = 0; i < 500; i++) {
        fprintf(fileA, "%d,%f,%f,%f,%f,%f,%f\n", i, resultExperimentsLeft[0][i], resultExperimentsRight[0][i], resultExperimentsLeft[1][i],resultExperimentsRight[1][i],resultExperimentsLeft[2][i],resultExperimentsRight[2][i]);
    }
    fclose(fileA);

    clock_t end = clock();

    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    printf("Tempo: %f\n", seconds);
    return 0;
}