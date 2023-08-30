// Simple CUDA example by Ingemar Ragnemalm 2009. Simplest possible?
// Assigns every element in an array with its index.
// Update 2022: Changed to cudaDeviceSynchronize.

// nvcc simple.cu -L /usr/local/cuda/lib -lcudart -o simple

#include <stdio.h>
#include <math.h>
const int N = 10; 
const int blocksize = 10; 

__global__ 
void simple(float *c) 
{
	float x = c[threadIdx.x];
	float qrt = sqrt(x);
	c[threadIdx.x] = qrt;
}

int main()
{
	float *c = new float[N];	
	float *cd;
	const int size = N*sizeof(float);
	
	

	cudaMalloc( (void**)&cd, size );

	for(int i =1; i<N;i++)
	{
		c[i] = i;
		printf("-%f-",sqrt(c[i]));
	}
	printf("\n");
	

	dim3 dimBlock( blocksize, 1 );
	dim3 dimGrid( 1, 1 );
	cudaMemcpy( cd, c, size, cudaMemcpyHostToDevice ); 
	simple<<<dimGrid, dimBlock>>>(cd);
	cudaDeviceSynchronize();
	cudaMemcpy( c, cd, size, cudaMemcpyDeviceToHost ); 
	cudaFree( cd );
	
	for (int i = 0; i < N; i++)
		printf("%f ", c[i]);
	printf("\n");
	delete[] c;
	printf("done\n");
	return EXIT_SUCCESS;
}
