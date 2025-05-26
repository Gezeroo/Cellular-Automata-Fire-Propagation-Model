#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define CELL_SIZE 10

#define COLS (SCREEN_WIDTH / CELL_SIZE)
#define ROWS (SCREEN_HEIGHT / CELL_SIZE)

int grid[COLS][ROWS];
int buffer[COLS][ROWS];

void InitGridRandom() {
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            grid[x][y] = GetRandomValue(0, 1);
        }
    }
}

int CountNeighbors(int x, int y) {
    int count = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;

            int nx = (x + dx + COLS) % COLS;
            int ny = (y + dy + ROWS) % ROWS;
            count += grid[nx][ny];
        }
    }
    return count;
}

void UpdateGrid() {
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            int neighbors = CountNeighbors(x, y);
            if (grid[x][y] == 1) {
                buffer[x][y] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            } else {
                buffer[x][y] = (neighbors == 3) ? 1 : 0;
            }
        }
    }

    // Copy buffer to grid
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            grid[x][y] = buffer[x][y];
        }
    }
}

void DrawaGrid() {
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if (grid[x][y] == 1) {
                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);
            }
        }
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Conway's Game of Life - Raylib");
    SetTargetFPS(10); // Adjust speed here

    InitGridRandom();

    bool paused = false;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) paused = !paused;
        if (IsKeyPressed(KEY_R)) InitGridRandom();

        if (!paused) UpdateGrid();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawaGrid();
        DrawText("SPACE: Pause/Resume  R: Reset", 10, 10, 20, GREEN);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}