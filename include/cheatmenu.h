#ifndef _STDBOOL_H
#include <stdbool.h>
#endif

#define CHEAT_BACKGROUND "cheat_back.png"

typedef struct score_s {
    int globabl_highscore;
    SemaphoreHandle_t lock;
}score_t;

typedef struct enabled_cheats {
    bool ignore_collision;
    SemaphoreHandle_t lock;
}enabled_cheats_t;

/*
    menu to set highscore and toggle collision
*/
void CheatmenuEnter(void);
void CheatmenuRun(void);
void CheatmenuExit(void);

bool CheatLockInit(void);
bool ScoreLockInit(void);

bool IgnoreCollision(void);
int HighScore(void);

void vCheatmenuTask(void *pcParameters);

extern enabled_cheats_t cheats;
extern score_t global_score;

extern TaskHandle_t CheatmenuTask;