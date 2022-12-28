#define PLAYER_SPRITE "../resources/doge1.png"
#define PIPE_TOP_SPRITE "../resources/pipe_top.png"
#define PIPE_BOTTOM_SPRITE "../resources/pipe_bottom.png" 
#define BACKGROUND_SPRITE "../resources/background.jpg"
#define GAME_OVER_SPRITE "../resources/gameover2.png"

#define JUMP_ANIMATION_SPRITE "../resources/doge_animation1.png"

#define DEATH_SOUND "../resources/waveforms/death.wav"
#define JUMP_SOUND "../resources/waveforms/jump.wav"

#define GAMEOVER_BOX_HEIGHT 300
#define GAMEOVER_BOX_WIDTH 600

#define GAMEOVER_BOX_HEIGHT_CONTENT (GAMEOVER_BOX_HEIGHT - 4)
#define GAMEOVER_BOX_WIDTH_CONTENT  (GAMEOVER_BOX_WIDTH - 4)

// todo: move these definitions to their own button C file
#define INGAME_BUTTON_WIDTH 140
#define INGAME_BUTTON_HEIGTH 30

#define INGAME_BUTTON_CONTENT_WIDTH  (INGAME_BUTTON_WIDTH  - 4)
#define INGAME_BUTTON_CONTENT_HEIGTH (INGAME_BUTTON_HEIGTH - 4)


void RendererEnter(void);
void RendererRun(void);
void RendererExit(void);
void vRendererTask(void *pcParameters);

extern TaskHandle_t RenderingTask;