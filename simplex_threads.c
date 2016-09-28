#include<sys/time.h>
#include<time.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include <unistd.h>

typedef struct {
	int tid;
	int ntasks;
	double **simplex;
	int *linha_alvo;
	int *coluna_alvo;
	int coluna_selecionada;
	double *menor;
	double *pivo;
	int linha_selecionada;
	double pivo_selecionado;
	int *flag;
} param_t;


void mostraMatriz(double **matriz);
int getColuna(int tid, int ntasks, double **simplex, int *coluna_alvo, double *menor);
int getLinha(int tid, int ntasks, double **simplex, int *linha_alvo, int coluna_selecionada, double *menor, double *pivo);
int divide_linha(int tid, int ntasks, double **simplex, int linha_selecionada, double pivo_selecionado);
int zera_coluna(int tid, int ntasks, double **simplex, int linha_selecionada, int coluna_selecionada);
int verifica_negativos(int tid, int ntasks, double **simplex, int *flag);

void *simplex_getcoluna(void *arg)
{
   param_t *p = (param_t *) arg;
   getColuna(p->tid, p->ntasks, p->simplex, p->coluna_alvo, p->menor);
}

void *simplex_getlinha(void *arg)
{
   param_t *p = (param_t *) arg;
   getLinha(p->tid, p->ntasks, p->simplex, p->linha_alvo, p->coluna_selecionada, p->menor, p->pivo);
}

void *simplex_divide_linha(void *arg)
{
   param_t *p = (param_t *) arg;
   divide_linha(p->tid, p->ntasks, p->simplex, p->linha_selecionada, p->pivo_selecionado);
}

void *simplex_zera_coluna(void *arg)
{
   param_t *p = (param_t *) arg;
   zera_coluna(p->tid, p->ntasks, p->simplex, p->linha_selecionada, p->coluna_selecionada);
}

void *simplex_verifica_negativo(void *arg)
{
   param_t *p = (param_t *) arg;
   verifica_negativos(p->tid, p->ntasks, p->simplex, p->flag);
}

int N_THREADS;
int nLinhas, nColunas;


