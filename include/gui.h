/*
	contains buttons which can be created at runtime;
	buttons are usually added into an array from which 
	they are updated and drawn;
	buttons can have a callbackfunction which is run
	when a button has been pressed, callback functions
	have take a pointer to their parent button as an arg;
	the button and its properties can be accessed in callback
	functions via the java-like "this" - "keyword"
*/
#ifndef __TUM_DRAW_H__
#include "TUM_Draw.h"
#endif

#ifndef this
#define this (*_local_instance_)
#endif

#define STD_BUTTON_W 150
#define STD_BUTTON_H 30

#define BUTTON_MAIN_SATURATED 0xe6611e
#define BUTTON_MAIN 0xe6824f
#define BUTTON_MAIN_SET Gray
#define BUTTON_BORDER 0x552F05
#define Light_Green 0x46bc40
#define Passive_Yellow 0xdfc42d
#define Dark_Red 0xaa0000

#define MAX_TEXT_LEN 256

typedef struct gui_button button_t;

struct gui_button{
	int main_color;
	int border_color;

	int position_x; // center of button
	int position_y;
	int width;
	int heigth;
	char button_text[MAX_TEXT_LEN];
	short button_text_length;

	int top_left_corner_x;
	int top_left_corner_y;
	int bottom_right_corner_x;
	int bottom_right_corner_y;

	void (*ActionWhenPressed)(button_t *_local_instance_);
};

typedef struct {
	int size;
	button_t **ui_button;
} button_array_t;

button_t *CreateButton(int main_color, int border_color, int x, int y, 
		       int width, int heigth, char *text,
		       void (*function)(button_t *_local_instance_));

void DrawButton(button_t *button_to_be_drawn);
void DrawButtons(button_array_t *buttons_to_be_drawn);
void AddButton(button_t *button_to_be_added, button_array_t *array_to_add_into);
void UpdateButton(button_t *button_to_be_updated, bool falling);
void UpdateButtons(button_array_t *buttons_to_be_updated);