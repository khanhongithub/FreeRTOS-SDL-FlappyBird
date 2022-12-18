#include <stdbool.h>

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"
#include "TUM_Font.h"
#include "TUM_Print.h"

#include "gui.h"

button_t *CreateButton(int main_color, int border_color, int x, int y,
		       int width, int heigth, char *text,
		       void (*function)(void))
{
	// malloc new space
	button_t *new_button = malloc(sizeof(button_t));

	// populate new struct
	(new_button->main_color) = main_color;
	(new_button->border_color) = border_color;
	(new_button->position_x) = x;
	(new_button->position_y) = y;
	(new_button->width) = width;
	(new_button->heigth) = heigth;

	(new_button->button_text_length) = strlen(text);
	(new_button->button_text) = malloc(strlen(text) + 1);
	//*(new_button -> button_text) = text;
	strcpy(new_button->button_text, text);

	// get width of text
	tumGetTextSize(text, (int *)&(new_button->button_text_length), NULL);

	// calculate corners
	(new_button->top_left_corner_x) = x - (width / 2);
	(new_button->top_left_corner_y) = y - (heigth / 2);
	(new_button->bottom_right_corner_x) = x + (width / 2);
	(new_button->bottom_right_corner_y) = y + (heigth / 2);

	(new_button->ActionWhenPressed) = function;

	// return button
	return new_button;
}

void DrawButton(button_t *button_to_be_drawn)
{
	tumDrawFilledBox(button_to_be_drawn->top_left_corner_x,
			 button_to_be_drawn->top_left_corner_y,
			 button_to_be_drawn->width, button_to_be_drawn->heigth,
			 button_to_be_drawn->main_color);

	tumDrawCenteredText(button_to_be_drawn->button_text,
			    button_to_be_drawn->position_x,
			    button_to_be_drawn->position_y, Black);
}

void DrawButtons(button_arry_t *buttons_to_be_drawn)
{
	for (int i = 0; i < buttons_to_be_drawn->size; i++) {
		DrawButton(buttons_to_be_drawn->ui_button[i]);
	}
}

void AddButton(button_t *button_to_be_added, button_arry_t *array_to_add_into)
{
	array_to_add_into->ui_button =
		realloc(array_to_add_into->ui_button,
			sizeof(button_t *) * (array_to_add_into->size + 1));

	array_to_add_into->ui_button[array_to_add_into->size] =
		button_to_be_added;
	array_to_add_into->size++;
}

void UpdateButton(button_t *button_to_be_updated, bool falling)
{
	static bool within_button_bound = false;
	static short int mouse_x, mouse_y = 0;

	if (falling) {
		// get mouse
		mouse_x = tumEventGetMouseX();
		mouse_y = tumEventGetMouseY();

		// check if mouse is inside button
		within_button_bound =
			(button_to_be_updated->top_left_corner_x < mouse_x) &&
			(button_to_be_updated->bottom_right_corner_x >
			 mouse_x) &&
			(button_to_be_updated->top_left_corner_y < mouse_y) &&
			(button_to_be_updated->bottom_right_corner_y > mouse_y);

		// if th mouse was inside the button trigger call function
		if (within_button_bound) {
			button_to_be_updated->ActionWhenPressed();
		}
	}
}

void UpdateButtons(button_arry_t *buttons_to_be_updated)
{
	static bool cur_left_click, prev_left_click,
		left_mouse_falling_edge = false;

	cur_left_click = tumEventGetMouseLeft();
	left_mouse_falling_edge = prev_left_click && !cur_left_click;
	prev_left_click = cur_left_click;

	for (int i = 0; i < (buttons_to_be_updated->size); i++) {
		UpdateButton(buttons_to_be_updated->ui_button[i],
			     left_mouse_falling_edge);
	}
}