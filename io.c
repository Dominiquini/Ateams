/*
 * io.c
 *
 *  Created on: 01/03/2010
 *      Author: iha
 */

#include "io.h"
#include "memoria.h"
#include "defs.h"
#include "lista.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


#define FALSE 0
#define TRUE 1

#define ORLIB 0
#define TAILLARD 1


extern ParametrosATEAMS *pATEAMS;
extern ParametrosAG *pAG;
extern ParametrosBT *pBT;
extern Dados *dados;


void menuAteams()
{
  int ch = 7;

  do {

    printf ("\n\t[1] Executar o programa.\n");
    printf ("\t[2] Carregar arquivo de dados.\n");
    printf ("\t[3] Carregar arquivo de parametros.\n");
    printf ("\t[4] Configurar parametros.\n");
    printf ("\t[7] Sair.\n");
    printf ("\n\tEscolha a opcao desejada: ");
    if (!scanf("%d", &ch))
      exit(1);
    printf ("\n");
    switch (ch) {
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
      break;
    default:
      break;
    }
  } while (ch != 7);
}


void menuConfigurarParametros (int argv, char *argc[])
{
  int ch = 0;

  do {
    printf ("\t[1] Configurar os parametros do A-TEAMS.\n");
    printf ("\t[2] Configurar os parametros do Algoritmo Genetico.\n");
    printf ("\t[3] Configurar os parametros da Busca Tabu.\n");
    printf ("\t[4] Sair.\n");
    printf ("\n\tEscolha a opcao desejada: ");
    if (!scanf("%d", &ch))
      (exit(1));

    switch (ch) {
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    default:
      break;
    }
  } while (ch != 0);
}

/* Mostra os parametros atuais do A-TEAMS
 * Permite alterar os parametros
 * Saída: 1 se foram alterados os dados, 0 caso contrário */
int menuConfigurarATEAMS (ParametrosATEAMS *p)
{
  int ch = 0;
  int gravar = 0;
  int tmp;
  char agente[3][20] = {	"Aleatório",
                                "Algoritmo Genetico",
                                "Busca Tabu"
  };
  char pa[2][100] = {	"Aceita qualquer solucao",
                        "Aceita se for melhor que a pior solucao da memoria"
  };
  char pd[3][100] = { "Destroi a pior solucao",
                      "Destroi segundo uma probabilidade Uniforme a menos da melhor solucao",
                      "Destroi segundo uma probabilidade Linear a menos da melhor solucao"
  };

  do {
    printf ("\t\t\t\tConfiguracao A-TEAMS:\n\n");
    printf ("\t[1] Agente a ser executado: \"%s\"\n", agente[p->agenteUtilizado]);
    printf ("\t[2] Tamanho da populacao principal: %d\n", p->tamanhoPopulacao);
    printf ("\t[3] Numero de iteracoes: %d\n", p->iteracoesAteams);
    printf ("\t[4] Politica de Aceitacao: \"%s\"\n", pa[p->politicaAceitacao]);
    printf ("\t[5] Politica de Destruicao: \"%s\"\n", pd[p->politicaDestruicao]);
    printf ("\t[6] Sair.\n");
    printf ("\n\tEscolha a opcao desejada: ");
    scanf("%d", &ch);
    switch (ch) {
    case 1:
      printf ("\n\n\tSelecione um agente a ser utilizado pelo ATEAMS:\n");
      printf ("\t[1] %s\n", agente[0]);
      printf ("\t[2] %s\n", agente[1]);
      printf ("\t[3] %s\n", agente[2]);
      scanf("%d", &tmp);
      p->agenteUtilizado = tmp - 1;
      gravar = 1;
      break;
    case 2:
      printf ("Digite o tamanho da populacao: ");
      scanf("%d\n", &p->tamanhoPopulacao);
      gravar = 1;
      break;
    case 3:
      printf ("Digite o numero de iteracoes: ");
      scanf("%d\n", &p->iteracoesAteams);
      gravar = 1;
      break;
    case 4:
      printf ("\t Selecione a politica de aceitacao:\n");
      printf ("\t[1] %s\n", pa[1]);
      printf ("\t[2] %s\n", pa[2]);
      scanf("%d", &p->politicaAceitacao);
      gravar = 1;
      break;
    case 5:
      printf ("\t Selecione a politica de destruicao:\n");
      printf ("\t[1] %s\n", pd[1]);
      printf ("\t[2] %s\n", pd[2]);
      printf ("\t[3] %s\n", pd[3]);
      printf ("\t[4] %s\n", pd[4]);
      printf ("\t[5] %s\n", pd[5]);
      scanf("%d", &p->politicaDestruicao);
      gravar = 1;
      break;
    case 6:
      ch = 6;
      break;
    default:
      break;
    }
  } while (ch != 6);
  return gravar;
}


