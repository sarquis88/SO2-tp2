#include "../include/bmp_editor.h"

struct _sbmp_image *bmp_original;
long radio;
uint16_t ** kernel, norm;

/**
 *  Funcion principal
 */
int32_t main( int32_t argc, char *argv[] )
  {

    // chequeo de argumentos
  	if ( argc < 2 ) {
  		printf("Uso: %s <cantidad_threads>\n", argv[0]);
  		exit(1);
  	}

    int8_t threads = (int8_t) strtol(argv[1], NULL, 10);
    omp_set_num_threads( threads );

  	struct sigaction sa;
    sa.sa_handler = rutina_salida;
  	sigaction(SIGINT, &sa,  NULL);

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

    kernel = calloc (KERNEL_SIZE, sizeof (int *));
    for (int k = 0; k < KERNEL_SIZE; k++)
      kernel[k] = calloc (KERNEL_SIZE, sizeof (uint16_t));
    set_kernel(kernel);

    norm = get_norm(kernel);

    printf("Contraste: %.2f\n", CONTRAST);
    printf("Brillo: %d\n", BRIGHTNESS);
    printf("Tamaño del kernel: %dx%d\n", KERNEL_SIZE, KERNEL_SIZE);
    printf("Cantidad de hilos: %d\n", threads );

    double start, seconds, acum_time;
    acum_time = 0;
    radio = 0;

    while (1)
      {
        printf("\nIngrese el radio de la imágen"
        " (en pixeles) o 'exit' para salir\n");

        int8_t corridas = 0;
        int8_t input = radio_input();
        if( input == INPUT_OK)
          corridas = 1;
        else if( input == INPUT_TEST)
          corridas = TEST_CORRIDAS;
        else if( input == INPUT_ERR || input == INPUT_EXIT)
          rutina_salida(SUCCES);

        for(int8_t i = 0; i < corridas; i++)
          {
            printf("Radio: %ld pixeles\n", radio);

            printf("Editando imágen...\n");
            start = get_time();
            if( edit_image() == FAILURE)
              exit(EXIT_FAILURE);
            seconds = get_time() - start;
            printf("Imágen editada\n");
            printf("Edición guardada en: %s\n", EDITED_IMAGE_PATH);

            printf("Tiempo: %0.2f segundos\n", seconds);

            if( input == INPUT_TEST )
              {
                acum_time += seconds;
                printf("Tiempo promedio: %0.2f segundos\n", acum_time / (i + 1) );
                printf("Corrida nro: %d\n\n", i + 1);
              }
          }
      }

    return SUCCES;
  }

/**
 *  Input del radio(pixeles) de la imágen y/o para salir del programa
 *  Si el input es 'test', el radio se hace maximo y se ejecutan 30 ediciones
 *  @return EXIT para peticion de salida
 *  @return OK para entrada de radio normal
 *  @return TEST para testear
 *  @return ERR si sucede error
 */
