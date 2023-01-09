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

void vInputForIP(void *pvParameters);
void InputForIPStart(void);
void InputForIPStop(void);

button_array_t mltplyr_config_button_array = { .size = 0 };
button_array_t *mltplyr_config_button_array_ptr = &mltplyr_config_button_array;

enum ip_entering_status {    
        untouched,
        in_progress,
        correct,
        incorrect
    };

TaskHandle_t MultiplayerConfigTask = NULL;

TaskHandle_t IPEnteringTask = NULL;

void vInputForIP(void *pvParameters)
{
    #define RISING_EDGE_VARS(x) \
        static bool cur_##x, prev_##x, rising_edge_##x = false;

    #define RISING_EDGE(x) \
    if (xSemaphoreTake(buttons.lock, 0) == pdPASS) \
    { \
        cur_##x = buttons.buttons[KEYCODE(#x)]; \
        rising_edge_##x = !prev_##x && cur_##x; \
        prev_##x = cur_##x; \
        xSemaphoreGive(buttons.lock); \
    }

    #define IP_INPUT_DEBOUNCE_KEYS() \
    RISING_EDGE(0); \
    RISING_EDGE(1); \
    RISING_EDGE(2); \
    RISING_EDGE(3); \
    RISING_EDGE(4); \
    RISING_EDGE(5); \
    RISING_EDGE(6); \
    RISING_EDGE(7); \
    RISING_EDGE(8); \
    RISING_EDGE(9); \
    RISING_EDGE(.); \
    RISING_EDGE(ESCAPE)

    #define IP_INPUT_GENERATE_DEBOUNCING_VARS() \
    RISING_EDGE_VARS(0); \
    RISING_EDGE_VARS(1); \
    RISING_EDGE_VARS(2); \
    RISING_EDGE_VARS(3); \
    RISING_EDGE_VARS(4); \
    RISING_EDGE_VARS(5); \
    RISING_EDGE_VARS(6); \
    RISING_EDGE_VARS(7); \
    RISING_EDGE_VARS(8); \
    RISING_EDGE_VARS(9); \
    RISING_EDGE_VARS(.); \
    RISING_EDGE_VARS(ESCAPE)
    
    IP_INPUT_GENERATE_DEBOUNCING_VARS();

    TickType_t last_wake_time = xTaskGetTickCount();

    while (1)
    {
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK | FETCH_EVENT_NO_GL_CHECK);
        xGetButtonInput();

        IP_INPUT_DEBOUNCE_KEYS();
        
        // check for rising edges and input them into IP constructer
         
        vTaskDelayUntil(&last_wake_time, MULTIPLAYERCONFIG_FREQUENCY);
    }
    
}

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

    // try to establish connection and see if it works
}

// functions for for reading in ip


void RemoveChar(char* s, char c)
{

    int j;
    int n = strlen(s);
    for (int i = j = 0; i < n; i++)
    {
        if (s[i] != c)
            s[j++] = s[i];
    }
    s[j] = '\0';
}

