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

#define SINGLEPLAYER_FREQUENCY pdMS_TO_TICKS(10)

TaskHandle_t SingleplayerTask = NULL;

public void SingleplayerEnter(void)
{
    if(xTaskCreate(vSingleplayerTask, "SingleplayerTask", 
                   mainGENERIC_STACK_SIZE, 
                   NULL, mainGENERIC_PRIORITY, 
                   SingleplayerTask) != pdPASS) {
        PRINT_ERROR("failed to create singleplayer task\n");
    }
    RendererEnter();
}

public void SingleplayerRun(void)
{

}

public void SingleplayerExit(void)
{
    vTaskDelete(SingleplayerTask);
    RendererExit();
}

public void vSingleplayerTask(void *pvParameters)
{
    // init
    bool running = true;
    short int global_counter = 0;
    short int collision_counter = 0;
    short int gap_counter = 0;
    short int obstacle_field = 0x0000;
    char second_pos = 0;
    short int player_position = 0;
    TickType_t last_wake_time = xTaskGetTickCount();
    game_data_t data = { 0 };

    while (running)
    {
        tumEventFetchEvents(FETCH_EVENT_NO_GL_CHECK | FETCH_EVENT_NO_GL_CHECK);
        global_counter = collision_counter + gap_counter;
        
        // counting is done twice as fast other wise the game is too easy
        gap_counter += 2 * (gap_counter < SPACE_BETWEEN);
        collision_counter += 2 * ((collision_counter < (OBSTACLE_WIDTH * 
                              STANDART_GRID_LENGTH) && gap_counter == SPACE_BETWEEN));

        //printf("gap: %d, col: %d, glob: %d\n", 
        //       gap_counter, collision_counter, global_counter);
        
        // update player position
        if(xSemaphoreTake(buttons.lock, 0) == pdPASS) {
            
            player_position -= 2 * buttons.buttons[KEYCODE(W)];
            player_position += 2 * buttons.buttons[KEYCODE(S)];

            xSemaphoreGive(buttons.lock); 
        }

        data.gamer_over = false; 
        second_pos = (obstacle_field << 4) >> 12;
        // check if collision counter is running and first bit is 1
        if(collision_counter != 0 && ((second_pos & 0b1000) != 0)) {
            second_pos &= 0b0111;
            // check for collision
            printf("sec: %d\npos: %d\n", second_pos, player_position);
            bool no_collision = 
            (12 - (second_pos + 2)) * STANDART_GRID_LENGTH - (MID_GAP / 2) <= 
            player_position - (PLAYER_RADIUS + 1)
            &&
            player_position + (PLAYER_RADIUS + 1) <=
            (12 - (second_pos + 2)) * STANDART_GRID_LENGTH + (MID_GAP / 2);
            printf("b: %d\n", no_collision);
            
            if(!no_collision) {
                //exit(EXIT_SUCCESS);
                data.gamer_over = true;
            }
        }

        data.global_counter = global_counter;
        data.obstacles = obstacle_field;
        data.player1_position = player_position;
        xQueueOverwrite(scene_queue, &data);

        // printf("size: %ld\n", sizeof(obstacle_field));
        // when collision_counter reaches its maxs it is reset
        // push over obstacle map
        if(global_counter >= STANDART_GAP_DISTANCE * STANDART_GRID_LENGTH) {
            obstacle_field <<= 4;
            obstacle_field |= rand() % 8 | 0b1000;
            printf("rand: %0X\n", obstacle_field);
            gap_counter = 0;
            collision_counter = 0;
        }        
        // generate new forth obstacle and bitor it into the map
        vTaskDelayUntil(&last_wake_time, SINGLEPLAYER_FREQUENCY);
    }    
}