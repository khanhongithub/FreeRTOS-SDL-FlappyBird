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
#include "singleplayer.h"
#include "renderer.h"

#define RENDER_FREQUENCY pdMS_TO_TICKS(5)

TaskHandle_t RenderingTask = NULL;

void DrawObstacle(short int x_position, char type, short int counter)
{
    short int middle_gap = MID_GAP;
    short int bottom_length = 0;
    short int top_length = 0;

    if((type & 0x8) == 0) {
        return;
    }
    type &= 0x7;

    // get obstacle
    top_length = 11 * STANDART_GRID_LENGTH - ((type + 1) * STANDART_GRID_LENGTH); 
    bottom_length = (type + 2) * STANDART_GRID_LENGTH ;//;
    // printf("tl: %d, t: %d\n", top_length, type);

    tumDrawFilledBox(x_position - counter,
                     0, 
                     OBSTACLE_WIDTH * STANDART_GRID_LENGTH, 
                     top_length - (middle_gap / 2), Green);

    tumDrawFilledBox(x_position - counter,
                     SCREEN_HEIGHT - bottom_length + (middle_gap / 2), 
                     OBSTACLE_WIDTH * STANDART_GRID_LENGTH, 
                     bottom_length - (middle_gap / 2), Green);
} 

void DrawPlayer(short int player_height, int color)
{
    tumDrawCircle(FIRST_POSITION + OBSTACLE_WIDTH * STANDART_GRID_LENGTH, 
                  player_height, PLAYER_RADIUS, Orange);
}

void RendererEnter(void)
{
    if(xTaskCreate(vRendererTask, "RendererTask", 
                   mainGENERIC_STACK_SIZE, 
                   NULL, mainGENERIC_PRIORITY + 1, 
                   RenderingTask) != pdPASS) {
        PRINT_ERROR("failed to create rendering task\n");
    }
}

void RendererRun(void)
{}

void RendererExit(void)
{
    vTaskDelete(RenderingTask);
}

void vRendererTask(void* pcParameters)
{
    game_data_t buffer;
    TickType_t last_wake_time = xTaskGetTickCount();

    tumDrawBindThread();
    while (1)
    {
        tumDrawClear(White);
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK);
        xQueuePeek(scene_queue, &buffer, 0);
        
        DrawObstacle(FIRST_POSITION, (buffer.obstacles & 0xF000) >> 12, 
                     buffer.global_counter);

        DrawObstacle(SECOND_POSITION, (buffer.obstacles & 0x0F00) >> 8, 
                     buffer.global_counter);

        DrawObstacle(THIRD_POSITION, (buffer.obstacles & 0x00F0) >> 4, 
                     buffer.global_counter);

        DrawObstacle(FOURTH_POSITION, (buffer.obstacles & 0x000F) >> 0, 
                     buffer.global_counter);

        DrawPlayer(buffer.player1_position, Orange);
        // moving background regradless what happens
        // raed from queue
        if(buffer.gamer_over)
            tumDrawFilledBox(0, 0, 100, SCREEN_HEIGHT, Red);
        tumDrawUpdateScreen();
        vTaskDelayUntil(&last_wake_time, RENDER_FREQUENCY);
    }
    
}