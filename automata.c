#include "automata.h"
#include <stdio.h>
#include <Math.h>
#include <stdlib.h>

Cell grid[COLS][ROWS];
double combustionMatrix[3][3];
double rMatrix[3][3];

/* ------- Parameters ------- */

double initFireParam = 0.6;
double stableFireParam = 1.0;
double emberFireParam = 0.2;

double humidity = 0.3;          // gamma
double windIntensity = 0;       // delta
double baseFireIntesity = 0.55;    // beta
Direction windDirection = SW;

double slopeCoeficient = 0.078; // alfa
double distanceBetweenCells = 8;

double calorie[3] = {0.24, 0.16, 0.08};

int idleTime = 0;
int alpha = 6;

/* -------------------------- */


void setWindMatrix()
{
    double arrayOfProbabilities[8];
    double shift;
    for (int i = 0; i < 8; i++)
    {
        shift = ((i + windDirection + 1) % 8);
        arrayOfProbabilities[i] = fabs((8.0 - shift * 2.0) / 8.0);
        if (arrayOfProbabilities[i] == 0.0)
            arrayOfProbabilities[i] = 7.0 / 8.0;
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

double CalculateHumidityFactorProbability()
{
    if (humidity > 0 && humidity <= 0.25)
        return 1.5;
    else if (humidity > 0.25 && humidity <= 0.5)
        return 1;
    else if (humidity > 0.5 && humidity <= 0.75)
        return 0.8;
    else if (humidity > 0.75 && humidity <= 1)
        return 0.6;
    return 0;
}

void SetProbabilities()
{
    for (int dx = 0; dx <= 2; dx++)
    {
        for (int dy = 0; dy <= 2; dy++)
        {
            if (dx == 1 && dy == 1)
            {
                combustionMatrix[dy][dx] = 0.0;
                continue;
            }
            double sigma = CalculateHumidityFactorProbability();
            double phi = (baseFireIntesity - (windIntensity * rMatrix[dx][dy])) * sigma;

            combustionMatrix[dx][dy] = phi;
        }
    }
}

void InitGrid(int type)
{
    resetHistory();
    int div = 128;
    int frac = COLS / div;
    int altitude = 1;

    for (int x = 0; x < COLS; x++)
    {
        for (int y = 0; y < ROWS; y++)
        {
            int zone = x / frac;
            if (x == COLS / 2 && y == ROWS / 2)
            {
                grid[x][y].state = initial_fire;
                grid[x][y].buffer = initial_fire;
                grid[x][y].altitude = (type > 5) ? 48 : 100;
                //grid[x][y].state = initial_fire;
                //grid[x][y].buffer = initial_fire;
                //grid[nx][ny].altitude = (type > 5) ? 48 : 100;
                continue;
            }
            else
            {
                switch (type)
                {
                case 0:
                    grid[x][y].state = vegetation_1;
                    grid[x][y].buffer = vegetation_1;
                    grid[x][y].altitude = 100;
                    break;
                case 1:
                    grid[x][y].state = vegetation_2;
                    grid[x][y].buffer = vegetation_2;
                    grid[x][y].altitude = 100;
                    break;
                case 2:
                    grid[x][y].state = vegetation_3;
                    grid[x][y].buffer = vegetation_3;
                    grid[x][y].altitude = 100;
                    break;
                case 3:
                    if (x > COLS / 2)
                    {
                        grid[x][y].state = vegetation_1;
                        grid[x][y].buffer = vegetation_1;
                    }
                    else
                    {
                        grid[x][y].state = vegetation_2;
                        grid[x][y].buffer = vegetation_2;
                    }
                    grid[x][y].altitude = 100;
                    break;
                case 4:
                    if (x > COLS / 2)
                    {
                        grid[x][y].state = vegetation_1;
                        grid[x][y].buffer = vegetation_1;
                    }
                    else
                    {
                        grid[x][y].state = vegetation_3;
                        grid[x][y].buffer = vegetation_3;
                    }
                    grid[x][y].altitude = 100;
                    break;
                case 5:
                    if (x > COLS / 2)
                    {
                        grid[x][y].state = vegetation_2;
                        grid[x][y].buffer = vegetation_2;
                    }
                    else
                    {
                        grid[x][y].state = vegetation_3;
                        grid[x][y].buffer = vegetation_3;
                    }
                    grid[x][y].altitude = 100;
                    break;
                case 6:
                    grid[x][y].state = vegetation_1;
                    grid[x][y].buffer = vegetation_1;
                    if (zone >= 0 && zone < div)
                    {
                        grid[x][y].altitude = (zone + 1) * altitude;
                    }
                    break;
                case 7:
                    grid[x][y].state = vegetation_2;
                    grid[x][y].buffer = vegetation_2;
                    if (zone >= 0 && zone < div)
                    {
                        grid[x][y].altitude = (zone + 1) * altitude;
                    }
                    break;
                case 8:
                    grid[x][y].state = vegetation_3;
                    grid[x][y].buffer = vegetation_3;
                    if (zone >= 0 && zone < div)
                    {
                        grid[x][y].altitude = (zone + 1) * altitude;
                    }
                    break;
                }

                grid[x][y].ticks = 0;
            }
        }
    }
}

void SaveInitialPreset()
{
    for (int x = 0; x < COLS; x++)
    {
        for (int y = 0; y < ROWS; y++)
        {
            if (grid[x][y].state == initial_fire)
                grid[x][y].initialState = vegetation_1;
            else
            {
                grid[x][y].initialState = grid[x][y].state;
            }
        }
    }
}

void spreadFire(int x, int y)
{
    unsigned int r = ((unsigned int)rand() << 15) | (unsigned int)rand();
    double rnd = r / (double)((1u << 30) - 1u);
    double elevation;
    int maxTicksEmber = 10;
    int maxTicksInitialFire = 3;
    int maxTicksStableFire = 3;
    switch (grid[x][y].state)
    {
    case vegetation_1:
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                if (dx == 0 && dy == 0)
                    continue;
                int nx = (x + dx);
                int ny = (y + dy);

                if (nx < 0 || nx >= COLS)
                    continue;
                if (ny < 0 || ny >= ROWS)
                    continue;

                if (abs(dx) == 1 && abs(dy) == 1)
                    elevation = exp(slopeCoeficient * atan((grid[x][y].altitude - grid[nx][ny].altitude) / (distanceBetweenCells * sqrt(2))));
                else
                    elevation = exp(slopeCoeficient * atan((grid[x][y].altitude - grid[nx][ny].altitude) / distanceBetweenCells));

                if ((grid[nx][ny].state == initial_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * initFireParam * calorie[0] * elevation))){
                    grid[x][y].buffer = initial_fire;
                    grid[x][y].burnHistory = 1;
                }
                    
                else if ((grid[nx][ny].state == stable_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * stableFireParam * calorie[0] * elevation))){
                    grid[x][y].burnHistory = 1;
                    grid[x][y].buffer = initial_fire;
                }
                    
                else if ((grid[nx][ny].state == ember) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * emberFireParam * calorie[0] * elevation))){
                    grid[x][y].buffer = initial_fire;
                    grid[x][y].burnHistory = 1;
                }
            }
        }
        break;
    case vegetation_2:
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                if (dx == 0 && dy == 0)
                    continue;
                int nx = (x + dx);
                int ny = (y + dy);

                if (nx < 0 || nx >= COLS)
                    continue;
                if (ny < 0 || ny >= ROWS)
                    continue;

                if (abs(dx) == 1 && abs(dy) == 1)
                    elevation = exp(slopeCoeficient * atan((grid[x][y].altitude - grid[nx][ny].altitude) / (distanceBetweenCells * sqrt(2))));
                else
                    elevation = exp(slopeCoeficient * atan((grid[x][y].altitude - grid[nx][ny].altitude) / distanceBetweenCells));

                if ((grid[nx][ny].state == initial_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * initFireParam * calorie[1] * elevation))){
                    grid[x][y].buffer = initial_fire;
                    grid[x][y].burnHistory = 1;
                }
                    
                else if ((grid[nx][ny].state == stable_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * stableFireParam * calorie[1] * elevation))){
                    grid[x][y].buffer = initial_fire;
                    grid[x][y].burnHistory = 1;
                }
                    
                else if ((grid[nx][ny].state == ember) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * emberFireParam * calorie[1] * elevation))){
                    grid[x][y].buffer = initial_fire;
                    grid[x][y].burnHistory = 1;
                }
            }
        }
        break;
    case vegetation_3:
        for (int dx = -1; dx <= 1; dx++)
        {
            for (int dy = -1; dy <= 1; dy++)
            {
                if (dx == 0 && dy == 0)
                    continue;
                int nx = (x + dx);
                int ny = (y + dy);

                if (nx < 0 || nx >= COLS)
                    continue;
                if (ny < 0 || ny >= ROWS)
                    continue;

                if (abs(dx) == 1 && abs(dy) == 1)
                    elevation = exp(slopeCoeficient * atan((grid[x][y].altitude - grid[nx][ny].altitude) / (distanceBetweenCells * sqrt(2))));
                else
                    elevation = exp(slopeCoeficient * atan((grid[x][y].altitude - grid[nx][ny].altitude) / distanceBetweenCells));

                if ((grid[nx][ny].state == initial_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * initFireParam * calorie[2] * elevation))){
                    grid[x][y].buffer = initial_fire;
                    grid[x][y].burnHistory = 1;
                }
                else if ((grid[nx][ny].state == stable_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * stableFireParam * calorie[2] * elevation)))
                    grid[x][y].buffer = initial_fire;
                else if ((grid[nx][ny].state == ember) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * emberFireParam * calorie[2] * elevation)))
                    grid[x][y].buffer = initial_fire;
            }
        }
        break;
    case initial_fire:
        if (humidity > 0.3)
        {
            if (grid[x][y].ticks > 2)
            {
                grid[x][y].ticks = 0;
                grid[x][y].buffer = stable_fire;
            }
            else
                grid[x][y].ticks++;
        }
        else
        {
            if (grid[x][y].ticks > maxTicksInitialFire)
            {
                grid[x][y].ticks = 0;
                grid[x][y].buffer = stable_fire;
            }
            else
                grid[x][y].ticks++;
        }
        break;
    case stable_fire:
        if (humidity <= 0.3)
        {
            if (grid[x][y].ticks > maxTicksStableFire * 5)
            {
                grid[x][y].ticks = 0;
                grid[x][y].buffer = ember;
            }
            else
                grid[x][y].ticks++;
        }
        else
        {
            if (grid[x][y].ticks > 4)
            {
                grid[x][y].ticks = 0;
                grid[x][y].buffer = ember;
            }
            else
                grid[x][y].ticks++;
        }
        break;
    case ember:
        if (humidity <= 0.3)
        {
            if (grid[x][y].ticks > maxTicksEmber / 3)
            {
                grid[x][y].ticks = 0;
                grid[x][y].buffer = ash;
            }
            else
                grid[x][y].ticks++;
        }
        else
        {
            if (grid[x][y].ticks > maxTicksEmber)
            {
                grid[x][y].ticks = 0;
                grid[x][y].buffer = ash;
            }
            else
                grid[x][y].ticks++;
        }
        break;
    case ash:
        double prob = (double)pow(grid[x][y].ticks - idleTime, 2) / (double)pow(10, alpha);
        if ((rnd <= prob) && (grid[x][y].ticks >= idleTime))
        {
            grid[x][y].ticks = 0;
            grid[x][y].buffer = grid[x][y].initialState;
        }
        else grid[x][y].ticks++;
        break;
    case water:
        grid[x][y].buffer = grid[x][y].state;
        break;
    default:
        grid[x][y].buffer = grid[x][y].state;
    }
}

void UpdateGrid()
{
    for (int x = 0; x < COLS; x++)
    {
        for (int y = 0; y < ROWS; y++)
        {
            spreadFire(x, y);
        }
    }

    for (int x = 0; x < COLS; x++)
    {
        for (int y = 0; y < ROWS; y++)
        {
            grid[x][y].state = grid[x][y].buffer;
        }
    }
}

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

double countBurnedCellsEachSide(double *left, double *right)
{
    double count = 0;
    for (int x = 0; x < COLS; x++)
    {
        for (int y = 0; y < ROWS; y++)
        {
            if (x < COLS / 2)
            {
                if (grid[x][y].burnHistory == 1)
                {
                    (*left)++;
                }
            }
            else
            {
                if (grid[x][y].burnHistory == 1)
                {
                    (*right)++;
                }
            }
        }
    }
    return count;
}

void resetHistory(){
    for(int x = 0; x < COLS; x++){
        for(int y = 0; y < ROWS; y++)
            if (x == COLS / 2 && y == ROWS / 2) grid[x][y].burnHistory = 1;
            else grid[x][y].burnHistory = 0;
    }
}