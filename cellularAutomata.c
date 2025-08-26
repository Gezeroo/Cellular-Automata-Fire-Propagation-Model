#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <Math.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024
#define CELL_SIZE 8

#define COLS (SCREEN_WIDTH / CELL_SIZE)
#define ROWS (SCREEN_HEIGHT / CELL_SIZE)

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

double humidity = 0.3;          // gamma
double windIntensity = 0.25;      // delta
double baseFireIntesity = 0.55; // beta
Direction windDirection = W;

double slopeCoeficient = 0.78; // alfa_r
double distanceBetweenCells = 8;

double calorie[3] = {0.24, 0.16, 0.08};

int idleTime = 150;
int alpha = 8;


/* -------------------------- */

CellState grid[COLS][ROWS];
CellState buffer[COLS][ROWS];
CellState initialStates[COLS][ROWS];
int ticks[COLS][ROWS];
double altitudes[COLS][ROWS];
double combustionMatrix[3][3];
double rMatrix[3][3];
bool paused = true;
bool HUD = true;
bool started = false;

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
    if (humidity >= 0 && humidity <= 0.25)
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
            double sigma = CalculateHumidityFactorProbability();
            double phi = (baseFireIntesity - (windIntensity * rMatrix[dx][dy])) * sigma;
            if (phi > 1)
                phi = 1;
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
                case 9:
                    if (x > COLS / 2 - 1)
                    {
                        grid[x][y] = vegetation_1;
                        buffer[x][y] = vegetation_1;
                    }
                    else
                    {
                        grid[x][y] = vegetation_2;
                        buffer[x][y] = vegetation_2;
                    }
                    if (zone >= 0 && zone < div)
                    {
                        altitudes[x][y] = (zone + 1) * altitude;
                    }
                    break;
                case 10:
                    if (x > COLS / 2 - 1)
                    {
                        grid[x][y] = vegetation_2;
                        buffer[x][y] = vegetation_2;
                    }
                    else
                    {
                        grid[x][y] = vegetation_1;
                        buffer[x][y] = vegetation_1;
                    }
                    if (zone >= 0 && zone < div)
                    {
                        altitudes[x][y] = (zone + 1) * altitude;
                    }
                    break;
                case 11:
                    if (x > COLS / 2 - 1)
                    {
                        grid[x][y] = vegetation_1;
                        buffer[x][y] = vegetation_1;
                    }
                    else
                    {
                        grid[x][y] = vegetation_3;
                        buffer[x][y] = vegetation_3;
                    }
                    if (zone >= 0 && zone < div)
                    {
                        altitudes[x][y] = (zone + 1) * altitude;
                    }
                    break;
                case 12:
                    if (x > COLS / 2 - 1)
                    {
                        grid[x][y] = vegetation_3;
                        buffer[x][y] = vegetation_3;
                    }
                    else
                    {
                        grid[x][y] = vegetation_1;
                        buffer[x][y] = vegetation_1;
                    }
                    if (zone >= 0 && zone < div)
                    {
                        altitudes[x][y] = (zone + 1) * altitude;
                    }
                    break;
                case 13:
                    if (x > COLS / 2 - 1)
                    {
                        grid[x][y] = vegetation_2;
                        buffer[x][y] = vegetation_2;
                    }
                    else
                    {
                        grid[x][y] = vegetation_3;
                        buffer[x][y] = vegetation_3;
                    }
                    if (zone >= 0 && zone < div)
                    {
                        altitudes[x][y] = (zone + 1) * altitude;
                    }
                    break;
                case 14:
                    if (x > COLS / 2 - 1)
                    {
                        grid[x][y] = vegetation_3;
                        buffer[x][y] = vegetation_3;
                    }
                    else
                    {
                        grid[x][y] = vegetation_2;
                        buffer[x][y] = vegetation_2;
                    }
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
    double rnd = (double)rand() / (double)RAND_MAX;
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

                if ((grid[nx][ny] == initial_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * initFireParam * calorie[0] * elevation)))
                    buffer[x][y] = initial_fire;
                else if ((grid[nx][ny] == stable_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * stableFireParam * calorie[0] * elevation)))
                    buffer[x][y] = initial_fire;
                else if ((grid[nx][ny] == ember) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * emberFireParam * calorie[0] * elevation)))
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

                if ((grid[nx][ny] == initial_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * initFireParam * calorie[1] * elevation)))
                    buffer[x][y] = initial_fire;
                else if ((grid[nx][ny] == stable_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * stableFireParam * calorie[1] * elevation)))
                    buffer[x][y] = initial_fire;
                else if ((grid[nx][ny] == ember) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * emberFireParam * calorie[1] * elevation)))
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

                if ((grid[nx][ny] == initial_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * initFireParam * calorie[2] * elevation)))
                    buffer[x][y] = initial_fire;
                else if ((grid[nx][ny] == stable_fire) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * stableFireParam * calorie[2] * elevation)))
                    buffer[x][y] = initial_fire;
                else if ((grid[nx][ny] == ember) && (rnd <= (combustionMatrix[dx + 1][dy + 1] * emberFireParam * calorie[2] * elevation)))
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
        double prob = pow(ticks[x][y] - idleTime, 2) / (double)pow(10, alpha);
        if ((rnd <= prob) && (ticks[x][y] >= idleTime))
        {
            ticks[x][y] = 0;
            buffer[x][y] = initialStates[x][y];
        }
        ticks[x][y]++;
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

