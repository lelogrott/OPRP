#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <mpi.h>
#include <string.h>

#define MASTER 0
#define MSG_TAG 0

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
void mostra_submatriz(double **matriz, int rank, int size);
int getColuna(int tid, int ntasks, double **simplex, int *coluna_alvo, double *menor);
int getLinha(int tid, int ntasks, double **simplex, int *linha_alvo, int coluna_selecionada, double *menor, double *pivo);
int divide_linha(int tid, int ntasks, double **simplex, int linha_selecionada, double pivo_selecionado);
int zera_coluna(double **simplex, int linha_selecionada, int coluna_selecionada, int size, int rank, double *vet_linha_selecionada);
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
	int flag_to_niggas= 1;
	N_THREADS = atoi(argv[1]);
		
	/** aloca matriz **/

	int *linha_alvo = (int*) malloc (sizeof(int) * N_THREADS);
	int *coluna_alvo = (int*) malloc (sizeof(int) * N_THREADS);
	double *menores = (double*) malloc (sizeof(double) * N_THREADS);
	double *pivo = (double*) malloc (sizeof(double) * N_THREADS);
	

	/** inicializa matriz **/
	

	
	threads = (pthread_t *) malloc(N_THREADS * sizeof(pthread_t));
	args = (param_t *) malloc(N_THREADS * sizeof(param_t));

	
	int rank,size;

	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;
	

	double **cars = NULL;
	double *data = NULL;
	double *vet_linha_selecionada = NULL;


	if(rank == 0)
	{
    	cars = (double**) malloc (sizeof(double*) * nLinhas);
		data = (double*) malloc (sizeof(double) * nLinhas * nColunas);
		vet_linha_selecionada = (double*) malloc (sizeof(double) * nColunas);
		
		for (i = 0; i < nLinhas; i++) {
	  		cars[i] = &data[nColunas * i];
		}
		for(i=1;i < size; i++)
		{
			MPI_Send(&nLinhas, 1, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
			MPI_Send(&nColunas, 1, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);			
		}
	}
	else
	{
		MPI_Recv(&nLinhas, 1, MPI_INT, 0, MSG_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv(&nColunas, 1, MPI_INT, 0, MSG_TAG, MPI_COMM_WORLD, &status);	
		
		data = (double*) malloc (sizeof(double) * (nLinhas/(size))*nColunas);
		cars = (double**) malloc (sizeof(double*) * (nLinhas/(size)));
		vet_linha_selecionada = (double*) malloc (sizeof(double) * nColunas);
			
		for (i = 0; i < (nLinhas/(size)); i++) 
		{
			cars[i] = &data[nColunas * i];
		}

	}

		for(i=0;i<nLinhas;i++)
		{
			for(j=0;j<nColunas;j++)
			{
				scanf("%lf", &cars[i][j]);
			}
		}


	flag = 1;
	
	gettimeofday(&timevalA,NULL);
	
	while( flag!=0 )
	{
		if(rank == MASTER)
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
		}
		/** INICIA PARTE 4 - ZERA COLUNA **/
		
		
		MPI_Scatter(&cars[0][0], (nLinhas/(size))*nColunas, MPI_DOUBLE, &cars[0][0], (nLinhas/(size))*nColunas, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
		
		
		if(rank == MASTER)
		{
			memcpy(vet_linha_selecionada, cars[linha_selecionada], sizeof(double)*nColunas);
			for(i=1; i < size; i++)
			{
				MPI_Send(&linha_selecionada, 1, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
				MPI_Send(&coluna_selecionada, 1, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
				MPI_Send(vet_linha_selecionada, nColunas, MPI_DOUBLE, i, MSG_TAG, MPI_COMM_WORLD);
			}
		}
		
		if(rank != MASTER)
		{
						
			MPI_Status status;
			
			MPI_Recv(&linha_selecionada, 1, MPI_INT, 0, MSG_TAG, MPI_COMM_WORLD, &status);
			MPI_Recv(&coluna_selecionada, 1, MPI_INT, 0, MSG_TAG, MPI_COMM_WORLD, &status);
			MPI_Recv(vet_linha_selecionada, nColunas, MPI_DOUBLE, 0, MSG_TAG, MPI_COMM_WORLD, &status);
		}
		
		zera_coluna(cars, linha_selecionada, coluna_selecionada, size, rank, vet_linha_selecionada);
		

		/** FINALIZA PARTE 4 - ZERA COLUNA **/
		
		MPI_Gather(&cars[0][0], (nLinhas/(size))*nColunas, MPI_DOUBLE, &cars[0][0], (nLinhas/(size))*nColunas, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		
		
		
		if(rank == MASTER)
		{
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
			for(i=1;i<size;i++)
			{
				MPI_Send(&flag, 1, MPI_INT, i, MSG_TAG, MPI_COMM_WORLD);
			}
		}
		else
		{
			MPI_Recv(&flag_to_niggas, 1, MPI_INT, 0, MSG_TAG, MPI_COMM_WORLD, &status);
			if(!flag_to_niggas)
				break;
		}
	}
	
		
	free(threads);
	free(args);
	
	gettimeofday(&timevalB,NULL);
	
	if(rank == MASTER)
	{
		printf("\nresultado: %lf\ntempo de execucao: %lf\n", cars[nLinhas-1][nColunas-1], timevalB.tv_sec-timevalA.tv_sec+(timevalB.tv_usec-timevalA.tv_usec)/(double)1000000);
		//mostraMatriz(cars);
	}

	MPI_Finalize();	
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

int zera_coluna(double **simplex, int linha_selecionada, int coluna_selecionada, 
				int size, int rank, double *vet_linha_selecionada)
{
    double coefi;
    int i,j,process=0;

    // for (i = 0; i < linha_selecionada; i+=nLinhas/size)
    // 	process++;
    process = (int) linha_selecionada/(nLinhas/size);
    //printf(">> rank %d linha %d  process %d \n", rank, linha_selecionada, process );

    // if(linha_selecionada < nLinhas/size)
    // 	process = 0;
    
    for(i = 0 ;i < (nLinhas/(size));++i)
    { 
    	if(simplex[i][coluna_selecionada] != 0)
    	{
    		if ((rank == process && ( i != linha_selecionada - rank*(nLinhas/size))) || rank != process)
    		{    			
				coefi = -1 * simplex[i][coluna_selecionada];  	
		    	
		    	for(j = 0 ;j < nColunas ;++j) 
		    	{
		    		simplex[i][j] = vet_linha_selecionada[j]* coefi + simplex[i][j];
		    	}
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

void mostra_submatriz(double **matriz, int rank, int size)
{
	int i,j;
	printf("-------------SUB MATRIZ - PROCESSO %d -------------\n", rank);
	for(i=0;i<(nLinhas/(size));i++)
	{
		for(j=0;j<nColunas;j++)
		{
			printf("%.3lf\t", matriz[i][j]);
		}
		printf("\n");
	}
	printf("----------------------------------------------------\n\n");

}