enum input_codes radio_input()
  {
    char input[INPUT_SIZE];
    while(1)
      {
        printf( "> " );
        fflush(stdout);

        if(fgets( input, INPUT_SIZE, stdin ) == NULL)
         {
           perror("ERROR EN INPUT. Errno");
           return INPUT_ERR;
         }

        if( strcmp(input, EXIT_MSG) == 0)
          return INPUT_EXIT;

        int16_t max_radio_value;
        if( bmp_original->info.image_width < bmp_original->info.image_height )
          max_radio_value = (int16_t) bmp_original->info.image_width;
        else
          max_radio_value = (int16_t) bmp_original->info.image_height;
        max_radio_value = ( (int16_t) (max_radio_value / 2 - 1) );

        if( strcmp(input, TEST_MSG) == 0)
          {
            radio = max_radio_value;
            return INPUT_TEST;
          }

        radio = strtol( input, NULL, 10);

        if(radio > 0 && radio <= max_radio_value)
          return INPUT_OK;

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

    struct _sbmp_image *bmp_edited = malloc(sizeof(struct _sbmp_image));
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

    int8_t area = 0;

    #pragma omp parallel for schedule(guided)
      for(int16_t i = 0; i < bmp_edited->info.image_height; i++)
        {
          for(int16_t j = 0; j < bmp_edited->info.image_width; j++)
            {
              struct position * position = malloc(sizeof(struct position));
              set_position(position, j, i);
              area = get_position_area(bmp_edited, position, center);
              if( area == EX_AREA )
                blure_pixel(bmp_edited, position);
              else if( area == IN_AREA)
                increase_pixel_contrast_brightness(bmp_edited, position);
              else
                edit_limits(bmp_edited, position);
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
 * @param bmp_edited imagen a editar
 * @param position position del pixel a analizar
 * @param center posicion del pixel central de imágen
 * @return EX_AREA para area externa o IN_AREA para area interna
 */
enum areas get_position_area( struct _sbmp_image * bmp_edited,
                              struct position * position,
                              struct position * center)
 {
   double distance;
   distance = pow(position->x - center->x, 2)
              + pow(position->y - center->y, 2);
   distance = sqrt(distance);

   if( distance <= radio)
    return IN_AREA;

   int8_t off = KERNEL_SIZE / 2;
   if(KERNEL_SIZE % 2 == 0)
    off--;

   if( position->x < off || position->y < off )
    return LIM_AREA;
   if(            position->x + off >= bmp_edited->info.image_width
              ||  position->y + off >= bmp_edited->info.image_height)
    return LIM_AREA;

   return EX_AREA;
 }

/**
 * Aumento del contraste y del brillo del pixel ingresado
 * El contraste aumenta en proporcion CONTRAST
 * El brillo aumenta en valor BRIGHTNESS
 * @param bmp_edited imagen a editar
 * @param position posicion del pixel a modificar
 */
void increase_pixel_contrast_brightness(struct _sbmp_image *bmp_edited,
                                        struct position * position)
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

/*
 * Edicion de limites de imágen en los cuales no se hace blureado ni aumento
 * de contraste o brillo. Estos pixeles se dejan iguales a los originales
 * @param position posicion del pixel
 * @param bmp_edited imagen a editar
 */
void edit_limits(struct _sbmp_image * bmp_edited, struct position * position)
  {
    bmp_edited->data[position->y][position->x].blue =
      bmp_original->data[position->y][position->x].blue;
    bmp_edited->data[position->y][position->x].green =
      bmp_original->data[position->y][position->x].green;
    bmp_edited->data[position->y][position->x].red =
      bmp_original->data[position->y][position->x].red;
  }

/**
 * Blurea el pixel ingresado realizando convolusion
 * @param position posicion del pixel a blurear
 */
void blure_pixel( struct _sbmp_image *bmp_edited, struct position * position)
  {
    int8_t off = KERNEL_SIZE / 2;
    if(KERNEL_SIZE % 2 == 0)
     off--;

    uint32_t acum_blue = 0;
    uint32_t acum_green = 0;
    uint32_t acum_red = 0;

    int8_t ker_i;
    int16_t bmp_i;
    int8_t ker_j;
    int16_t bmp_j;

    for(int8_t i = (int8_t) -off; i <= off; i++)
      {

        ker_i  = ( (int8_t)  (i + off) );
        bmp_i = ( (int16_t) (i + position->y) );

        for(int8_t j = (int8_t) -off; j <= off; j++)
          {

            ker_j  = ( (int8_t)  (j + off) );
            bmp_j = ( (int16_t) (j + position->x) );

            acum_blue   += (  (uint32_t) kernel[ker_i][ker_j]
                              * bmp_original->data[bmp_i][bmp_j].blue);

            acum_green  += (  (uint32_t) kernel[ker_i][ker_j]
                              * bmp_original->data[bmp_i][bmp_j].green);

            acum_red    += (   (uint32_t) kernel[ker_i][ker_j]
                              * bmp_original->data[bmp_i][bmp_j].red);
          }
      }

      bmp_edited->data[position->y][position->x].blue = ( (uint8_t)
                  (acum_blue / norm) );
      bmp_edited->data[position->y][position->x].green = ( (uint8_t)
                  (acum_green / norm) );
      bmp_edited->data[position->y][position->x].red = ( (uint8_t)
                  (acum_red / norm) );
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
 * Obtengo tiempo "actual"
 * @return tiempo
 */
double get_time()
  {
    return  omp_get_wtime();
  }

/**
 * Inicializacion de kernel
 * @param kernel matriz kernel a setear
 */
void set_kernel(uint16_t ** kernel)
  {
    for(int8_t i = 0; i < KERNEL_SIZE; i++)
      {
        for(int8_t j = 0; j < KERNEL_SIZE; j++)
          {
            kernel[i][j] = 1;
          }
      }

    if(KERNEL_SIZE % 2 == 0)
      {
        kernel[KERNEL_SIZE / 2]     [KERNEL_SIZE / 2] = 2;
        kernel[KERNEL_SIZE / 2]     [KERNEL_SIZE / 2 - 1] = 2;
        kernel[KERNEL_SIZE / 2 - 1] [KERNEL_SIZE / 2] = 2;
        kernel[KERNEL_SIZE / 2 - 1] [KERNEL_SIZE / 2 - 1] = 2;
      }
    else
      kernel[KERNEL_SIZE / 2][KERNEL_SIZE / 2] = 2;
  }

/**
 *  Getter de valor de normalizacion
 *  @param kernel kernel usado
 *  @return valor de la normalizacion
 */
uint16_t get_norm(uint16_t ** kernel)
  {
    uint16_t norm = 0;
    for (int16_t i = 0; i < KERNEL_SIZE; i++)
      {
        for (int j = 0; j < KERNEL_SIZE; j++)
          {
            norm = ( (uint16_t) ( norm + kernel[i][j] ) );
          }
      }
    return norm;
  }
