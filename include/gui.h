typedef struct {
	int main_color;
	int border_color;

	int position_x; // center of button
	int position_y;
	int width;
	int heigth;
	char *button_text;
	short button_text_length;

	int top_left_corner_x;
	int top_left_corner_y;
	int bottom_right_corner_x;
	int bottom_right_corner_y;

	void (*ActionWhenPressed)(void);
} button_t;

typedef struct {
	int size;
	button_t **ui_button;
} button_arry_t;

button_t *CreateButton(int main_color, int border_color, int x, int y,
		       int width, int heigth, char *text,
		       void (*function)(void));

void DrawButton(button_t *button_to_be_drawn);
void DrawButtons(button_arry_t *buttons_to_be_drawn);
void AddButton(button_t *button_to_be_added, button_arry_t *array_to_add_into);
void UpdateButton(button_t *button_to_be_updated, bool falling);
void UpdateButtons(button_arry_t *buttons_to_be_updated);