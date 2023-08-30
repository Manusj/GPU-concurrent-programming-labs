// Lab 5, image filters with CUDA.

// Compile with a command-line similar to Lab 4:
// nvcc filter.cu -c -arch=sm_30 -o filter.o
// g++ filter.o milli.c readppm.c -lGL -lm -lcuda -lcudart -L/usr/local/cuda/lib -lglut -o filter
// or (multicore lab)
// nvcc filter.cu -c -arch=sm_20 -o filter.o
// g++ filter.o milli.c readppm.c -lGL -lm -lcuda -L/usr/local/cuda/lib64 -lcudart -lglut -o filter

// 2017-11-27: Early pre-release, dubbed "beta".
// 2017-12-03: First official version! Brand new lab 5 based on the old lab 6.
// Better variable names, better prepared for some lab tasks. More changes may come
// but I call this version 1.0b2.
// 2017-12-04: Two fixes: Added command-lines (above), fixed a bug in computeImages
// that allocated too much memory. b3
// 2017-12-04: More fixes: Tightened up the kernel with edge clamping.
// Less code, nicer result (no borders). Cleaned up some messed up X and Y. b4
// 2022-12-07: A correction for a deprecated function.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#ifdef __APPLE__
  #include <GLUT/glut.h>
  #include <OpenGL/gl.h>
#else
  #include <GL/glut.h>
#endif
#include "readppm.h"
#include "milli.h"

// Use these for setting shared memory size.
#define maxKernelSizeX 10
#define maxKernelSizeY 10
#define blocksize 16
#define image_size 512

cudaEvent_t startEvent;
cudaEvent_t endEvent;

__global__ void filter(unsigned char *image, unsigned char *out, const unsigned int imagesizex, const unsigned int imagesizey, const int kernelsizex, const int kernelsizey)
{ 
  // map from blockIdx to pixel position
	int x = blockIdx.x * blockDim.x + threadIdx.x;
	int y = blockIdx.y * blockDim.y + threadIdx.y;

  int dy, dx;
  unsigned int sumx, sumy, sumz;

//initilize shared memory	TODO
	__shared__ unsigned char shared_mem[(blocksize+2*maxKernelSizeX+1)*(blocksize+2*maxKernelSizeY+1)*3];//but dimension is wrong maybe go for imagex/gridx * imagey/gridy *3 but we need those from input
	//for now we keep it as max size anyway
	//shared mem is same as block + kernel
	//start of our filling set blockIdx.x * blockDim.x  this is output 

	//start of our data read set would be blockIdx.x * blockDim.x  - kernelx this is input
	//write same for y		watch out for x3, for now we say it doesnt matter
	unsigned int my_shared_mem_index_x = threadIdx.x;
	unsigned int my_shared_mem_index_y = threadIdx.y;
	//for(dy=-kernelsizey;dy<=kernelsizey;dy++)
		//for(dx=-kernelsizex;dx<=kernelsizex;dx++)
	if((my_shared_mem_index_x) % ((2*kernelsizex)+1)==0)
		if((my_shared_mem_index_y) % ((2*kernelsizey)+1)==0)
	for(dy=-kernelsizey;dy<=kernelsizey;dy++)
		for(dx=-kernelsizex;dx<=kernelsizex;dx++)
		{
			int yy = min(max(y+dy, 0), imagesizey-1);//this is border
			int xx = min(max(x+dx, 0), imagesizex-1);//this is border

			int shared_index_x= min(max(my_shared_mem_index_x+dx, 0), blocksize+2*kernelsizex);
			int shared_index_y= min(max(my_shared_mem_index_y+dy, 0), blocksize+2*kernelsizey);
			shared_mem[(shared_index_x+((shared_index_y)*(blocksize+2*kernelsizex)))*3+0] = image[((yy)*imagesizex+(xx))*3+0];		//now these change
			shared_mem[(shared_index_x+((shared_index_y)*(blocksize+2*kernelsizex)))*3+1] = image[((yy)*imagesizex+(xx))*3+1];		//TODO
			shared_mem[(shared_index_x+((shared_index_y)*(blocksize+2*kernelsizex)))*3+2] = image[((yy)*imagesizex+(xx))*3+2];
		}

	//synchronize
	__syncthreads();
	//done
	//if(threadIdx.x==0&&blockIdx.x==0&&threadIdx.y==0&&blockIdx.y==0)
	//	printf("boop kernelsizex = %d \n",kernelsizex);

  int divby = (2*kernelsizex+1)*(2*kernelsizey+1); // Works for box filters only!
	
	if (x < imagesizex && y < imagesizey) // If inside image
	{
// Filter kernel (simple box filter)
	sumx=0;sumy=0;sumz=0;
	for(dy=-kernelsizey;dy<=kernelsizey;dy++)
	{
		for(dx=-kernelsizex;dx<=kernelsizex;dx++)	
		{
			// Use max and min to avoid branching!
			int yy = min(max(y+dy, 0), imagesizey-1);//this is border
			int xx = min(max(x+dx, 0), imagesizex-1);//this is border
			
			//int shared_index_x= min(max(x+dx, 0), blocksize+2*kernelsizex-1);
			//int shared_index_y= min(max(y+dy, 0), blocksize+2*kernelsizey-1);
			int shared_index_x= min(max(my_shared_mem_index_x+dx, 0), blocksize+2*kernelsizex);
			int shared_index_y= min(max(my_shared_mem_index_y+dy, 0), blocksize+2*kernelsizey);
			sumx += shared_mem[(shared_index_x+((shared_index_y)*(blocksize+2*kernelsizex)))*3+0];		//now these change
			sumy += shared_mem[(shared_index_x+((shared_index_y)*(blocksize+2*kernelsizex)))*3+1];		//TODO
			sumz += shared_mem[(shared_index_x+((shared_index_y)*(blocksize+2*kernelsizex)))*3+2];
			
		
			//sumx += image[((yy)*imagesizex+(xx))*3+0];
			//sumy += image[((yy)*imagesizex+(xx))*3+1];
			//sumz += image[((yy)*imagesizex+(xx))*3+2];
		
		/*
			//if(my_shared_mem_index_x>20&&my_shared_mem_index_y>20)
			{
			out[(y*imagesizex+x)*3+0] = shared_mem[(shared_index_x+((shared_index_y)*(blocksize+2*kernelsizex)))*3+0];
			out[(y*imagesizex+x)*3+1] = shared_mem[(shared_index_x+((shared_index_y)*(blocksize+2*kernelsizex)))*3+1];
			out[(y*imagesizex+x)*3+2] = shared_mem[(shared_index_x+((shared_index_y)*(blocksize+2*kernelsizex)))*3+2];
			}
		*/
		}
	}
	//if(threadIdx.x==0&&blockIdx.x==0&&threadIdx.y==0&&blockIdx.y==0)
	//	printf("sumy= %u \n",sumy);
	//if(threadIdx.x==0&&blockIdx.x==0&&threadIdx.y==0&&blockIdx.y==0)
	//	printf("divby= %d \n",divby);
	out[(y*imagesizex+x)*3+0] = sumx/divby;
	out[(y*imagesizex+x)*3+1] = sumy/divby;
	out[(y*imagesizex+x)*3+2] = sumz/divby;
	//if(threadIdx.x==0&&blockIdx.x==0&&threadIdx.y==0&&blockIdx.y==0)
	//	printf("out= %uc \n",out[(y*imagesizex+x)*3+1]);

	}
}

