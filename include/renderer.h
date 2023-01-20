#define PLAYER_SPRITE "../resources/doge1.png"
#define PIPE_TOP_SPRITE "../resources/pipe_top.png"
#define PIPE_BOTTOM_SPRITE "../resources/pipe_bottom.png" 
#define BACKGROUND_SPRITE "../resources/background.jpg"
#define GAME_OVER_SPRITE "../resources/gameover2.png"
#define FLOOR_SPRITE "../resources/floor.png"
#define GET_READY_SPRITE "../resources/get_ready.png"

#define DOGE_DEATH_SPRITE "../resources/doge_death.png"

#define JUMP_ANIMATION_SPRITE "../resources/doge_animation1.png"

#define MEDAL_BRONZE_SPRITE "../resources/medal_bronze.png" 
#define MEDAL_SILVER_SPRITE "../resources/medal_silver.png"
#define MEDAL_GOLD_SPRITE   "../resources/medal_gold.png"
#define MEDAL_PLATIN_SPRITE "../resources/medal_platin.png"

#define DEATH_SOUND "../resources/waveforms/death.wav"
#define JUMP_SOUND "../resources/waveforms/jump.wav"

#define SINGLEPLAYER_USUAL_POS_X (FIRST_POSITION + 3 \
        * PLAYER_RADIUS + 3 * OBSTACLE_WIDTH + 5)

#define GAMEOVER_BOX_HEIGHT 300
#define GAMEOVER_BOX_WIDTH 600

#define USUAL_POS -1
#define SPRITE_PADDING 8

#define BUTTON_DEATH_W 100
#define BUTTON_DEATH_H 30

#define PIPE_SCALE_FACTOR 1.98

#define GAMEOVER_BOX_HEIGHT_CONTENT (GAMEOVER_BOX_HEIGHT - 4)
#define GAMEOVER_BOX_WIDTH_CONTENT  (GAMEOVER_BOX_WIDTH - 4)

#define SCORE_NUM_X 10
#define STD_BUTTON_SPACING 15

#define INGAME_BUTTON_WIDTH 140
#define INGAME_BUTTON_HEIGTH 30

#define MEDAL_X (SCREEN_WIDTH / 2 - 48)
#define MEDAL_Y (SCREEN_HEIGHT / 2 - 48)

#define INGAME_BUTTON_CONTENT_WIDTH  (INGAME_BUTTON_WIDTH  - 4)
#define INGAME_BUTTON_CONTENT_HEIGTH (INGAME_BUTTON_HEIGTH - 4)

#define PAUSE_BOX_WIDTH 150
#define PAUSE_BOX_HEIGTH 200
#define PAUSE_BOX_WIDTH_CONTENT (PAUSE_BOX_WIDTH - 4)
#define PAUSE_BOX_HEIGTH_CONTENT (PAUSE_BOX_HEIGTH - 4) 
#define PAUSE_MENU_PADDING 20

#define SGLPLY_MENU_BORDER 0x52394a
#define SGLPLY_MENU_MAIN 0xded794

void DrawBackground(void);
void DrawPlayer(TickType_t xLastFrameTime, int player_x, int player_y, bool dead,
                short int speed);
void InitDrawPlayersprite(void);

void RendererEnter(void);
void RendererRun(void);
void RendererExit(void);
void vRendererTask(void *pcParameters);

extern TaskHandle_t RenderingTask;