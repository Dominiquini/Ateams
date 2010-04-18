/*
 * makespan.c
 *
 *  Created on: 07/03/2010
 *      Author: Rafael
 */

#include "defs.h"
#include "makespan.h"
#include <stdarg.h>
#include <stdlib.h>


void imprimir(int N, int M, int ***esc)
{
  printf("\n");

  for(int i = 0; i < M; i++)
    {
      printf("maq %d: ", i+1);
      for(int j = 0; j < N; j++)
        {
          int k = esc[i][j][2] - esc[i][j][1];
          int spc = j == 0 ? esc[i][j][1] : esc[i][j][1] - esc[i][j-1][2];
          while(spc--)
            printf(" ");

          while(k--)
            printf("%d", esc[i][j][0]+1);
        }
      printf("\n");
    }
  return;
}

/*
 * Devolvo o makespan quando a solução é factível e a o escalonamento, devolve
 * zero caso contrário. No caso de solução válida, deve liberar escal depois do
 * seu uso.
 */
int makespan(int** job, int** dur, int** seq, int N, int M, int**** escal)
{
  int findOrdem(int M, int maq, int* job)
  {
    int pos;

    for(int i = 0; i < M; i++)
      if(job[i] == maq)
        {
          pos = i;
          break;
        }

    return pos;
  }

  int ***esc, **tmp, *pos, i, j, k, max, cont;
  int ajob, apos, ainic, afim, time;

  pos = (int*) superMalloc(1, M);
  tmp = (int**) superMalloc(2, N, M+1);
  esc = (int***) superMalloc(3, M, N, 3);

  for (i = 0; i < N; i++)
    for(j = 0; j <= M; j++)
      tmp[i][j] = j == 0 ? 0 : -1;

  for (i = 0; i < M; i++)
    for(j = 0; j < N; j++)
      esc[i][j][0] = -1;

  for(i = 0; i < M; i++)
    pos[i] = 0;

  cont = i = j = k = 0;
  max = M * N;

  while((cont < max) && k <= N)
    {
      if(pos[i] != -1)
        {
          ajob = seq[i][pos[i]];
          apos = findOrdem(M, i, job[ajob]);
          ainic = tmp[ajob][apos];
        }
      else
        ainic = -1;

      if(ainic != -1)
        {
          cont++;
          k = 0;

          esc[i][pos[i]][0] = ajob;
          if(pos[i] == 0)
            {
              esc[i][pos[i]][1] = ainic;
            }
          else
            {
              afim = esc[i][pos[i] - 1][2];

              esc[i][pos[i]][1] = afim > ainic ? afim : ainic;
            }
          esc[i][pos[i]][2] = esc[i][pos[i]][1] + dur[ajob][apos];

          tmp[ajob][apos+1] = esc[i][pos[i]][2];

          pos[i] = pos[i] == N - 1 ? -1 : pos[i] + 1;
        }
      else
        {
          k++;
          i = (i + 1) % M;
        }
    }

  if(k != N + 1)
    {
      for(time = 0, i = 0; i < N; i++)
        {
          if(tmp[i][M] > time)
            time = tmp[i][M];
        }
      *escal = esc;

      superFree(2, tmp, N);
      superFree(1, pos);

      return time;
    }
  else
    {
      superFree(3, esc, M, N);
      superFree(2, tmp, N);
      superFree(1, pos);

      return 0;
    }
}


int find(int job, int* vet, int N)
{
  for(int i = 0; i < N; i++)
    if(vet[i] == job)
      return i;

  return -1;
}

int ordem(int* vet, int a, int b, int N)
{
  int posA = find(a, vet, N), posB = find(b, vet, N);

  if(posB < posA)
    {
      int tmp = vet[posB];
      vet[posB] = vet[posA];
      vet[posA] = tmp;

      return 1;
    }
  else
    {
      return -1;
    }
}

int** binTOsymb(int** job, char** bin, int N, int M)
{
  int **resp = NULL, *tmp = NULL, loop = 0, max = N*(N-1)/2, lin = 0;
  int pos = -1, A = -1, jobA = 0, jobB = 1;

  resp = (int**) malloc(M * sizeof(int*));

  for(int m = 0; m < M; m++)
    {
      tmp = (int*) superMalloc(1, M);
      for(int i = 0; i < M; i++)
        tmp[i] = i;

      loop = 0;
      while(loop <= max)
        {
          if(A != jobA)
            {
              pos = find(m, job[jobA], M);
              A = jobA;
            }

          if(bin[lin++][pos] == '1')
            if(ordem(tmp, jobA, jobB++, N) == -1)
              loop++;
            else
              loop = 0;
          else
            if(ordem(tmp, jobB++, jobA, N) == -1)
              loop++;
            else
              loop = 0;

          if(jobB == N)
            {
              jobA++;
              jobB = jobA + 1;
            }

          if(jobA == N - 1)
            {
              jobA = 0;
              jobB = jobA + 1;
              A = -1;
              lin = 0;
            }
        }
      resp[m] = tmp;
    }
  return resp;
}