int main(int argc, char* argv[]) 
{
	struct timeval timevalA;
	struct timeval timevalB;
	
	pthread_t *threads;
	param_t *args;
	
	scanf("%d %d\n", &nLinhas, &nColunas);
	
	int i,j, flag;
	double pivo_selecionado,menor = 5000;
	int coluna_selecionada = 0;
	int linha_selecionada = 0;
	
	N_THREADS = atoi(argv[1]);
	/** aloca matriz **/
	double **cars = (double**) malloc (sizeof(double*) * nLinhas);
	int *linha_alvo = (int*) malloc (sizeof(int) * N_THREADS);
	int *coluna_alvo = (int*) malloc (sizeof(int) * N_THREADS);
	double *menores = (double*) malloc (sizeof(double) * N_THREADS);
	double *pivo = (double*) malloc (sizeof(double) * N_THREADS);
	
	
	for (i=0;i<nLinhas;i++)
	{
		cars[i] = (double*) malloc (sizeof(double) * nColunas);
	}
	
	
	
	/** inicializa matriz **/
	
	for(i=0;i<nLinhas;i++)
	{
		for(j=0;j<nColunas;j++)
		{
			scanf("%lf", &cars[i][j]);
		}
	}
	
	threads = (pthread_t *) malloc(N_THREADS * sizeof(pthread_t));
	args = (param_t *) malloc(N_THREADS * sizeof(param_t));

	gettimeofday(&timevalA,NULL);
	
	flag = 1;
	
	for(;flag!=0;)
	{
		/** INICIA PARTE 1 - GET COLUNA**/	

		for (i = 0; i < N_THREADS; i++)
		{
			args[i].tid = i;
			args[i].ntasks = N_THREADS;
			args[i].simplex = cars;
			args[i].coluna_alvo = coluna_alvo;
			args[i].menor = menores;
			pthread_create(&threads[i], NULL, simplex_getcoluna, (void *) (args+i));
		}
	
		for (i = 0; i < N_THREADS; i++)
		{
		  pthread_join(threads[i], NULL);
		}
	
		/** FINALIZA PARTE 1 - GET COLUNA**/
		

		/** COMPARA COLUNAS E SELECIONA A QUE POSSUI O MENOR ELEMENTO **/
		
		for(i=0; i< N_THREADS; i++)
		{
			
			if(menores[i] < menor)
			{
				menor = menores[i];		
				coluna_selecionada = coluna_alvo[i];
			}
		}
		//printf(">> menor: %lf\n\n>> coluna: %d\n\n", menor, coluna_selecionada);
		if(menor < 0)
		{
			/** INICIA PARTE 2 - GET LINHA E PIVO **/	
	
			for (i = 0; i < N_THREADS; i++)
			{
				args[i].tid = i;
				args[i].ntasks = N_THREADS;
				args[i].simplex = cars;
				args[i].linha_alvo = linha_alvo;
				args[i].coluna_selecionada = coluna_selecionada;
				args[i].menor = menores;
				args[i].pivo = pivo;
				pthread_create(&threads[i], NULL, simplex_getlinha, (void *) (args+i));
			}
	
			for (i = 0; i < N_THREADS; i++)
			{
			  pthread_join(threads[i], NULL);
			}
	
			/** FINALIZA PARTE 2 - GET LINHA E PIVO **/	
			menor = 5000;
			
			for(i=0; i< N_THREADS; i++)
			{
				if( menores[i] < menor)
				{
					menor = menores[i];		
					linha_selecionada = linha_alvo[i];
				}
			}
		
			pivo_selecionado  = cars[linha_selecionada][coluna_selecionada];
			
			//printf(">> menor: %lf\n\n>> linha: %d\n\n>> pivo: %lf\n\n", menor, linha_selecionada, pivo_selecionado);
			
			/** INICIA PARTE 3 - DIVIDE LINHA POR PIVO **/
		
			for (i = 0; i < N_THREADS; i++)
			{
				args[i].tid = i;
				args[i].ntasks = N_THREADS;
				args[i].simplex = cars;
				args[i].linha_selecionada = linha_selecionada;
				args[i].pivo_selecionado = pivo_selecionado;
				pthread_create(&threads[i], NULL, simplex_divide_linha, (void *) (args+i));
			}
	
			for (i = 0; i < N_THREADS; i++)
			{
			  pthread_join(threads[i], NULL);
			}
			/** FINALIZA PARTE 3 - DIVIDE LINHA POR PIVO **/
		}
		
		/** INICIA PARTE 4 - ZERA COLUNA **/
	
		for (i = 0; i < N_THREADS; i++)
		{
			args[i].tid = i;
			args[i].ntasks = N_THREADS;
			args[i].simplex = cars;
			args[i].linha_selecionada = linha_selecionada;
			args[i].coluna_selecionada = coluna_selecionada;
			pthread_create(&threads[i], NULL, simplex_zera_coluna, (void *) (args+i));
		}

		for (i = 0; i < N_THREADS; i++)
		{
		  pthread_join(threads[i], NULL);
		}
	
		/** FINALIZA PARTE 4 - ZERA COLUNA **/
	
		flag =0;
		/** INICIA PARTE 5 - VERIFICA NEGATIVOS **/
	
		for (i = 0; i < N_THREADS; i++)
		{
			args[i].tid = i;
			args[i].ntasks = N_THREADS;
			args[i].simplex = cars;
			args[i].flag = &flag;
			pthread_create(&threads[i], NULL, simplex_verifica_negativo, (void *) (args+i));
		}

		for (i = 0; i < N_THREADS; i++)
		{
		  pthread_join(threads[i], NULL);
		}
	
		/** FINALIZA PARTE 5 - VERIFICA NEGATIVOS **/
		//printf(">>flag: %d\n\n", flag); //sleep(2);
	}
	
	free(threads);
	free(args);
	
	gettimeofday(&timevalB,NULL);

	//mostraMatriz(cars);
	printf("\nresultado: %lf\ntempo de execucao: %lf\n", cars[nLinhas-1][nColunas-1], timevalB.tv_sec-timevalA.tv_sec+(timevalB.tv_usec-timevalA.tv_usec)/(double)1000000);

	
	
	return 0;
}

