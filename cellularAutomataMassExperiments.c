#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <Math.h>


#define COLS 128
#define ROWS 128

typedef enum{
    vegetation_1,
    vegetation_2, 
    vegetation_3,
    initial_fire, 
    stable_fire, 
    ember, 
    water,  
    ash
}CellState;

typedef enum{
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW
} Direction;

/* ------- Parameters ------- */

double initFireParam = 0.6;
double stableFireParam = 1.0;
double emberFireParam = 0.2;

double humidity = 0.2; //gamma

double windIntensity = 0; //delta
double baseFireIntesity = 0.38; //beta
Direction windDirection = SE;

double calorie[3] = {0.24,0.16,0.08};
int alpha = 8;

/* -------------------------- */

CellState grid[COLS][ROWS];
CellState buffer[COLS][ROWS];
CellState initialStates[COLS][ROWS];
int ticks[COLS][ROWS];
double combustionMatrix[3][3];
double rMatrix[3][3];
bool paused = true;
bool HUD = true;
bool started = false;

void setWindMatrix(){
    double arrayOfProbabilities[8];
    double shift;
    for(int i = 0; i < 8; i++){
        shift = ((i + windDirection + 1) % 8);
        arrayOfProbabilities[i] = fabs((8 - shift*2) / 8);
        if(arrayOfProbabilities[i] == 0) arrayOfProbabilities[i] = 7/8;
    }
     rMatrix[0][0] = arrayOfProbabilities[0];
     rMatrix[0][1] = arrayOfProbabilities[1];
     rMatrix[0][2] = arrayOfProbabilities[2];
     rMatrix[1][2] = arrayOfProbabilities[3];
     rMatrix[2][2] = arrayOfProbabilities[4];
     rMatrix[2][1] = arrayOfProbabilities[5];
     rMatrix[2][0] = arrayOfProbabilities[6];
     rMatrix[1][0] = arrayOfProbabilities[7];
     rMatrix[1][1] = 0;
}

double CalculateHumidityFactorProbability(){
    if(humidity > 0 && humidity <= 0.25)
        return 1.5;
    else if(humidity > 0.25 && humidity <= 0.5)
        return 1;
    else if(humidity > 0.5 && humidity <= 0.75)
        return 0.8;
    else if(humidity > 0.75 && humidity <= 1)
        return 0.6;
    return 0;
}

void SetProbabilities(){
    for (int dx = 0; dx <= 2; dx++) {
        for (int dy = 0; dy <= 2; dy++){
            if (dx == 1 && dy == 1) {
                combustionMatrix[dy][dx] = 0.0f;
                continue;
            } 
            double sigma = CalculateHumidityFactorProbability();
            double phi = (baseFireIntesity - (windIntensity * rMatrix[dx][dy])) * sigma;
            if (phi > 1) phi = 1;
            if(phi < 0) phi = 0;

            combustionMatrix[dx][dy] = phi;
        }
    }
}

void InitGrid(int type){
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if(x == COLS/2 && y == ROWS/2) {grid[x][y] = initial_fire; buffer[x][y] = initial_fire;}
            else{
                if(type == 0){
                    grid[x][y] = vegetation_1;
                    buffer[x][y] = vegetation_1;
                }
                else if(type == 1){
                    grid[x][y] = vegetation_2;
                    buffer[x][y] = vegetation_2;
                }
                    
                else if(type == 2){
                    grid[x][y] = vegetation_3;
                    buffer[x][y] = vegetation_3;
                }
                    
                ticks[x][y] = 0;
            }
        }
    }
}

void SaveInitialPreset(){
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if(grid[x][y] == initial_fire) initialStates[x][y] = vegetation_1;
            else{
                initialStates[x][y] = grid[x][y];
            }
        }
    }
}

