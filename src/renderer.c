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

image_handle_t player_sprite = NULL;
image_handle_t pipe_bottom = NULL;
image_handle_t pipe_top = NULL;
image_handle_t background_sprite = NULL;
image_handle_t gameover_sprite = NULL;

void checkDraw(unsigned char status, const char *msg)
{
    if (status) {
        if (msg)
            fprintf(stderr, "[ERROR] %s, %s\n", msg,
                    tumGetErrorMessage());
        else {
            fprintf(stderr, "[ERROR] %s\n", tumGetErrorMessage());
        }
    }
}

void DrawBackground(void)
{
    short int bg_width = 0;
    static float counter = 0; 

    if(background_sprite == NULL) {    
        background_sprite = tumDrawLoadScaledImage(BACKGROUND_SPRITE, 1.15);
        bg_width = tumDrawGetLoadedImageWidth(background_sprite);
    }
    bg_width = tumDrawGetLoadedImageWidth(background_sprite);
    // counter has length of image width for seemless texture scrolling
    counter = counter + 0.25;
    counter *= (counter <= bg_width);
    
    tumDrawLoadedImage(background_sprite, - counter, 0);
    tumDrawLoadedImage(background_sprite, - counter + bg_width, 0);   
    tumDrawLoadedImage(background_sprite, - counter + 2 * bg_width, 0);
    tumDrawLoadedImage(background_sprite, - counter + 3 * bg_width, 0);  
    tumDrawLoadedImage(background_sprite, - counter + 4 * bg_width, 0);    
}

void DrawObstacle(short int x_position, char type, short int counter)
{
    static int image_height_top_pipe;
    static int image_height_bottom_pipe;
    short int middle_gap = MID_GAP;
    short int bottom_length = 0;
    short int top_length = 0;

    if(pipe_top == NULL) 
    {
        pipe_top = tumDrawLoadScaledImage(PIPE_TOP_SPRITE, 1.98);
        //printf("loaded image\n");
    }

    if(pipe_bottom == NULL) 
    {
        pipe_bottom = tumDrawLoadScaledImage(PIPE_BOTTOM_SPRITE, 1.98);
        //printf("loaded image\n");
    }

    if((type & 0x8) == 0) {
        return;
    }
    type &= 0x7; // <- removing 4th bit

    // get obstacle
    top_length = 11 * STANDART_GRID_LENGTH - ((type + 1) * STANDART_GRID_LENGTH); 
    bottom_length = (type + 2) * STANDART_GRID_LENGTH;
    // printf("tl: %d, t: %d\n", top_length, type);
    
    // top pipe 
    #if 0
    tumDrawFilledBox(x_position - counter,
                     0, 
                     OBSTACLE_WIDTH * STANDART_GRID_LENGTH, 
                     top_length - (middle_gap / 2), Green);
    #endif   
    if((image_height_top_pipe = tumDrawGetLoadedImageHeight(pipe_top)) != -1) {
        tumDrawLoadedImage(pipe_top, 
                           x_position - counter - 2, /* pipe image isnt centered*/
                           top_length - (middle_gap / 2) - image_height_top_pipe);
    }

    // bottom pipe
    #if 0
    tumDrawFilledBox(x_position - counter,
                     SCREEN_HEIGHT - bottom_length + (middle_gap / 2), 
                     OBSTACLE_WIDTH * STANDART_GRID_LENGTH, 
                     bottom_length - (middle_gap / 2), Green);;
    #endif
    if((image_height_bottom_pipe = tumDrawGetLoadedImageHeight(pipe_bottom)) != -1) {
        tumDrawLoadedImage(pipe_bottom, 
                           x_position - counter - 2, /* pipe image isnt centered*/
                           SCREEN_HEIGHT - bottom_length + (middle_gap / 2) ); 
    }
} 