int getColuna(int tid, int ntasks, double **simplex, int *coluna_alvo, double *menor) {

	int i,j,limite;
	menor[tid] = 5000;
	
	if(tid == ntasks- 1)
    	limite = nColunas;
    else
    	limite = tid*(nColunas/ntasks) + nColunas/ntasks;
    	
	for(i = tid*(nColunas/ntasks) ;i < limite;++i) 
	{ 
		if(simplex[nLinhas-1][i] < menor[tid])
		{
			menor[tid] = simplex[nLinhas-1][i];
			coluna_alvo[tid] = i;
		}
	}
	return 0;
}

int getLinha(int tid, int ntasks, double **simplex, int *linha_alvo, int coluna_selecionada, double *menor, double *pivo)
{
	int i,limite;
	menor[tid] = 5000;
	//printf("@@ tid: %d\n", tid);
	
	if(tid == ntasks- 1)
    	limite = nLinhas;
    else
    	limite = tid*(nLinhas/ntasks) + nLinhas/ntasks;
    	
	for(i = tid*(nLinhas/ntasks) ;i < limite;++i) 
	{ 
		if(simplex[i][nColunas-1] != 0 && simplex[i][coluna_selecionada] != 0)
		{
			if(simplex[i][nColunas-1]/simplex[i][coluna_selecionada] < menor[tid] && simplex[i][coluna_selecionada] > 0)
			{
				menor[tid] = simplex[i][nColunas-1]/simplex[i][coluna_selecionada];
				pivo[tid] = simplex[i][coluna_selecionada];
				linha_alvo[tid] = i;
			}
		}	
	}
	return 0;
}

int divide_linha(int tid, int ntasks, double **simplex, int linha_selecionada, double pivo_selecionado)
{
	int i,limite;
	if(tid == ntasks- 1)
    	limite = nColunas;
    else
    	limite = tid*(nColunas/ntasks) + nColunas/ntasks;
	for(i = tid*(nColunas/ntasks) ;i < limite;++i)  
	{ 
		simplex[linha_selecionada][i] = simplex[linha_selecionada][i]/pivo_selecionado;
	}	
	return 0;
 }

int zera_coluna(int tid, int ntasks, double **simplex, int linha_selecionada, int coluna_selecionada)
{

    double coefi;
    int i,j, limite;
    if(tid == ntasks- 1)
    	limite = nLinhas;
    else
    	limite = tid*(nLinhas/ntasks) + nLinhas/ntasks;
    	
    for(i = tid*(nLinhas/ntasks) ;i < limite;++i)
    { 
    	if(simplex[i][coluna_selecionada] != 0 && i != linha_selecionada)
    	{
    		coefi = -1 * simplex[i][coluna_selecionada];  	
	    	
	    	for(j = 0 ;j < nColunas ;++j) 
	    	{
	    		simplex[i][j] = simplex[linha_selecionada][j]* coefi + simplex[i][j];
	    	}	
    	}	
    }
    return 0;
}

int verifica_negativos(int tid, int ntasks, double **simplex, int *flag)
{
    int i,limite;
    if(tid == ntasks- 1)
    	limite = nColunas;
    else
    	limite = tid*(nColunas/ntasks) + nColunas/ntasks;
    for(i = tid*(nColunas/ntasks) ;i < limite;++i) 
    { 
    	if(simplex[nLinhas-1][i]<0)
    	{
    		*flag=1;
    		return 0;
    	}
    }
    return 0;
}

void mostraMatriz(double **matriz)
{
	int i,j;
	printf("-------------MATRIZ-------------\n");
	for(i=0;i<nLinhas;i++)
	{
		for(j=0;j<nColunas;j++)
		{
			printf("%.3lf\t", matriz[i][j]);
		}
		printf("\n");
	}
	printf("--------------------------------\n");
}

