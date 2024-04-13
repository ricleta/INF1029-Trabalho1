// Ricardo Bastos Leta Vieira 2110526

// A+0 | B+0 | C+0
// A+0 | B+1 | C+1
// A+1 | B+2 | C+0
// A+1 | B+3 | C+1
// A+2 | B+4 | C+0
// A+2 | B+5 | C+1
// A+3 | B+6 | C+0
// A+3 | B+7 | C+1
// A+4 | B+0 | C+2
// melhor metodo
// percorrer tudo por linha
// iterar pelos elementos de uma linha de a
    // multiplicar pelos elementos da msm linha de b
    // voltar ao comeco de c e multiplicar pela proxima linha de b

// manter ponteiro para o inicio da linha para resetar 
// NAO USAR INDEXACAO

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

int matrix_matrix_mult(struct matrix *a, struct matrix *b, struct matrix *c)
{
    if (a->width != b->height)
    {
        fprintf(stderr, "Impossivel multplicar matrizes de tamanhos imcompativeis\n");
        exit(2);
    }

    float *iniB = b->rows;
    float *iniC = c->rows;
    float *iterA = a->rows;
    float *iterB = b->rows;
    float *iterC = c->rows;
  
    __m256 aux_a = _mm256_set1_ps(*iterA);
    __m256 aux_b;
    __m256 aux_c;

    // ocorre numero de vezes = numero de linhas de A
    for (int i = 0; i < a->height; i++)
    {
      // reseta b
      iterB = iniB;

      // ocorre numero de vezes = numero de linhas de B
      for (int j = 0; j < b->height; j++)
      {
        // seta iterador de C para o comeco da linha atual de C
        iterC = iniC + i * c->width;

        // itera pelos elementos na linha atual de C e D
        for (int k = 0; k < c->width; k += 8)
        {
          // carrega 8 elementos de B e C
          aux_b = _mm256_loadu_ps(iterB);
          aux_c = _mm256_loadu_ps(iterC);

          // multiplica e soma elemento atual de A pelos elemtos de B 
          aux_c = _mm256_fmadd_ps(aux_a, aux_b, aux_c);
          _mm256_storeu_ps(iterC, aux_c);

          iterB += 8;
          iterC += 8;
        }

        // toda vez que a linha de B muda, passa para o proximo elemento de A
        iterA++; 
        aux_a = _mm256_set1_ps(*iterA);
      }
    }
  
    return 1;
}

// Funcao auxiliar para multiplicacao de matriz por escalar chamada pelas threads
void *aux_scalar_mult(Scalar_thread_args *args)
{
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
      fprintf(stderr, "Erro ao alocar memoria para thread args\n");
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