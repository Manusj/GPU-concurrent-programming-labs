// Simple CUDA example by Ingemar Ragnemalm 2009. Simplest possible?
// Assigns every element in an array with its index.
// Update 2022: Changed to cudaDeviceSynchronize.

// nvcc simple.cu -L /usr/local/cuda/lib -lcudart -o simple

#include <stdio.h>
#include <math.h>
const int N = 64; 
const int blocksize = 32; 
const int gridsize = 2;
cudaEvent_t startEvent;
cudaEvent_t endEvent;

__global__ 
void simple(float *c) 
{
	float x = c[threadIdx.x];
	float qrt = sqrt(x);
	c[threadIdx.x] = qrt;
}

__global__ 
void add_matrix(float *a, float *b, float *c, int N)
{
	int index;
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;

	index = i + j*N;
	c[index] = a[index] + b[index];
}

int main()
{
	float *a = new float[N*N];	
    float *b = new float[N*N];
    float *c = new float[N*N];		
	float *dest;
    float *a_gpu;
    float *b_gpu;
	float timediff;
	const int size = N*N*sizeof(float);
	cudaEventCreate(&startEvent);
	cudaEventCreate(&endEvent);
	
	

	cudaMalloc( (void**)&dest, size);
    cudaMalloc( (void**)&a_gpu, size);
    cudaMalloc( (void**)&b_gpu, size);

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			a[i+j*N] = 10 + i;
			b[i+j*N] = (float)j / N;
		}
	

	dim3 dimBlock( blocksize, blocksize );
	dim3 dimGrid( gridsize, gridsize );
	cudaMemcpy( a_gpu, a, size, cudaMemcpyHostToDevice ); 
    cudaMemcpy( b_gpu, b, size, cudaMemcpyHostToDevice ); 
	cudaEventSynchronize(startEvent);
	cudaEventRecord(startEvent, 0);
	add_matrix<<<dimGrid, dimBlock>>>(a_gpu, b_gpu, dest, N);
	cudaDeviceSynchronize();
	cudaEventSynchronize(endEvent);
	cudaEventRecord(endEvent, 0);
	cudaEventElapsedTime(&timediff, startEvent, endEvent);
	
	cudaMemcpy( c, dest, size, cudaMemcpyDeviceToHost ); 
	cudaFree( dest );
    cudaFree( a_gpu );
    cudaFree( b_gpu );
	
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			printf("%0.2f ", c[i+j*N]);
		}
		printf("\n");
	}
	printf("\n");
	delete[] a;
    delete[] b;
    delete[] c;
	printf("done\n");
	printf("timediff - %lf\n");
	return EXIT_SUCCESS;
}

/* Best Gpu performance
const int N = 32; 
const int blocksize = 32; 
const int gridsize = 1;
*/
