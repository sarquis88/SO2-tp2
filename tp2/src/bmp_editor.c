#include "../include/bmp_editor.h"

long radio;
char input[INPUT_SIZE];
struct _sbmp_image *bmp_original;
struct _sbmp_image *bmp_edited;
uint8_t kernel[KERNEL_SIZE][KERNEL_SIZE];

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

    // seteo de kernel
    for(int32_t i = 0; i < KERNEL_SIZE; i++)
      {
        for(int32_t j = 0; j < KERNEL_SIZE; j++)
          {
            kernel[i][j] = 1;
          }
      }
    kernel[KERNEL_SIZE / 2][KERNEL_SIZE / 2] = 2;

    printf("Abriendo imágen...\n");
    if( open_image() == FAILURE)
      exit(EXIT_FAILURE);
    printf("Imágen abierta\n");

    while (1)
      {
        printf("\nIngrese el radio de la imágen"
        " (en pixeles) o 'exit' para salir\n");
        if( radio_input() == FAILURE)
          rutina_salida();
        else
          printf("Radio: %ld pixeles\n", radio);

        printf("Contraste: %.2f\n", CONTRAST);
        printf("Brillo: %d\n", BRIGHTNESS);
        printf("Tamaño del kernel: %dx%d\n", KERNEL_SIZE, KERNEL_SIZE);

        printf("Editando imágen...\n");
        if( edit_image() == FAILURE)
          exit(EXIT_FAILURE);
        printf("Imágen editada\n");
        printf("Edición guardada en: %s\n", EDITED_IMAGE_PATH);
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
        max_radio_value = max_radio_value / 2 - 1;

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
 * Blurea los pixeles fuera del area central
 * Aumenta contraste y brillo de los pixeles centrales
 * @return FAILURE en caso de error, de lo contrario SUCCES
 */
enum return_values edit_image()
  {
    bmp_edited = malloc(sizeof(struct _sbmp_image));
    if( sbmp_initialize_bmp(  bmp_edited,
                              (uint32_t) bmp_original->info.image_height,
                              (uint32_t) bmp_original->info.image_width)
                              == SBMP_ERROR_PARAM )
     {
       perror("ERROR INICIALIZANDO IMAGEN. Errno");
       return FAILURE;
     }

    struct position *center = malloc(sizeof(struct position));
    center->y = bmp_edited->info.image_height / 2;
    center->x = bmp_edited->info.image_width / 2;

    #pragma omp parallel for
      for(int32_t i = 0; i < bmp_edited->info.image_height; i++)
        {
          for(int32_t j = 0; j < bmp_edited->info.image_width; j++)
            {
              struct position * position = malloc(sizeof(struct position));
              position->x = j;
              position->y = i;
              if( get_position_area(position, center) == EX_AREA )
                blure_pixel(position);
              else
                increase_pixel_contrast_brightness(position);
              free(position);
            }
        }

    sbmp_save_bmp(EDITED_IMAGE_PATH, bmp_edited);
    free(bmp_edited);
    return SUCCES;
  }

/**
 * Indica el area en la cual se encuentra el pixel ingresado con respecto
 * al centro de la imagen
 * @param position position del pixel a analizar
 * @param center posicion del pixel central de imágen
 * @return EX_AREA para area externa o IN_AREA para area interna
 */
enum areas get_position_area(struct position * position,
   struct position * center)
 {
   double distance;
   distance = pow(position->x - center->x, 2)
              + pow(position->y - center->y, 2);
   distance = sqrt(distance);

   if( distance <= radio)
    return IN_AREA;
   else
    return EX_AREA;
 }

/**
 * Aumento del contraste y del brillo del pixel ingresado
 * El contraste aumenta en proporcion CONTRAST
 * El brillo aumenta en valor BRIGHTNESS
 * @param position posicion del pixel a modificar
 */
void increase_pixel_contrast_brightness(struct position * position)
  {
    int32_t aux;

    aux = ( (int32_t) (bmp_original->data[position->y][position->x].blue
            * CONTRAST + BRIGHTNESS) );
    if(aux >= 255)
      aux = 255;
    bmp_edited->data[position->y][position->x].blue = (uint8_t) aux;

    aux = ( (int32_t) (bmp_original->data[position->y][position->x].green
            * CONTRAST + BRIGHTNESS) );
    if(aux >= 255)
      aux = 255;
    bmp_edited->data[position->y][position->x].green = (uint8_t) aux;

    aux = ( (int32_t) (bmp_original->data[position->y][position->x].red
            * CONTRAST + BRIGHTNESS) );
    if(aux >= 255)
      aux = 255;
    bmp_edited->data[position->y][position->x].red = (uint8_t) aux;
  }

/**
 * Blurea el pixel ingresado realizando convolusion
 * @param position posicion del pixel a blurear
 */
void blure_pixel(struct position * position)
  {
  int32_t off = KERNEL_SIZE / 2;

    // chequeo de bordes: si el pixel está en el borde, no se hace nada
    if( position->x < off || position->y < off)
      return;
    if( position->x + off >= bmp_edited->info.image_width
        ||  position->y + off >= bmp_edited->info.image_height)
      return;

    uint32_t acum_blue = 0;
    uint32_t acum_green = 0;
    uint32_t acum_red = 0;

    int32_t i = -1 * off;
    for(; i <= off; i++)
      {
        int32_t j = -1 * off;
        for(; j <= off; j++)
          {
            int32_t ker_i = i + off;
            int32_t ker_j = j + off;
            int32_t bmp_i = i + position->y;
            int32_t bmp_j = j + position->x;

            acum_blue   = ( (uint32_t) (acum_blue
                          + (uint32_t) kernel[ker_i][ker_j]
                          * bmp_original->data[bmp_i][bmp_j].blue));

            acum_green  = ( (uint32_t) (acum_green
                          + (uint32_t) kernel[ker_i][ker_j]
                          * bmp_original->data[bmp_i][bmp_j].green));

            acum_red    = ( (uint32_t) (acum_red
                          + (uint32_t) kernel[ker_i][ker_j]
                          * bmp_original->data[bmp_i][bmp_j].red));
          }
      }

      // chequeo de overflow (TODO: podria omitirse)
      uint32_t aux;

      aux = acum_blue / (uint32_t) pow(KERNEL_SIZE, 2);
      if(aux > 255)
        aux = 255;
      bmp_edited->data[position->y][position->x].blue = (uint8_t) aux;

      aux = acum_green / (uint32_t) pow(KERNEL_SIZE, 2);
      if(aux > 255)
        aux = 255;
      bmp_edited->data[position->y][position->x].green = (uint8_t) aux;

      aux = acum_red / (uint32_t) pow(KERNEL_SIZE, 2);
      if(aux > 255)
        aux = 255;
      bmp_edited->data[position->y][position->x].red = (uint8_t) aux;
  }
