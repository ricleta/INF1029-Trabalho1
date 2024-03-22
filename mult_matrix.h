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
#include "matrix_lib.h"

int matrix_matrix_mult(struct matrix *matrixA, struct matrix * 
matrixB, struct matrix * matrixC)
{
    if (matrixA->width != matrixB->height)
    {
        perror("Impossivel multplicar matrizes de tamanhos imcompativeis");
        exit(-1);
    }

    float *iniB = matrixB->rows;
    float *iniC = matrixC->rows;
    float *iterA = matrixA->rows;
    float *iterB = matrixB->rows;
    float *iterC = matrixC->rows;

    // ocorre numero de vezes = numero de linhas de A
    for (int i = 0; i < matrixA->height; i++)
    {
      // reseta b
      iterB = iniB;
      // ocorre numero de vezes = numero de linhas de B
      for (int j = 0; j < matrixB->height; j++)
      {
        // seta iterador de C para o comeco da linha atual de C
        iterC = iniC + i * matrixC->width;

        iterA++; // toda vez que a linha de B muda, passa para o proximo elemento de A
        
        // itera pelos elementos na linha atual de C e D
        for (int k = 0; k < matrixC->width; i++)
        {
          *iterC++ += *iterB++ * *iterA;
        }
      }
    }

    return 0;
}