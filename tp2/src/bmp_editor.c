  #include "../include/bmp_editor.h"

struct _sbmp_image *bmp_original;
long radio;
uint16_t ** kernel, norm;
int8_t off;
double time_s;

/**
 *  Funcion principal
 */
int32_t
main( int32_t argc, char *argv[] )
  {
    uint8_t brillo;
    double contraste;
    int8_t threads;

    // chequeo de argumentos
  	if ( argc < 2 )
      {
    		printf("Uso: %s <cantidad_threads> <brillo> <contraste>\n", argv[0]);
    		rutina_salida(EXIT_FAILURE);
  	  }
    if ( argc < 4 )
      {
        brillo = DEFAULT_BRIGHTNESS;
        contraste = DEFAULT_CONTRAST;
      }
    else
      {
        brillo = (uint8_t) strtol(argv[2], NULL, 10);
        contraste = (double) strtod(argv[3], NULL);
      }

    threads = (int8_t) strtol(argv[1], NULL, 10);
    if(!threads)
      threads = 1;
    omp_set_num_threads( threads );

    struct sigaction sig_act;
    sig_act.sa_handler = rutina_salida;
  	sigaction(SIGINT, &sig_act,  NULL);

    printf("\n");
    printf("<----------------------------------------------------------->\n");
    printf("<-------------Editor de imágenes en formato BMP------------->\n");
    printf("<----------------------------------------------------------->\n");
    printf("\n");
    fflush(stdout);

    printf("Abriendo imágen...\n");
    if( open_image() == FAILURE)
      rutina_salida(EXIT_FAILURE);
    printf("Imágen abierta\n");

    printf("Seteando kernel...\n");
    kernel = calloc (KERNEL_SIZE, sizeof (int8_t *));
    for (int8_t k = 0; k < KERNEL_SIZE; k++)
      kernel[k] = calloc (KERNEL_SIZE, sizeof (uint16_t));
    set_kernel(kernel);
    norm = get_norm(kernel);
    printf("Kernel seteado\n");

    printf("Contraste: %.2f\n", contraste);
    printf("Brillo: %d\n", brillo);
    printf("Tamaño del kernel: %dx%d\n", KERNEL_SIZE, KERNEL_SIZE);
    printf("Cantidad de hilos: %d\n", threads );

    printf("\n");
    printf("<----------------------------------------------------------->\n");
    printf("<----------------------------------------------------------->\n");
    printf("<----------------------------------------------------------->\n");
    printf("\n");
    fflush(stdout);

    radio = 0;

    while (1)
      {
        printf("\nIngrese el radio de la imágen"
        " (en pixeles) o 'exit' para salir\n");

        int8_t corridas = 0;
        int8_t input = radio_input();
        switch (input) {
          case INPUT_OK:
            {
              corridas = 1;
              break;
            }
          case INPUT_TEST:
            {
              corridas = TEST_CORRIDAS;
              break;
            }
          case INPUT_EXIT:
            {
              for(int8_t i = 0; i < KERNEL_SIZE; i++)
                free(kernel[i]);
              free(kernel);

              for(int32_t i = 0; i < bmp_original->info.image_height ; i++)
                free(bmp_original->data[i]);
              free(bmp_original->data);
              free(bmp_original);

              rutina_salida(SUCCES);
              break;
            }
          default:
            {
              rutina_salida(EXIT_FAILURE);
              break;
            }
        }

        double acum_time = 0;
        double tiempos[corridas];
        for(int8_t i = 0; i < corridas; i++)
          {
            printf("Radio: %ld pixeles\n", radio);
            printf("Editando imágen...\n");
            if( edit_image(brillo, contraste) == FAILURE)
              rutina_salida(EXIT_FAILURE);
            printf("Imágen editada\n");
            printf("Edición guardada en: %s\n", EDITED_IMAGE_PATH);
            printf("Tiempo: %0.2f segundos\n", time_s);

            if( input == INPUT_TEST ) // en modo test
              {                       // calculos estadisticos
                double avg;
                tiempos[i] = time_s;
                acum_time += time_s;
                avg = acum_time / (i + 1);
                printf("Tiempo promedio: %0.2f segundos\n", avg);
                printf("Corrida nro: %d\n\n", i + 1);

                if( i == corridas - 1 )
                  {
                    acum_time = 0;
                    for(int8_t j = 0; j < corridas; j++)
                      {
                        if( tiempos[j] < avg )
                          acum_time += avg - tiempos[j];
                        else
                          acum_time += tiempos[j] - avg;
                      }
                    double desv = acum_time / corridas;
                    printf("Test terminado\n");
                    printf("Tiempo promedio: %0.2f segundos\n", avg);
                    printf("Desvio: %0.2f segundos\n", desv);
                  }
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
enum input_codes
radio_input()
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
 *  @param sig parametro de salida
 */
void
rutina_salida(int32_t sig)
  {
    if(sig)
      printf("\nSaliendo con errores...\n");
    else
      printf("\nSaliendo sin errores...\n");

    exit(sig);
  }

/**
 * Inicializa imágen original en la struct bmp_image
 * Si se realiza con exito, se imprimen detalles de la misma
 * @return SUCCES en exito, de lo contrario FAILURE
 */
enum return_values
open_image()
  {
    bmp_original = malloc( sizeof(struct _sbmp_image) );
    if( sbmp_load_bmp(ORIGINAL_IMAGE_PATH, bmp_original) == SBMP_OK )
      {
        printf("Imágen abierta\n");
        printf("Tamaño: %u [MB]\n", bmp_original->info.image_size / 1048576);
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
 * @param brillo cantidad de brillo a agregar
 * @param contraste cantidad de contraste a agregar
 * @return FAILURE en caso de error, de lo contrario SUCCES
 */
enum return_values
edit_image(uint8_t brillo, double contraste)
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

    off = KERNEL_SIZE / 2;
    if(KERNEL_SIZE % 2 == 0)
      off--;

    int16_t center_x = (int16_t) (bmp_edited->info.image_width / 2);
    int16_t center_y = (int16_t) (bmp_edited->info.image_height / 2);

    int8_t area = 0;

    double start = get_time();
    #pragma omp parallel for schedule(static)
    for(int16_t i = 0; i < bmp_edited->info.image_height; i++)
      {
        for(int16_t j = 0; j < bmp_edited->info.image_width; j++)
          {
            area = get_position_area( bmp_edited, j, i, center_x, center_y);
            switch (area)
              {
                case EX_AREA:
                  {
                    blure_pixel(bmp_edited, j, i);
                    break;
                  }
                case IN_AREA:
                  {
                    increase_pixel_contrast_brightness( bmp_edited, j, i,
                                                        brillo, contraste);
                    break;
                  }
                case LIM_AREA:
                  {
                    edit_limits(bmp_edited, j, i);
                    break;
                  }
                default:
                  {
                    printf("ERROR ANALIZANDO AREA DE PIXEL\n");
                    rutina_salida(EXIT_FAILURE);
                  }
              }
          }
      }
    time_s = get_time() - start;

    sbmp_save_bmp(EDITED_IMAGE_PATH, bmp_edited);

    for(int32_t i = 0; i < bmp_edited->info.image_height ; i++)
      free(bmp_edited->data[i]);
    free(bmp_edited->data);
    free(bmp_edited);

    return SUCCES;
  }

/**
 * Indica el area en la cual se encuentra el pixel ingresado con respecto
 * al centro de la imagen
 * @param bmp_edited imagen a editar
 * @param pixel_x coordenada x de pixel
 * @param pixel_y coordenada y de pixel
 * @param center_x coordenada x de centro
 * @param center_y coordenada y de pixel
 * @return EX_AREA para area externa o IN_AREA para area interna
 */
enum areas
get_position_area( struct _sbmp_image * bmp_edited,
                              int16_t pixel_x, int16_t pixel_y,
                              int16_t center_x, int16_t center_y)
 {
   double distance;

   distance = (pixel_x - center_x) * (pixel_x - center_x) +
              (pixel_y - center_y) * (pixel_y - center_y);
   if( distance <= radio * radio)
    return IN_AREA;

   if( pixel_x < off || pixel_y < off )
    return LIM_AREA;
   if(            pixel_x + off >= bmp_edited->info.image_width
              ||  pixel_y + off >= bmp_edited->info.image_height)
    return LIM_AREA;

   return EX_AREA;
 }

/**
 * Aumento del contraste y del brillo del pixel ingresado
 * El contraste aumenta en proporcion contraste
 * El brillo aumenta en valor brillo
 * @param bmp_edited imagen a editar
 * @param pixel_x coordenada x de pixel
 * @param pixel_y coordenada y de pixel
 * @param brillo cantidad de brillo a agregar al pixel
 * @param contraste cantidad de contraste a agrear al pixel
 */
void
increase_pixel_contrast_brightness(struct _sbmp_image *bmp_edited,
                                        int16_t pixel_x, int16_t pixel_y,
                                        uint8_t brillo, double contraste)
  {
    int16_t aux;

    aux = ( (int16_t) (bmp_original->data[pixel_y][pixel_x].blue
            * contraste + brillo) );
    if(aux >= 255)
        aux = 255;
    bmp_edited->data[pixel_y][pixel_x].blue = (uint8_t) aux;

    aux = ( (int16_t) (bmp_original->data[pixel_y][pixel_x].green
            * contraste + brillo) );
    if(aux >= 255)
        aux = 255;
    bmp_edited->data[pixel_y][pixel_x].green = (uint8_t) aux;

    aux = ( (int16_t) (bmp_original->data[pixel_y][pixel_x].red
            * contraste + brillo) );
    if(aux >= 255)
        aux = 255;
    bmp_edited->data[pixel_y][pixel_x].red = (uint8_t) aux;
  }

/*
 * Edicion de limites de imágen en los cuales no se hace blureado ni aumento
 * de contraste o brillo. Estos pixeles se dejan iguales a los originales
 * @param bmp_edited imagen a editar
 * @param pixel_x coordenada x de pixel
 * @param pixel_y coordenada y de pixel
 */
void
edit_limits(struct _sbmp_image * bmp_edited, int16_t pixel_x,
                                                  int16_t pixel_y)
  {
    bmp_edited->data[pixel_y][pixel_x].blue =
      bmp_original->data[pixel_y][pixel_x].blue;
    bmp_edited->data[pixel_y][pixel_x].green =
      bmp_original->data[pixel_y][pixel_x].green;
    bmp_edited->data[pixel_y][pixel_x].red =
      bmp_original->data[pixel_y][pixel_x].red;
  }

/**
 * Blurea el pixel ingresado realizando convolusion
 * @param bmp_edited imagen a editar
 * @param pixel_x coordenada x de pixel
 * @param pixel_y coordenada y de pixel
 */
void
blure_pixel( struct _sbmp_image *bmp_edited, int16_t pixel_x,
                                                  int16_t pixel_y)
  {
    uint32_t acum_blue = 0;
    uint32_t acum_green = 0;
    uint32_t acum_red = 0;

    for(int8_t i = (int8_t) -off; i <= off; i++)
      {
        for(int8_t j = (int8_t) -off; j <= off; j++)
          {
            acum_blue   += (  (uint32_t) kernel[i + off][j + off] *
                            bmp_original->data[i + pixel_y][j + pixel_x].blue);

            acum_green  += (  (uint32_t) kernel[i + off][j + off] *
                            bmp_original->data[i + pixel_y][j + pixel_x].green);

            acum_red    += (   (uint32_t) kernel[i + off][j + off] *
                            bmp_original->data[i + pixel_y][j + pixel_x].red);
          }
      }

      bmp_edited->data[pixel_y][pixel_x].blue = ( (uint8_t)
                  (acum_blue / norm) );
      bmp_edited->data[pixel_y][pixel_x].green = ( (uint8_t)
                  (acum_green / norm) );
      bmp_edited->data[pixel_y][pixel_x].red = ( (uint8_t)
                  (acum_red / norm) );
  }

/**
 * Obtengo tiempo "actual"
 * @return tiempo
 */
double
get_time()
  {
    return  omp_get_wtime();
  }

/**
 *  Getter de valor de normalizacion
 *  @param kernel kernel usado
 *  @return valor de la normalizacion
 */
uint16_t
get_norm(uint16_t ** kernel)
  {
    uint16_t norm_aux = 0;
    for (int16_t i = 0; i < KERNEL_SIZE; i++)
      {
        for (int16_t j = 0; j < KERNEL_SIZE; j++)
          {
            norm_aux = ( (uint16_t) ( norm_aux + kernel[i][j] ) );
          }
      }
    return norm_aux;
  }

  /**
   * Inicializacion de kernel
   * @param kernel matriz kernel a setear
   */
void
set_kernel(uint16_t **kernel)
  {
    uint16_t st_val = 1;
    for (int8_t j = 0; j < KERNEL_SIZE; j++)
      {
        kernel[0][j] = st_val;
        kernel[KERNEL_SIZE - 1][j] = st_val;
      }

    for (int8_t i = 1; i < KERNEL_SIZE / 2 + 1; i++)
      {
        for (int8_t j = 0; j < KERNEL_SIZE; j++)
          {
            if (j >= i && j < (KERNEL_SIZE - i))
              kernel[i][j] = (uint16_t) (kernel[i - 1][j] + (uint16_t) 1);
            else
              kernel[i][j] = kernel[i - 1][j];
          }
      }
    for (int8_t i = 1; i < KERNEL_SIZE / 2; i++)
      {
        for (int8_t j = 0; j < KERNEL_SIZE; j++)
          {
            int8_t index = ( (int8_t) (KERNEL_SIZE / 2 - i) );
            if(KERNEL_SIZE % 2 == 0)
              index--;
            kernel[i + KERNEL_SIZE / 2][j] = kernel[index][j];
          }
      }
  }
