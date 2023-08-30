
class CLWrapperClass_median_precompiled_Overlap2DKernel_median_kernel
{
public:

	static cl_kernel kernels(size_t deviceID, cl_kernel *newkernel = nullptr)
	{
		static cl_kernel arr[8]; // Hard-coded maximum
		if (newkernel)
		{
			arr[deviceID] = *newkernel;
			return nullptr;
		}
		else return arr[deviceID];
	}

	static void initialize()
	{
		static bool initialized = false;
		if (initialized)
			return;

		std::string source = skepu::backend::cl_helpers::replaceSizeT(R"###(
typedef struct {
	__local unsigned char *data;
	int oi, oj;
	size_t stride;
} skepu_region2d_unsigned__space__char;

static unsigned char skepu_region_access_2d_unsigned__space__char(skepu_region2d_unsigned__space__char r, int i, int j)
{ return r.data[i * r.stride + j]; }

#define SKEPU_USING_BACKEND_CL 1

typedef struct{
	size_t i;
} index1_t;

typedef struct {
	size_t row;
	size_t col;
} index2_t;

typedef struct {
	size_t i;
	size_t j;
	size_t k;
} index3_t;

typedef struct {
	size_t i;
	size_t j;
	size_t k;
	size_t l;
} index4_t;

static size_t get_device_id()
{
	return SKEPU_INTERNAL_DEVICE_ID;
}

#define VARIANT_OPENCL(block) block
#define VARIANT_CPU(block)
#define VARIANT_OPENMP(block)
#define VARIANT_CUDA(block)

// Size of basic integer types defined in OpenCL standard.
// Emulate stdint.h based on this.
typedef uchar    uint8_t;
typedef ushort   uint16_t;
typedef uint     uint32_t;
typedef ulong    uint64_t;

typedef char     int8_t;
typedef short    int16_t;
typedef int      int32_t;
typedef long     int64_t;

enum {
	SKEPU_EDGE_NONE = 0,
	SKEPU_EDGE_CYCLIC = 1,
	SKEPU_EDGE_DUPLICATE = 2,
	SKEPU_EDGE_PAD = 3,
};

static unsigned char median_kernel(skepu_region2d_unsigned__space__char image, unsigned long elemPerPx, const int region)
{
	// your code here
	
	int size = (image.oi*2 + 1) * (image.oi*2 + 1);
	int temparray[101*101]; // array to sort
	int k = 0;

	for (int y = -image.oi; y <= image.oi; ++y)
		for (int x = -image.oj; x <= image.oj; x += elemPerPx)
			temparray[k++] = skepu_region_access_2d_unsigned__space__char(image,y, x);
		

	for (int i = 0; i < size - 1; i++) {			//can be ran half way
      int min = i;
      for (int j = i + 1; j < size; j++)
      	if (temparray[j] < temparray[min])
      		min = j;
      int temp = temparray[i];
      temparray[i] = temparray[min];
      temparray[min] = temp;
   }

   return temparray[size/2 + 1];

  /*
  	int size = (image.oi*2 + 1) * (image.oi*2 + 1);
	//skepu::Region2D<unsigned char> im2 = image;
	int a=0;
	char data[101][101];
	//for(int x = -image.oj; x <= image.oj; x += elemPerPx)
	for(int x = -image.oj; x <= image.oj; x += elemPerPx,a++)
	{
		//sort y based
		int b=0;
		for (int y = -image.oi; y <= image.oi; ++y,b++)		//can be ran half way
		{
			int best=-image.oi;
			for (int k = y; k <= image.oi; k++)
			{
			
				if(image(k,x)<image(best,x))
					best=k;
			}
			data[b][a]=image(best,x);
			
		}
	}
	//we have max len in a
	int b=0;
	for(b=0; b < a; b++)		//can be done half way
	{
		int best=0;
		for (int k = b; k <a; k++)
		{
		
			if(data[a/2][k]<data[a/2][best])
				best=k;
		}
		char temp = data[b][a/2+a%2];
		data[b][a/2+a%2]= data[best][a/2+a%2];
		data[best][a/2+a%2]= temp;
	}
	
	return data[a/2+a%2][a/2+a%2];
*/
}


__kernel void median_precompiled_Overlap2DKernel_median_kernel( __global unsigned char* skepu_output, __global unsigned char* image, unsigned long elemPerPx, const int region, 
	size_t out_rows, size_t out_cols, size_t skepu_overlap_y, size_t skepu_overlap_x,
	size_t in_pitch, size_t sharedRows, size_t sharedCols,
	__local unsigned char* sdata)
{
	size_t xx = ((size_t)(get_global_id(0) / get_local_size(0))) * get_local_size(0);
	size_t yy = ((size_t)(get_global_id(1) / get_local_size(1))) * get_local_size(1);
	size_t x = get_global_id(0);
	size_t y = get_global_id(1);
	skepu_region2d_unsigned__space__char skepu_region = { .data = &sdata[(get_local_id(1) + skepu_overlap_y) * sharedCols + (get_local_id(0) + skepu_overlap_x)], .oi = skepu_overlap_y, .oj = skepu_overlap_x, .stride = sharedCols };

	

	if (x < out_cols + skepu_overlap_x * 2 && y < out_rows + skepu_overlap_y * 2)
	{
		size_t sharedIdx = get_local_id(1) * sharedCols + get_local_id(0);
		sdata[sharedIdx]= image[y * in_pitch + x];

		size_t shared_x = get_local_id(0)+get_local_size(0);
		size_t shared_y = get_local_id(1);
		while (shared_y < sharedRows)
		{
			while (shared_x < sharedCols)
			{
				sharedIdx = shared_y * sharedCols + shared_x;
				sdata[sharedIdx] = image[(yy + shared_y) * in_pitch + xx + shared_x];
				shared_x = shared_x + get_local_size(0);
			}
			shared_x = get_local_id(0);
			shared_y = shared_y + get_local_size(1);
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);
	

	if (x < out_cols && y < out_rows)
	{
#if !0
		skepu_output[y * out_cols + x] = median_kernel(skepu_region, elemPerPx, region);
#else
		skepu_multiple skepu_out_temp = median_kernel(skepu_region, elemPerPx, region);
		
#endif
	}
}
)###");

		// Builds the code and creates kernel for all devices
		size_t counter = 0;
		for (skepu::backend::Device_CL *device : skepu::backend::Environment<int>::getInstance()->m_devices_CL)
		{
			cl_int err;
			cl_program program = skepu::backend::cl_helpers::buildProgram(device, source);
			cl_kernel kernel = clCreateKernel(program, "median_precompiled_Overlap2DKernel_median_kernel", &err);
			CL_CHECK_ERROR(err, "Error creating MapOverlap 2D kernel 'median_precompiled_Overlap2DKernel_median_kernel'");

			kernels(counter++, &kernel);
		}

		initialized = true;
	}

	static void mapOverlap2D
	(
		size_t deviceID, size_t localSize[2], size_t globalSize[2],
		 skepu::backend::DeviceMemPointer_CL<unsigned char> *skepu_output, skepu::backend::DeviceMemPointer_CL<unsigned char> *skepu_input, unsigned long elemPerPx, const int region, 
		size_t out_rows, size_t out_cols, size_t skepu_overlap_y, size_t skepu_overlap_x,
		size_t in_pitch, size_t sharedRows, size_t sharedCols,
		size_t sharedMemSize
	)
	{
		skepu::backend::cl_helpers::setKernelArgs(kernels(deviceID),  skepu_output->getDeviceDataPointer(), skepu_input->getDeviceDataPointer(), elemPerPx, region, 
			out_rows, out_cols, skepu_overlap_y, skepu_overlap_x, in_pitch, sharedRows, sharedCols);
		clSetKernelArg(kernels(deviceID), 4 + 7, sharedMemSize, NULL);
		cl_int err = clEnqueueNDRangeKernel(skepu::backend::Environment<int>::getInstance()->m_devices_CL.at(deviceID)->getQueue(),
			kernels(deviceID), 2, NULL, globalSize, localSize, 0, NULL, NULL);
		CL_CHECK_ERROR(err, "Error launching MapOverlap 2D kernel");
	}
};