Color AdjustBrightness(Color base, float factor)
{
    int r = (int)(base.r * factor);
    int g = (int)(base.g * factor);
    int b = (int)(base.b * factor);

    if (r > 255)
        r = 255;
    if (r < 0)
        r = 0;
    if (g > 255)
        g = 255;
    if (g < 0)
        g = 0;
    if (b > 255)
        b = 255;
    if (b < 0)
        b = 0;

    return (Color){r, g, b, base.a};
}

void PaintGrid()
{
    int minAlt = altitudes[0][0];
    int maxAlt = altitudes[0][0];
    for (int x = 0; x < COLS; x++)
    {
        for (int y = 0; y < ROWS; y++)
        {
            if (altitudes[x][y] < minAlt)
                minAlt = altitudes[x][y];
            if (altitudes[x][y] > maxAlt)
                maxAlt = altitudes[x][y];
        }
    }

    for (int x = 0; x < COLS; x++)
    {
        float brightnessLevels[5] = {0.75f, 0.85f, 1.0f, 1.1f, 1.2f};
        for (int y = 0; y < ROWS; y++)
        {
            float norm = 1;
            if (maxAlt != minAlt)
            {
                norm = (float)(altitudes[x][y] - minAlt) / (maxAlt - minAlt);
            }

            int level = (int)(norm * 4 + 0.5f);
            if (level < 0)
                level = 0;
            if (level > 4)
                level = 4;
            float brightnessFactor = brightnessLevels[level];

            Color color;
            switch (grid[x][y])
            {
            case vegetation_1:
                color = (Color){156, 203, 102, 255};
                break;
            case vegetation_2:
                color = (Color){124, 177, 66, 255};
                break;
            case vegetation_3:
                color = (Color){50, 99, 30, 255};
                break;
            case water:
                color = BLUE;
                break;
            case initial_fire:
                color = ORANGE;
                break;
            case stable_fire:
                color = RED;
                break;
            case ember:
                color = MAROON;
                break;
            case ash:
                color = GRAY;
                break;
            }

            color = AdjustBrightness(color, brightnessFactor);
            DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);

            int px = x * CELL_SIZE;
            int py = y * CELL_SIZE;

            Vector2 center = {px + CELL_SIZE / 2.0f, py + CELL_SIZE / 2.0f};
            float offset = CELL_SIZE / 2.0f;

            int currentAltitude = altitudes[x][y];

            int dx[4] = {0, 0, -1, 1};
            int dy[4] = {-1, 1, 0, 0};

            for (int i = 0; i < 4; i++)
            {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS)
                {
                    int neighborAltitude = altitudes[nx][ny];

                    if (currentAltitude > neighborAltitude)
                    {
                        Vector2 start = {0}, end = {0};

                        if (dx[i] == 0 && dy[i] == -1)
                        {
                            start = (Vector2){px, py};
                            end = (Vector2){px + CELL_SIZE, py};
                        }
                        else if (dx[i] == 0 && dy[i] == 1)
                        {
                            start = (Vector2){px, py + CELL_SIZE - 1};
                            end = (Vector2){px + CELL_SIZE, py + CELL_SIZE - 1};
                        }
                        else if (dx[i] == -1 && dy[i] == 0)
                        {
                            start = (Vector2){px, py};
                            end = (Vector2){px, py + CELL_SIZE};
                        }
                        else if (dx[i] == 1 && dy[i] == 0)
                        {
                            start = (Vector2){px + CELL_SIZE - 1, py};
                            end = (Vector2){px + CELL_SIZE - 1, py + CELL_SIZE};
                        }

                        DrawLineEx(start, end, 1.5f, DARKGRAY);
                    }
                }
            }
        }
    }
}

float GetAngleFromDirection(int dir)
{
    return dir * 45.0f; // Each step is 45 degrees
}

