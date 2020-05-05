#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <signal.h>

#include "simple_bmp.h"

#define INPUT_SIZE 32
#define TEST_CORRIDAS 30
#define EXIT_MSG "exit\n"
#define TEST_MSG "test\n"

#define CONTRAST    ( (double) 1.3 )
#define BRIGHTNESS  ( (uint8_t) 30 )

#define KERNEL_SIZE    ( (int8_t) 41 )

#define ORIGINAL_IMAGE_PATH "./resources/original_image.bmp"
#define EDITED_IMAGE_PATH "./resources/edited_image.bmp"

typedef struct position {
  int16_t x;
  int16_t y;
} position;

enum areas {
  EX_AREA = 0,
  IN_AREA = 1,
  LIM_AREA = 2
};

enum return_values {
  FAILURE = 0,
  SUCCES = 1
};

enum input_codes {
  INPUT_ERR = 0,
  INPUT_OK = 1,
  INPUT_EXIT = 2,
  INPUT_TEST = 3
};

enum areas get_position_area( struct _sbmp_image *,
                              struct position*, struct position*);
enum input_codes radio_input();
enum return_values edit_image();
enum return_values open_image();
void set_kernel(uint16_t**);
void rutina_salida(int32_t);
void view_images();
void blure_pixel(struct _sbmp_image *, struct position *);
void increase_pixel_contrast_brightness(struct _sbmp_image *,
                                        struct position *);
void edit_limits(struct _sbmp_image *, struct position *);
void set_position(struct position *, int16_t, int16_t);
uint16_t get_norm(uint16_t**);
double get_time();