int menuConfigurarAG (ParametrosAG *p)
{
  int ch = 0;
  int tmp;
  int gravar = 0;
  char s[6][100] = {	"\"Crossover Uniforme - Seleção Biclassista (porcentagem  dos melhores e do piores indivíduos)\"",
                        "\"Crossover Uniforme - Selecao aleatoria\"",
                        "",
                        "",
                        ""
  };
  char pl[5][100] = {	"\"Le as np melhores\"",
                        "\"Le np sequencias randomicamente (Probabilidade Uniforme) da melhor para pior\"",
                        "\"Le np sequencias com Probabilidade Linear da melhor para pior\"",
                        "\"Le np sequencias com Probabilidade Linear da pior para melhor\"",
                        "\"Le np sequencias randomicamente (Probabilidade Uniforme) da pior para melhor\""
  };

  do {
    printf ("\t\t\t\tConfiguracao AG:\n\n");
    printf ("\t[1] Tipo de selecao: %s\n", s[p->selecao]);
    printf ("\t[2] Tamanho da populacao inicial: (>0) %d\n", p->tamanhoPopulacao);
    printf ("\t[3] Numero de iteracoes: (>0) %d\n", p->numeroIteracoes);
    printf ("\t[4] Politica de Leitura: %s\n", pl[p->politicaLeitura]);
    printf ("\t[5] Probabilidade de crossover: (0.001;100) %.3f\n", p->probabilidadeCrossover);
    printf ("\t[6] Probabilidade de mutacoes: (0.001;100) %.3f\n", p->probabilidadeMutacoes);
    printf ("\t[7] Sair.\n");
    printf ("\n\tEscolha a opcao desejada: ");
    scanf("%d", &ch);
    switch (ch) {
    case 1:
      printf ("\n\n\tSelecione o tipo de selecao utilizado pelo Algoritmo Genetico:\n");
      printf ("\t[1] %s\n", s[0]);
      printf ("\t[2] %s\n", s[1]);
      printf ("\t[3] %s\n", s[2]);
      printf ("\t[4] %s\n", s[3]);
      scanf("%d", &tmp);
      p->selecao = tmp - 1;
      gravar = 1;
      break;
    case 2:
      printf ("\tDigite o tamanho da populacao inicial: (>0): ");
      scanf("%d", &p->tamanhoPopulacao);
      gravar = 1;
      break;
    case 3:
      printf ("\tDigite o numero de iteracoes: (>0)");
      scanf("%d", &p->numeroIteracoes);
      gravar = 1;
      break;
    case 4:
      printf ("\tSelecione a politica de Leitura: \n");
      printf ("\t[1] %s\n", pl[0]);
      printf ("\t[2] %s\n", pl[1]);
      printf ("\t[3] %s\n", pl[2]);
      printf ("\t[4] %s\n", pl[3]);
      printf ("\t[5] %s\n", pl[4]);
      scanf("%d", &tmp);
      p->politicaLeitura = tmp - 1;
      gravar = 1;
      break;
    case 5:
      printf ("\tDigite a probabilidade de crossover: (0.001;100) %.3f\n", p->probabilidadeCrossover);
      scanf("%f", &p->probabilidadeCrossover);
      gravar = 1;
      break;
    case 6:
      printf ("\tDigite a probabilidade de mutacoes: (0.001;100) %.3f\n", p->probabilidadeMutacoes);
      scanf("%f", &p->probabilidadeMutacoes);
      ch = 6;
      break;
    default:
      break;
    }
  } while (ch != 7);
  return gravar;
}

