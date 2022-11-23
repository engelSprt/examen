#include "omp.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 1

int main() {
  omp_set_num_threads(NUM_THREADS);
  const double TiempoInicio = omp_get_wtime();

  FILE *image, *outputImage, *lecturas;
  image = fopen("imagen9.bmp", "rb");      // Imagen Imagen de entrada
  outputImage = fopen("imagen_salida.bmp", "wb"); // Imagen Imagen de salida
  int desenfoque = 41; // tamaño del arreglo a promediar
  int mitad = (int)desenfoque / 2;
  long ancho;
  long alto;
  unsigned char r, g, b; // Pixel
  unsigned char *puntero;
  unsigned char xx[54]; // arreglo para la cabecera de la imagen
  int contador = 0;

  for (int i = 0; i < 54; i++) {
    xx[i] = fgetc(image);
    fputc(xx[i], outputImage); // Copia cabecera a nueva imagen
  }

  ancho = (long)xx[20] * 65536 + (long)xx[19] * 256 + (long)xx[18];
  alto = (long)xx[24] * 65536 + (long)xx[23] * 256 + (long)xx[22];

  printf("largo img %li\n", alto);
  printf("ancho img %li\n", ancho);

  puntero = (unsigned char *)malloc(alto * ancho * 3 * sizeof(unsigned char));
  unsigned char imgDeEntrada[alto][ancho];
  unsigned char imgDesenfocada[alto][ancho];

  // Se comienza a leer el valor para cada pixel y se procesa para convertirlo a
  // escala de grises, esto facilitará e trabajo para aplicar el efecto de blur
  // a la imagen.
  while (!feof(image)) {
    b = fgetc(image);
    g = fgetc(image);
    r = fgetc(image);

    //unsigned char pixel = 0.21 * r + 0.72 * g + 0.07 * b;

    puntero[contador] = b;
    puntero[contador + 1] = g;
    puntero[contador + 2] = r;
    contador++;
  }

  int cont = 0;
  for (int i = 0; i < alto; i++) {
    for (int j = 0; j < ancho; j++) {
      imgDeEntrada[i][j] = puntero[cont];
      cont += 1;
    }
  }
  // Se realiza el efecto de blurring
  for (int i = mitad; i < alto - mitad; i++)
    for (int j = mitad; j < ancho - mitad; j++) {
      int suma = 0;
      for (int x = -mitad; x <= mitad; x++)
        for (int y = -mitad; y <= mitad; y++)
          suma += imgDeEntrada[i + x][j + y];
      imgDesenfocada[i][j] =
          (int)suma /
          (desenfoque * desenfoque); // Se promedia el valor del pixel
    }

  cont = 0;
  for (int i = 0; i < alto; i++) {
    for (int j = 0; j < ancho; j++) {
      puntero[cont] = imgDeEntrada[i][j]; //Se coloca el pixel B original
      puntero[cont + 1] = imgDeEntrada[i][j]; //Se coloca el pixel G original
      puntero[cont + 2] = imgDesenfocada[i][j];
      cont += 1;
    }
  }

// Se paraleliza la tarea para dar una mayor rapidez
#pragma omp parallel
  {
#pragma omp for schedule(dynamic)
    // Se genera la imagen de salida
    for (int i = 0; i < alto * ancho * 3; ++i) {
      fputc(puntero[i], outputImage);
      fputc(puntero[i + 1], outputImage);
      fputc(puntero[i + 2], outputImage);
    }
  }

  free(puntero);
  fclose(image);
  fclose(outputImage);
  const double tiempoFinal = omp_get_wtime(); // tiempo total de la ejecucion
  printf("tomo (%lf) segundos\n", (tiempoFinal - TiempoInicio));
  return 0;
}
