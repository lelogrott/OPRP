#include <stdio.h>

__global__ void vector_add(int *a, int *b, int *c)
{
	int indexX = blockIdx.x * blockDim.x + threadIdx.x;
	int indexY = blockIdx.y * blockDim.y + threadIdx.y;
	c[indexX + indexY] = a[indexX + indexY] * b[indexX + indexY];
}



#define N 20
#define THREADS_PER_BLOCK 512

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

	return 0;
}
