#include<sys/time.h>
#include<time.h>
#include<stdlib.h>
#include<stdio.h>
#include<omp.h>

int N_THREADS;

int nLinhas, nColunas;

void mostraMatriz(double **matriz);
int getPivo(double **simplex, int *linha_alvo, int *coluna_alvo);
int zeraColuna(double **simplex, int linha_alvo, int coluna_alvo);
int verificaNegativos(double **simplex);

int main(int argc, char* argv[]) 
{
	struct timeval timevalA;
	struct timeval timevalB;
	
	N_THREADS = atoi(argv[1]);
	
	scanf("%d %d\n", &nLinhas, &nColunas);
	
	int i,j, flag, linha_alvo, coluna_alvo;
	
	/** aloca matriz **/
	double **cars = (double**) malloc (sizeof(double*) * nLinhas);
	
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
	
	//mostraMatriz(cars);
	gettimeofday(&timevalA,NULL);
	
	flag = verificaNegativos(cars); 
	for(;flag!=0;)
	{
		getPivo(cars, &linha_alvo, &coluna_alvo);
		zeraColuna(cars, linha_alvo, coluna_alvo);	
		flag =verificaNegativos(cars);
	}

	gettimeofday(&timevalB,NULL);

	//mostraMatriz(cars);
	printf("\nresultado: %lf\ntempo de execucao: %lf\n", cars[nLinhas-1][nColunas-1], timevalB.tv_sec-timevalA.tv_sec+(timevalB.tv_usec-timevalA.tv_usec)/(double)1000000);
	
	
	return 0;
}

int getPivo(double **simplex, int *linha_alvo, int *coluna_alvo) {

	int linhaSelecionada,colunaSelecionada, i,j;
	double pivo,menor=5000;
	
	#pragma omp parallel for shared(menor, simplex, colunaSelecionada) private(i) num_threads(N_THREADS)
	for (i=0; i < nColunas; i++) 
	{ 
		
			#pragma omp critical
			if(simplex[nLinhas-1][i] < menor)
			{
				menor = simplex[nLinhas-1][i];
				colunaSelecionada = i;
			}
		
	}
	
	
	if(menor >= 0)
	{
		return 0;
	}
	
	menor = 5000;
	#pragma omp parallel for shared(menor, simplex, pivo, linhaSelecionada) private(i) num_threads(N_THREADS)
	for (i=0; i < nLinhas; i++) 
	{ 
		if(simplex[i][nColunas-1] != 0 && simplex[i][colunaSelecionada] != 0)
		{	
				
				#pragma omp critical
				if(simplex[i][nColunas-1]/simplex[i][colunaSelecionada] < menor && simplex[i][colunaSelecionada] > 0)
				{
					menor = (simplex[i][nColunas-1]/simplex[i][colunaSelecionada]);
					pivo = simplex[i][colunaSelecionada];
					linhaSelecionada = i;
				}	
					
		}	
	}
	#pragma omp parallel for shared(simplex, pivo, linhaSelecionada) private(i) num_threads(N_THREADS)
	for (i=0; i < nColunas; i++) 
	{ 
		simplex[linhaSelecionada][i] = simplex[linhaSelecionada][i]/pivo;
	}

	*linha_alvo = linhaSelecionada;
	*coluna_alvo = colunaSelecionada;
	
	return 0;
 }

int zeraColuna(double **simplex, int linha_alvo, int coluna_alvo){

    double coefi;
    int i,j;
	#pragma omp parallel for shared (simplex, coluna_alvo, linha_alvo) private(i, coefi,j) num_threads(N_THREADS)
	for (i=0; i < nLinhas; i++) 
	{ 
		if(simplex[i][coluna_alvo] != 0 && i != linha_alvo)
		{
			coefi = -1 * simplex[i][coluna_alvo];  	
			for (j=0; j < nColunas; j++) 
			{
				simplex[i][j] = simplex[linha_alvo][j]* coefi + simplex[i][j];
			}	
		}	
	}
    
    return 0;
}

int verificaNegativos(double **simplex)
{
    int i, flag=0;
	#pragma omp parallel for shared(simplex) private(i) num_threads(N_THREADS)
    for (i=0; i < nColunas ; i++)
    { 
    	if(simplex[nLinhas-1][i]<0)
    	{
    		flag=1;
    	}
    }
    return flag;
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

