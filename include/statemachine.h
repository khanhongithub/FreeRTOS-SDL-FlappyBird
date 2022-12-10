#include <stdbool.h>
bool AddState(char *task_name, 
                     void (*enter) (void), 
                     void (*run) (void), 
                     void (*exit) (void));
void vStatemachineTask(void *pvParameters);