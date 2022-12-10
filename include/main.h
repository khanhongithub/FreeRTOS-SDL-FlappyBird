extern TaskHandle_t StatemachineTask;

extern TaskHandle_t RotatingTask; 

extern TaskHandle_t RenderingTask;
extern TaskHandle_t BlinkLTask;
extern TaskHandle_t BlinkRTask;

extern StaticTask_t BlinkLTaskTCB;
extern StackType_t BlinkLStack[2560];

extern TaskHandle_t CountingLTask;
extern TaskHandle_t CountingRTask;
extern TaskHandle_t CountResetTask;
extern TaskHandle_t AutomaticCounterTask;

extern SemaphoreHandle_t counter_l_sem;
extern SemaphoreHandle_t counter_r_sem;
extern SemaphoreHandle_t binary_notify_sem;
extern SemaphoreHandle_t binary_wake_task3_sem;

extern TaskHandle_t OutputTask;
extern TaskHandle_t Task1;
extern TaskHandle_t Task2;
extern TaskHandle_t Task3;
extern TaskHandle_t Task4;

extern QueueHandle_t cir_r_queue;
extern QueueHandle_t cir_l_queue;

extern QueueHandle_t event_queue;

#define DEBUG 0

#if (DEBUG == 1)
    #define DEBUG_PRINT(msg, ...)\
fprintf(stderr, "\n[DEBUG] " msg); 
#endif
#if (DEBUG == 0)
    #define DEBUG_PRINT(msg) /**/ 
#endif

#define private /*private*/
#define public /*public*/