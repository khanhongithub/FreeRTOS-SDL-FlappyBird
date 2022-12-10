#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>

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
    
#include "main.h"
#include "statemachine.h"
#include "resources.h"

#define STATEMACHINE_FREQUENCY 80

TickType_t xLastWakeTime;


typedef struct state
{
    unsigned char id;
    bool init_flag;
    bool exit_flag;
    void (*enter) (void);
    void (*run) (void);
    void (*exit) (void);
}state_t;

typedef struct // hold the currently existing states 
{
    int state_counter;
    state_t **states; 
}state_array_t;

state_array_t state_array = { 0 };

private int OneIfStateChanged()
{
    int change_state = 0;
    static bool prev_E, cur_E = 0; // <- previous and current state of key which is either 0 or 1
   
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        cur_E = buttons.buttons[KEYCODE(E)];
        change_state = !prev_E & cur_E;  
        prev_E = cur_E;

        xSemaphoreGive(buttons.lock);
        return change_state;
    }
    return 0;
}

public bool AddState(char *task_name, 
                     void (*enter) (void), 
                     void (*run) (void), 
                     void (*exit) (void))
{
    state_t *ret = calloc(1, sizeof(state_t));
    if(!ret)
        return false;

    (ret -> enter) = enter;
    (ret -> run) = run;
    (ret -> exit) = exit;
    
    // pointer to first element of statearray = firstp
    state_array.states = realloc(state_array.states, 
                                 sizeof(state_t *) * (state_array.state_counter + 1));
    state_array.states[state_array.state_counter] = ret;
    state_array.state_counter++;

    // assigne id
    return true;
}

public void vStatemachineTask(void *pvParameters)
{
    // Initialise the xLastWakeTime variable with the current time.
    int evaluate = OneIfStateChanged();
    static int current_state = 0; 
    
    xLastWakeTime = xTaskGetTickCount();
    while(1)
    {        
        start:
        vTaskDelayUntil(&xLastWakeTime, STATEMACHINE_FREQUENCY);
        
        DEBUG_PRINT("statemachine\n");
        xGetButtonInput();
        if(state_array.state_counter == 0) { // <- prevents segfault
            DEBUG_PRINT("no states found\n");
            //exit(EXIT_SUCCESS);
            goto start;
        }
        
        // tumEventFetchEvents(FETCH_EVENT_NONBLOCK | FETCH_EVENT_NO_GL_CHECK);
        
        evaluate = OneIfStateChanged();
        state_array.states[current_state] -> exit_flag = (bool) evaluate;
        #if DEBUG
        if(evaluate)
        {
            fprintf(stderr, "\nreceived signal to exit\n exit flag of processs:%d is %d\n",
            current_state, state_array.states[current_state] -> exit_flag);
        }        
        #endif
        // main state machine
        if((state_array.states[current_state] -> init_flag) != true) {
            state_array.states[current_state] -> enter(); // <-- run code for init
            state_array.states[current_state] -> init_flag = true;
        }
            
        if((state_array.states[current_state] -> init_flag) == true) {
            state_array.states[current_state] -> run(); // <-- run code normally
        }

        if((state_array.states[current_state] -> exit_flag) == true) {
        
            state_array.states[current_state] -> exit(); // <-- run code on exit
            state_array.states[current_state] -> exit_flag = false;
            state_array.states[current_state] -> init_flag = false;
            current_state++;
            if(current_state >= state_array.state_counter) {
                DEBUG_PRINT("reached end of state list, returning to start \n");
                current_state = 0;
            }
        }   
    }
}