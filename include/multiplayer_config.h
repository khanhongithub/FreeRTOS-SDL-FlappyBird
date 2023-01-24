/*
    used to configure multiplayer and conduct a handshake between 
    the 2 players
*/
#ifndef _STDBOOL_H
#include <stdbool.h>
#endif

//#include "gui.h" //to use button_array_t

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
// if those arent included the defintion of 
// SemphoreHandle_t isnt for some reason
// this somehow happens only in this file (?)

#define MULTI_BACKGROUND "../resources/multi_back.png"

#define host true
#define client false

#define untouched 0
#define in_progress 1
#define correct 2
#define incorrect 3

#define RISING_EDGE_VARS(x) \
        static bool cur_##x, prev_##x; \
        static bool rising_edge_##x = false;

#define RISING_EDGE(x) \
    if (xSemaphoreTake(buttons.lock, 0) == pdPASS) \
    { \
        cur_##x = buttons.buttons[KEYCODE(x)]; \
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
    RISING_EDGE(PERIOD)

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
    RISING_EDGE_VARS(PERIOD)

#define to ,
// "to" must be expanded to "," first:
#define IF_PRESSED_APPEND(input) IF_PRESSED_APPEND_comma(input)
#define IF_PRESSED_APPEND_comma(key, ip) \
do { if (rising_edge_##key) {   \
    done = IPCreate(key + '0', ip); \
}} while (0)

typedef char ipv4[16];

typedef struct multiplayer_config {
    bool type; // <- either host or client
    SemaphoreHandle_t lock;
    ipv4 own_ip; // <- own instance
    ipv4 other_ip; // <- instance that is being connected to

}multiplayer_config_t;

typedef struct multiplayer_connected{
    bool connected;
    SemaphoreHandle_t lock;
}multiplayer_connect_t;
extern multiplayer_connect_t mltplyr_connect;

extern multiplayer_config_t mltplyr_cfg;

void MultiplayerConfigEnter(void);
void MultiplayerConfigRun(void);
void MultiplayerConfigExit(void);
void vMultiplayerConfigTask(void *pvParameters);