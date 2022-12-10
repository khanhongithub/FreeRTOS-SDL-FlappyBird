#include "TUM_Draw.h"
#include <math.h>

#define PI 3.1415
#define RADIUS 25
#define CUBEDIMENSION 50
#define MOVING_TEXT_WIDTH_PADDING 10
#define GENERIC_ROTATION_SPEED 0.5 // just a reasonable speed
#define FPS_AVERAGE_COUNT 50


typedef struct rotating_coords{
    coord_t c1;
    coord_t c2;        
}rotating_coords_t;

rotating_coords_t circlecoord(coord_t origin, int radius, 
                              double speed, int counter);
coord_t horizontal_oscilation(coord_t origin, int amplitude,
                              double speed, int counter);
void DrawBottomText(char *input_string, coord_t origin);
void DrawTopText(char *input_string, int counter, coord_t origin);
void DrawMouseInfo(coord_t origin);
void DrawButtonInfo(coord_t origin);
void DrawTriangleCircleSquare(int counter, coord_t origin);
void vDrawFPS(void);
