#include "../include/bmp_editor.h"

long radio;
char input[INPUT_SIZE];
struct _sbmp_image *bmp_image;

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
    
    open_image();

    // while (1)
    //   {
    //     if( !radio_input() )
    //       rutina_salida();
    //     else
    //       printf("Radio: %ld pixeles\n", radio);
    //
    //     printf("Editando imágen: %s\n", ORIGINAL_IMAGE_NAME);
    //     printf("Edición terminada\n");
    //     printf("Imágen guardada en '%s%s'\n", RESOURCES_PATH,
    //      EDITED_IMAGE_NAME);
    //   }
  }

/**
 *  Input del radio(pixeles) de la imágen y/o para salir del programa
 *  @return 0 para peticion de salida, de lo contrario 1
 */
int32_t radio_input()
  {
    char* in;
    printf("\nIngrese el radio de la imágen (en pixeles) o 'exit' para salir\n");
    do
      {
        printf( "> " );
        fflush(stdout);
        in = fgets( input, INPUT_SIZE, stdin );
      } while( in == NULL);

    if( strcmp(input, EXIT_MSG) == 0)
      return 0;
    else
      {
        radio = strtol( input, NULL, 10);
        if( radio == LONG_MIN || radio == LONG_MAX)
          {
            perror("NUMERO INGRESADO FUERA DE RANGO. Errno");
            exit(EXIT_FAILURE);
          }
        return 1;
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
 */
void open_image()
  {
    printf("Abriendo imágen %s ...\n", ORIGINAL_IMAGE_NAME);
    char path[ strlen(RESOURCES_PATH) + strlen(ORIGINAL_IMAGE_NAME) + 1];
    sprintf(path, "%s%s", RESOURCES_PATH, ORIGINAL_IMAGE_NAME);

    bmp_image = malloc( sizeof(struct _sbmp_image) );
    if( sbmp_load_bmp(path, bmp_image) == SBMP_OK )
      {
        printf("Imágen abierta\n");
        printf("Tamaño: %d [MB]\n", bmp_image->info.image_size / 1048576);
        printf("Alto: %d [pixeles]\n", bmp_image->info.image_height);
        printf("Ancho: %d [pixeles]\n", bmp_image->info.image_width);
        fflush(stdout);
      }
    else
      {
        perror("ERROR ABRIENDO IMAGEN. Errno");
        printf("Path: %s\n", path);
        exit(EXIT_FAILURE);
      }
  }
