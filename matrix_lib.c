// Ricardo Bastos Leta Vieira 2110526

#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include "matrix_lib.h"

int matrix_matrix_mult(struct matrix *a, struct matrix *b, struct matrix *c)
{
  if (a->width != b->height)
  {
    fprintf(stderr, "Impossivel multplicar matrizes de tamanhos imcompativeis\n");
    return 0;
  }

  // float *iniB = b->rows;
  float *iniC = c->rows;
  float *iterA = a->rows;
  float *iterB = b->rows;
  float *iterC = c->rows;

  for (int i = 0; i < a->height; i++, iterA += a->width)
  {
    iterC = iniC;

    for (int j = 0; j < b->width; j++, iterC += b->width)
    {
      __m256 c = _mm256_setzero_ps();
      float *a_row = iterA;
      float *b_col = iterB + j;
      __m256 b_column = _mm256_setzero_ps();

      for (int k = 0; k < a->width; k += 8, a_row += 8, b_col += 8 * b->width)
      {
        __m256 a = _mm256_loadu_ps(a_row);

        for (int l = 0; l < 8; l++)
        {
          float b_val = iterB[l * b->width]; // get the l-th element in the column
          b_col[l] = b_val;                  // store the value in the vector
        }

        b_column = _mm256_loadu_ps(b_col);
        
        // printf("----------\n");
        // printf("i: %d, j: %d, k: %d\n", i, j, k);
        // printf("a_row: %f %f %f %f %f %f %f %f\n", a_row[0], a_row[1], a_row[2], a_row[3], a_row[4], a_row[5], a_row[6], a_row[7]);
        // printf("b_col: %f %f %f %f %f %f %f %f\n", b_col[0], b_col[1], b_col[2], b_col[3], b_col[4], b_col[5], b_col[6], b_col[7]);
        // printf("----------\n");

        c = _mm256_fmadd_ps(a, b_column, c);
      }
      _mm256_storeu_ps(iterC, c);

      // printf("##############\n");
      // printf("c: %f %f %f %f %f %f %f %f\n", iterC[0], iterC[1], iterC[2], iterC[3], iterC[4], iterC[5], iterC[6], iterC[7]);
      // printf("##############\n");
    }
  }

  return 1;
}

int scalar_matrix_mult(float scalar_value, struct matrix *matrix)
{
  float *p = matrix->rows;
  __m256 scalar = _mm256_set1_ps(scalar_value);

  for (long i = 0; i < matrix->height * matrix->width; i += 8)
  {
    __m256 row = _mm256_loadu_ps(p);
    row = _mm256_mul_ps(row, scalar);
    _mm256_storeu_ps(p, row);
    
    p += 8;
  }

  return 1;
}