#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define CELL_SIZE 10

#define COLS (SCREEN_WIDTH / CELL_SIZE)
#define ROWS (SCREEN_HEIGHT / CELL_SIZE)


typedef enum{
    green,  
    blue,   
    black
}CellState;

CellState grid[COLS][ROWS];
CellState buffer[COLS][ROWS];
bool type = false;

void InitGrid(){
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
                grid[x][y] = green;
                buffer[x][y] = green;
        }
    }
}
void InitGridRandom() {
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            int r = rand() % 100;
            if(r < 5)
                grid[x][y] = blue;
            else if(r <= 40)
                grid[x][y] = black;
            else
                grid[x][y] = green;
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
            if (grid[nx][ny] == black)
                count++;
        }
    }
    return count;
}

void contaminate(int x, int y){
    int direction[3] = {-1, 0, 1};
    int dx = rand() % 3;
    int dy = rand() % 3;
    int nx = (x + direction[dx] + COLS) % COLS;
    int ny = (y + direction[dy] + ROWS) % ROWS;
    if(grid[x][y] == black)
        if(grid[nx][ny] == green)    
            buffer[nx][ny] = black;
    else buffer[x][y] == grid[x][y];
}

void UpdateGrid() {
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if(!type){
                int neighbors = CountNeighbors(x, y);
                CellState current = grid[x][y];
                if (current == black)
                    buffer[x][y] = (neighbors == 2 || neighbors == 3) ? black : green;
                else if (current == green)
                    buffer[x][y] = (neighbors == 3) ? black : green;
                else buffer[x][y] = current;    
            }
            else
                contaminate(x,y);
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
            Color color;
            switch (grid[x][y]) {
                case green: color = GREEN; break;
                case blue: color = BLUE; break;
                case black: color = BLACK; break;
            }
            DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
        }
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Conway's Game of Life - Raylib");
    SetTargetFPS(10); // Adjust speed here
    
    InitGridRandom();
    bool paused = false;
    CellState brush = green;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            paused = !paused;
            if(paused)
                SetTargetFPS(60);
            else
                SetTargetFPS(10);
        }
        if (IsKeyPressed(KEY_R)) {
            if(type) InitGrid();
            else InitGridRandom();
        }
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&  paused) {
            Vector2 mouse = GetMousePosition();
            int x = mouse.x / CELL_SIZE;
            int y = mouse.y / CELL_SIZE;

            if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
                if (paused) grid[x][y] = brush;
                buffer[x][y] = brush;
            }
        }
        if (IsKeyPressed(KEY_G)) brush = green;
        if (IsKeyPressed(KEY_B)) brush = blue;
        if (IsKeyPressed(KEY_X)) brush = black;
        if (IsKeyPressed(KEY_E)) type = !type;

        if (!paused) UpdateGrid();

        BeginDrawing();
        ClearBackground(WHITE);
        DrawaGrid();
        DrawText("SPACE: Start/Pause  R: Reset  E: Change type of rule", 10, 10, 20, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}