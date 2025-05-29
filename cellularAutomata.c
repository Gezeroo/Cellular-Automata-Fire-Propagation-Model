#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024
#define CELL_SIZE 1

#define COLS (SCREEN_WIDTH / CELL_SIZE)
#define ROWS (SCREEN_HEIGHT / CELL_SIZE)


typedef enum{
    vegetation, //vegetation 
    initial_fire, //initial fire
    stable_fire, //stable fire
    ember, //ember
    water,   //water 
    ash, //ash
    black 
}CellState;

CellState grid[COLS][ROWS];
CellState buffer[COLS][ROWS];
int ticks[COLS][ROWS];
bool type = true;
bool paused = true;
bool HUD = true;

void InitGrid(){
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
                grid[x][y] = vegetation;
                buffer[x][y] = vegetation;
        }
    }
}
void InitGridRandom() {
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            int r = rand() % 100;
            if(r < 5)
                grid[x][y] = water;
            else if(r <= 40)
                grid[x][y] = black;
            else
                grid[x][y] = vegetation;
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
    int count = 0;

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;

            int nx = (x + dx);
            int ny = (y + dy);
            if((nx >= COLS) || (nx <= 0) || (ny >= ROWS) || (ny <= 0)) continue;
            if (grid[nx][ny] == initial_fire || grid[nx][ny] == stable_fire || grid[nx][ny] == ember)
                count++;
        }
    }
    if((grid[x][y] == vegetation) && (rand() % 100 < (12.5*count)))
        buffer[x][y] = initial_fire;
    else if(grid[x][y] == initial_fire){
        if(ticks[x][y] > 3){
            ticks[x][y] = 0;
            buffer[x][y] = stable_fire;
        }
        else ticks[x][y]++;
    }
    else if(grid[x][y] == stable_fire){
        if(ticks[x][y] > 5){
            ticks[x][y] = 0;
            buffer[x][y] = ember;
        }
        else ticks[x][y]++;
    }
    else if(grid[x][y] == ember){
        if(ticks[x][y] > 10){
            ticks[x][y] = 0;
            buffer[x][y] = ash;
        }
        else ticks[x][y]++;
    }
    else if(grid[x][y] == ash){
        if(rand() % 100 < (ticks[x][y])/50){
            ticks[x][y] = 0;
            buffer[x][y] = vegetation;
        }
        else ticks[x][y]++;
    }
    else if(grid[x][y] == water) buffer[x][y] = grid[x][y];
}

void UpdateGrid() {
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if(!type){
                int neighbors = CountNeighbors(x, y);
                CellState current = grid[x][y];
                if (current == black)
                    buffer[x][y] = (neighbors == 2 || neighbors == 3) ? black : vegetation;
                else if (current == vegetation)
                    buffer[x][y] = (neighbors == 3) ? black : vegetation;
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
                case vegetation: color = GREEN; break;
                case water: color = BLUE; break;
                case initial_fire: color = ORANGE; break;
                case stable_fire: color = RED; break;
                case ember: color = MAROON; break;
                case ash: color = GRAY; break; 
                case black: color = BLACK; break;
            }
            DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
        }
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Conway's Game of Life - Raylib");
    SetTargetFPS(60);
    for(int i = 0; i < COLS; i++)
        for(int j = 0; j < ROWS; j++)
            ticks[i][j] = 0;

    InitGrid();
    CellState brush = vegetation;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            DrawText("Paused", 10, 10, 20, RED);
            paused = !paused;
            if(paused)
                SetTargetFPS(120);
            else
                SetTargetFPS(60);
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
        if (IsKeyPressed(KEY_G)) brush = vegetation;
        if (IsKeyPressed(KEY_B)) brush = water;
        if (IsKeyPressed(KEY_X)) brush = black;
        if (IsKeyPressed(KEY_E)) type = !type;
        if (IsKeyPressed(KEY_O)) brush = initial_fire;
        if (IsKeyPressed(KEY_H)) HUD = !HUD;

        if (!paused) UpdateGrid();

        BeginDrawing();
        ClearBackground(WHITE);
        DrawaGrid();
        if(HUD){
            DrawText("SPACE: Start/Pause | R: Reset | E: Toggle Rules | H: Hide HUD", 10, 10, 20, RED);

            // Show paused state and brush color
            if (paused) {
                DrawText("PAUSED - Drawing Enabled", 10, 40, 20, DARKGRAY);

                // Show current brush color
                Color brushColor = (brush == vegetation) ? GREEN : (brush == water) ? BLUE : (brush == initial_fire) ? ORANGE : BLACK;
                DrawText("Brush:", 10, 70, 20, DARKGRAY);
                DrawRectangle(80, 70, 24, 24, DARKGRAY);
                DrawRectangle(82, 72, 20, 20, brushColor);
                

                DrawText("Use G (Green), B (Blue), O (Orange), X (Black) to change brush", 10, 100, 20, DARKGRAY);
            } 
            else
                DrawText("RUNNING - Drawing Disabled", 10, 40, 20, DARKGRAY);

            if(!type)
                DrawText("RULE - Game of Life", (SCREEN_WIDTH/2) + 100, 40, 20, DARKGRAY);

            else
                DrawText("RULE - Random Spread", (SCREEN_WIDTH/2) + 100, 40, 20, DARKGRAY);
        }
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}