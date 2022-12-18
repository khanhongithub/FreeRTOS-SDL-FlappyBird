extern TaskHandle_t StatemachineTask;

extern QueueHandle_t scene_queue;

extern SemaphoreHandle_t restart_signal_singleplayer;

#define DEBUG 0
#if (DEBUG == 1)
    #define DEBUG_PRINT(msg, ...)\
fprintf(stderr, "\n[DEBUG] " msg); 
#endif
#if (DEBUG == 0)
    #define DEBUG_PRINT(msg) /**/ 
#endif // debug
#define private /*private*/
#define public /*public*/
