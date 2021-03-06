#include<sys/time.h>
#include<time.h>
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include <unistd.h>
#include <mpi.h>

#define MASTER 0
#define MSG_TAG 0

int main(int argc, char **argv)
{

	int rank,size;

	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;

	int nLinhas, nColunas, i, j;
	
	scanf("%d %d", &nLinhas, &nColunas);
	
	double **cars = NULL;
	double *data = NULL;
	if(rank == 0)
	{
    	cars = (double**) malloc (sizeof(double*) * nLinhas);
		data = (double*) malloc (sizeof(double) * nLinhas * nColunas);

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
		printf("processo %d recebeu nlinhas = %d e ncolunas = %d\n\n", rank, nLinhas, nColunas);
		
		data = (double*) malloc (sizeof(double) * (nLinhas/(size))*nColunas);
		cars = (double**) malloc (sizeof(double*) * (nLinhas/(size)));
		
		for (i = 0; i < (nLinhas/(size)); i++) 
		{
			cars[i] = &data[nColunas * i];
		}

	}
	
		
	printf("Rank: %d Size: %d\n", rank, size);	
	/** inicializa matriz **/
	if(rank == 0)
		for(i=0;i<nLinhas;i++)
			for(j=0;j<nColunas;j++)
				scanf("%lf", &cars[i][j]);
	
	
	MPI_Scatter(&cars[0][0], (nLinhas/(size))*nColunas, MPI_DOUBLE, &cars[0][0], (nLinhas/(size))*nColunas, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
		
	if(rank ==0)
		cars[5][5] = 666;
	else
		printf("%d %.2lf\n\n", rank, cars[0][0]);
		
	if(rank == 0)
	{
		for(i=0;i < nLinhas;i++)
		{
			for(j=0;j<nColunas;j++)
			{
				printf("%.1lf\t", cars[i][j]);
			}
			printf("\n");
		}
	}

	MPI_Gather(&cars[0][0], (nLinhas/(size))*nColunas, MPI_DOUBLE, &cars[0][0], (nLinhas/(size))*nColunas, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	if(rank == 0)
	{
		for(i=0;i < nLinhas;i++)
		{
			for(j=0;j<nColunas;j++)
			{
				printf("%.1lf\t", cars[i][j]);
			}
			printf("\n");
		}
	}


	MPI_Finalize();
	return 0;
}




