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

#include "statemachine.h"
#include "main.h"
#include "priorties.h"
#include "resources.h"
#include "renderer.h"
#include "singleplayer.h" // to use the background function
#include "main_menu.h"
#include "gui.h"

#define MAINMENU_FREQUENCY pdMS_TO_TICKS(25)

TaskHandle_t MenuTask = NULL;

button_array_t  mainmenu_button_array = { .size = 0 };
button_array_t *mainmenu_button_array_ptr = &mainmenu_button_array;

image_handle_t background_sprite1 = NULL;
image_handle_t player_sprite1 = NULL;
image_handle_t start_sprite = NULL;
image_handle_t multiplayer_sprite = NULL;
image_handle_t title_sprite = NULL;

void SinglePlayerMode(button_t *_local_instance_)
{
   SetNextState(1);
}

void MultiplayerConfigMode(button_t *_local_instance_)
{
   SetNextState(2);
}

void Cheats(button_t *_local_instance_)
{
   SetNextState(3);
}

void ExitGame(button_t *_local_instance_)
{
    exit(EXIT_SUCCESS);
}

void MenuInit()
{
    static bool inited = false;

    if (!inited) 
    {
        AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                               SCREEN_WIDTH / 2,
                               BUTTON_START_Y,
                               BUTTON_MAIN_MENU_W, BUTTON_MAIN_MENU_H, 
                               "Single Player", SinglePlayerMode),
                               mainmenu_button_array_ptr);

        AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                               SCREEN_WIDTH / 2,
                               BUTTON_START_Y + 75,
                               BUTTON_MAIN_MENU_W, BUTTON_MAIN_MENU_H, 
                               "Multiplayer", MultiplayerConfigMode),
                               mainmenu_button_array_ptr);

        AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                               SCREEN_WIDTH / 2,
                               BUTTON_START_Y + 150,
                               BUTTON_MAIN_MENU_W, BUTTON_MAIN_MENU_H, 
                               "Cheat Menu", Cheats),
                               mainmenu_button_array_ptr);

        AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                               SCREEN_WIDTH / 2,
                               BUTTON_START_Y + 225,
                               BUTTON_MAIN_MENU_W, BUTTON_MAIN_MENU_H, 
                               "Exit", ExitGame),
                               mainmenu_button_array_ptr);

        title_sprite = tumDrawLoadImage(TITLE_SPRITE);

        player_sprite1 = tumDrawLoadScaledImage(PLAYER_SPRITE, 0.1);

        inited = true;
    }
}

void DrawMenuScreen(void)
{   
    static int image_h = 0, image_w = 0;
    tumGetImageSize(TITLE_SPRITE, &image_w, &image_h);
    tumDrawLoadedImage(title_sprite,
                       (SCREEN_WIDTH / 2) - (image_w / 2),
                       (SCREEN_HEIGHT / 5) - image_h / 2);
}

void vDrawMenuTask(void* pcParameters)
{
    TickType_t last_wake_time = xTaskGetTickCount();
    TickType_t last_wake_time_animation = xTaskGetTickCount();

    InitDrawPlayersprite();

    tumDrawBindThread();
    while (1)
    {
        tumDrawClear(White);
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK);
 
        DrawBackground();
        DrawMenuScreen();

        DrawPlayer(last_wake_time_animation, SCREEN_WIDTH / 5, 
                    11 * SCREEN_HEIGHT / 15, false, 0);
        last_wake_time_animation = xTaskGetTickCount();

        UpdateButtons(mainmenu_button_array_ptr); 
        DrawButtons(mainmenu_button_array_ptr);

        tumDrawUpdateScreen();
        vTaskDelayUntil(&last_wake_time, MAINMENU_FREQUENCY);
    }   
}

void MenuTaskEnter(void)
{
    xTaskCreate(vDrawMenuTask, "DrawmenuTask", 
                mainGENERIC_STACK_SIZE, 
                NULL, mainGENERIC_PRIORITY, &MenuTask);
    MenuInit();

}

void MenuTaskRun(void)
{}

void MenuTaskExit(void)
{
    vTaskDelete(MenuTask);
}
