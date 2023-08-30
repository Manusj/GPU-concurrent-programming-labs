/*
 * Placeholder OpenCL kernel
 */

__kernel void find_max(__global unsigned int *data, const unsigned int length)
{ 
  unsigned int pos = 0;
  unsigned int val;

  //Something should happen here
  int id = get_global_id(0);
  unsigned int i = 0;
  if(id==0)
  {
	  int m = data[0];
	  for (i=0;i<length/2;i++) // Loop over data
	  {
	  	if (data[i] > m)
	  		m = data[i];
	  }
    data[0]= m;
  }
  if(id==1)
  {
	  int m = data[length/2];
	  for (i=length/2;i<length;i++) // Loop over data
	  {
	  	if (data[i] > m)
	  		m = data[i];
	  }
    data[length/2]= m;
  }
  barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
  if(id==0)
  {
    if(data[0]<data[length/2])
      data[0]=data[length/2];
  }
}
