#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <Math.h>

#define COLS 128
#define ROWS 128
#define MAXTS 500
#define MAXTESTS 100

typedef enum
{
    vegetation_1,
    vegetation_2,
    vegetation_3,
    initial_fire,
    stable_fire,
    ember,
    water,
    ash
} CellState;

typedef enum
{
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

double humidity = 0.8;          // gamma
double windIntensity = 0;       // delta
double baseFireIntesity = 1; // beta
Direction windDirection = N;

double slopeCoeficient = 0; // alfa
double distanceBetweenCells = 8;

double calorie[3] = {0.24, 0.16, 0.08};

int idleTime = 0;
int alpha = 6;

/* -------------------------- */

CellState grid[COLS][ROWS];
CellState buffer[COLS][ROWS];
CellState initialStates[COLS][ROWS];
int ticks[COLS][ROWS];
double altitudes[COLS][ROWS];
double combustionMatrix[3][3];
double rMatrix[3][3];
double burnHistory[COLS][ROWS];

void setWindMatrix()
{
    double arrayOfProbabilities[8];
    double shift;
    for (int i = 0; i < 8; i++)
    {
        shift = ((i + windDirection + 1) % 8);
        arrayOfProbabilities[i] = fabs((8 - shift * 2) / 8);
        if (arrayOfProbabilities[i] == 0)
            arrayOfProbabilities[i] = 7 / 8;
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
                combustionMatrix[dy][dx] = 0.0f;
                continue;
            }
            double phi = (baseFireIntesity - (windIntensity * rMatrix[dx][dy]));
            if (phi < 0)
                phi = 0;

            combustionMatrix[dx][dy] = phi;
        }
    }
}

void InitGrid(int type)
{
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
                grid[x][y] = initial_fire;
                buffer[x][y] = initial_fire;
                altitudes[x][y] = (type > 5) ? 48 : 100;
                continue;
            }
            else
            {
                switch (type)
                {
                case 0:
                    grid[x][y] = vegetation_1;
                    buffer[x][y] = vegetation_1;
                    altitudes[x][y] = 100;
                    break;
                case 1:
                    grid[x][y] = vegetation_2;
                    buffer[x][y] = vegetation_2;
                    altitudes[x][y] = 100;
                    break;
                case 2:
                    grid[x][y] = vegetation_3;
                    buffer[x][y] = vegetation_3;
                    altitudes[x][y] = 100;
                    break;
                case 3:
                    if (x > COLS / 2)
                    {
                        grid[x][y] = vegetation_1;
                        buffer[x][y] = vegetation_1;
                    }
                    else
                    {
                        grid[x][y] = vegetation_2;
                        buffer[x][y] = vegetation_2;
                    }
                    altitudes[x][y] = 100;
                    break;
                case 4:
                    if (x > COLS / 2)
                    {
                        grid[x][y] = vegetation_1;
                        buffer[x][y] = vegetation_1;
                    }
                    else
                    {
                        grid[x][y] = vegetation_3;
                        buffer[x][y] = vegetation_3;
                    }
                    altitudes[x][y] = 100;
                    break;
                case 5:
                    if (x > COLS / 2)
                    {
                        grid[x][y] = vegetation_2;
                        buffer[x][y] = vegetation_2;
                    }
                    else
                    {
                        grid[x][y] = vegetation_3;
                        buffer[x][y] = vegetation_3;
                    }
                    altitudes[x][y] = 100;
                    break;
                case 6:
                    grid[x][y] = vegetation_1;
                    buffer[x][y] = vegetation_1;
                    if (zone >= 0 && zone < div)
                    {
                        altitudes[x][y] = (zone + 1) * altitude;
                    }
                    break;
                case 7:
                    grid[x][y] = vegetation_2;
                    buffer[x][y] = vegetation_2;
                    if (zone >= 0 && zone < div)
                    {
                        altitudes[x][y] = (zone + 1) * altitude;
                    }
                    break;
                case 8:
                    grid[x][y] = vegetation_3;
                    buffer[x][y] = vegetation_3;
                    if (zone >= 0 && zone < div)
                    {
                        altitudes[x][y] = (zone + 1) * altitude;
                    }
                    break;
                }

                ticks[x][y] = 0;
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
            if (grid[x][y] == initial_fire)
                initialStates[x][y] = vegetation_1;
            else
            {
                initialStates[x][y] = grid[x][y];
            }
        }
    }
}

