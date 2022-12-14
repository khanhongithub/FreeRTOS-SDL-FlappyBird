#define PLAYER_SPRITE "../resources/doge.png"
#define PIPE_TOP_SPRITE "../resources/pipe_top.png"
#define PIPE_BOTTOM_SPRITE "../resources/pipe_bottom.png" 
#define BACKGROUND_SPRITE "../resources/background.jpg"
#define GAME_OVER_SPRITE "../resources/gameover.png"

#define GAMEOVER_BOX_HEIGHT 300
#define GAMEOVER_BOX_WIDTH 700

#define GAMEOVER_BOX_HEIGHT_CONTENT (GAMEOVER_BOX_HEIGHT - 4)
#define GAMEOVER_BOX_WIDTH_CONTENT  (GAMEOVER_BOX_WIDTH - 4)

void RendererEnter(void);
void RendererRun(void);
void RendererExit(void);
void vRendererTask(void* pcParameters);

extern TaskHandle_t RenderingTask;