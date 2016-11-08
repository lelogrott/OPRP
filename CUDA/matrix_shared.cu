#include <stdio.h>

#define N 3
#define THREADS_PER_BLOCK 512


__global__ void vector_add(int *a, int *b, int *c)
{
	int indexX = blockIdx.x * blockDim.x + threadIdx.x;
	int indexY = blockIdx.y * blockDim.y + threadIdx.y;
	int comeco,j = (indexX + indexY) - ((indexX + indexY)/N) * N;
	comeco = (indexX + indexY) - (indexX + indexY)%N; 
	for(int i = 0; i < N; i++)
	{
		c[indexX + indexY] += a[comeco + i] * b[j];
		j+=N;
	}
	//printf("\n\n>> threadID = %d; X = %d; Y = %d\n\n", threadIdx.x, indexX, indexY);
	
}
__global__ void MatMul(float* A, float* B, float* C, int ARows, int ACols, int BRows, int BCols, int CRows, int CCols) {

    float CValue = 0;

    int Row = blockIdx.y*TILE_DIM + threadIdx.y;
    int Col = blockIdx.x*TILE_DIM + threadIdx.x;

    __shared__ float As[TILE_DIM][TILE_DIM];
    __shared__ float Bs[TILE_DIM][TILE_DIM];

    for (int k = 0; k < (TILE_DIM + ACols - 1)/TILE_DIM; k++) {

         if (k*TILE_DIM + threadIdx.x < ACols && Row < ARows)   As[threadIdx.y][threadIdx.x] = A[Row*ACols + k*TILE_DIM + threadIdx.x];
         else                                                   As[threadIdx.y][threadIdx.x] = 0.0;

         if (k*TILE_DIM + threadIdx.y < BRows && Col < BCols)   Bs[threadIdx.y][threadIdx.x] = B[(k*TILE_DIM + threadIdx.y)*BCols + Col];
         else                                                   Bs[threadIdx.y][threadIdx.x] = 0.0;

         __syncthreads();

         for (int n = 0; n < TILE_DIM; ++n) CValue += As[threadIdx.y][n] * Bs[n][threadIdx.x];

         __syncthreads();
    }

    if (Row < CRows && Col < CCols) C[((blockIdx.y * blockDim.y + threadIdx.y)*CCols)+(blockIdx.x*blockDim.x)+threadIdx.x]=CValue;
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
		a[i] = b[i] = i;
		c[i] = 0;
	}
	
	
	cudaMemcpy( d_a, a, N * size, cudaMemcpyHostToDevice );
	cudaMemcpy( d_b, b, N * size, cudaMemcpyHostToDevice );
	cudaMemcpy( d_c, c, N * size, cudaMemcpyHostToDevice );
	
	dim3 blocos = dim3((N + (THREADS_PER_BLOCK -1)) / THREADS_PER_BLOCK, (N + (THREADS_PER_BLOCK - 1)) / THREADS_PER_BLOCK, 1);
	dim3 t = dim3(THREADS_PER_BLOCK);

	vector_add<<< blocos,t >>>(d_a, d_b, d_c);

	cudaMemcpy( c, d_c, N * size, cudaMemcpyDeviceToHost );

	printf("c[0] = %d\n",c[0]);
	printf("c[%d] = %d\n", N*N-1, c[N*N-1]);

	free(a);
	free(b);
	free(c);

	cudaFree( d_a );
	cudaFree( d_b );
	cudaFree( d_c );

	cudaDeviceReset();

	return 0;
}
