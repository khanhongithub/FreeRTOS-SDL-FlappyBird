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
        DEBUG_PRINT("failed to create singleplayer task\n");
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
    xSemaphoreGive(buttons.lock); 
}

public void vSingleplayerTask(void *pvParameters)
{
    // init, innit?
    reset:;

    static struct timespec the_time;
    static short int high_score = 0;

    short int global_counter = 0;
    short int collision_counter = 0;
    short int score = 0;
    bool cur_SPACE, prev_SPACE, flap = false;
    short int gap_counter = 0;
    short int obstacle_field = 0x0000;
    char second_pos = 0;
    double player_position = SCREEN_HEIGHT / 2;
    double vertical_speed = 0;
    TickType_t last_wake_time = xTaskGetTickCount();
    game_data_t data = { 0 };

    while(1) 
    {
       
        tumEventFetchEvents(FETCH_EVENT_NO_GL_CHECK | FETCH_EVENT_NONBLOCK);
        data.gamer_over = false;

        global_counter = collision_counter + gap_counter;

        // counting is done twice as fast other wise the game is too easy
        gap_counter += 2 * (gap_counter < SPACE_BETWEEN);
        collision_counter += 2 * ((collision_counter < (OBSTACLE_WIDTH * 
                            STANDART_GRID_LENGTH) && gap_counter == SPACE_BETWEEN));

        //printf("gap: %d, col: %d, glob: %d\n", 
        //       gap_counter, collision_counter, global_counter);

        // update player position when jumping
        if(xSemaphoreTake(buttons.lock, 0) == pdPASS) {

            cur_SPACE = buttons.buttons[KEYCODE(SPACE)];
            flap = !prev_SPACE && cur_SPACE;
            prev_SPACE = cur_SPACE;
            xSemaphoreGive(buttons.lock); 

            data.jump = flap;
            if(flap)
                vertical_speed = -4.1;
        }
        player_position += vertical_speed;

        // ensure player is within legal position
        if(player_position - PLAYER_RADIUS <= 0) {
            player_position = PLAYER_RADIUS + 1;
            vertical_speed = 0;
            //data.gamer_over = true;
        }
        else if(player_position + PLAYER_RADIUS + 1 >= SCREEN_HEIGHT) {
            player_position = SCREEN_HEIGHT - PLAYER_RADIUS - 1;
            vertical_speed = 0;
            data.gamer_over = true;
        }
        else {
            vertical_speed += GRAVITY;
        }

        second_pos = (obstacle_field << 4) >> 12; // <- second pipe for collision
        // check if collision counter is running and first bit is 1
        if(collision_counter != 0 && ((second_pos & 0x8) != 0)) {

            second_pos &= 0x7;
            // printf("sec: %d\npos: %f\n", second_pos, player_position);

            bool no_collision = 
            (12 - (second_pos + 2)) * STANDART_GRID_LENGTH - (MID_GAP / 2) <= 
            player_position - (PLAYER_RADIUS + 1)
            &&
            player_position + (PLAYER_RADIUS + 1) <=
            (12 - (second_pos + 2)) * STANDART_GRID_LENGTH + (MID_GAP / 2);
                
            if(!no_collision) {
                // todo: pause screen and throw menu
                data.gamer_over = true;
            }
        }

        // detect tha player has left obstcle
        score += (gap_counter == PLAYER_RADIUS && 
                       ((obstacle_field & 0xA000) != 0));
        if(score >= high_score)
            high_score = score;

        data.global_counter = global_counter;
        data.obstacles = obstacle_field;
        data.score = score;
        data.highscore = high_score;
        data.player1_position = player_position;
        xQueueOverwrite(scene_queue, &data);

        // halt game and wait for input to reset or go to menu
        if(data.gamer_over) {
            while(1)
            {
                    if(xSemaphoreTake(restart_signal_singleplayer, 0))
                        goto reset;
            }
        }
            
        // when collision_counter reaches its maxs it is reset
        if(global_counter >= STANDART_GAP_DISTANCE * STANDART_GRID_LENGTH) {
            clock_gettime(CLOCK_REALTIME,&the_time);
            obstacle_field <<= 4;
            // generate new forth obstacle and bitwise-or it into the map
            obstacle_field |= (rand() &+ the_time.tv_nsec) % 8 | 0x8;
            // printf("rand: %X\n", obstacle_field);
            gap_counter = 0;
            collision_counter = 0;
        }

        vTaskDelayUntil(&last_wake_time, SINGLEPLAYER_FREQUENCY);
    }    
}