char** symbTObin(int** job, int** sym, int N, int M)
{
  char** resp = NULL, max = N*(N-1)/2, maq = -1;
  int pos = -1, A = -1, jobA = 0, jobB = 1;

  resp = (char**) malloc(sizeof(char*) * max);
  for(int i = 0; i < max; i++)
    resp[i] = (char*) malloc(sizeof(char) * M);

  for(int i = 0; i < max; i++)
    {
      if(A != jobA)
        {
          pos = jobA;
          A = jobA;
        }

      for(int j = 0; j < M; j++)
        {
          maq = job[pos][j];

          if(find(jobA, sym[maq], N) < find(jobB, sym[maq], N))
            resp[i][j] = '1';
          else
            resp[i][j] = '0';
        }

      if(++jobB == N)
        {
          jobA++;
          jobB = jobA + 1;
        }
    }
  return resp;
}


int** leseq(int N, int M, FILE *seq)
{
  int **tarefa = NULL;
  int i, j;

  tarefa = (int **) malloc(N * sizeof(int *));
  for (i = 0; i < N; i++) {
    tarefa[i] = (int *) malloc(M * sizeof(int));
  }

  /* Leitura da matriz de tempos */
  for (i = 0; i < N; i++) {
    for (j = 0; j < M; j++) {
      fscanf(seq,"%d", &tarefa[i][j]);
      tarefa[i][j]--;
    }
  }

  return tarefa;
}

char** lebin(int N, int M, FILE *seq)
{
  char **tarefa = NULL;
  int i, j, lin = N*(N-1)/2;

  tarefa = (char **) malloc(lin * sizeof(char *));
  for (i = 0; i < lin; i++) {
    tarefa[i] = (char *) malloc(M * sizeof(char));
  }

  /* Leitura da matriz de tempos */
  for (i = 0; i < lin; i++)
    for (j = 0; j < M; j++)
      fscanf(seq,"%c ", &tarefa[i][j]);

  return tarefa;
}


void *superMalloc(int dim, ...)
{
  int *tmp1, **tmp2, ***tmp3;
  va_list args;
  int dims[3];

  va_start(args, dim);

  for(int i = 0; i < dim; i++) {
    dims[i] = va_arg(args, int);
  }

  va_end(args);

  switch(dim)
    {
    case 1:
      tmp1 = (int*) malloc(sizeof(int) * dims[0]);
      return (void*)tmp1;
      break;

    case 2:
      tmp2 = (int**) malloc(sizeof(int*) * dims[0]);
      for(int i = 0; i < dims[0]; i++)
        tmp2[i] = (int*) malloc(sizeof(int) * dims[1]);
      return (void*)tmp2;
      break;

    case 3:
      tmp3 = (int***) malloc(sizeof(int**) * dims[0]);
      for(int i = 0; i < dims[0]; i++) {
        tmp3[i] = (int**) malloc(sizeof(int*) * dims[1]);
        for(int j = 0; j < dims[1]; j++)
          tmp3[i][j] = (int*) malloc(sizeof(int) * dims[2]);
      }
      return (void*)tmp3;
      break;

    default:
      return NULL;
      break;
    }
}

void superFree(int dim, void* mem, ...)
{
  int *tmp1, **tmp2, ***tmp3;
  va_list args;
  int dims[2];

  va_start(args, mem);

  for(int i = 0; i < dim-1; i++)
    dims[i] = va_arg(args, int);

  va_end(args);

  switch(dim)
    {
    case 1:
      tmp1 = (int*) mem;
      free(tmp1);
      break;

    case 2:
      tmp2 = (int**) mem;
      for(int i = 0; i < dims[0]; i++)
        free(tmp2[i]);
      free(tmp2);
      break;

    case 3:
      tmp3 = (int***) mem;
      for(int i = 0; i < dims[0]; i++) {
        for(int j = 0; j < dims[1]; j++)
          free(tmp3[i][j]);
        free(tmp3[i]);
      }
      free(tmp3);
      break;

    default:
      break;
    }
}
