
#define PLAYER_SPRITE1 "../resources/doge.png"
#define PIPE_TOP_SPRITE "../resources/pipe_top.png"
#define PIPE_BOTTOM_SPRITE "../resources/pipe_bottom.png" 
#define BACKGROUND_SPRITE "../resources/background.jpg"
//#define GAME_OVER_SPRITE "../resources/gameover.png"

#define START_SPRITE "../resources/start.png"
#define MULTIPLAYER_SPRITE "../resources/multiplayer.jpeg"
#define FLAPPYDOGE_SPRITE "../resources/flappydoge.png" 

TaskHandle_t SingleplayerTask;

void MenuTaskEnter(void);
void MenuTaskRun(void);
void MenuTaskExit(void);
void DrawMenuTask(void* pcParameters);