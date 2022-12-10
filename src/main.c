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

// handles for all the tasks
TaskHandle_t StatemachineTask = NULL;

buttons_buffer_t buttons = { 0 };

int main(int argc, char *argv[])
{
    char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]);
    printf("Initializing: ");

    if (tumDrawInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize drawing");
        goto err_init_drawing;
    }

    if (tumEventInit()) {
        PRINT_ERROR("Failed to initialize events");
        goto err_init_events;
    }

    if (tumSoundInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize audio");
        goto err_init_audio;
    }
    
    if (!buttonsInit())
        goto err_buttons_lock;

    // create queues
    
    // counting Semaphore
    
    // add states    
        AddState("Singlplayer",);
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
err_semaphores:    
err_StatemachineTask:
    vTaskDelete(StatemachineTask);
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