#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "simple_bmp.h"

#define INPUT_SIZE 255
#define EXIT_MSG "exit\n"

#define ORIGINAL_IMAGE_PATH "./resources/original_image.bmp"
#define ORIGINAL_IMAGE_NAME "original_image.bmp"
#define EDITED_IMAGE_PATH "./resources/edited_image.bmp"
#define EDITED_IMAGE_NAME "edited_image.bmp"

typedef struct pixel {
  int32_t x;
  int32_t y;
} pixel;

enum areas {
  EX_AREA = 0,
  IN_AREA = 1
};

enum succes {
  SUCCES = 1,
  FAIL = 0
} ;

enum succes radio_input();
enum areas get_pixel_area();
enum succes edit_image();
enum succes open_image();
void rutina_salida();
void view_images();
