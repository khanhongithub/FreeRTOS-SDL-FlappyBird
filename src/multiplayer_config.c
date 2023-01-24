#include <stdio.h>
#include <stdlib.h>

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
#include "resources.h"
#include "handshake_host.h"



#define MULTIPLAYERCONFIG_FREQUENCY pdMS_TO_TICKS(35)
#define IP_ENTERING_FREQUENCY pdMS_TO_TICKS(20)

#define BOTTOM_BOX_WIDTH SCREEN_WIDTH
#define BOTTOM_BOX_HEIGHT 2 * SCREEN_HEIGHT / 9


void vInputForIP(void *pvParameters);
void InputForIPStart(void);
void InputForIPStop(void);


void RemoveChar(char* s, char c);
bool IPCreate(char input, ipv4 output);

typedef struct ip_button_information {
    short int ip_entering_status;
    button_t *ip_button_ptr;
    SemaphoreHandle_t lock;
} ip_button_information_t;

ip_button_information_t ip_button_info = { .ip_entering_status = 0};

multiplayer_config_t mltplyr_cfg = { .own_ip = "127.0.0.1\0" };


button_array_t mltplyr_config_button_array = { .size = 0 };
button_array_t *mltplyr_config_button_array_ptr = &mltplyr_config_button_array;

image_handle_t multiplayerconfig_background_sprite = NULL;

TaskHandle_t MultiplayerConfigTask = NULL;
TaskHandle_t IPEnteringTask = NULL;



void InputForIPStart(void) 
{
    if(xTaskCreate(vInputForIP, "InputForIPTask", 
                   mainGENERIC_STACK_SIZE / 2, 
                   NULL, mainGENERIC_PRIORITY - 2, 
                   &IPEnteringTask) != pdPASS) {
        DEBUG_PRINT("failed to listener task\n");
    }
}

void InputForIPStop(void) 
{
    vTaskDelete(IPEnteringTask);
}

void vInputForIP(void *pvParameters)
{
    // send current ipv4 to queue
    ipv4 current_input_ip = "xxx.xxx.xxx.xxx\0";
    bool done = false;

    IP_INPUT_GENERATE_DEBOUNCING_VARS();

    TickType_t last_wake_time = xTaskGetTickCount();
    TickType_t start_time = xTaskGetTickCount();

    while (1)
    {
        // update and debouce all num, dot and escape keys
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK | FETCH_EVENT_NO_GL_CHECK);
        xGetButtonInput();
        IP_INPUT_DEBOUNCE_KEYS();
        
        // if state was changed and this task wasnt terminated it should
        // stop after 60s
        if (xTaskGetTickCount() - start_time >= pdMS_TO_TICKS(60000))
        {
            vTaskDelete(NULL);
        }
        
        // check for rising edges and input them into IP constructer
        IF_PRESSED_APPEND(1 to current_input_ip);
        IF_PRESSED_APPEND(2 to current_input_ip);
        IF_PRESSED_APPEND(3 to current_input_ip);
        IF_PRESSED_APPEND(4 to current_input_ip);
        IF_PRESSED_APPEND(5 to current_input_ip);
        IF_PRESSED_APPEND(6 to current_input_ip);
        IF_PRESSED_APPEND(7 to current_input_ip);
        IF_PRESSED_APPEND(8 to current_input_ip);
        IF_PRESSED_APPEND(9 to current_input_ip);
        IF_PRESSED_APPEND(0 to current_input_ip);

        strcpy(ip_button_info.ip_button_ptr->button_text, current_input_ip);
        
        if (rising_edge_PERIOD)
        {
            done = IPCreate('.', current_input_ip);
        }

        if (done) 
        {
            // set color to green
            if (xSemaphoreTake(ip_button_info.lock, portMAX_DELAY) == pdPASS) {
                ip_button_info.ip_button_ptr->main_color = Light_Green;

                strcpy(ip_button_info.ip_button_ptr->button_text, current_input_ip);
                strcpy(mltplyr_cfg.other_ip, current_input_ip);
                
                ip_button_info.ip_entering_status = correct;
                
                xSemaphoreGive(ip_button_info.lock);
                vTaskDelete(NULL);
            }
        }
        vTaskDelayUntil(&last_wake_time, IP_ENTERING_FREQUENCY);
    }
    
}