void spreadFire(int x, int y){
    int idleTime = 100;
    double rnd = (double)rand() / (double)RAND_MAX;
    int maxTicksEmber = 10;
    int maxTicksInitialFire = 3;
    int maxTicksStableFire = 3;
    switch(grid[x][y]){
        case vegetation_1:
            for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                int nx = (x + dx);
                int ny = (y + dy);

                if(nx < 0 || nx >= COLS) continue;
                if(ny < 0 || ny >= ROWS) continue;

                if((grid[nx][ny] == initial_fire) && (rnd <= (combustionMatrix[dx+1][dy+1] * initFireParam * calorie[0])))
                    buffer[x][y] = initial_fire;
                else if((grid[nx][ny] == stable_fire) && (rnd <= (combustionMatrix[dx+1][dy+1] * stableFireParam * calorie[0])))
                    buffer[x][y] = initial_fire;
                else if((grid[nx][ny] == ember) && (rnd <= (combustionMatrix[dx+1][dy+1] * emberFireParam * calorie[0])))
                    buffer[x][y] = initial_fire;
                }
            }
        break;
        case vegetation_2:
            for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                int nx = (x + dx);
                int ny = (y + dy);

                if(nx < 0 || nx >= COLS) continue;
                if(ny < 0 || ny >= ROWS) continue;

                if((grid[nx][ny] == initial_fire) && (rnd <= (combustionMatrix[dx+1][dy+1] * initFireParam * calorie[1])))
                    buffer[x][y] = initial_fire;
                else if((grid[nx][ny] == stable_fire) && (rnd <= (combustionMatrix[dx+1][dy+1] * stableFireParam * calorie[1])))
                    buffer[x][y] = initial_fire;
                else if((grid[nx][ny] == ember) && (rnd <= (combustionMatrix[dx+1][dy+1] * emberFireParam * calorie[1])))
                    buffer[x][y] = initial_fire;
                }
            }
        break;
        case vegetation_3:
            for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                int nx = (x + dx);
                int ny = (y + dy);

                if(nx < 0 || nx >= COLS) continue;
                if(ny < 0 || ny >= ROWS) continue;

                if((grid[nx][ny] == initial_fire) && (rnd <= (combustionMatrix[dx+1][dy+1] * initFireParam * calorie[2])))
                    buffer[x][y] = initial_fire;
                else if((grid[nx][ny] == stable_fire) && (rnd <= (combustionMatrix[dx+1][dy+1] * stableFireParam * calorie[2])))
                    buffer[x][y] = initial_fire;
                else if((grid[nx][ny] == ember) && (rnd <= (combustionMatrix[dx+1][dy+1] * emberFireParam * calorie[2])))
                    buffer[x][y] = initial_fire;
                }
            }
        break;
        case initial_fire:
            if(humidity > 0.3){
                if(ticks[x][y] > 2){
                    ticks[x][y] = 0;
                    buffer[x][y] = stable_fire;
                }
                else ticks[x][y]++;
            }
            else{
                if(ticks[x][y] > maxTicksInitialFire){
                    ticks[x][y] = 0;
                    buffer[x][y] = stable_fire;
                }
                else ticks[x][y]++;
            }
        break;
        case stable_fire:
            if(humidity <= 0.3){
                if(ticks[x][y] > maxTicksStableFire*5){
                    ticks[x][y] = 0;
                    buffer[x][y] = ember;
                }
                else ticks[x][y]++;
            }
            else{
                if(ticks[x][y] > 4){
                    ticks[x][y] = 0;
                    buffer[x][y] = ember;
                }
                else ticks[x][y]++;
            }
        break;
        case ember:
            if(humidity <= 0.3){
                if(ticks[x][y] > maxTicksEmber/3){
                    ticks[x][y] = 0;
                    buffer[x][y] = ash;
                }
                else ticks[x][y]++;
            }
            else{
                if(ticks[x][y] > maxTicksEmber){
                    ticks[x][y] = 0;
                    buffer[x][y] = ash;
                }
                else ticks[x][y]++;
            }
        break;
        case ash:
            rnd = (double)rand() / (double)RAND_MAX;
            if(rnd <= (pow(ticks[x][y]-idleTime,2))/pow(10,alpha) && (ticks[x][y] >= idleTime)){
                ticks[x][y] = 0;
                //buffer[x][y] = initialStates[x][y];
            }
            ticks[x][y]++;
        break;
        case water:
            buffer[x][y] = grid[x][y]; break;
        default: buffer[x][y] = grid[x][y];
    }
}

void UpdateGrid() {
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            spreadFire(x,y);
        }
    }

    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            grid[x][y] = buffer[x][y];
        }
    }
}

double countBurnedCells(){
    int count = 0;
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if(grid[x][y] == initial_fire || grid[x][y] == stable_fire || grid[x][y] == ember || grid[x][y] == ash){
                count++;
            }
        }
    }
    return (double)count;
}

int main() {
    setWindMatrix();
    SetProbabilities();
    int test = 0;
    double actualBurtCellQtd = 0;
    double lastBurtCellQtd = 0;
    double sumExperiment1[500] = {0};
    double sumExperiment2[500] = {0};
    double sumExperiment3[500] = {0};
    double experimentMean[500];
    double experimentMean1[500];
    double experimentMean2[500];
    double experimentMean3[500];

    InitGrid(0);
    clock_t start = clock();
    for(int test = 0; test < 100; test++){
        InitGrid(0);
        for(int ts = 0; ts < 500; ts++){
            UpdateGrid();
            sumExperiment1[ts] += countBurnedCells();
        }
    }

    for(int test = 0; test < 100; test++){
        InitGrid(1);
        for(int ts = 0; ts < 500; ts++){
            UpdateGrid();
            sumExperiment2[ts] += countBurnedCells();
        }
    }

    for(int test = 0; test < 100; test++){
        InitGrid(2);
        for(int ts = 0; ts < 500; ts++){
            UpdateGrid();
            sumExperiment3[ts] += countBurnedCells();
        }
    }
    clock_t end = clock();

    for(int i = 0; i < 500; i++){
        double mean = sumExperiment1[i]/100;
        double mean2 = sumExperiment2[i]/100;
        double mean3 = sumExperiment3[i]/100;

        experimentMean1[i] = mean/(COLS*ROWS);
        experimentMean2[i] = mean2/(COLS*ROWS);
        experimentMean3[i] = mean3/(COLS*ROWS);
    }


    FILE *file = fopen("output.csv", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    fprintf(file, "index,veg1,veg2,veg3\n");

    for (int i = 0; i < 500; i++) {
        fprintf(file, "%d,%f,%f,%f\n", i, experimentMean1[i], experimentMean2[i], experimentMean3[i]);
    }

    fclose(file);

    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    printf("Tempo: %f\n", seconds);
    return 0;
}