#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "simple_bmp.h"

#define INPUT_SIZE 32
#define EXIT_MSG "exit\n"

#define CONTRAST    ( (double) 1.3 )
#define BRIGHTNESS  ( (uint8_t) 30 )

// tiene que ser impar !!!
#define KERNEL_SIZE    ( (int8_t) 19 )

#define ORIGINAL_IMAGE_PATH "./resources/original_image.bmp"
#define EDITED_IMAGE_PATH "./resources/edited_image.bmp"

typedef struct position {
  int32_t x;
  int32_t y;
} position;

enum areas {
  EX_AREA = 0,
  IN_AREA = 1
};

enum return_values {
  SUCCES = 1,
  FAILURE = 0
} ;

enum areas get_position_area();
enum return_values radio_input();
enum return_values edit_image();
enum return_values open_image();
void set_kernel(uint8_t**);
void rutina_salida();
void view_images();
void blure_pixel(struct position *);
void increase_pixel_contrast_brightness(struct position *);
void set_position(struct position *, int32_t, int32_t);
