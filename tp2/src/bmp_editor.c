#include "../include/bmp_editor.h"

long radio;
char input[INPUT_SIZE];
struct _sbmp_image *bmp_original;
int8_t w[3][3] =
  {
    {1, 1, 1},
    {1, 2, 1},
    {1, 1, 1}
  };

/**
 *  Funcion principal
 */
int32_t main()
  {
    printf("\n");
    printf("<----------------------------------------------------------->\n");
    printf("<-------------Editor de imágenes en formato BMP------------->\n");
    printf("<----------------------------------------------------------->\n");
    printf("\n");
    fflush(stdout);

    printf("Abriendo imágen...\n");
    if( open_image() == FAILURE)
      exit(EXIT_FAILURE);
    printf("Imágen abierta\n");

    while (1)
      {
        printf("\nIngrese el radio de la imágen (en pixeles) o 'exit' para salir\n");
        if( radio_input() == FAILURE)
          rutina_salida();
        else
          printf("Radio: %ld pixeles\n", radio);

        printf("Contraste: %.2f\n", CONTRAST);
        printf("Brillo: %d\n", BRIGHTNESS);

        printf("Editando imágen...\n");
        if( edit_image() == FAILURE)
          exit(EXIT_FAILURE);
        printf("Imágen editada\n");

        printf("Mostrando imágenes...\n");
        view_images();
      }

    return SUCCES;
  }

/**
 *  Input del radio(pixeles) de la imágen y/o para salir del programa
 *  @return FAILURE para peticion de salida, de lo contrario SUCCES
 */
enum return_values radio_input()
  {
    while(1)
      {
        printf( "> " );
        fflush(stdout);

        if(fgets( input, INPUT_SIZE, stdin ) == NULL)
         {
           perror("ERROR EN INPUT. Errno");
           exit(EXIT_FAILURE);
         }

        if( strcmp(input, EXIT_MSG) == 0)
          return FAILURE;

        radio = strtol( input, NULL, 10);

        int32_t max_radio_value;
        if( bmp_original->info.image_width < bmp_original->info.image_height )
          max_radio_value = bmp_original->info.image_width;
        else
          max_radio_value = bmp_original->info.image_height;
        max_radio_value = max_radio_value / 2;

        if(radio > 0 && radio <= max_radio_value)
          return SUCCES;

        printf("< Valores aceptados: 1 - %d\n", max_radio_value);
      }
  }

/**
 *  Rutina de ejecución cuando se sale del programa sin errores
 */
void rutina_salida()
  {
    printf("\nSaliendo...\n");
    exit(SUCCES);
  }

/**
 * Inicializa imágen original en la struct bmp_image
 * Si se realiza con exito, se imprimen detalles de la misma
 * @return SUCCES en exito, de lo contrario FAILURE
 */
enum return_values open_image()
  {
    bmp_original = malloc( sizeof(struct _sbmp_image) );
    if( sbmp_load_bmp(ORIGINAL_IMAGE_PATH, bmp_original) == SBMP_OK )
      {
        printf("Imágen abierta\n");
        printf("Tamaño: %d [MB]\n", bmp_original->info.image_size / 1048576);
        printf("Alto: %d [pixeles]\n", bmp_original->info.image_height);
        printf("Ancho: %d [pixeles]\n", bmp_original->info.image_width);
        fflush(stdout);
        return SUCCES;
      }
    else
      {
        perror("ERROR ABRIENDO IMAGEN. Errno");
        printf("Path: %s\n", ORIGINAL_IMAGE_PATH);
        return FAILURE;
      }
  }

/**
 * Edicion de imágen
 */
enum return_values edit_image()
  {
    struct _sbmp_image * bmp_edited = malloc(sizeof(struct _sbmp_image));
    if( sbmp_load_bmp(ORIGINAL_IMAGE_PATH, bmp_edited) == SBMP_ERROR_FILE )
     {
       perror("ERROR ABRIENDO IMAGEN. Errno");
       printf("Path: %s\n", ORIGINAL_IMAGE_PATH);
       return FAILURE;
     }

    struct pixel *center = malloc(sizeof(struct pixel));
    center->y = bmp_edited->info.image_height / 2;
    center->x = bmp_edited->info.image_width / 2;

    struct pixel *actual_pixel = malloc(sizeof(struct pixel));

    for(int32_t i = 0; i < bmp_edited->info.image_height; i++)
      {
        for(int32_t j = 0; j < bmp_edited->info.image_width; j++)
          {
            actual_pixel->x = j;
            actual_pixel->y = i;

            if( get_pixel_area(actual_pixel, center, radio) == IN_AREA )
              {
                  increase_contrast(&bmp_edited->data[i][j]);
                  increase_brightness(&bmp_edited->data[i][j]);
              }
          }
      }

    sbmp_save_bmp(EDITED_IMAGE_PATH, bmp_edited);
    free(bmp_edited);
    return SUCCES;
  }

/**
 * Muestra por syscall ambas imágenes
 * SHOTWELL necesario
 */
void view_images()
 {
   char syscall_original[ strlen("shotwell ") +
    strlen(ORIGINAL_IMAGE_PATH) + 2];
   sprintf(syscall_original, "shotwell %s&", ORIGINAL_IMAGE_PATH);
   system(syscall_original);

   char syscall_edited[ strlen("shotwell ") + strlen(EDITED_IMAGE_PATH) + 1];
   sprintf(syscall_edited, "shotwell %s", EDITED_IMAGE_PATH);
   system(syscall_edited);
 }

/**
 * Indica el area en la cual se encuentra el pixel ingresado con respecto
 * al centro de la imagen
 * @param pixel pixel a analizar
 * @param center pixel central de imágen
 * @return EX_AREA para area externa o IN_AREA para area interna
 */
enum areas get_pixel_area(struct pixel * pixel, struct pixel * center)
 {
   double distance;
   distance = pow(pixel->x - center->x, 2) + pow(pixel->y - center->y, 2);
   distance = sqrt(distance);

   if( distance <= radio)
    return IN_AREA;
   else
    return EX_AREA;
 }

/**
 * Aumento del contraste del pixel en proporcion CONTRAST
 */
void increase_contrast(struct _sbmp_raw_data * data)
  {
    int32_t aux;

    aux = (int32_t) (data->blue * CONTRAST);
    if(aux >= 255)
      data->blue = 255;
    else
      data->blue = (uint8_t) aux;

    aux = (int32_t) (data->green * CONTRAST);
    if(aux >= 255)
      data->green = 255;
    else
      data->green = (uint8_t) aux;

    aux = (int32_t) (data->red * CONTRAST);
    if(aux >= 255)
      data->red = 255;
    else
      data->red = (uint8_t) aux;

  }

/**
 * Aumento del brillo del pixel en cantidad BRIGHTNESS
 */
void increase_brightness(struct _sbmp_raw_data * data)
  {
    int32_t aux;

    aux = data->blue + BRIGHTNESS;
    if(aux >= 255)
      data->blue = 255;
    else
      data->blue = (uint8_t) aux;

    aux = data->green + BRIGHTNESS;
    if(aux >= 255)
      data->green = 255;
    else
      data->green = (uint8_t) aux;

    aux = data->red + BRIGHTNESS;
    if(aux >= 255)
      data->red = 255;
    else
      data->red = (uint8_t) aux;
  }