void DrawGameoverScreen(short int high_score, short int score)
{
    static int image_height, image_width;
    if(gameover_sprite == NULL) {
        gameover_sprite = tumDrawLoadScaledImage(GAME_OVER_SPRITE, 0.5);
    } 

    tumDrawFilledBox(SCREEN_WIDTH / 2 - (GAMEOVER_BOX_WIDTH / 2), 
                                 (SCREEN_HEIGHT / 2) - (GAMEOVER_BOX_HEIGHT / 2), 
                                 GAMEOVER_BOX_WIDTH, 
                                 GAMEOVER_BOX_HEIGHT, 
                                 0x52394a);
    
    tumDrawFilledBox((SCREEN_WIDTH / 2) - (GAMEOVER_BOX_WIDTH_CONTENT / 2), 
                                 SCREEN_HEIGHT / 2 - 
                                 (GAMEOVER_BOX_HEIGHT_CONTENT / 2), 
                                 GAMEOVER_BOX_WIDTH_CONTENT, 
                                 GAMEOVER_BOX_HEIGHT_CONTENT, 
                                 0xded794);

    if((image_height = tumDrawGetLoadedImageHeight(gameover_sprite)) != -1 &&
       (image_width = tumDrawGetLoadedImageWidth(gameover_sprite)) != -1) {
       tumDrawLoadedImage(gameover_sprite,
                          SCREEN_WIDTH / 2
                          - image_width / 2,
                          SCREEN_HEIGHT / 2 - image_height / 2);
    }

}

void DrawPlayer(short int player_height, int color)
{
    static int image_height;
    if(player_sprite == NULL) 
    {
        player_sprite = tumDrawLoadScaledImage(PLAYER_SPRITE, 0.09);
        //printf("loaded image\n");
    }
    /*
    tumDrawCircle(FIRST_POSITION + OBSTACLE_WIDTH * STANDART_GRID_LENGTH, 
                  player_height, PLAYER_RADIUS, Orange);
    */

    if((image_height = tumDrawGetLoadedImageHeight(player_sprite)) != -1) {
        tumDrawLoadedImage(player_sprite,
                           FIRST_POSITION + OBSTACLE_WIDTH * STANDART_GRID_LENGTH 
                           - image_height / 2,
                           player_height - image_height / 2 + 4);
        // printf("heiu: %d\n", image_height);
    }
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
    char highscore_text[30];
    char score_text[30];
    TickType_t last_wake_time = xTaskGetTickCount();

    tumDrawBindThread();
    while (1)
    {
        tumDrawClear(White);
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK);
        xQueuePeek(scene_queue, &buffer, 0);
        
        DrawBackground();

        DrawObstacle(FIRST_POSITION, (buffer.obstacles & 0xF000) >> 12, 
                     buffer.global_counter);

        DrawObstacle(SECOND_POSITION, (buffer.obstacles & 0x0F00) >> 8, 
                     buffer.global_counter);

        DrawObstacle(THIRD_POSITION, (buffer.obstacles & 0x00F0) >> 4, 
                     buffer.global_counter);

        DrawObstacle(FOURTH_POSITION, (buffer.obstacles & 0x000F) >> 0, 
                     buffer.global_counter);

        DrawPlayer(buffer.player1_position, Orange);

        sprintf(highscore_text, "high score: %d", buffer.highscore);
        tumDrawText(highscore_text,
        10,
        SCREEN_HEIGHT / 20,
        Black);

        sprintf(score_text, "score: %d", buffer.score);
        tumDrawText(score_text,
        10,
        SCREEN_HEIGHT / 20 + 15,
        Black);

        // moving background regradless what happens
        // raed from queue
        if(buffer.gamer_over) {
            tumEventFetchEvents(FETCH_EVENT_NONBLOCK);
            DrawGameoverScreen(0, 0);  
            tumDrawUpdateScreen();
        }
            
        tumDrawUpdateScreen();
        vTaskDelayUntil(&last_wake_time, RENDER_FREQUENCY);
    }   
}