/*
    custom state machine used to transition between states
*/
#ifndef _STDBOOL_H
#include <stdbool.h>
#endif

typedef struct next_state_s {
    SemaphoreHandle_t lock;
    short int next_state;
}next_state_t;

bool StateLockInit(void);
bool SetNextState(short int next_state);

bool AddState(char *task_name, 
                     void (*enter) (void), 
                     void (*run) (void), 
                     void (*exit) (void));
void vStatemachineTask(void *pvParameters);