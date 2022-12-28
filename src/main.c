/**
 * @file timef.c
 * @author Nino-tum
 * @date November 2022
 * @brief  main of the project which initializes task and scheuduler
 */

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
#include "cheatmenu.h"

// handles for all the tasks
TaskHandle_t StatemachineTask = NULL;

QueueHandle_t scene_queue = NULL;

SemaphoreHandle_t restart_signal_singleplayer = NULL;

buttons_buffer_t buttons = { 0 };

int main(int argc, char *argv[])
{
    char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]);
    printf("Initializing: ");

    if (tumDrawInit(bin_folder_path)) {
        DEBUG_PRINT("Failed to initialize drawing");
        goto err_init_drawing;
    }

    if (tumEventInit()) {
        DEBUG_PRINT("Failed to initialize events");
        goto err_init_events;
    }

    if (tumSoundInit(bin_folder_path)) {
        DEBUG_PRINT("Failed to initialize audio");
        goto err_init_audio;
    }
    
    if (!buttonsInit())
        goto err_buttons_lock;

    if (!CheatLockInit())
        goto err_buttons_lock;

    if (!ScoreLockInit())
        goto err_buttons_lock;

    if (!StateLockInit())
        goto err_buttons_lock;
    
    // create queues
    scene_queue = xQueueCreate(1,sizeof(game_data_t));
    if(scene_queue == NULL) {
        DEBUG_PRINT("failed to create scene queue");
        goto err_queue;
    }

    restart_signal_singleplayer = xSemaphoreCreateBinary();
    if(restart_signal_singleplayer == NULL) {
        DEBUG_PRINT("failed to create semaphore");
        goto err_semaphore;
    }
    
    // counting Semaphore
    
    // add states    
        // main menu
        AddState("Singlplayer", SingleplayerEnter, SingleplayerRun, SingleplayerExit);
        // multiplayer  
        AddState("Cheatmenu", CheatmenuEnter, CheatmenuRun, CheatmenuExit);
    // create statemachine

    #if 1
    if(xTaskCreate(vStatemachineTask, "StatemachineTask", 
                   STATEMACHINE_STACK_SIZE, 
                   NULL, STATEMACHINE_PRIORTY, 
                   &StatemachineTask) != pdPASS) {
        goto err_StatemachineTask;
    }
    #endif
    
    vTaskStartScheduler();
    
    return EXIT_SUCCESS;
//err_semaphores:    
err_StatemachineTask:
    vTaskDelete(StatemachineTask);
err_semaphore:
err_queue:
err_buttons_lock:
    tumSoundExit();
err_init_audio:
    tumEventExit();
err_init_events:
    tumDrawExit();
err_init_drawing:
vSemaphoreDelete(buttons.lock);
    return EXIT_FAILURE;
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vMainQueueSendPassed(void)
{
    /* This is just an example implementation of the "queue send" trace hook. */
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
    struct timespec xTimeToSleep, xTimeSlept;
    /* Makes the process more agreeable when using the Posix simulator. */
    xTimeToSleep.tv_sec = 1;
    xTimeToSleep.tv_nsec = 0;
    nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}