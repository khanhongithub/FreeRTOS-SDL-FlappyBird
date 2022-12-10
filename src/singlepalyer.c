#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"
#include "TUM_Font.h"

#include "animations.h"
#include "timef.h"

#include "statemachine.h"
#include "main.h"
#include "priorties.h"
#include "resources.h"

#define STANDART_GAP_DISTANCE 8
#define FIRST_GAP_DISTANCE STANDART_GAP_DISTANCE - 2
#define STANDART_GRID_LENGTH 20
#define OBSTACLE_WIDTH 2

#define FIRST_POSITION FIRST_GAP_DISTANCE * STANDART_GRID_LENGTH
#define SECOND_POSITION (FIRST_GAP_DISTANCE + STANDART_GAP_DISTANCE) * STANDART_GRID_LENGTH
#define THIRD_POSITION (FIRST_GAP_DISTANCE + 2 * STANDART_GAP_DISTANCE) * STANDART_GRID_LENGTH
#define FOURTH_POSITION (FIRST_GAP_DISTANCE + 3 * STANDART_GAP_DISTANCE) * STANDART_GRID_LENGTH

TaskHandle_t SingleplayerTask = NULL;

typedef struct {
    char first : 4; 
    char second : 4;
    char third : 4;
    char fourth : 4;
} obstacle_field;

public void SingleplayerEnter(void)
{
    if(xTaskCreate(vSingleplayerTask, "SingleplayerTask", 
                   mainGENERIC_STACK_SIZE, 
                   NULL, mainGENERIC_PRIORITY, 
                   SingleplayerTask) != pdPASS) {
        PRINT_ERROR("failed to create singleplayer task\n");
    }
}

public void SingleplayerRun(void)
{

}

public void SingleplayerExit(void)
{
    vTaskDelete(SingleplayerTask);
}

public void vSingleplayerTask(void *pvParameters)
{
    // init
    bool running = true;
    short int global_counter, collision_counter, gap_counter = 0;
    obstacle_field = 0x0000;

    while (running)
    {
        global_counter = collision_counter + gap_counter;
        // gap_counter += (gap_counter < (STANDART_GAP_DISTANCE - OBSTACLE_WIDTH) * STANDART_GRID_LENGTH);
        // collision_counter += ((collision_counter < OBSTACLE_WIDTH * STANDART_GRID_LENGTH));
        if(collision_counter != 0) {
            // check for collision
        }
        printf("size: %d\n", sizeof(obstacle_field));
        
        // when collision_counter reaches its maxs it is reset
        // push over obstacle map 
        // generate new forth obstacle and bitor it into the map
    }    
}