bool IPCreate(char input, ipv4 output)
{
    static ipv4 ip_temp = "xxx.xxx.xxx.xxx\0";
    static int8_t index = 0;
    bool done = false;
    short int temp_sub_value = 0;

    // if . is pressed section is considered done
    // and index is placed at the start of of next section
    if (input == '.') {
	    switch (index) {
	    case 0 ... 3:
	        index = 4;
	        break;
	    case 4 ... 7:
	        index = 8;
	        break;
	    case 8 ... 11:
	        index = 12;
	        break;
	    case 12 ... 14:
	        index = 14;
	        done = true;
	        break;
	    default:
	        break;
	    }
    } else if (isdigit(input) && index <= 14) {
	    switch (index) {
	    case 3:
	    case 7:
	    case 11:
	        // do nothing
	        break;

	    case 14:
	        ip_temp[14] = input;
	        done = true;
	        break;

	    default:
	        ip_temp[index] = input;
	        index++;
	        break;
	    }
    }

    strcpy(output, ip_temp);
    printf("index:%d\n\n", index);

    if (done) {
	    for (int i = 0; i < 4; i++) {
	        if (ip_temp[i * 4] == 'x')
	    	    ip_temp[i * 4] = '0';
	        else if (isdigit(ip_temp[i * 4 + 2])) {
	    	    // extract subpart (.123.) from ip
	    	    temp_sub_value = (ip_temp[i * 4] - '0') * 100 +
	                (ip_temp[i * 4 + 1] - '0') * 10 +
	                ip_temp[i * 4 + 2] - '0';

	    	    // the subpart is 8bit so it can
	    	    // only be 255 max, otherwise it is reset
	    	    if (temp_sub_value > 255) {
	    		    ip_temp[i * 4] = '2';
	    		    ip_temp[i * 4 + 1] = '5';
	    		    ip_temp[i * 4 + 2] = '5';
	    	    }
	        }
	    }
	    // purging placeholders
	    RemoveChar(ip_temp, 'x');
	    strcpy(output, ip_temp);

	    // reseting static values
	    strcpy(ip_temp, "xxx.xxx.xxx.xxx\0");
	    index = 0;
    }
    return done;
}

void ReadIP(button_t *_local_instance_)
{
    ip_entering_status = untouched;

    switch (ip_entering_status)
    {
    case untouched:
        this.main_color = Passive_Yellow;
        ip_entering_status = in_progress;
        // start listener
        break;
    case in_progress:
        this.main_color = Silver;
        ip_entering_status = untouched;
        // stop listener
        break;
    default:
        break;
    }

    this.main_color = Passive_Yellow;
    // parse ip
}

void ExitMultiplayerConfig(button_t *_local_instance_)
{
    vTaskDelay(pdMS_TO_TICKS(100));
    exit(EXIT_SUCCESS);
}

void MultiplayerInit(void) 
{

AddButton(CreateButton(BUTTON_MAIN_SET, BUTTON_BORDER, 
          4 * SCREEN_WIDTH / 5,
          SCREEN_HEIGHT / 3,
          STD_BUTTON_W, STD_BUTTON_H, "Host / Client", ToggleHostClient),
          mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN_SET, BUTTON_BORDER, 
          4 * SCREEN_WIDTH / 5,
          SCREEN_HEIGHT / 2,
          STD_BUTTON_W, STD_BUTTON_H, "IP:xxx.xxx.xxx.xxx", ReadIP), 
          mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN_SET, BUTTON_BORDER, 
          4 * SCREEN_WIDTH / 5,
          2 * SCREEN_HEIGHT / 3,
          STD_BUTTON_W, STD_BUTTON_H, "Connect", EstablishConnection),
          mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
          SCREEN_WIDTH / 5,
          SCREEN_HEIGHT / 3,
          STD_BUTTON_W, STD_BUTTON_H, "character 1", NULL),
          mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
          SCREEN_WIDTH / 5,
          SCREEN_HEIGHT / 2,
          STD_BUTTON_W, STD_BUTTON_H, "character 2", NULL),
          mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
          SCREEN_WIDTH / 5,
          2 * SCREEN_HEIGHT / 3,
          STD_BUTTON_W, STD_BUTTON_H, "character 3", NULL),
          mltplyr_config_button_array_ptr);

AddButton(CreateButton(BUTTON_MAIN, BUTTON_BORDER, 
          SCREEN_WIDTH / 5,
          7 * SCREEN_HEIGHT / 8,
          STD_BUTTON_W, STD_BUTTON_H, "back", ExitMultiplayerConfig),
          mltplyr_config_button_array_ptr);

AddButton(CreateButton(Light_Green, BUTTON_BORDER, 
          4 * SCREEN_WIDTH / 5,
          7 * SCREEN_HEIGHT / 8,
          STD_BUTTON_W, STD_BUTTON_H, "START", NULL),
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