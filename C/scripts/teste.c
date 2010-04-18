/*
 * teste.c
 *
 *  Created on: 14/03/2010
 *      Author: iha
 */

#if 0

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define NTESTES		3 /* numero de vezes que o programa controle é chamado */

#define PARAM_AG_INICIO 1     //param%d ,   PARAM_INI <%d< PARAM_FIM
#define PARAM_AG_FIM 1


#define DADO_INI 01         //tar%d   ,   DADO_INI <%d< DADO_FIM
#define DADO_FIM 40

#define PATH_RESUL "resultados/la"    // Arquivo de resultados:
// #define PATH_PARAM_ATEAMS "parametros/ateams/paramATeams"
#define PATH_PARAM "parametros/default.param"

#define PROB "la"
#define PATH_DADOS "dados/la"
#define PATH_GRAFICOS "resultados/grafico"

int main(void)  {
	int t = 0, j = 0, p = 0;
	char comando[500], results[50], param[50], dados[50];
	FILE *indiv;


	for(p = PARAM_AG_INICIO; p <= PARAM_AG_FIM; p++) {
		for(t = DADO_INI; t <= DADO_FIM; t++) {
			sprintf(dados,"%s%02d.prb",PATH_DADOS, t);
			sprintf(results,"%s%02d.res",PATH_RESUL, t);
			sprintf(param,"%s",PATH_PARAM);

			for(j = 1; j <= NTESTES; j++) {
				sprintf(comando,"./ateams %s %s %s %s %d",dados, param, results, "0", t);
				printf("%s\n",comando);
				fflush(stdout);
				system(comando);

			}
		}
	}
}
#endif