int main()
{
    srand(77);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "AC - Raylib");
    SetTargetFPS(120);
    setWindMatrix();
    SetProbabilities();

    CellState brush = vegetation_1;
    int gridType = 0;

    InitGrid(gridType);

    Texture2D arrow = LoadTexture("sprites/windArrowBigArrow.png");
    Texture2D circle = LoadTexture("sprites/windArrowBigBorder.png");
    Vector2 center = {64, SCREEN_HEIGHT - circle.height / 2.0f};
    Vector2 origin = {circle.width / 2.0f, circle.height / 2.0f};
    float angle = GetAngleFromDirection(windDirection);

    int ts = 0;
    int altitude = 100;
    while (!WindowShouldClose())
    {
        if (ts == 20 || ts == 50 || ts == 100 || ts == 200 || ts == 300)
            paused = true;
        if (IsKeyPressed(KEY_ENTER))
        {
            DrawText("Simulation not initiated", 10, 10, 20, RED);
            if (started)
            {
                paused = true;
                ts = 0;
                InitGrid(gridType);
            }
            else
            {
                SaveInitialPreset();
                paused = false;
            }
            started = !started;
            if (started)
                SetTargetFPS(50);
            else
                SetTargetFPS(120);
        }

        if (IsKeyPressed(KEY_SPACE) && started)
        {
            paused = !paused;
        }

        if (IsKeyPressed(KEY_R))
        {
            ts = 0;
            InitGrid(gridType);
        }

        if (IsKeyPressed(KEY_UP))
        {
            altitude++;
        }
        if (IsKeyPressed(KEY_DOWN))
        {
            altitude--;
        }
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !started)
        {
            Vector2 mouse = GetMousePosition();
            int x = mouse.x / CELL_SIZE;
            int y = mouse.y / CELL_SIZE;

            if (x >= 0 && x < COLS && y >= 0 && y < ROWS)
            {
                if (!started)
                {
                    grid[x][y] = brush;
                    buffer[x][y] = brush;
                }
                altitudes[x][y] = altitude;
            }
        }
        if (IsKeyPressed(KEY_G))
        {
            switch (brush)
            {
            case vegetation_1:
                brush = vegetation_2;
                break;
            case vegetation_2:
                brush = vegetation_3;
                break;
            default:
                brush = vegetation_1;
            }
        }
        if (IsKeyPressed(KEY_F))
        {
            gridType++;
            if (gridType > 14)
                gridType = 0;
            if (!started)
                InitGrid(gridType);
        }
        if (IsKeyPressed(KEY_B))
            brush = water;
        if (IsKeyPressed(KEY_O))
            brush = initial_fire;
        if (IsKeyPressed(KEY_H))
            HUD = !HUD;

        if (!paused)
        {
            UpdateGrid();
            ts++;
        }

        BeginDrawing();
        ClearBackground(WHITE);
        PaintGrid();

        if (HUD)
        {
            DrawText("ENTER: Begin/Stop Simulation | SPACE: Start/Pause | R: Reset | H: Hide HUD", 10, 10, 20, RED);

            if (!started)
            {
                DrawText("PRESS ENTER TO START SIMULATION - Drawing enabled for initial preset", 10, 40, 20, DARKGRAY);

                Color brushColor = (brush == vegetation_1) ? GREEN : (brush == vegetation_2) ? (Color){66, 142, 57, 255}
                                                                 : (brush == vegetation_3)   ? (Color){58, 89, 54, 255}
                                                                 : (brush == water)          ? BLUE
                                                                 : (brush == initial_fire)   ? ORANGE
                                                                                             : BLACK;
                DrawText("Brush:", 10, 70, 20, DARKGRAY);
                DrawRectangle(80, 70, 24, 24, DARKGRAY);
                DrawRectangle(82, 72, 20, 20, brushColor);

                DrawText("Use G (Cicle Vegetations), B (Water), O (Initial Fire), X (Black) to change brush", 10, 100, 20, DARKGRAY);
            }
            else if (paused)
            {
                DrawText("PAUSED - Drawing disabled", 10, 40, 20, DARKGRAY);
            }
            else
                DrawText("RUNNING - Drawing disabled", 10, 40, 20, DARKGRAY);

            DrawTexturePro(arrow,
                           (Rectangle){0, 0, (float)arrow.width + 1, (float)arrow.height + 1},
                           (Rectangle){center.x, center.y, arrow.width, arrow.height},
                           origin,
                           angle,
                           WHITE);

            DrawTexturePro(circle,
                           (Rectangle){0, 0, (float)arrow.width, (float)arrow.height},
                           (Rectangle){center.x, center.y, arrow.width, arrow.height},
                           origin,
                           0,
                           WHITE);

            char intensityString[32];
            sprintf(intensityString, "Wind: %.0f %%", windIntensity * 100);
            int textWidth = MeasureText(intensityString, 20);
            DrawText(intensityString, center.x + 145 - textWidth / 2, center.y - 10, 20, DARKGRAY);

            char humString[20];
            sprintf(humString, "Humidity: %.0f %%", humidity * 100);
            textWidth = MeasureText(intensityString, 20);
            DrawText(humString, center.x + 145 - textWidth / 2, center.y + 10, 20, DARKGRAY);
        }
        char tsString[16];
        sprintf(tsString, "%d ts", ts);
        DrawText(tsString, SCREEN_WIDTH - center.x - 30, center.y, 20, WHITE);

        EndDrawing();
    }

    UnloadTexture(arrow);
    UnloadTexture(circle);
    CloseWindow();

    return 0;
}