#include "../include/bmp_editor.h"

long radio;
char input[INPUT_SIZE];
struct _sbmp_image *bmp_original;
struct _sbmp_image *bmp_edited;
uint8_t kernel[KERNEL_SIZE][KERNEL_SIZE];
double start, seconds;

/**
 *  Funcion principal
 */
int32_t main( int32_t argc, char *argv[] )
  {

    // chequeo de argumentos
  	if ( argc < 2 ) {
  		printf("Uso: %s <multithread>\n", argv[0]);
      printf("0 -> multithread desactivado\n1 -> multithread activado\n");
  		exit(1);
  	}

    if( strtol(argv[1], NULL, 10) == 0 )
      omp_set_num_threads( 1 );
    else
      omp_set_num_threads( omp_get_max_threads() );

    // activar handler para SIGINT
  	struct sigaction sa;
    sa.sa_handler = rutina_salida;
  	sigaction(SIGINT, &sa,  NULL);

    printf("\n");
    printf("<----------------------------------------------------------->\n");
    printf("<-------------Editor de imágenes en formato BMP------------->\n");
    printf("<----------------------------------------------------------->\n");
    printf("\n");
    fflush(stdout);

    // seteo de kernel
    for(int8_t i = 0; i < KERNEL_SIZE; i++)
      {
        for(int8_t j = 0; j < KERNEL_SIZE; j++)
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
          rutina_salida(SUCCES);
        else
          printf("Radio: %ld pixeles\n", radio);

        printf("Contraste: %.2f\n", CONTRAST);
        printf("Brillo: %d\n", BRIGHTNESS);
        printf("Tamaño del kernel: %dx%d\n", KERNEL_SIZE, KERNEL_SIZE);

        printf("Editando imágen...\n");
        start_time();
        if( edit_image() == FAILURE)
          exit(EXIT_FAILURE);
        printf("Imágen editada\n");
        printf("Edición guardada en: %s\n", EDITED_IMAGE_PATH);
        stop_time();
        printf("Tiempo: %0.2f segundos\n", seconds);
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

        int16_t max_radio_value;
        if( bmp_original->info.image_width < bmp_original->info.image_height )
          max_radio_value = (int16_t) bmp_original->info.image_width;
        else
          max_radio_value = (int16_t) bmp_original->info.image_height;
        max_radio_value = ( (int16_t) (max_radio_value / 2 - 1) );

        if(radio > 0 && radio <= max_radio_value)
          return SUCCES;

        printf("< Valores aceptados: 1 - %d\n", max_radio_value);
      }
  }

/**
 *  Rutina de ejecución cuando se sale del programa sin errores
 */
void rutina_salida(int32_t sig)
  {
    if(sig)
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
    set_position( center, ( (int16_t) (bmp_edited->info.image_width / 2) ),
                          ( (int16_t) (bmp_edited->info.image_height / 2) ) );

    #pragma omp parallel for
      for(int16_t i = 0; i < bmp_edited->info.image_height; i++)
        {
          for(int16_t j = 0; j < bmp_edited->info.image_width; j++)
            {
              struct position * position = malloc(sizeof(struct position));
              set_position(position, j, i);
              if( get_position_area(position, center) == EX_AREA )
                blure_pixel(position);
              else
                increase_pixel_contrast_brightness(position);
              free(position);
            }
        }

    sbmp_save_bmp(EDITED_IMAGE_PATH, bmp_edited);
    free(bmp_edited);
    free(center);
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
    int16_t aux;

    aux = ( (int16_t) (bmp_original->data[position->y][position->x].blue
            * CONTRAST + BRIGHTNESS) );
    if(aux >= 255)
        aux = 255;
    bmp_edited->data[position->y][position->x].blue = (uint8_t) aux;

    aux = ( (int16_t) (bmp_original->data[position->y][position->x].green
            * CONTRAST + BRIGHTNESS) );
    if(aux >= 255)
        aux = 255;
    bmp_edited->data[position->y][position->x].green = (uint8_t) aux;

    aux = ( (int16_t) (bmp_original->data[position->y][position->x].red
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
    int8_t off = KERNEL_SIZE / 2;

    // si el pixel está en el borde, se hace igual a la imagen original
    // ( sin blure ni nada )
    int8_t borde = 0;
    if( position->x < off || position->y < off)
      borde = 1;
    else if(      position->x + off >= bmp_edited->info.image_width
              ||  position->y + off >= bmp_edited->info.image_height)
      borde = 1;
    if(borde)
      {
        bmp_edited->data[position->y][position->x].blue =
          bmp_original->data[position->y][position->x].blue;
        bmp_edited->data[position->y][position->x].green =
          bmp_original->data[position->y][position->x].green;
        bmp_edited->data[position->y][position->x].red =
          bmp_original->data[position->y][position->x].red;
        return;
      }

    uint32_t acum_blue = 0;
    uint32_t acum_green = 0;
    uint32_t acum_red = 0;

    for(int8_t i = (int8_t) -off; i <= off; i++)
      {
        for(int8_t j = (int8_t) -off; j <= off; j++)
          {
            int8_t ker_i  = ( (int8_t)  (i + off) );
            int8_t ker_j  = ( (int8_t)  (j + off) );
            int16_t bmp_i = ( (int16_t) (i + position->y) );
            int16_t bmp_j = ( (int16_t) (j + position->x) );

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

      bmp_edited->data[position->y][position->x].blue = ( (uint8_t)
                  (acum_blue / pow(KERNEL_SIZE, 2)) );
      bmp_edited->data[position->y][position->x].green = ( (uint8_t)
                  (acum_green / pow(KERNEL_SIZE, 2)) );
      bmp_edited->data[position->y][position->x].red = ( (uint8_t)
                  (acum_red / pow(KERNEL_SIZE, 2)) );
  }

/*
 * Setea la posicion de la estructura ingresada
 * @param position struct de posicion inicializada
 * @param x coordenada x
 * @param y coordenada y
 */
void set_position(struct position * position, int16_t x, int16_t y)
  {
    position->x = x;
    position->y = y;
  }

/**
 * Comienzo a medir tiempo
 */
void start_time()
  {
    start = omp_get_wtime();
  }

/**
 * Termino de medir tiempo
 */
void stop_time()
  {
    seconds = omp_get_wtime() - start;
  }
