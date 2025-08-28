#ifndef AUTOMATA_H
#define AUTOMATA_H

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024
#define CELL_SIZE 8

#define COLS (SCREEN_WIDTH / CELL_SIZE)
#define ROWS (SCREEN_HEIGHT / CELL_SIZE)
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

typedef struct{
    CellState state;
    CellState buffer;
    CellState initialState;
    int ticks;
    double altitude;
    int burnHistory;
}Cell;

/* ------- Parameters ------- */

extern double initFireParam;
extern double stableFireParam;
extern double emberFireParam;

extern double humidity;         // gamma
extern double windIntensity;      // delta
extern double baseFireIntesity;   // beta
extern Direction windDirection;

extern double slopeCoeficient;
extern double distanceBetweenCells;

extern double calorie[3];

extern int idleTime;
extern int alpha;

/* -------------------------- */
extern Cell grid[COLS][ROWS];
void setWindMatrix();
double CalculateHumidityFactorProbability();
void SetProbabilities();
void InitGrid(int type);
void SaveInitialPreset();
void spreadFire(int x, int y);
void UpdateGrid();

#endif