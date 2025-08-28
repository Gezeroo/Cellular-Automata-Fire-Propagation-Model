#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <Math.h>
#include "automata.h"

double countBurnedCells()
{
    double count = 0;
    for (int x = 0; x < COLS; x++)
    {
        for (int y = 0; y < ROWS; y++)
        {
            if(grid[x][y].burnHistory == 1)
                count++;
        }
    }
    return count;
}

double countBurnedCellsLeftRight(double *left, double *right)
{
    int count = 0;
    for (int x = 0; x < COLS; x++)
    {
        for (int y = 0; y < ROWS; y++)
        {
            if (x < COLS / 2)
            {
                if (grid[x][y].state == initial_fire || grid[x][y].state == stable_fire || grid[x][y].state == ember || grid[x][y].state == ash)
                {
                    (*left)++;
                }
            }
            else
            {
                if (grid[x][y].state == initial_fire || grid[x][y].state == stable_fire || grid[x][y].state == ember || grid[x][y].state == ash)
                {
                    (*right)++;
                }
            }
        }
    }
    return (double)count;
}

void resetHistory(){
    for(int x = 0; x < COLS; x++){
        for(int y = 0; y < ROWS; y++)
            if (x == COLS / 2 && y == ROWS / 2) grid[x][y].burnHistory = 1;
            else grid[x][y].burnHistory = 0;
    }
}

int main()
{
    setWindMatrix();
    SetProbabilities();

    InitGrid(0);
    clock_t start = clock();

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
            resetHistory();
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

/*
double alphas[5] = {0.078, 0.24, 0.5, 0.76,1};
for(int k = 0; k < 5; k++){
    double sumExperiment4Left[500] = {0};
    double sumExperiment4Right[500] = {0};
    double sumExperiment5Left[500] = {0};
    double sumExperiment5Right[500] = {0};
    double sumExperiment6Left[500] = {0};
    double sumExperiment6Right[500] = {0};

    slopeCoeficient = alphas[k];

    for(int test = 0; test < 100; test++){
        InitGrid(6);
        for(int ts = 0; ts < 500; ts++){
            UpdateGrid();
            countBurnedCellsLeftRight(&sumExperiment4Left[ts],&sumExperiment4Right[ts]);
        }
    }

    for(int test = 0; test < 100; test++){
        InitGrid(7);
        for(int ts = 0; ts < 500; ts++){
            UpdateGrid();
            countBurnedCellsLeftRight(&sumExperiment5Left[ts],&sumExperiment5Right[ts]);
        }
    }

    for(int test = 0; test < 100; test++){
        InitGrid(8);
        for(int ts = 0; ts < 500; ts++){
            UpdateGrid();
            countBurnedCellsLeftRight(&sumExperiment6Left[ts],&sumExperiment6Right[ts]);
        }
    }


    double experimentMean4L[500];
    double experimentMean4R[500];
    double experimentMean5L[500];
    double experimentMean5R[500];
    double experimentMean6L[500];
    double experimentMean6R[500];

    for(int i = 0; i < 500; i++){
        double mean4L = sumExperiment4Left[i]/100;
        double mean4R = sumExperiment4Right[i]/100;
        double mean5L = sumExperiment5Left[i]/100;
        double mean5R = sumExperiment5Right[i]/100;
        double mean6L = sumExperiment6Left[i]/100;
        double mean6R = sumExperiment6Right[i]/100;



        experimentMean4L[i] = mean4L/(COLS*ROWS);
        experimentMean4R[i] = mean4R/(COLS*ROWS);
        experimentMean5L[i] = mean5L/(COLS*ROWS);
        experimentMean5R[i] = mean5R/(COLS*ROWS);
        experimentMean6L[i] = mean6L/(COLS*ROWS);
        experimentMean6R[i] = mean6R/(COLS*ROWS);
    }

    char filename[30];
    sprintf(filename, "%s_%d_.csv", "outputAlpha", k);
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    fprintf(file, "index,Aveg1L,Aveg1R,Aveg2L,Aveg2R,Aveg3L,Aveg4L\n");

    for (int i = 0; i < 500; i++) {
        fprintf(file, "%d,%f,%f,%f,%f,%f,%f\n", i, experimentMean4L[i], experimentMean4R[i], experimentMean5L[i],experimentMean5R[i],experimentMean6L[i],experimentMean6R[i]);
    }

    fclose(file);


}
*/

clock_t end = clock();

float seconds = (float)(end - start) / CLOCKS_PER_SEC;
printf("Tempo: %f\n", seconds);
return 0;
}