// Global variables for image data

unsigned char *image, *pixels, *dev_bitmap, *dev_input;
unsigned int imagesizey, imagesizex; // Image size

////////////////////////////////////////////////////////////////////////////////
// main computation function
////////////////////////////////////////////////////////////////////////////////
void computeImages(int kernelsizex, int kernelsizey)
{
	if (kernelsizex > maxKernelSizeX || kernelsizey > maxKernelSizeY)
	{
		printf("Kernel size out of bounds!\n");
		return;
	}
	float timediff;
	cudaEventCreate(&startEvent);
	cudaEventCreate(&endEvent);

	pixels = (unsigned char *) malloc(imagesizex*imagesizey*3);
	cudaMalloc( (void**)&dev_input, imagesizex*imagesizey*3);
	cudaMemcpy( dev_input, image, imagesizey*imagesizex*3, cudaMemcpyHostToDevice );
	cudaMalloc( (void**)&dev_bitmap, imagesizex*imagesizey*3);
	dim3 block(blocksize,blocksize);		//preferably 32
	dim3 grid(imagesizex/blocksize,imagesizey/blocksize);		//should change to imagesize/blocksize
	cudaEventSynchronize(startEvent);
	cudaEventRecord(startEvent, 0);
	filter<<<grid,block>>>(dev_input, dev_bitmap, imagesizex, imagesizey, kernelsizex, kernelsizey); // Awful load balance
	cudaDeviceSynchronize();
	cudaEventRecord(endEvent, 0);
	cudaEventSynchronize(endEvent);
	cudaEventElapsedTime(&timediff, startEvent, endEvent);
	printf("timediff - %lf\n",timediff);
//	Check for errors!
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess)
        printf("Error: %s\n", cudaGetErrorString(err));
	cudaMemcpy( pixels, dev_bitmap, imagesizey*imagesizex*3, cudaMemcpyDeviceToHost );
	cudaFree( dev_bitmap );
	cudaFree( dev_input );
}

// Display images
void Draw()
{
// Dump the whole picture onto the screen.	
	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	glClear( GL_COLOR_BUFFER_BIT );

	if (imagesizey >= imagesizex)
	{ // Not wide - probably square. Original left, result right.
		glRasterPos2f(-1, -1);
		glDrawPixels( imagesizex, imagesizey, GL_RGB, GL_UNSIGNED_BYTE, image );
		glRasterPos2i(0, -1);
		glDrawPixels( imagesizex, imagesizey, GL_RGB, GL_UNSIGNED_BYTE,  pixels);
	}
	else
	{ // Wide image! Original on top, result below.
		glRasterPos2f(-1, -1);
		glDrawPixels( imagesizex, imagesizey, GL_RGB, GL_UNSIGNED_BYTE, pixels );
		glRasterPos2i(-1, 0);
		glDrawPixels( imagesizex, imagesizey, GL_RGB, GL_UNSIGNED_BYTE, image );
	}
	glFlush();
}

// Main program, inits
int main( int argc, char** argv) 
{
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGBA );

	if (argc > 1)
		image = readppm(argv[1], (int *)&imagesizex, (int *)&imagesizey);
	else
		image = readppm((char *)"maskros512.ppm", (int *)&imagesizex, (int *)&imagesizey);

	if (imagesizey >= imagesizex)
		glutInitWindowSize( imagesizex*2, imagesizey );
	else
		glutInitWindowSize( imagesizex, imagesizey*2 );
	glutCreateWindow("Lab 5");
	glutDisplayFunc(Draw);

	ResetMilli();

	computeImages(2, 2);

// You can save the result to a file like this:
//	writeppm("out.ppm", imagesizey, imagesizex, pixels);

	glutMainLoop();
	return 0;
}
