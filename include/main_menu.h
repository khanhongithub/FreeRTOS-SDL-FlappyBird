/*
    main menu of the game, housing buttons. Used to switch to different states
*/
#define TITLE_SPRITE "../resources/flappydoge.png" 

#define BUTTON_START_Y 5 * SCREEN_HEIGHT / 12
#define BUTTON_MAIN_MENU_H 35
#define BUTTON_MAIN_MENU_W 260

void MenuTaskEnter(void);
void MenuTaskRun(void);
void MenuTaskExit(void);
void DrawMenuTask(void* pcParameters);