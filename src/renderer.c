#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
#include "gui.h"

#define RENDER_FREQUENCY pdMS_TO_TICKS(5)
#define USUAL_POS -1

TaskHandle_t RenderingTask = NULL;

image_handle_t player_sprite = NULL;
image_handle_t pipe_bottom = NULL;
image_handle_t pipe_top = NULL;
image_handle_t background_sprite = NULL;
image_handle_t gameover_sprite = NULL;

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
    }

    if(pipe_bottom == NULL) 
    {
        pipe_bottom = tumDrawLoadScaledImage(PIPE_BOTTOM_SPRITE, 1.98);
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
    if((image_height_top_pipe = tumDrawGetLoadedImageHeight(pipe_top)) != -1) {
        tumDrawLoadedImage(pipe_top, 
                           x_position - counter - 2, /* pipe image isnt centered*/
                           top_length - (middle_gap / 2) - image_height_top_pipe);
    }

    // bottom pipe
    if((image_height_bottom_pipe = tumDrawGetLoadedImageHeight(pipe_bottom)) != -1) {
        tumDrawLoadedImage(pipe_bottom, 
                           x_position - counter - 2, /* pipe image isnt centered*/
                           SCREEN_HEIGHT - bottom_length + (middle_gap / 2) ); 
    }
} 

void DrawGameoverScreen(short int high_score, short int score)
{
    char high_score_text[30];
    char score_text[30];
    static int image_height, image_width;
    if(gameover_sprite == NULL) {
        gameover_sprite = tumDrawLoadScaledImage(GAME_OVER_SPRITE, 0.25);
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
                          SCREEN_HEIGHT / 3 - image_height / 2);

        sprintf(high_score_text, "best: %d", high_score);
        sprintf(score_text, "score: %d", score);
        
        tumDrawText(high_score_text, 
                    5 * SCREEN_WIDTH / 8,
                    SCREEN_HEIGHT / 2 + 15,
                    Black);

        tumDrawText(score_text, 
                    5 * SCREEN_WIDTH / 8,
                    SCREEN_HEIGHT / 2 - 15,
                    Black);
    }         
}

spritesheet_handle_t jump_spritesheet = NULL;
sequence_handle_t forward_sequence = NULL;

void InitDrawPlayersprite(void)
{
    static bool inited = false;
    if (!inited)
    {
        char *jump_spritesheet_path = tumUtilFindResourcePath(JUMP_ANIMATION_SPRITE);
        image_handle_t jump_spritesheet_image =
            tumDrawLoadScaledImage(jump_spritesheet_path, 1.00);
        
        jump_spritesheet =
            tumDrawLoadSpritesheet(jump_spritesheet_image, 8, 1);    
        
        animation_handle_t jump_animation =
            tumDrawAnimationCreate(jump_spritesheet);
        
        tumDrawAnimationAddSequence(jump_animation, "FORWARDS", 0, 0,
                                    SPRITE_SEQUENCE_HORIZONTAL_POS, 8);
        forward_sequence =
            tumDrawAnimationSequenceInstantiate(jump_animation, "FORWARDS", 300);

        inited = true;
    }
}

void DrawPlayer(TickType_t xLastFrameTime, int player_x, int player_y)
{
    if (player_x == -1)
    {
        player_x = FIRST_POSITION + 3 * PLAYER_RADIUS + 3 * OBSTACLE_WIDTH + 5;
    }
    tumDrawAnimationDrawFrame(
        forward_sequence,
        xTaskGetTickCount() - xLastFrameTime,
        player_x, player_y - 3 - 2 * PLAYER_RADIUS);
}

void DrawPlayerHitBox(short int player_height, int color)
{
    if(player_sprite == NULL) 
    {
        player_sprite = tumDrawLoadScaledImage(PLAYER_SPRITE, 0.09);
    }

    tumDrawCircle(FIRST_POSITION + PLAYER_RADIUS + 
                  OBSTACLE_WIDTH * STANDART_GRID_LENGTH, 
                   player_height, PLAYER_RADIUS, Orange);
}

