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
#define SPRITE_PADDING 8

#define BUTTON_DEATH_W 100
#define BUTTON_DEATH_H 30

TaskHandle_t RenderingTask = NULL;

image_handle_t player_sprite = NULL;
image_handle_t pipe_bottom = NULL;
image_handle_t pipe_top = NULL;
image_handle_t background_sprite = NULL;
image_handle_t gameover_sprite = NULL;
image_handle_t doge_death_sprite = NULL;
image_handle_t get_ready_sprite = NULL;

void DrawBackground(void)
{
    short int bg_width = 0;
    static float counter = 0; 

    if (background_sprite == NULL) {
        background_sprite = tumDrawLoadImage(BACKGROUND_SPRITE);
        bg_width = tumDrawGetLoadedImageWidth(background_sprite);
    }

    bg_width = tumDrawGetLoadedImageWidth(background_sprite);
    
    // counter has length of image width for seemless texture scrolling
    counter = counter + 0.25;
    counter *= (counter <= bg_width);
    
    for (int i = 0; i < 5; i++)
        tumDrawLoadedImage(background_sprite, - counter + i * bg_width, 0);
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
    static image_handle_t medal_bronze_sprite = NULL;    
    static image_handle_t medal_silver_sprite = NULL;
    static image_handle_t medal_gold_sprite = NULL;    
    static image_handle_t medal_platin_sprite = NULL;
    
    char high_score_text[30];
    char score_text[30];
    static int image_height, image_width;
    if(gameover_sprite == NULL) {
        gameover_sprite = tumDrawLoadScaledImage(GAME_OVER_SPRITE, 0.25);
    } 
    
    if (medal_bronze_sprite == NULL || medal_silver_sprite == NULL || 
        medal_gold_sprite == NULL || medal_platin_sprite == NULL)
    {
        medal_bronze_sprite = tumDrawLoadImage(MEDAL_BRONZE_SPRITE);
        medal_silver_sprite = tumDrawLoadImage(MEDAL_SILVER_SPRITE);
        medal_gold_sprite = tumDrawLoadImage(MEDAL_GOLD_SPRITE);
        medal_platin_sprite = tumDrawLoadImage(MEDAL_PLATIN_SPRITE);
    }

    tumDrawFilledBox(SCREEN_WIDTH / 2 - (GAMEOVER_BOX_WIDTH / 2), 
                                 (SCREEN_HEIGHT / 2) - (GAMEOVER_BOX_HEIGHT / 2), 
                                 GAMEOVER_BOX_WIDTH, 
                                 GAMEOVER_BOX_HEIGHT, 
                                 SGLPLY_MENU_BORDER);

    tumDrawFilledBox((SCREEN_WIDTH / 2) - (GAMEOVER_BOX_WIDTH_CONTENT / 2), 
                                 SCREEN_HEIGHT / 2 - 
                                 (GAMEOVER_BOX_HEIGHT_CONTENT / 2), 
                                 GAMEOVER_BOX_WIDTH_CONTENT, 
                                 GAMEOVER_BOX_HEIGHT_CONTENT, 
                                 SGLPLY_MENU_MAIN);

    switch (score)
    {
    case 0 ... 9:
        break;
    
    case 10 ... 19:
        tumDrawLoadedImage(medal_bronze_sprite, MEDAL_X, MEDAL_Y);
        break;

    case 20 ... 29:
        tumDrawLoadedImage(medal_silver_sprite, MEDAL_X, MEDAL_Y);
        break;
        
    case 30 ... 39:
        tumDrawLoadedImage(medal_gold_sprite, MEDAL_X, MEDAL_Y);
        break;
    
    default: // 40 and beyond
        tumDrawLoadedImage(medal_platin_sprite, MEDAL_X, MEDAL_Y);
        break;
    }

    if((image_height = tumDrawGetLoadedImageHeight(gameover_sprite)) != -1 &&
       (image_width = tumDrawGetLoadedImageWidth(gameover_sprite)) != -1) {
       
       tumDrawLoadedImage(gameover_sprite, SCREEN_WIDTH / 2 - image_width / 2,
                          SCREEN_HEIGHT / 6 - image_height / 2);

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

void DrawPlayer(TickType_t xLastFrameTime, int player_x, int player_y, bool dead,
                short int speed)
{
    if (player_x == -1)
    {
        player_x = FIRST_POSITION + 3 * PLAYER_RADIUS + 3 * OBSTACLE_WIDTH + 5;
    }

    if (doge_death_sprite == NULL)
    {
        doge_death_sprite = tumDrawLoadImage(DOGE_DEATH_SPRITE);
    }

    if (dead)
    {
        tumDrawLoadedImage(doge_death_sprite, player_x, 
                            player_y - SPRITE_PADDING - 2 * PLAYER_RADIUS);
    }
    else
    {
        tumDrawAnimationDrawFrame(forward_sequence,
                                  xTaskGetTickCount() - xLastFrameTime, player_x, 
                                  player_y - SPRITE_PADDING - 2 * PLAYER_RADIUS);

    }
}

void DrawPlayerHitBox(short int player_height, int color)
{
    tumDrawFilledBox(FIRST_POSITION + 
                     OBSTACLE_WIDTH * STANDART_GRID_LENGTH,
                     player_height - PLAYER_RADIUS , 
                     2 * PLAYER_RADIUS, 2 * PLAYER_RADIUS,
                     Red);
}

void DrawScores(game_data_t *buffer) 
{
    char highscore_text[30];
    char score_text[30];
        sprintf(highscore_text, "high score: %d", buffer->highscore);
        tumDrawText(highscore_text,
                    10,
                    SCREEN_HEIGHT / 20,
                    Black);

        sprintf(score_text, "score: %d", buffer->score);
        tumDrawText(score_text,
                    10,
                    SCREEN_HEIGHT / 20 + 15,
                    Black);
}

void DrawPauseMenu(void)
{
    tumDrawFilledBox(SCREEN_WIDTH / 2 - (PAUSE_BOX_WIDTH / 2), 
                     (SCREEN_HEIGHT / 2) - (PAUSE_BOX_HEIGTH / 2), 
                     PAUSE_BOX_WIDTH, 
                     PAUSE_BOX_HEIGTH, 
                     SGLPLY_MENU_BORDER);

    tumDrawFilledBox((SCREEN_WIDTH / 2) - (PAUSE_BOX_WIDTH_CONTENT / 2), 
                     SCREEN_HEIGHT / 2 - 
                     (PAUSE_BOX_HEIGTH_CONTENT / 2), 
                     PAUSE_BOX_WIDTH_CONTENT, 
                     PAUSE_BOX_HEIGTH_CONTENT, 
                     SGLPLY_MENU_MAIN);
    tumDrawCenteredText("Pause", SCREEN_WIDTH / 2, 
                        SCREEN_HEIGHT / 3 + PAUSE_MENU_PADDING, Black);
}

void DrawFloor(void)
{
    static image_handle_t floor_sprite = NULL;
    static int counter = 0;
    if (floor_sprite == NULL)
    {
        floor_sprite = tumDrawLoadImage(FLOOR_SPRITE);
    }
    
    tumDrawLoadedImage(floor_sprite, -counter , SCREEN_HEIGHT - FLOOR_HEIGTH);
    tumDrawLoadedImage(floor_sprite, -counter + SCREEN_WIDTH, 
                                        SCREEN_HEIGHT - FLOOR_HEIGTH);
    counter++;
    if (counter >= SCREEN_WIDTH)
    {
        counter = 0;
    }
}

void DrawGetReady(void)
{
    static int image_h = 0, image_w = 0;
    if (get_ready_sprite == NULL || image_h == 0 || image_w == 0)
    {
        get_ready_sprite = tumDrawLoadImage(GET_READY_SPRITE);
        tumGetImageSize(GET_READY_SPRITE, &image_w, &image_h);
    }
    tumDrawLoadedImage(get_ready_sprite, (SCREEN_WIDTH / 2 - image_w / 2), 
                       (SCREEN_HEIGHT / 5 - image_h / 2));
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

void RestartGameSinglePlayer(button_t *_local_instance_)
{
    xSemaphoreGive(restart_signal_singleplayer);
}

void ResumeGameSinglePlayer(button_t *_local_instance_)
{
    xSemaphoreGive(resume_signal_singleplayer);
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
    
    buffer.gamer_over = false;
    buffer.pause = false;
    bool played_sound = false;

    static button_array_t gameover_buttons = { .size = 0 };
    static button_array_t *gameover_buttons_ptr = &gameover_buttons;
    
    AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                                    SCREEN_WIDTH / 3,
                                    SCREEN_HEIGHT / 2 - 30,
                                    BUTTON_DEATH_W, BUTTON_DEATH_H, "Restart", 
                                    RestartGameSinglePlayer),
                                    gameover_buttons_ptr);
    AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                                    SCREEN_WIDTH / 3,
                                    SCREEN_HEIGHT / 2 + 30,
                                    BUTTON_DEATH_W, BUTTON_DEATH_H, "Exit", 
                                    ExitSinglePlayer), 
                                    gameover_buttons_ptr);

    static button_array_t resume_buttons = { .size = 0 };
    static button_array_t *resume_buttons_ptr = &resume_buttons;
    
    AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                                    SCREEN_WIDTH / 2,
                                    SCREEN_HEIGHT / 2 - 30,
                                    BUTTON_DEATH_W, BUTTON_DEATH_H, "Resume", 
                                    ResumeGameSinglePlayer),
                                    resume_buttons_ptr);
    AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                                    SCREEN_WIDTH / 2,
                                    SCREEN_HEIGHT / 2 + 30,
                                    BUTTON_DEATH_W, BUTTON_DEATH_H, "Exit", 
                                    ExitSinglePlayer), 
                                    resume_buttons_ptr);

    TickType_t last_wake_time = xTaskGetTickCount();
    TickType_t last_wake_time_animation = xTaskGetTickCount();
    
    xQueueOverwrite(scene_queue, &buffer);

    tumDrawBindThread();
    while (1)
    {        
        while (buffer.pause)
        {
            xQueuePeek(scene_queue, &buffer, 0);
            DrawPauseMenu();
            DrawScores(&buffer);
            UpdateButtons(resume_buttons_ptr);
            DrawButtons(resume_buttons_ptr);
            tumDrawUpdateScreen();
            vTaskDelayUntil(&last_wake_time, RENDER_FREQUENCY);
        }

        tumDrawClear(White);
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK);
        xQueuePeek(scene_queue, &buffer, 0);

        DrawBackground();        
        DrawFloor();

        DrawObstacle(FIRST_POSITION, (buffer.obstacles & 0xF000) >> 12, 
                     buffer.global_counter);

        DrawObstacle(SECOND_POSITION, (buffer.obstacles & 0x0F00) >> 8, 
                     buffer.global_counter);

        DrawObstacle(THIRD_POSITION, (buffer.obstacles & 0x00F0) >> 4, 
                     buffer.global_counter);

        DrawObstacle(FOURTH_POSITION, (buffer.obstacles & 0x000F), 
                     buffer.global_counter);
        #if DEBUG
            DrawPlayerHitBox(buffer.player1_position, Orange);
        #endif

        DrawScores(&buffer);
        DrawPlayer(last_wake_time_animation, USUAL_POS, buffer.player1_position, 
                    buffer.gamer_over, 0); 
        last_wake_time_animation = xTaskGetTickCount();

        if(buffer.jump && !tumSoundLoadUserSample(JUMP_SOUND)) {
                tumSoundPlayUserSample(JUMP_SOUND);
        }

        if (buffer.waiting) {
            DrawGetReady();
        }

        // moving background regradless what happens
        // raed from queue
        if(buffer.gamer_over) {
            if(!tumSoundLoadUserSample(DEATH_SOUND) && !played_sound) {
                tumSoundPlayUserSample("../resources/waveforms/death.wav");
                played_sound = true;            
            }

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