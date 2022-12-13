#define PLAYER_SPRITE "../resources/doge.png"
#define PIPE_TOP_SPRITE "../resources/pipe_top.png"
#define PIPE_BOTTOM_SPRITE "../resources/pipe_bottom.png" 
#define BACKGROUND_SPRITE "../resources/background.jpg"

void RendererEnter(void);
void RendererRun(void);
void RendererExit(void);
void vRendererTask(void* pcParameters);

extern TaskHandle_t RenderingTask; 