int menuConfigurarBT (ParametrosBT *p)
{
  int ch = 0;
  int gravar = 0;
  char politicaLeitura[4][200] = {
    "Lê da melhor para a pior solucao com distribuicao linear de probabilidade nunca le uma sequencia ja lida",
    "Lê da pior para a melhor solucao com distribuicao linear de probabilidade nunca le uma sequencia ja lida",
    "Da melhor para a pior solucao escolhendo a melhor que ainda não leu",
    "Da pior para a melhor solucao escolhendo a pior que ainda não leu"
  };

  do {
    printf ("\n\n\t\t\t\tConfiguracao Busca Tabu:\n\n");
    printf ("\t[1] Politica de leitura: \"%s\"\n", politicaLeitura[p->politicaLeitura]);
    printf ("\t[2] Numero de iteracoes Tabu: %d\n", p->numeroIteracoes);
    printf ("\t[3] Tamanho da lista Tabu: %d\n", p->tamanhoListaTabu);
    printf ("\t[4] Sair.\n");
    printf ("\n\tEscolha a opcao desejada: ");
    scanf("%d", &ch);
    switch (ch) {
    case 1:
      gravar = 1;
      break;
    case 2:
      gravar = 1;
      break;
    case 3:
      gravar = 1;
      break;
    default:
      break;
    }
  } while (ch != 4);
  return gravar;
}

void leArgumentos(int argc, char *argv[], FILE **dados, FILE **parametros, FILE **resultados, int *tipoArquivoDados) {

  *tipoArquivoDados = atoi (argv[4]);


  /* Arquivo de Dados */
  if ((*dados = fopen(argv[1], "r")) == NULL) {
    printf("\n\nArquivo \"%s\" nao encontrado\n\n",argv[1]);
    exit(1);
  }
  /* Arquivo de Parametros do Ateams*/
  if ((*parametros = fopen(argv[2], "r")) == NULL) {
    printf("\n\nArquivo \"%s\" nao encontrado\n\n",argv[2]);
    exit(1);
  }
  /* Arquivo de Resultados */
  if ((*resultados = fopen(argv[3], "a")) == NULL) {
    printf("\n\nArquivo \"%s\" nao encontrado\n\n",argv[3]);
    exit(1);
  }

}



void lerArquivoDados (int tipoArquivo, FILE *f)
{
  int i, j;

  if (tipoArquivo == ORLIB) {
    if (!fgets (dados->nome, 100, f))
      exit(1);
    if (!fscanf (f, "%d %d", &dados->N, &dados->M))
      exit(1);
    dados->m_op = alocaMatriz (dados->N, dados->M);
    dados->t_op = alocaMatriz (dados->N, dados->M);
    for (i = 0; i < dados->N; i++) {
      for (j = 0; j < dados->M; j++) {
        if (!fscanf (f, "%d %d", &dados->m_op[i][j], &dados->t_op[i][j]))
          exit(1);
      }
    }
  }
  else if (tipoArquivo == TAILLARD) {
    /* TODO: fazer a leitura de dados taillard */
  }
  else {
    printf("Tipo de arquivo nao definido.\n");
    /* TODO: melhorar a mensagem.*/
    exit(1);
  }
}

void lerArquivoParametros (FILE *f)
{
  float porcentagemPop;
  float porcentagemLeituraATEAMS;

  fscanf (f, "%d\n", &pATEAMS->agenteUtilizado);
  fscanf (f, "%d\n", &pATEAMS->iteracoesAteams);
  fscanf (f, "%d\n", &pATEAMS->maxTempo);
  fscanf (f, "%d\n", &pATEAMS->politicaAceitacao);
  fscanf (f, "%d\n", &pATEAMS->politicaDestruicao);
  fscanf (f, "%d\n", &pATEAMS->tamanhoPopulacao);
  // fscanf (f, "%d\n", &pATEAMS->makespanBest);

  fscanf (f, "%d\n", &pAG->numeroIteracoes);
  fscanf (f, "%d\n", &pAG->politicaLeitura);
  fscanf (f, "%f\n", &porcentagemLeituraATEAMS);
  fscanf (f, "%f\n", &porcentagemPop);
  fscanf (f, "%f\n", &pAG->probabilidadeCrossover);
  fscanf (f, "%f\n", &pAG->probabilidadeMutacoes);
  fscanf (f, "%d\n", &pAG->selecao);

  fscanf (f, "%d\n", &pBT->numeroIteracoes);
  fscanf (f, "%d\n", &pBT->politicaLeitura);
  // fscanf (f, "%d\n", &pBT->k);

  /* tamanho da memoria do algoritmo genetico deve ser >= memoria ATEAMS*/
  pAG->tamanhoPopulacao = pATEAMS->tamanhoPopulacao * (1 + porcentagemPop);
  pAG->quantidadeLeituraMemoriaATEAMS = pATEAMS->tamanhoPopulacao * porcentagemLeituraATEAMS;
  pBT->k = pBT->numeroIteracoes/5;
}

