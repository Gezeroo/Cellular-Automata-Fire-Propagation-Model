#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define SCREEN_WIDTH 1020
#define SCREEN_HEIGHT 1020
#define CELL_SIZE 10

#define COLS (SCREEN_WIDTH / CELL_SIZE)
#define ROWS (SCREEN_HEIGHT / CELL_SIZE)


typedef enum{
    vegetation, 
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
} direction;

CellState grid[COLS][ROWS];
CellState buffer[COLS][ROWS];
int ticks[COLS][ROWS];
bool paused = true;
bool HUD = true;

//Parameters
float initFireParam = 0.006;
float stableFireParam = 1.0;
float emberFireParam = 0.2;
float windIntensity = 0.5;

void InitGrid(){
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
                grid[x][y] = vegetation;
                buffer[x][y] = vegetation;
        }
    }
}

void spreadFire(int x, int y){
    int direction[3] = {-1, 0, 1};

    if(grid[x][y] == vegetation){
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                int nx = (x + dx);
                int ny = (y + dy);
                
                int rnd = rand()%100;
                if(nx >= ROWS || nx < 0 || ny >= COLS || ny < 0) continue;

                if((grid[nx][ny] == initial_fire) && rnd< (initFireParam * 100))
                    buffer[x][y] = initial_fire;
                else if((grid[nx][ny] == stable_fire) && rnd < (stableFireParam * 100))
                    buffer[x][y] = initial_fire;
                else if((grid[nx][ny] == ember) && rnd < (stableFireParam * 100))
                    buffer[x][y] = initial_fire;
            }
        }
    }

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
        if(rand() % 100 < (ticks[x][y])/100){
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
                spreadFire(x,y);
        }
    }

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
            }
            DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
        }
    }
}

float GetAngleFromDirection(int dir) {
    return dir * 45.0f; // Each step is 45 degrees
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "AC - Raylib");
    SetTargetFPS(120);

    for(int i = 0; i < COLS; i++)
        for(int j = 0; j < ROWS; j++)
            ticks[i][j] = 0;
    
    InitGrid();

    CellState brush = vegetation;
    Texture2D arrow = LoadTexture("sprites/windArrowBigArrow.png");
    Texture2D circle = LoadTexture("sprites/windArrowBigBorder.png");
    Vector2 center = { 64, SCREEN_HEIGHT  - circle.height / 2.0f };
    Vector2 origin = { circle.width / 2.0f, circle.height / 2.0f };
    float angle = GetAngleFromDirection(SE);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            DrawText("Paused", 10, 10, 20, RED);
            paused = !paused;
            if(paused)
                SetTargetFPS(120);
            else
                SetTargetFPS(10);
        }
        if (IsKeyPressed(KEY_R)) {
            InitGrid();
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
        if (IsKeyPressed(KEY_O)) brush = initial_fire;
        if (IsKeyPressed(KEY_H)) HUD = !HUD;
 
        if (!paused) UpdateGrid();

        BeginDrawing();
        ClearBackground(WHITE);
        DrawaGrid();
        if(HUD){
            DrawText("SPACE: Start/Pause | R: Reset | H: Hide HUD", 10, 10, 20, RED);

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
        }
        
        if(HUD){
            DrawTexturePro(arrow,
            (Rectangle){ 0, 0, (float)arrow.width +1, (float)arrow.height +1 },
            (Rectangle){ center.x, center.y, arrow.width, arrow.height },
            origin,
            angle,
            WHITE);

            DrawTexturePro(circle,
            (Rectangle){ 0, 0, (float)arrow.width, (float)arrow.height },
            (Rectangle){ center.x, center.y, arrow.width, arrow.height },
            origin,
            0,
            WHITE);
        
            char intensityString[32];
            sprintf(intensityString, "Wind: %.1f m/s", windIntensity);
            int textWidth = MeasureText(intensityString, 20);
            DrawText(intensityString, center.x + 140 - textWidth / 2, center.y, 20, DARKGRAY);
        }
        
        EndDrawing();
    }
    UnloadTexture(arrow);
    UnloadTexture(circle);
    CloseWindow();
    return 0;
}