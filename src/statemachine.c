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
#include "TUM_Print.h"
#include "TUM_Utils.h"
#include "TUM_Font.h" 
    
#include "main.h"
#include "statemachine.h"
#include "resources.h"

#define STATEMACHINE_FREQUENCY 30

TickType_t xLastWakeTime;

next_state_t next_state;

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

bool StateLockInit(void)
{
    next_state.lock = xSemaphoreCreateMutex(); // Mutex for access to button queue
    
    if (!next_state.lock) {
        fprintf(stderr, "Failed to create next state lock");
        return false;
    }
    return true;
}

short int GetNextState(void)
{
    static short int next_state_buffer;

    assert(&next_state != NULL);
    if (xSemaphoreTake(next_state.lock, 0)) {
        //fprints(stderr, "the next state is %d\n", next_state.next_state);
        next_state_buffer = next_state.next_state;
        next_state_buffer %= state_array.state_counter; // <- rollover in case of 
                                                        // invalid index
        xSemaphoreGive(next_state.lock);
    }
    return next_state_buffer;
}

bool SetNextState(short int next_state_to_be_set)
{
    #if DEBUG
    fprints(stderr, "settingnext state to %d\n", next_state_to_be_set);
    #endif

    if (xSemaphoreTake(next_state.lock, portMAX_DELAY)) {
        next_state.next_state = next_state_to_be_set;
        xSemaphoreGive(next_state.lock);
        return true;
    }
    return false;
}

bool OneIfStateChanged(void)
{
    bool change_state = false;
    static short int prev_state, cur_state;
    assert(&next_state != NULL);

    if (xSemaphoreTake(next_state.lock, 0) == pdPASS) {
        cur_state = next_state.next_state;
        change_state = prev_state != cur_state;
        prev_state = cur_state; 
        xSemaphoreGive(next_state.lock);
        #if DEBUG
        fprints(stderr, "=======pre: %d cur: %d\n", prev_state, cur_state);
        #endif
    }
    return change_state;
}

bool AddState(char *task_name, 
                     void (*enter) (void), 
                     void (*run) (void), 
                     void (*exit) (void))
{
    state_t *ret = calloc(1, sizeof(state_t));
    if(!ret)
        return false;

    (ret->enter) = enter;
    (ret->run) = run;
    (ret->exit) = exit;
    
    // pointer to first element of statearray = firstp
    state_array.states = realloc(state_array.states, 
                                 sizeof(state_t *) * (state_array.state_counter + 1));
    state_array.states[state_array.state_counter] = ret;
    state_array.state_counter++;

    return true;
}

void vStatemachineTask(void *pvParameters)
{

    bool state_changed = OneIfStateChanged();
    static int short current_state = 0; 
    
    xLastWakeTime = xTaskGetTickCount();
    while(1)
    {        
        start:
        vTaskDelayUntil(&xLastWakeTime, STATEMACHINE_FREQUENCY);
        
        DEBUG_PRINT("statemachine\n");
        xGetButtonInput();
        if(state_array.state_counter == 0) {
            DEBUG_PRINT("no states found\n");
            // exit(EXIT_SUCCESS);
            goto start;
        }
        
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK | FETCH_EVENT_NO_GL_CHECK);
        

        if (current_state != GetNextState())
        {
            #if DEBUG
            fprints(stderr, "#####next state is differnet that current state\n");            
            #endif
            state_array.states[current_state] -> exit_flag = true;
            #if DEBUG
            fprints(stderr, "\n exiting\n exit flag of processs:%d is %d\n",
            current_state, state_array.states[current_state] -> exit_flag);
            #endif
        }

        // main state machine

        // enter
        if((state_array.states[current_state] -> init_flag) != true) {
            state_array.states[current_state] -> enter();
            state_array.states[current_state] -> init_flag = true;
        }
        
        // run
        if((state_array.states[current_state] -> init_flag) == true) {
            state_array.states[current_state] -> run();
        }

        // exit
        if((state_array.states[current_state] -> exit_flag) == true) {
            state_array.states[current_state] -> exit();
            state_array.states[current_state] -> exit_flag = false;
            state_array.states[current_state] -> init_flag = false;
        
            current_state = GetNextState();   
            #if DEBUG 
            fprints(stderr, "-----new state: %d\n", current_state);
            fprints(stderr, "///////exiting\n");
            #endif

            if(current_state >= state_array.state_counter) {
                #if DEBUG
                fprints(stderr, "reached end of state list, returning to start \n");
                #endif
                current_state = 0;
            }
        }   
    }
}