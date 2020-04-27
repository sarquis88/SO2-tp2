#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#include "simple_bmp.h"

#define INPUT_SIZE 255
#define EXIT_MSG "exit\n"

#define RESOURCES_PATH "./resources/"
#define ORIGINAL_IMAGE_NAME "original_image.bmp"
#define EDITED_IMAGE_NAME "edited_image.bmp"

int32_t radio_input();
void rutina_salida();
void open_image();
