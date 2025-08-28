#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <Math.h>
#include "automata.h"



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
    int minAlt = grid[0][0].altitude;
    int maxAlt = grid[0][0].altitude;
    for (int x = 0; x < COLS; x++)
    {
        for (int y = 0; y < ROWS; y++)
        {
            if (grid[x][y].altitude < minAlt)
                minAlt = grid[x][y].altitude;
            if (grid[x][y].altitude > maxAlt)
                maxAlt = grid[x][y].altitude;
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
                norm = (float)(grid[x][y].altitude - minAlt) / (maxAlt - minAlt);
            }

            int level = (int)(norm * 4 + 0.5f);
            if (level < 0)
                level = 0;
            if (level > 4)
                level = 4;
            float brightnessFactor = brightnessLevels[level];

            Color color;
            switch (grid[x][y].state)
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

            int currentAltitude = grid[x][y].altitude;

            int dx[4] = {0, 0, -1, 1};
            int dy[4] = {-1, 1, 0, 0};

            for (int i = 0; i < 4; i++)
            {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS)
                {
                    int neighborAltitude = grid[nx][ny].altitude;

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
    bool HUD = true;
    bool started = false;
    bool paused = true;

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
        if (ts == 20 || ts == 50 || ts == 100 || ts == 200 || ts == 300 || ts == 500)
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
                    grid[x][y].state = brush;
                    grid[x][y].buffer = brush;
                }
                grid[x][y].altitude = altitude;
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
            if (gridType > 8)
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

        char countString[16];
        sprintf(countString, "%.f%%", (countBurnedCells()/((COLS)*(ROWS)))*100);
        DrawText(countString, SCREEN_WIDTH - center.x - 30, center.y + 20, 20, WHITE);

        EndDrawing();
    }

    UnloadTexture(arrow);
    UnloadTexture(circle);
    CloseWindow();

    return 0;
}