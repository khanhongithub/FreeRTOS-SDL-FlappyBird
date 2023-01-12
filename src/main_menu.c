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
#include "main_menu.h"
#include "gui.h"
#include "multiplayer_config.h"

#define MAINMENU_FREQUENCY pdMS_TO_TICKS(10)

TaskHandle_t   MenuTask = NULL;

button_array_t  mainmenu_button_array = { .size = 0 };
button_array_t *mainmenu_button_array_ptr = &mainmenu_button_array;

image_handle_t background_sprite1 = NULL;
image_handle_t player_sprite1 = NULL;
image_handle_t start_sprite = NULL;
image_handle_t multiplayer_sprite = NULL;
image_handle_t flappydoge_sprite = NULL;


void MultiplayerConfigMode(button_t *_local_instance_)
{
   SetNextState(3);
//   MultiplayerConfigEnter();
}
void Cheats(button_t *_local_instance_)
{
   SetNextState(1);
//   MultiplayerConfigEnter();
}

void SinglePlayerMode(button_t *_local_instance_)
{
   SetNextState(2);
//   MultiplayerConfigEnter();
}


void DrawMenuScreen()
{
    
    /*
    player_sprite1 = tumDrawLoadScaledImage(PLAYER_SPRITE, 1);
    tumDrawLoadedImage(player_sprite1,
                          SCREEN_WIDTH / 2,
                          SCREEN_HEIGHT / 2 );
    
   
    start_sprite = tumDrawLoadScaledImage(START_SPRITE, 1);
    tumDrawLoadedImage(start_sprite,
                          SCREEN_WIDTH / 2 ,
                          SCREEN_HEIGHT / 2);
    
   
    multiplayer_sprite = tumDrawLoadScaledImage(MULTIPLAYER_SPRITE, 1);
    tumDrawLoadedImage(multiplayer_sprite,
                          SCREEN_WIDTH / 2 - 350,
                          SCREEN_HEIGHT / 2);
    */
    AddButton(CreateButton(Blue, White, 
                                    SCREEN_WIDTH / 2,
                                    SCREEN_HEIGHT / 2,
                                    150, 30, "Single Player", SinglePlayerMode),
                                    mainmenu_button_array_ptr);
    AddButton(CreateButton(Blue, White, 
                                    SCREEN_WIDTH / 2,
                                    SCREEN_HEIGHT / 2 + 100,
                                    150, 30, "Multiplayer", MultiplayerConfigMode),
                                    mainmenu_button_array_ptr);
    AddButton(CreateButton(Blue, White, 
                                    SCREEN_WIDTH / 2,
                                    SCREEN_HEIGHT / 2 + 200,
                                    150, 30, "Cheat", Cheats),
                                    mainmenu_button_array_ptr);
    flappydoge_sprite = tumDrawLoadScaledImage(FLAPPYDOGE_SPRITE, 0.7);
    tumDrawLoadedImage(flappydoge_sprite,
                          SCREEN_WIDTH / 2 - 130,
                          SCREEN_HEIGHT / 2 - 200);
    player_sprite1 = tumDrawLoadScaledImage(PLAYER_SPRITE, 0.1);
    tumDrawLoadedImage(player_sprite1,
                          SCREEN_WIDTH / 2 - 70,
                          SCREEN_HEIGHT / 2 - 150);
    
}


void DrawBackground1(void)
{
    short int bg_width = 0;
    static float counter = 0; 

    if(background_sprite1 == NULL) {    
        background_sprite1 = tumDrawLoadScaledImage(BACKGROUND_SPRITE, 1.15);
        bg_width = tumDrawGetLoadedImageWidth(background_sprite1);
    }
    bg_width = tumDrawGetLoadedImageWidth(background_sprite1);
    // counter has length of image width for seemless texture scrolling
    counter = counter + 0.25;
    counter *= (counter <= bg_width);
    
    tumDrawLoadedImage(background_sprite1, - counter, 0);
    tumDrawLoadedImage(background_sprite1, - counter + bg_width, 0);   
    tumDrawLoadedImage(background_sprite1, - counter + 2 * bg_width, 0);
    tumDrawLoadedImage(background_sprite1, - counter + 3 * bg_width, 0);  
    tumDrawLoadedImage(background_sprite1, - counter + 4 * bg_width, 0);    
}

void DrawMenuTask(void* pcParameters)
{
    TickType_t last_wake_time = xTaskGetTickCount();

    tumDrawBindThread();
    while (1)
    {
        tumDrawClear(White);
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK);
 
        DrawBackground1();

        DrawMenuScreen();
        UpdateButtons(mainmenu_button_array_ptr); 
        DrawButtons(mainmenu_button_array_ptr);

       
        // moving background regradless what happens
        // raed from queue  
        tumDrawUpdateScreen();
        vTaskDelayUntil(&last_wake_time, MAINMENU_FREQUENCY);
    }   
}

void MenuTaskEnter(void)
{
    xTaskCreate(DrawMenuTask, "DrawmenuTask", 
                   mainGENERIC_STACK_SIZE, 
                   NULL, mainGENERIC_PRIORITY + 1, 
                   &MenuTask) ;

}

void MenuTaskRun(void)
{}

void MenuTaskExit(void)
{
    vTaskDelete(MenuTask);
}
