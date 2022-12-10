/*
* @brief this should be used to access resources
* which are protected by locks, semaphores and mutexes 
*
*/
#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Event.h"
#include "TUM_Draw.h"
#include "resources.h"

// init mouse with proper values

// buttons
void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }    
}

bool buttonsInit(void)
{
    buttons.lock = xSemaphoreCreateMutex(); // Mutex for access to button queue
    if (!buttons.lock) {
        fprintf(stderr, "Failed to create buttons lock");
        return false;
    }
    return true;
}