#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <assert.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Font.h"
#include "TUM_Utils.h"
#include "TUM_Font.h"

#include "animations.h"
#include "timef.h"

#include "statemachine.h"
#include "main.h"
#include "priorties.h"
#include "resources.h"
//#include "singleplayer.h"
#include "gui.h"
#include "cheatmenu.h"

#define CHEATMENU_FREQUENCY pdMS_TO_TICKS(30)

TaskHandle_t CheatmenuTask = NULL;

enabled_cheats_t cheats = { .ignore_collision = false };
score_t global_score = { .globabl_highscore = 0 };

bool CheatLockInit(void)
{
    cheats.lock = xSemaphoreCreateMutex(); // Mutex for access to button queue
    
    if (!cheats.lock) {
        fprintf(stderr, "Failed to create cheats lock");
        return false;
    }
    return true;
}

bool ScoreLockInit(void)
{
    global_score.lock = xSemaphoreCreateMutex(); // Mutex for access to button queue
    
    if (!global_score.lock) {
        fprintf(stderr, "Failed to create score lock");
        return false;
    }
    return true;
}

void ReturnToMenu(void)
{ 
    SetNextState(1);
}

void CheatmenuEnter(void)
{
    if(xTaskCreate(vCheatmenuTask, "CheatmenuTask", 
                   mainGENERIC_STACK_SIZE, 
                   NULL, mainGENERIC_PRIORITY, 
                   &CheatmenuTask) != pdPASS) {
        DEBUG_PRINT("failed to cheatmenu task\n");
    }
}

void CheatmenuRun(void)
{}

void CheatmenuExit(void)
{
    vTaskDelete(CheatmenuTask);
}

// methods for interacting with cheat structures

bool IgnoreCollision(void)
{
    static bool return_value;
    
    assert(cheats.lock != NULL);
    if (xSemaphoreTake(cheats.lock, 0) == pdPASS) {
        return_value = cheats.ignore_collision;
        xSemaphoreGive(cheats.lock);
    }
    return return_value;
}

int HighScore(void) {
    static int cheat_high_score;
    
    assert(global_score.lock != NULL);
    if (xSemaphoreTake(global_score.lock, 0) == pdPASS) {
        cheat_high_score = global_score.globabl_highscore;
        xSemaphoreGive(global_score.lock);
    }
    return cheat_high_score;
}


void ToggleCollision(button_t *_local_instance_)
{
    if (xSemaphoreTake(cheats.lock, 0)) {
        cheats.ignore_collision ^= 1; // <- toggles collision
        this.main_color = cheats.ignore_collision * Red +
                !cheats.ignore_collision * Green;
        xSemaphoreGive(cheats.lock);
    }
}

void ChangeScore(short int delta)
{
    if (xSemaphoreTake(global_score.lock, 0) == pdPASS) {
        global_score.globabl_highscore += delta;
        xSemaphoreGive(global_score.lock);
    }
}

void IncreaseScoreBy1(void) {
    ChangeScore(1);
}
void IncreaseScoreBy10(void) {
    ChangeScore(10);
}
void IncreaseScoreBy100(void) {
    ChangeScore(100);
}
void IncreaseScoreBy1000(void) {
    ChangeScore(1000);
}

void vCheatmenuTask(void *pcParameters) {

    button_array_t cheat_menu_buttons = { .size = 0 };
    button_array_t *cheat_menu_buttons_ptr = &cheat_menu_buttons;

    AddButton(CreateButton(Green, 0x552F05, 
                                    SCREEN_WIDTH / 3,
                                    SCREEN_HEIGHT / 2 - 30,
                                    150, 30, "Toggle Collision", ToggleCollision),
                                    cheat_menu_buttons_ptr);

    AddButton(CreateButton(0xe6611e, 0x552F05, 
                                    2 * SCREEN_WIDTH / 3,
                                    SCREEN_HEIGHT / 2 - 30,
                                    150, 30, "+ 1", IncreaseScoreBy1),
                                    cheat_menu_buttons_ptr);

    AddButton(CreateButton(0xe6611e, 0x552F05, 
                                    2 * SCREEN_WIDTH / 3,
                                    SCREEN_HEIGHT / 2,
                                    150, 30, "+ 10", IncreaseScoreBy10),
                                    cheat_menu_buttons_ptr);

    AddButton(CreateButton(0xe6611e, 0x552F05, 
                                    2 * SCREEN_WIDTH / 3 + 150,
                                    SCREEN_HEIGHT / 2 - 30,
                                    150, 30, "+ 100", IncreaseScoreBy100),
                                    cheat_menu_buttons_ptr);
    
    AddButton(CreateButton(0xe6611e, 0x552F05, 
                                    2 * SCREEN_WIDTH / 3 + 150,
                                    SCREEN_HEIGHT / 2,
                                    150, 30, "+ 1000", IncreaseScoreBy1000),
                                    cheat_menu_buttons_ptr);

    AddButton(CreateButton(0xe6611e, 0x552F05, 
                                    SCREEN_WIDTH / 2,
                                    3 * SCREEN_HEIGHT / 4,
                                    150, 30, "Back", ReturnToMenu),
                                    cheat_menu_buttons_ptr);

    TickType_t last_wake_time = xTaskGetTickCount();
    tumDrawBindThread();
    while(1)
    {
        tumDrawClear(White);
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK);
        
        if (xSemaphoreTake(cheats.lock, 0) == pdPASS) {
            if (cheats.ignore_collision) 
                tumDrawCenteredText("Collision: Off", SCREEN_WIDTH / 3, 
                                    SCREEN_HEIGHT / 2, Black);
            else
                tumDrawCenteredText("Collision: On ", SCREEN_WIDTH / 3, 
                SCREEN_HEIGHT / 2, Black);
            xSemaphoreGive(cheats.lock);
        }
        
        if (xSemaphoreTake(global_score.lock, 0)) {
            char score_string[50];
            sprintf(score_string, "Highscore: %d", global_score.globabl_highscore);
            
            tumDrawCenteredText(score_string,
                                SCREEN_WIDTH / 2 + 270,
                                SCREEN_HEIGHT / 2 + 50, Black);
            xSemaphoreGive(global_score.lock);
        }    

        UpdateButtons(cheat_menu_buttons_ptr);
        DrawButtons(cheat_menu_buttons_ptr);
        
        tumDrawUpdateScreen();
        vTaskDelayUntil(&last_wake_time, CHEATMENU_FREQUENCY);
    }
}