void spreadFire(int x, int y)
{
    double sigma = CalculateHumidityFactorProbability();
    double rnd = ((rand() << 15) | rand()) / (double)((1 << 30) - 1);
    double elevation;
    int maxTicksEmber = 10;
    int maxTicksInitialFire = 3;
    int maxTicksStableFire = 3;
    switch (grid[x][y])
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

                if ((dx == dy))
                    elevation = exp(slopeCoeficient * atan((altitudes[nx][ny] - altitudes[x][y]) / (distanceBetweenCells * sqrt(2))));
                else
                    elevation = exp(slopeCoeficient * atan((altitudes[nx][ny] - altitudes[x][y]) / distanceBetweenCells));

                if ((grid[nx][ny] == initial_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * initFireParam * calorie[0] * elevation * sigma))){
                    buffer[x][y] = initial_fire;
                    burnHistory[x][y] = 1;
                }
                    
                else if ((grid[nx][ny] == stable_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * stableFireParam * calorie[0] * elevation * sigma)))
                    buffer[x][y] = initial_fire;
                else if ((grid[nx][ny] == ember) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * emberFireParam * calorie[0] * elevation * sigma)))
                    buffer[x][y] = initial_fire;
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

                if ((dx == dy))
                    elevation = exp(slopeCoeficient * atan((altitudes[nx][ny] - altitudes[x][y]) / (distanceBetweenCells * sqrt(2))));
                else
                    elevation = exp(slopeCoeficient * atan((altitudes[nx][ny] - altitudes[x][y]) / distanceBetweenCells));

                if ((grid[nx][ny] == initial_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * initFireParam * calorie[1] * elevation * sigma))){
                    buffer[x][y] = initial_fire;
                    burnHistory[x][y] = 1;
                }
                    
                else if ((grid[nx][ny] == stable_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * stableFireParam * calorie[1] * elevation * sigma)))
                    buffer[x][y] = initial_fire;
                else if ((grid[nx][ny] == ember) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * emberFireParam * calorie[1] * elevation * sigma)))
                    buffer[x][y] = initial_fire;
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

                if ((dx == dy))
                    elevation = exp(slopeCoeficient * atan((altitudes[nx][ny] - altitudes[x][y]) / (distanceBetweenCells * sqrt(2))));
                else
                    elevation = exp(slopeCoeficient * atan((altitudes[nx][ny] - altitudes[x][y]) / distanceBetweenCells));

                if ((grid[nx][ny] == initial_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * initFireParam * calorie[2] * elevation * sigma))){
                    buffer[x][y] = initial_fire;
                    burnHistory[x][y] = 1;
                }
                else if ((grid[nx][ny] == stable_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * stableFireParam * calorie[2] * elevation * sigma)))
                    buffer[x][y] = initial_fire;
                else if ((grid[nx][ny] == ember) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * emberFireParam * calorie[2] * elevation * sigma)))
                    buffer[x][y] = initial_fire;
            }
        }
        break;
    case initial_fire:
        if (humidity > 0.3)
        {
            if (ticks[x][y] > 2)
            {
                ticks[x][y] = 0;
                buffer[x][y] = stable_fire;
            }
            else
                ticks[x][y]++;
        }
        else
        {
            if (ticks[x][y] > maxTicksInitialFire)
            {
                ticks[x][y] = 0;
                buffer[x][y] = stable_fire;
            }
            else
                ticks[x][y]++;
        }
        break;
    case stable_fire:
        if (humidity <= 0.3)
        {
            if (ticks[x][y] > maxTicksStableFire * 5)
            {
                ticks[x][y] = 0;
                buffer[x][y] = ember;
            }
            else
                ticks[x][y]++;
        }
        else
        {
            if (ticks[x][y] > 4)
            {
                ticks[x][y] = 0;
                buffer[x][y] = ember;
            }
            else
                ticks[x][y]++;
        }
        break;
    case ember:
        if (humidity <= 0.3)
        {
            if (ticks[x][y] > maxTicksEmber / 3)
            {
                ticks[x][y] = 0;
                buffer[x][y] = ash;
            }
            else
                ticks[x][y]++;
        }
        else
        {
            if (ticks[x][y] > maxTicksEmber)
            {
                ticks[x][y] = 0;
                buffer[x][y] = ash;
            }
            else
                ticks[x][y]++;
        }
        break;
    case ash:
        double prob = (double)pow(ticks[x][y] - idleTime, 2) / (double)pow(10, alpha);
        if ((rnd <= prob) && (ticks[x][y] >= idleTime))
        {
            ticks[x][y] = 0;
            buffer[x][y] = initialStates[x][y];
        }
        else ticks[x][y]++;
        break;
    case water:
        buffer[x][y] = grid[x][y];
        break;
    default:
        buffer[x][y] = grid[x][y];
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
            grid[x][y] = buffer[x][y];
        }
    }
}

double countBurnedCells()
{
    int count = 0;
    for (int x = 0; x < COLS; x++)
    {
        for (int y = 0; y < ROWS; y++)
        {
            if(burnHistory[x][y] == 1)
                count++;
        }
    }
    return (double)count;
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
                if (grid[x][y] == initial_fire || grid[x][y] == stable_fire || grid[x][y] == ember || grid[x][y] == ash)
                {
                    (*left)++;
                }
            }
            else
            {
                if (grid[x][y] == initial_fire || grid[x][y] == stable_fire || grid[x][y] == ember || grid[x][y] == ash)
                {
                    (*right)++;
                }
            }
        }
    }
    return (double)count;
}

void resetHistory(){
    for(int i = 0; i < COLS; i++){
        for(int j = 0; j < ROWS; j++)
            burnHistory[i][j] = 0;
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
                sumExperiments[i][ts] += countBurnedCells();
            }
            resetHistory();
        }
        for (int j = 0; j < MAXTS; j++)
        {
            double mean = sumExperiments[i][j]/100;
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