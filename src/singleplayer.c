#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
#include "TUM_Print.h"

#include "statemachine.h"
#include "main.h"
#include "priorties.h"
#include "resources.h"
#include "singleplayer.h"
#include "renderer.h"
#include "cheatmenu.h"

#define SINGLEPLAYER_FREQUENCY pdMS_TO_TICKS(9)

TaskHandle_t SingleplayerTask = NULL;

public void SingleplayerEnter(void)
{
    if(xTaskCreate(vSingleplayerTask, "SingleplayerTask", 
                   mainGENERIC_STACK_SIZE, 
                   NULL, mainGENERIC_PRIORITY, 
                   &SingleplayerTask) != pdPASS) {
        DEBUG_PRINT("failed to create singleplayer task\n");
    }
    RendererEnter();
}

public void SingleplayerRun(void)
{}

public void SingleplayerExit(void)
{
    vTaskDelete(SingleplayerTask);
    RendererExit();
    xSemaphoreGive(buttons.lock); 
}

public void vSingleplayerTask(void *pvParameters)
{
    reset:;

    static struct timespec the_time;
    static short int high_score = 0;

    bool space_pressed_first_time = false;
    short int global_counter = 0;
    short int collision_counter = 0;
    short int score = 0;
    bool ignore_collision = false;
    bool cur_SPACE, prev_SPACE, flap = false;
    bool cur_ESCAPE, prev_ESCAPE, paused = false;
    short int gap_counter = 0;
    short int obstacle_field = 0x0000;
    char opening_mid = 0;
    double player_position = SCREEN_HEIGHT / 2;
    double vertical_speed = 0;
    TickType_t last_wake_time = xTaskGetTickCount();
    game_data_t data = { 0 };

    // ensure the queue exists in the fist place
    assert(scene_queue != NULL);
    
    data.player1_position = player_position;
    data.gamer_over = false;
    xQueueOverwrite(scene_queue, &data);

    score = HighScore();
    ignore_collision = IgnoreCollision();
    
    while(1) 
    {
        data.gamer_over = false;
        
        while(!space_pressed_first_time) {
            if(xSemaphoreTake(buttons.lock, 0) != pdPASS) {
                xGetButtonInput();
                tumEventFetchEvents(FETCH_EVENT_NONBLOCK);
                space_pressed_first_time = (bool)buttons.buttons[KEYCODE(SPACE)];
                data.waiting = !space_pressed_first_time;
                xQueueOverwrite(scene_queue, &data);
                xSemaphoreGive(buttons.lock);
            }
            vTaskDelayUntil(&last_wake_time, SINGLEPLAYER_FREQUENCY);
        }
        
        do
        {
            if(xSemaphoreTake(buttons.lock, 0) != pdPASS) {          
                cur_ESCAPE = buttons.buttons[KEYCODE(ESCAPE)];
                if (!prev_ESCAPE && cur_ESCAPE)
                {
                    paused ^= true; // toggle 
                }
            
                prev_ESCAPE = cur_ESCAPE;
                xSemaphoreGive(buttons.lock);
            }

            if (xSemaphoreTake(resume_signal_singleplayer, 0) == pdPASS)
            {
                paused = false;
            }

            if (paused)
            {
                data.pause = paused;
                xQueueOverwrite(scene_queue, &data);
                vTaskDelayUntil(&last_wake_time, SINGLEPLAYER_FREQUENCY);
            }
            
        } while (paused);
                
        global_counter = collision_counter + gap_counter;

        // counting is done twice as fast other wise the game is too easy
        if (gap_counter < SPACE_BETWEEN)
        {
            gap_counter += 2;
        }
        
        if ((collision_counter < (OBSTACLE_WIDTH * 
                STANDART_GRID_LENGTH) && gap_counter == SPACE_BETWEEN))
        {        
            collision_counter += 2;
        }
                
        //physics
       
        // update player position when jumping
        if(xSemaphoreTake(buttons.lock, 0) == pdPASS) {
             xGetButtonInput();
            tumEventFetchEvents(FETCH_EVENT_NO_GL_CHECK | FETCH_EVENT_NONBLOCK);
            cur_SPACE = buttons.buttons[KEYCODE(SPACE)];
            flap = !prev_SPACE && cur_SPACE;
            prev_SPACE = cur_SPACE;
            xSemaphoreGive(buttons.lock);
        }

        data.jump = flap;
            if(flap)
                vertical_speed = JUMP_IMPULSE_SPEED;

        player_position += vertical_speed;

        // ensure player is within legal position
        if(player_position - PLAYER_RADIUS <= 0) {
            player_position = PLAYER_RADIUS + 1;
            vertical_speed = 0;
            //data.gamer_over = true;
        }
        else if (player_position + PLAYER_RADIUS + 1 >= SCREEN_HEIGHT) {
            player_position = SCREEN_HEIGHT - PLAYER_RADIUS - 1;
            vertical_speed = 0;
            data.gamer_over = !ignore_collision;
        }
        else {
            vertical_speed += GRAVITY;
        }

        // colision
        if (player_position + (PLAYER_RADIUS + 1) 
                    >= SCREEN_HEIGHT - FLOOR_HEIGTH)
        {
            data.gamer_over = !ignore_collision;
        }

        opening_mid = (obstacle_field << 4) >> 12; // <- second pipe for collis.
        // check if collision counter is running and first bit is 1
        if ( !ignore_collision && (collision_counter != 0 
           || gap_counter >= SPACE_BETWEEN - (2 * PLAYER_RADIUS)) 
           && ((opening_mid & 0b1000) != 0)) { // <- check if obstacle visible

            opening_mid &= 0b0111; // <- extracts obstacle type

            bool no_collision = 
            (FLOOR_HEIGTH - opening_mid) * STANDART_GRID_LENGTH - (MID_GAP / 2) <= 
            player_position - (PLAYER_RADIUS + 1) // top pipe
            &&
            player_position + (PLAYER_RADIUS + 1) <= // bottom pipe
            (FLOOR_HEIGTH - opening_mid) * STANDART_GRID_LENGTH + (MID_GAP / 2)
            && 
            !ignore_collision; // bypass through cheatmenu;

            if(!no_collision) {
                data.gamer_over = true;
            }
        }

        // detect that player has left obstcle
        score += (gap_counter == PLAYER_RADIUS && 
                 ((obstacle_field & 0xA000) != 0));
        // update high score
        if(score >= high_score)
            high_score = score;

        // pack up data and send to renderer
        data.pause = paused;
        data.global_counter = global_counter;
        data.obstacles = obstacle_field;
        data.score = score;
        data.highscore = high_score;
        data.player1_position = player_position;
        xQueueOverwrite(scene_queue, &data);

        // halt game and wait for input to reset
        if(data.gamer_over) {
            while(1)
            {
                if(xSemaphoreTake(restart_signal_singleplayer, 0) == pdPASS)
                    goto reset;
            }
        }
            
        // generate new obstacle
        if(global_counter >= STANDART_GAP_DISTANCE * STANDART_GRID_LENGTH) {
            clock_gettime(CLOCK_REALTIME, &the_time);
            obstacle_field <<= 4;
            // generate new obstacle and bitwise-or to the others
            obstacle_field |= (rand() & the_time.tv_nsec) % 8 | 0x8;
            
            // reset counters
            gap_counter = 0;
            collision_counter = 0;
        }

        vTaskDelayUntil(&last_wake_time, SINGLEPLAYER_FREQUENCY);
    }    
}