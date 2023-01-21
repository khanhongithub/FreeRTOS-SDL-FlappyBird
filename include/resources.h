/*
    header for button related input handling
*/
#ifndef _STDBOOL_H
#include <stdbool.h>
#endif

// buttons
#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
}buttons_buffer_t;

extern buttons_buffer_t buttons;

bool buttonsInit(void);
void xGetButtonInput(void);