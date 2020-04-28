#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "simple_bmp.h"

#define INPUT_SIZE 255
#define EXIT_MSG "exit\n"

#define CONTRAST    ( (double) 1.3 )
#define BRIGHTNESS  ( (uint8_t) 30 )

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

enum return_values {
  SUCCES = 1,
  FAILURE = 0
} ;

enum areas get_pixel_area();
enum return_values radio_input();
enum return_values edit_image();
enum return_values open_image();
void rutina_salida();
void view_images();

void increase_contrast(struct _sbmp_raw_data*);
void increase_brightness(struct _sbmp_raw_data*);
