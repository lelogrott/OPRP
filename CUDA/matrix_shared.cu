#include <stdio.h>

#define N 32
#define THREADS_PER_BLOCK 16*16
#define TILE_DIM 16

__global__ void vector_add(int* A, int* B, int* C) {

    int valor_c = 0;

    int linha = blockIdx.y * TILE_DIM + threadIdx.y;
    int coluna = blockIdx.x * TILE_DIM + threadIdx.x;
	//printf(">> %d %d\n", linha, coluna);
    __shared__ int As[TILE_DIM][TILE_DIM];
    __shared__ int Bs[TILE_DIM][TILE_DIM];

    for (int k = 0; k < (N*N)/(TILE_DIM*TILE_DIM); k++)
    {
     
       	As[threadIdx.y][threadIdx.x] = A[linha*N + coluna];
       	Bs[threadIdx.y][threadIdx.x] = B[linha*N + coluna];

        __syncthreads();

        for (int i = 0; i < TILE_DIM; ++i) 
        	valor_c += As[i][threadIdx.x] * Bs[threadIdx.y][i];

        __syncthreads();
    }

    if (linha < N && coluna < N)
    { 
/*    	C[linha*N + coluna] = valor_c;
    	if(linha*N + coluna == 1023 || linha*N + coluna == 0)
    		printf("valor = %d\nposicao = %d\n", valor_c, linha*N + coluna);
*/
	   	C[1023] = 500;

    }

}

int main()
{
	int *a, *b, *c;
	int *d_a, *d_b, *d_c;
	int size = N * sizeof(int);

	cudaMalloc( (void **) &d_a, N * size );
	cudaMalloc( (void **) &d_b, N * size );
	cudaMalloc( (void **) &d_c, N * size );

	a = (int *)malloc(N * size);
	b = (int *)malloc(N * size);
	c = (int *)malloc(N * size);

	for( int i = 0; i < N * N; i++)
	{
		a[i] = b[i] = 1;
		c[i] = 0;
	}
	
	
	cudaMemcpy( d_a, a, N * size, cudaMemcpyHostToDevice );
	cudaMemcpy( d_b, b, N * size, cudaMemcpyHostToDevice );
	cudaMemcpy( d_c, c, N * size, cudaMemcpyHostToDevice );
	
	dim3 blocos = dim3((N*N)/THREADS_PER_BLOCK, (N*N)/THREADS_PER_BLOCK, 1);
	dim3 t = dim3(16,16);

	vector_add<<< blocos,t >>>(d_a, d_b, d_c);

	cudaMemcpy( c, d_c, N * size, cudaMemcpyDeviceToHost );

	printf("c[0] = %d\n",c[0]);
	printf("c[%d] = %d\n", N*N-1, c[N*N-1]);
	printf("size = %d\n",size);
	
	free(a);
	free(b);
	free(c);

	cudaFree( d_a );
	cudaFree( d_b );
	cudaFree( d_c );

	cudaDeviceReset();

	return 0;
}
