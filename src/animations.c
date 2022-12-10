/**
 * @file animations.c
 * @author Nino-tum
 * @date November 2022
 * @brief provides functions related to animation and drawing of objects
 * in the scope of this project
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"
#include "TUM_Font.h"

#include "main.h"
#include "animations.h"
#include "resources.h"


// converts increments of counter by 1 to 1°
double counter_to_degrees(int counter)
{
    return (2 * PI * 1/360 * (double)counter);
}

// returns 2 alligned coordinates which orbit around "origin" with "radius"
rotating_coords_t circlecoord(coord_t origin, int radius,
                              double speed, int counter)
{
    double x_offset, y_offset, t; // t as in time

    t = counter_to_degrees(counter); // one increment of counter -> 1°            

    x_offset = (double)radius * cos(speed * t);
    y_offset = (double)radius * sin(speed * t);

    coord_t return_c1 = {origin.x + x_offset, origin.y + y_offset};
    coord_t return_c2 = {origin.x - x_offset, origin.y - y_offset};      
    rotating_coords_t return_data;
    return_data.c1 = return_c1;
    return_data.c2 = return_c2;

    return return_data;             
}

// returns 1 coordinate which oscilates along the horizontal axis of the lenght 2 * "amplitude"
coord_t horizontal_oscilation(coord_t origin, int amplitude,
                              double speed, int counter)
{
    double x, y, t;

    t = counter_to_degrees(counter); // one increment of counter -> 1°
    y = (double)origin.y;
    x = (double)amplitude * sin((double)speed * t) + (double) origin.x;
    coord_t data_to_return = {(int) x, (int) y};

    return data_to_return;
}

void DrawBottomText(char *input_string, coord_t origin)
{
    int input_string_width = -1;

    // gets size of the input string, saved in input_string_width
    // returns 0 on success 
    if (!tumGetTextSize((char *)input_string,
            &input_string_width, NULL)) {
        // sanity check
        assert( input_string_width > 0 );

        // height should be 7/8 of the screen height 
        // -> origin + 3/8 screen heigth 
        tumDrawText(input_string,
                    origin.x -
                    input_string_width / 2,
                    origin.y + 3 * SCREEN_HEIGHT / 8 - DEFAULT_FONT_SIZE / 2,
                    White);
        }            
}

void DrawTopText(char *input_string, int counter, coord_t origin)
{
    int input_string_width = -1;

    if (!tumGetTextSize((char *)input_string, &input_string_width, NULL)) {
        // sanity check
        assert( input_string_width > 0 );
        int offset_x = origin.x - SCREEN_WIDTH / 2;

        // height should be 1/8 of the screen height 
        // -> origin - 3/8 screen heigth 
        coord_t origin_top = { SCREEN_WIDTH / 2 - input_string_width / 2 
                              + offset_x,
                              origin.y - 3 * SCREEN_HEIGHT / 8 
                              - DEFAULT_FONT_SIZE / 2};
        coord_t input_string_moving_coord = 
        horizontal_oscilation(origin_top,
                              SCREEN_WIDTH / 2 - input_string_width / 2 
                              - MOVING_TEXT_WIDTH_PADDING, 
                              GENERIC_ROTATION_SPEED, counter);
        tumDrawText(input_string, input_string_moving_coord.x,
                    input_string_moving_coord.y , Orange);
        }
}

void DrawMouseInfo(coord_t origin)
{
    static int mouse_x, mouse_y, mouse_left, mouse_right, mouse_mid = 0;
    static char str_m_x[30], str_m_y[30], str_m_r[30], str_m_m[30], str_m_l[30];
    static int padding_right = 0;

    // getting x position which is supposed to be 3/4 of screen width,
    // origin already contains 1/2 of the screen width 
    padding_right = origin.x + SCREEN_WIDTH / 4;
    mouse_x = tumEventGetMouseX();
    mouse_y = tumEventGetMouseY();
    mouse_right = tumEventGetMouseRight();
    mouse_mid = tumEventGetMouseMiddle();
    mouse_left = tumEventGetMouseLeft();

    // displays information about mouse
    sprintf(str_m_x, "mouse x: %5d", mouse_x);
    sprintf(str_m_y, "mouse y: %5d", mouse_y);
    sprintf(str_m_r, "mouse right: %d", mouse_right);
    sprintf(str_m_m, "mouse mid: %d", mouse_mid);
    sprintf(str_m_l, "mouse left: %d", mouse_left);
    tumDrawText(str_m_x, padding_right, origin.y - 30, Purple);
    tumDrawText(str_m_y, padding_right, origin.y - 15, Purple);
    tumDrawText(str_m_r, padding_right, origin.y , Purple);
    tumDrawText(str_m_m, padding_right, origin.y + 15, Purple);
    tumDrawText(str_m_l, padding_right, origin.y + 30, Purple);
}

void DrawButtonInfo(coord_t origin)
{
    static char str_reset_values[20] = { 0 };
    static char str_A[20] = { 0 };
    static char str_B[20] = { 0 };
    static char str_C[20] = { 0 };
    static char str_D[20] = { 0 };
    static bool reset_values = 0;

    static bool prev_A = 0; // previous 
    static bool cur_A = 0; // and current state of key which is either 0 or 1
    static int A_counter = 0;

    static bool prev_B = 0;
    static bool cur_B = 0;
    static int B_counter = 0;
    
    static bool prev_C = 0;
    static bool cur_C = 0;
    static int C_counter = 0;
    
    static bool prev_D = 0;
    static bool cur_D = 0;
    static int D_counter = 0;    

    static int padding_left = 0;
    padding_left = origin.x - SCREEN_WIDTH / 2 + 50;

    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE)
    {   // DEBOUNCING:
        // counter is increased when the button was previously released
        // and is now being pressed --> rising edge detection
        cur_A = buttons.buttons[KEYCODE(A)];
        A_counter += !prev_A & cur_A;  
        prev_A = cur_A;

        cur_B = buttons.buttons[KEYCODE(B)];
        B_counter += !prev_B & cur_B;
        prev_B = cur_B;
        
        cur_C = buttons.buttons[KEYCODE(C)];
        C_counter += !prev_C & cur_C;
        prev_C = cur_C;

        cur_D = buttons.buttons[KEYCODE(D)];
        D_counter += !prev_D & cur_D;
        prev_D = cur_D;       

        // if the right mouse button is pressed values are set to zero
        reset_values = (bool) tumEventGetMouseRight();
        A_counter *= !reset_values;
        B_counter *= !reset_values;
        C_counter *= !reset_values;
        D_counter *= !reset_values;

        // displays current count of each button
        sprintf(str_A, "A: %d", A_counter);
        sprintf(str_B, "B: %d", B_counter);
        sprintf(str_C, "C: %d", C_counter);
        sprintf(str_D, "D: %d", D_counter);
        tumDrawText(str_A, padding_left, origin.y - 30, Yellow);
        tumDrawText(str_B, padding_left, origin.y - 10, Yellow);
        tumDrawText(str_C, padding_left, origin.y + 10, Yellow);
        tumDrawText(str_D, padding_left, origin.y + 30, Yellow);

        // displays if status of reset
        sprintf(str_reset_values, "reset: %1d", reset_values);
        tumDrawText(str_reset_values, padding_left, origin.y + 50, Green);
    }    
    xSemaphoreGive(buttons.lock);
}

void DrawTriangleCircleSquare(int counter, coord_t origin)
{
    static coord_t triang_cords[3] = { 0 };

    // generates 2 coords for the objects orbiting the triangle
    rotating_coords_t rot = circlecoord(origin, 100, 
                                        GENERIC_ROTATION_SPEED, 
                                        counter);
    triang_cords[0] = (coord_t){ origin.x, origin.y - 20};
    triang_cords[1] = (coord_t){ origin.x + 20, origin.y + 20};
    triang_cords[2] = (coord_t){ origin.x - 20, origin.y + 20};
    
    // drawing objects with coordinates from above 
    tumDrawTriangle(triang_cords, Yellow);      
    tumDrawCircle(rot.c1.x, rot.c1.y, RADIUS, Blue);
    tumDrawFilledBox(rot.c2.x - (CUBEDIMENSION / 2),
                     rot.c2.y - (CUBEDIMENSION / 2),
                     CUBEDIMENSION, CUBEDIMENSION, Red);      
}

void vDrawFPS(void)
{
    static unsigned int periods[FPS_AVERAGE_COUNT] = { 0 };
    static unsigned int periods_total = 0;
    static unsigned int index = 0;
    static unsigned int average_count = 0;
    static TickType_t xLastWakeTime = 0, prevWakeTime = 0;
    static char str[10] = { 0 };
    static int text_width;
    int fps = 0;
    // font_handle_t cur_font = tumFontGetCurFontHandle();

    if (average_count < FPS_AVERAGE_COUNT) {
        average_count++;
    }
    else {
        periods_total -= periods[index];
    }

    xLastWakeTime = xTaskGetTickCount();

    if (prevWakeTime != xLastWakeTime) {
        periods[index] =
            configTICK_RATE_HZ / (xLastWakeTime - prevWakeTime);
        prevWakeTime = xLastWakeTime;
    }
    else {
        periods[index] = 0;
    }

    periods_total += periods[index];

    if (index == (FPS_AVERAGE_COUNT - 1)) {
        index = 0;
    }
    else {
        index++;
    }

    fps = periods_total / average_count;

    //tumFontSelectFontFromName(FPS_FONT);

    sprintf(str, "FPS: %2d", fps);

    if (!tumGetTextSize((char *)str, &text_width, NULL)) {
        tumDrawText(str, 
                    SCREEN_WIDTH - text_width - 50,
                    SCREEN_HEIGHT - DEFAULT_FONT_SIZE * 1.5 - 10,
                    Skyblue);
    }    
    // tumFontSelectFontFromHandle(cur_font);
    // tumFontPutFontHandle(cur_font);
}