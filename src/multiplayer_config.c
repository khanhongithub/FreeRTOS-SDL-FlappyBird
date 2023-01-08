#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Font.h"
#include "TUM_Utils.h"
#include "TUM_Print.h"
#include "TUM_Font.h"

#include "statemachine.h"
#include "main.h"
#include "priorties.h"
#include "multiplayer_config.h"
#include "gui.h"

#define MULTIPLAYERCONFIG_FREQUENCY pdMS_TO_TICKS(35)

#define BOTTOM_BOX_WIDTH SCREEN_WIDTH
#define BOTTOM_BOX_HEIGHT 2 * SCREEN_HEIGHT / 9

button_array_t mltplyr_config_button_array = { .size = 0 };
button_array_t *mltplyr_config_button_array_ptr = &mltplyr_config_button_array;

TaskHandle_t MultiplayerConfigTask = NULL;

void ToggleHostClient(button_t *_local_instance_)
{
    static connection_mode mode = client;

    if (mode != client)
    {
        this.main_color = Olive;
        this.button_text = "Host";
        mode = client;
    }
    else
    {
        this.main_color = Skyblue;
        this.button_text = "Client";
        mode = host;
    }
}

void EstablishConnection(button_t *_local_instance_)
{
    // fprints(stderr, "callback\n");
    static bool connected = false;
    
    if (connected)
    {
        this.main_color = Light_Green;
        connected = false;
    }
    
    else  
    {
        this.main_color = Dark_Red;
        connected = true;
    }
}

void MultiplayerInit(void) 
{

AddButton(CreateButton(BUTTON_MAIN_SET, BUTTON_BORDER, 
                                    SCREEN_WIDTH / 5,
                                    SCREEN_HEIGHT / 3,
                                    150, 30, "Host / Client", ToggleHostClient),
                                    mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN_SET, BUTTON_BORDER, 
                                    SCREEN_WIDTH / 5,
                                    SCREEN_HEIGHT / 2,
                                    150, 30, "IP:127.000.000.001", NULL),
                                    mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN_SET, BUTTON_BORDER, 
                                    SCREEN_WIDTH / 5,
                                    2 * SCREEN_HEIGHT / 3,
                                    150, 30, "Connect", EstablishConnection),
                                    mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                                    4 * SCREEN_WIDTH / 5,
                                    SCREEN_HEIGHT / 3,
                                    150, 30, "character 1", NULL),
                                    mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                                    4 * SCREEN_WIDTH / 5,
                                    SCREEN_HEIGHT / 2,
                                    150, 30, "character 2", NULL),
                                    mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                                    4 * SCREEN_WIDTH / 5,
                                    2 * SCREEN_HEIGHT / 3,
                                    150, 30, "character 3", NULL),
                                    mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
                                    SCREEN_WIDTH / 5,
                                    7 * SCREEN_HEIGHT / 8,
                                    150, 30, "back", NULL),
                                    mltplyr_config_button_array_ptr);

AddButton(CreateButton(Light_Green, BUTTON_BORDER, 
                                    4 * SCREEN_WIDTH / 5,
                                    7 * SCREEN_HEIGHT / 8,
                                    150, 30, "START", NULL),
                                    mltplyr_config_button_array_ptr);
}

void MultiplayerConfigEnter(void)
{
    static bool inited = false;

    if(xTaskCreate(vMultiplayerConfigTask, "CheatmenuTask", 
               mainGENERIC_STACK_SIZE, 
               NULL, mainGENERIC_PRIORITY, 
               &MultiplayerConfigTask) != pdPASS) {
        DEBUG_PRINT("failed to cheatmenu task\n");
    }

    if (!inited)
    {
        MultiplayerInit();
        inited = true;
    }
}

void MultiplayerConfigRun(void)
{}

void MultiplayerConfigExit(void)
{
    vTaskDelete(MultiplayerConfigTask);
}

void vMultiplayerConfigTask(void *pvArgs)
{
    
    tumDrawBindThread();
    tumDrawClear(White);

    TickType_t last_wake_time = xTaskGetTickCount();
    while (1)
    {
        tumDrawClear(Silver);
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK);

        tumDrawFilledBox(0, SCREEN_HEIGHT - BOTTOM_BOX_HEIGHT, 
                        BOTTOM_BOX_WIDTH, BOTTOM_BOX_HEIGHT, 
                        Teal);

        UpdateButtons(mltplyr_config_button_array_ptr); 
        DrawButtons(mltplyr_config_button_array_ptr);

        tumDrawUpdateScreen();
        vTaskDelayUntil(&last_wake_time, MULTIPLAYERCONFIG_FREQUENCY);
    }
    
}