void MultiplayerConfigInit(void)
{
    static bool inited = false;
    if (!inited)
    {
        mltplyr_cfg.lock = xSemaphoreCreateMutex();
        inited = true;
    }
}

void InitIPButtonInfo(void) 
{
    static bool inited = false;
    if (!inited)
    {
        ip_button_info.lock = xSemaphoreCreateMutex();
        inited = true;
    }
}



void ToggleHostClient(button_t *_local_instance_)
{
    static bool mode = client;
    if (mode != client)
    {
        this.main_color = Olive;
        strcpy(this.button_text, "Host");
        mltplyr_cfg.type = host;
        mode = client;
    }
    else
    {
        this.main_color = Skyblue;
        strcpy(this.button_text, "Client");
        mltplyr_cfg.type = client;
        mode = host;
    }
}

void EstablishConnection(button_t *_local_instance_)
{
    static bool connected = false;
    
    // connected = ConnectTo();

    if (connected)
    {
        this.main_color = Light_Green;
    }
    else  
    {
        this.main_color = Dark_Red;
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
    static short int index = 0;
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

    if (done) {
	    for (int i = 0; i < 4; i++) {
	        if (ip_temp[i * 4] == 'x') // <- if first char is x change to 0
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
    if (xSemaphoreTake(ip_button_info.lock, 0) == pdPASS) {

        switch (ip_button_info.ip_entering_status)
        {
        case correct: // ip was correct, new one is entered now
        case incorrect: // ip syntactically incorrect, shouldnt happen
            strcpy(this.button_text, "IP:xxx.xxx.xxx.xxx");
        case untouched:
            this.main_color = Passive_Yellow;
            ip_button_info.ip_entering_status = in_progress;
            // start listener
            InputForIPStart();
            break;

        case in_progress:
            this.main_color = BUTTON_MAIN_SET; // return to untouched
            ip_button_info.ip_entering_status = untouched;
            strcpy(this.button_text, "IP:xxx.xxx.xxx.xxx");
            // stop listener
            InputForIPStop();
            break;

        default:
            break;
        }
        xSemaphoreGive(ip_button_info.lock);
    }
    else {
        // if lock wasnt obtained for some reason
        this.main_color = Purple;
    }
}

void ExitMultiplayerConfig(button_t *_local_instance_)
{
    SetNextState(0);
}

void MultiplayerConfigMenuInit(void) 
{
    static bool inited = false;

    if (!inited) { // very important this isnt run more than once      

    AddButton(CreateButton(BUTTON_MAIN_SET, BUTTON_BORDER, 
              4 * SCREEN_WIDTH / 5,
              SCREEN_HEIGHT / 3,
              STD_BUTTON_W, STD_BUTTON_H, "Host / Client", ToggleHostClient),
              mltplyr_config_button_array_ptr);

    button_t *just_created_ip_button = CreateButton(BUTTON_MAIN_SET, 
                                       BUTTON_BORDER, 4 * SCREEN_WIDTH / 5,
                                       SCREEN_HEIGHT / 2,
                                       STD_BUTTON_W, STD_BUTTON_H, 
                                       "IP:xxx.xxx.xxx.xxx", ReadIP);

    ip_button_info.ip_button_ptr = just_created_ip_button;

    AddButton(just_created_ip_button, mltplyr_config_button_array_ptr);

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

    inited = true;
    }
}

void DrawMultiplayerConfigBackground(void)
{
    if (multiplayerconfig_background_sprite == NULL)
    {
        multiplayerconfig_background_sprite = tumDrawLoadImage(MULTI_BACKGROUND);
    }
    tumDrawLoadedImage(multiplayerconfig_background_sprite, 0, 0);
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
    
    HandshakeTaskEnter();
        
 
 
    if (!inited)
    {
        MultiplayerConfigInit();
        MultiplayerConfigMenuInit();
        InitIPButtonInfo();
        inited = true;
    }
}

void MultiplayerConfigRun(void)
{}

void MultiplayerConfigExit(void)
{
    vTaskDelete(MultiplayerConfigTask);

    HandshakeTaskStop();

}

void vMultiplayerConfigTask(void *pvArgs)
{
    tumDrawBindThread();
    tumDrawClear(White);

    TickType_t last_wake_time = xTaskGetTickCount();
    while (1)
    {
        DEBUG_PRINT("MultiplayerConfig\n");
        //ToggleHostClient(this);
        DrawMultiplayerConfigBackground();
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