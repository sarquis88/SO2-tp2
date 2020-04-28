#include "../include/bmp_editor.h"

long radio;
char input[INPUT_SIZE];
struct _sbmp_image *bmp_original;

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
    if( open_image() == FAIL)
      exit(EXIT_FAILURE);
    printf("Imágen abierta\n");

    while (1)
      {
        printf("\nIngrese el radio de la imágen (en pixeles) o 'exit' para salir\n");
        if( radio_input() == FAIL)
          rutina_salida();
        else
          printf("Radio: %ld pixeles\n", radio);

        printf("Editando imágen...\n");
        if( edit_image() == FAIL)
          exit(EXIT_FAILURE);
        printf("Imágen editada\n");

        printf("Mostrando imágenes...\n");
        view_images();
      }

    return SUCCES;
  }

/**
 *  Input del radio(pixeles) de la imágen y/o para salir del programa
 *  @return FAIL para peticion de salida, de lo contrario SUCCES
 */
enum succes radio_input()
  {
    char* in;
    do
      {
        printf( "> " );
        fflush(stdout);
        in = fgets( input, INPUT_SIZE, stdin );
      } while( in == NULL);

    if( strcmp(input, EXIT_MSG) == 0)
      return FAIL;
    else
      {
        radio = strtol( input, NULL, 10);
        if( radio == LONG_MIN || radio == LONG_MAX)
          {
            perror("NUMERO INGRESADO FUERA DE RANGO. Errno");
            exit(EXIT_FAILURE);
          }
        return SUCCES;
      }
  }

/**
 *  Rutina de ejecución cuando se sale del programa sin errores
 */
void rutina_salida()
  {
    printf("\nSaliendo...\n");
    exit(0);
  }

/**
 * Inicializa imágen original en la struct bmp_image
 * Si se realiza con exito, se imprimen detalles de la misma
 * @return SUCCES en exito, de lo contrario FAIL
 */
enum succes open_image()
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
        return FAIL;
      }
  }

/**
 * Edicion de imágen
 */
enum succes edit_image()
  {
    struct _sbmp_image * bmp_edited = malloc(sizeof(struct _sbmp_image));
    if( sbmp_load_bmp(ORIGINAL_IMAGE_PATH, bmp_edited) == SBMP_ERROR_FILE )
     {
       perror("ERROR ABRIENDO IMAGEN. Errno");
       printf("Path: %s\n", ORIGINAL_IMAGE_PATH);
       return FAIL;
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
              bmp_edited->data[i][j].green = bmp_edited->data[i][j].green / 2;
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