void RendererEnter(void)
{
    static bool inited = false;
    if(xTaskCreate(vRendererTask, "RendererTask", 
                   mainGENERIC_STACK_SIZE, 
                   NULL, mainGENERIC_PRIORITY + 1, 
                   &RenderingTask) != pdPASS) {
        DEBUG_PRINT("failed to create rendering task\n");
    }

    if (!inited)
    {
        InitDrawPlayersprite();
        inited = true;
    }
    
}

void RestarGameSinglePlayer(button_t *_local_instance_)
{
    xSemaphoreGive(restart_signal_singleplayer);
}

void ExitSinglePlayer(button_t  *_local_instance_)
{
    SetNextState(0);
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
    
    buffer.gamer_over = false;
    bool played_sound = false;

    button_array_t gameover_buttons = { .size = 0 };
    button_array_t *gameover_buttons_ptr = &gameover_buttons;
    
    AddButton(CreateButton(0xe6611e, 0x552F05, 
                                    SCREEN_WIDTH / 3,
                                    SCREEN_HEIGHT / 2 - 30,
                                    100, 30, "Restart", RestarGameSinglePlayer),
                                    gameover_buttons_ptr);
    AddButton(CreateButton(0xe6611e, 0x552F05, 
                                    SCREEN_WIDTH / 3,
                                    SCREEN_HEIGHT / 2 + 30,
                                    100, 30, "Exit", ExitSinglePlayer), 
                                    gameover_buttons_ptr);

    TickType_t last_wake_time = xTaskGetTickCount();
    TickType_t last_wake_time_animation = xTaskGetTickCount();
    
    xQueueOverwrite(scene_queue, &buffer);

    tumDrawBindThread();
    while (1)
    {
        tumDrawClear(White);
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK);
        xQueuePeek(scene_queue, &buffer, 0);
        
        DrawBackground();

        tumDrawFilledBox(0 , SCREEN_HEIGHT - FLOOR_HEIGTH, 
                            SCREEN_WIDTH, FLOOR_HEIGTH, 0xDED798);

        tumDrawFilledBox(0 , SCREEN_HEIGHT - FLOOR_HEIGTH, 
                            SCREEN_WIDTH, 1, 0x543847);

        DrawObstacle(FIRST_POSITION, (buffer.obstacles & 0xF000) >> 12, 
                     buffer.global_counter);

        DrawObstacle(SECOND_POSITION, (buffer.obstacles & 0x0F00) >> 8, 
                     buffer.global_counter);

        DrawObstacle(THIRD_POSITION, (buffer.obstacles & 0x00F0) >> 4, 
                     buffer.global_counter);

        DrawObstacle(FOURTH_POSITION, (buffer.obstacles & 0x000F) >> 0, 
                     buffer.global_counter);
        #if DEBUG
            DrawPlayerHitBox(buffer.player1_position, Orange);
        #endif

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
        
        DrawPlayer(last_wake_time_animation, USUAL_POS, buffer.player1_position); 
        last_wake_time_animation = xTaskGetTickCount();

        if(buffer.jump && !tumSoundLoadUserSample(JUMP_SOUND)) {
                tumSoundPlayUserSample(JUMP_SOUND);
        }

        // moving background regradless what happens
        // raed from queue
        if(buffer.gamer_over) {
            if(!tumSoundLoadUserSample(DEATH_SOUND) && !played_sound) {
                tumSoundPlayUserSample("../resources/waveforms/death.wav");
                played_sound = true;            
            }

            tumEventFetchEvents(FETCH_EVENT_NONBLOCK);
            DrawGameoverScreen(buffer.highscore, buffer.score);
            UpdateButtons(gameover_buttons_ptr);
            DrawButtons(gameover_buttons_ptr);
            tumDrawUpdateScreen();
        }

        // if sound has already been play and game has been restarted
        // sound_played flag is reset
        played_sound = buffer.gamer_over && played_sound;

        tumDrawUpdateScreen();
        vTaskDelayUntil(&last_wake_time, RENDER_FREQUENCY);
    }   
}