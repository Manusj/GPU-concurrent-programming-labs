/*
 * Placeholder OpenCL kernel
 */

__kernel void find_max(__global unsigned int *data, const unsigned int length)
{ 
  unsigned int pos = 0;
  unsigned int val;
  unsigned int __local buf[512];
  //Something should happen here
  int id = get_global_id(0);
  unsigned int i = 0;
  //if(id<512)
  {
    int start = (length/512) * id;
    int end = (length/512) * (id+1);
	  int m = data[start];
	  for(i=start;i<end;i++) // Loop over data
	  {
	  	if (data[i] > m)
	  		m = data[i];
	  }
    data[start]= m;
  }
  barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
  if(id==0)
  {
    int m = data[0];
    for(int i = 0; i< length; i+=(length/512))
    {  
      if (data[i] > m)
	  		m = data[i];
    }
    data[0]= m;
  }
}
