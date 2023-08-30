/*
 * Placeholder OpenCL kernel
 */

__kernel void find_max(__global unsigned int *data, const unsigned int length, const int stride)
{ 
  int id = get_global_id(0);
  //if(id==0)
  //  printf("Stride = %d\n",stride);
  
  int offset = stride*2;
  if(id%offset==0)
  {
    if(data[id]<data[id+stride])
    {
      data[id]=data[id+stride];
    }
  }

  
}