int *lerArquivoBestMakespan (FILE *f, int tamanho)
{
  int *best;
  int i;

  best = (int*) malloc (tamanho * sizeof(int));

  for (i = 0; i < tamanho; i++)
    fscanf (f, "%d\n", &best[i]);

  return best;
}


void escreveArquivoParametros (char nomeArquivo[100], ParametrosATEAMS *pATEAMS, ParametrosAG *pAG, ParametrosBT *pBT)
{
  FILE *f;

  if ((f = fopen(nomeArquivo, "r+")) == NULL) {
    printf ("Nao foi possivel gravar o arquivo: %s\n", nomeArquivo);
    exit(1);
  }
  else {
    fprintf (f, "%d\n", pATEAMS->agenteUtilizado);
    fprintf (f, "%d\n", pATEAMS->iteracoesAteams);
    fprintf (f, "%d\n", pATEAMS->politicaAceitacao);
    fprintf (f, "%d\n", pATEAMS->politicaDestruicao);
    fprintf (f, "%d\n", pATEAMS->tamanhoPopulacao);
    fprintf (f, "%d\n", pAG->numeroIteracoes);
    fprintf (f, "%d\n", pAG->politicaLeitura);
    fprintf (f, "%f\n", pAG->probabilidadeCrossover);
    fprintf (f, "%f\n", pAG->probabilidadeMutacoes);
    fprintf (f, "%d\n", pAG->selecao);
    fprintf (f, "%d\n", pAG->tamanhoPopulacao);
    fprintf (f, "%d\n", pBT->numeroIteracoes);
    fprintf (f, "%d\n", pBT->politicaLeitura);
    printf ("Arquivo %s gravado.\n", nomeArquivo);
  }
  fclose (f);
}

void imprimeResultado (struct timeval tv1, struct timeval tv2, int s, int msize, no *lista, FILE *resultados, int makespanInicial)
{
  no *aux;

  aux = retornaElemento(lista, 1, msize);
  fprintf(resultados,"%8d %8d %8d\n",makespanInicial, aux->makespan, s);
  /*fprintf (indiv,"%7d m %2d s %3d ms\n",min,s,ms); *//* tempo processamento */
}


/*
 *  Funcoes que inicializam os parametros com valores padrao
 *  FIXME: obs.: note que por enquanto sao valores aleatorios
 */
void setParametrosATEAMS (ParametrosATEAMS *p)
{
  p->agenteUtilizado = 0;
  p->iteracoesAteams = 10;
  p->politicaAceitacao = 1;
  p->politicaDestruicao = 1;
  p->tamanhoPopulacao = 100;
}

void setParametrosAG (ParametrosAG *p)
{
  p->numeroIteracoes = 10;
  p->politicaLeitura = 1;
  p->probabilidadeCrossover = .80;
  p->probabilidadeMutacoes = .01;
  p->selecao = 1;
  p->tamanhoPopulacao = 100;
}

void setParametrosBT (ParametrosBT *p)
{
  p->numeroIteracoes = 10;
  p->politicaLeitura = 1;
  p->tamanhoListaTabu = 10;
}


void mostraMatriz (int linhas, int colunas, int **M)
{
  int i, j;
  for (i = 0; i < linhas; i++) {
    for (j = 0; j < colunas; j++)
      printf("%5d", M[i][j]);
    printf("\n");
  }
  printf("\n");
}
