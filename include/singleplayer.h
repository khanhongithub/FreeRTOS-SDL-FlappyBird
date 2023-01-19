#ifndef _STDBOOL_H
#include <stdbool.h>
#endif

#define STANDART_GAP_DISTANCE 8
#define FIRST_GAP_DISTANCE (STANDART_GAP_DISTANCE - 2)
#define STANDART_GRID_LENGTH 48
#define OBSTACLE_WIDTH 2
#define SPACE_BETWEEN ((STANDART_GAP_DISTANCE - OBSTACLE_WIDTH) * STANDART_GRID_LENGTH)
#define MID_GAP (3.5 * STANDART_GRID_LENGTH)
#define FLOOR_HEIGTH 10

#define FIRST_POSITION (FIRST_GAP_DISTANCE * STANDART_GRID_LENGTH)
#define SECOND_POSITION (FIRST_GAP_DISTANCE + STANDART_GAP_DISTANCE) * STANDART_GRID_LENGTH
#define THIRD_POSITION (FIRST_GAP_DISTANCE + 2 * STANDART_GAP_DISTANCE) * STANDART_GRID_LENGTH
#define FOURTH_POSITION (FIRST_GAP_DISTANCE + 3 * STANDART_GAP_DISTANCE) * STANDART_GRID_LENGTH

#define PLAYER_RADIUS 18

#define GRAVITY 0.1

typedef struct game_data {
    short int highscore;
    short int score;
    short int obstacles;
    short int global_counter;  
    short int player1_position;
    bool gamer_over;
    char jump;
}game_data_t;

void SingleplayerEnter(void);
void SingleplayerRun(void);
void SingleplayerExit(void);
void vSingleplayerTask(void *pvParameters);