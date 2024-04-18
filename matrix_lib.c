// Ricardo Bastos Leta Vieira 2110526

#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <pthread.h>
#include "matrix_lib.h"

#define NUM_THREADS 4

typedef struct _scalar_thread_args
{
    int thread_id;
    float scalar_value;
    struct matrix *matrix;
} Scalar_thread_args;

typedef struct _matrix_matrix_thread_args
{
    int thread_id;
    struct matrix *a;
    struct matrix *b;
    struct matrix *c;
} Matrix_matrix_thread_args;

void *aux_matrix_matrix_mult(void *args_void)
{
  Matrix_matrix_thread_args *args = (Matrix_matrix_thread_args *)args_void;

  struct matrix *a = args->a;
  struct matrix *b = args->b;
  struct matrix *c = args->c;

  int n_linhas = args->thread_id * (a->height / NUM_THREADS);
  
  int iniA = n_linhas;
  int endA = iniA + (n_linhas);

  float *iniB = b->rows;
  float *iniC = c->rows;
  float *iterA = a->rows + iniA;
  float *iterB = b->rows;
  float *iterC = iniC;

  __m256 aux_a = _mm256_set1_ps(*iterA);
  __m256 aux_b;
  __m256 aux_c;

  // occurs number of times = number of rows of A
  for (int i = iniA; i < endA; i++)
  {
    // reset b
    iterB = iniB;

    // occurs number of times = number of rows of B
    for (int j = 0; j < b->height; j++)
    {
      // set C iterator to the beginning of the current row of C
      iterC = iniC + i * c->width;

      // iterate through the elements in the current row of C and D
      for (int k = 0; k < c->width; k += 8)
      {
        // load 8 elements from B and C
        aux_b = _mm256_loadu_ps(iterB);
        aux_c = _mm256_loadu_ps(iterC);

        // multiply and add current element of A by the elements of B
        aux_c = _mm256_fmadd_ps(aux_a, aux_b, aux_c);
        _mm256_storeu_ps(iterC, aux_c);

        iterB += 8;
        iterC += 8;
      }

      // every time the row of B changes, move to the next element of A
      iterA++;
      aux_a = _mm256_set1_ps(*iterA);
    }
  }

  pthread_exit(NULL);
}

int matrix_matrix_mult(struct matrix *a, struct matrix *b, struct matrix *c)
{
    if (a->width != b->height)
    {
        fprintf(stderr, "Impossivel multplicar matrizes de tamanhos imcompativeis\n");
        exit(2);
    }
  
    pthread_t threads[NUM_THREADS];

    Matrix_matrix_thread_args *args;

    if (!(args = (Matrix_matrix_thread_args *)malloc(NUM_THREADS * sizeof(Matrix_matrix_thread_args))))
    {
      fprintf(stderr, "Erro ao alocar memoria para thread args em matrix_matrix_mult\n");
      exit(4);
    }

    // Cria as threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        args[i].thread_id = i;
        args[i].a = a;
        args[i].b = b;
        args[i].c = c;

        pthread_create(&threads[i], NULL, aux_matrix_matrix_mult, &args[i]);
    }

    // Espera todas as threads terminarem
    for (int i = 0; i < NUM_THREADS; i++)
    {
      pthread_join(threads[i], NULL);
    }

    free(args);
    
    return 1;
}

// Funcao auxiliar para multiplicacao de matriz por escalar chamada pelas threads
void *aux_scalar_mult(void *args_void)
{
  Scalar_thread_args * args = (Scalar_thread_args *)args_void;

  int thread_id = args->thread_id;
  int matrix_size = args->matrix->height * args->matrix->width;
  int ini = thread_id * (matrix_size / NUM_THREADS);
  int end = (thread_id + 1) * (matrix_size / NUM_THREADS);
  
  // Iterator depends on the thread running, adding ini indicates the appropriate start for the thread
  float *p = args->matrix->rows + ini;

  __m256 scalar = _mm256_set1_ps(args->scalar_value);
  __m256 aux;
  
  // Perform scalar multiplication on the assigned portion of the matrix
  for (int i = ini; i < end ; i += 8)
  {
    aux = _mm256_loadu_ps(p);
    aux = _mm256_mul_ps(aux, scalar);
    _mm256_storeu_ps(p, aux);
    p += 8;
  }

  pthread_exit(NULL);
}

int scalar_matrix_mult(float scalar_value, struct matrix *matrix)
{
    pthread_t threads[NUM_THREADS];
    Scalar_thread_args *args;

    if (!(args = (Scalar_thread_args *)malloc(NUM_THREADS * sizeof(Scalar_thread_args))))
    {
      fprintf(stderr, "Erro ao alocar memoria para thread args em scalar_matrix_mult\n");
      exit(3);
    }

    // Cria as threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        args[i].thread_id = i;
        args[i].scalar_value = scalar_value;
        args[i].matrix = matrix;

        pthread_create(&threads[i], NULL, aux_scalar_mult, &args[i]);
    }

    // Espera todas as threads terminarem
    for (int i = 0; i < NUM_THREADS; i++)
    {
      pthread_join(threads[i], NULL);
    }

    free(args);

    return 1;
}