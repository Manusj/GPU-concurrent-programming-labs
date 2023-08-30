/*
 * Placeholder OpenCL kernel
 */

static void exchange(unsigned int *i, unsigned int *j)
{
	int k;
	k = *i;
	*i = *j;
	*j = k;
}


__kernel void bitonic(__global unsigned int *data, const unsigned int length)
{ 
  unsigned int pos = 0;
  unsigned int val;
  unsigned int i,j,k;
  //Something should happen here

  //data[get_global_id(0)]=get_global_id(0);
  unsigned int id = get_global_id(0);
  for (k=2;k<=length;k=2*k) // Outer loop, double size for each step
  {
    for (j=k>>1;j>0;j=j>>1) // Inner loop, half size for each step
    {
      if(id%k==0 && id!=length)
        for (i=id;i<(id+k);i++) // Loop over data
        {
          if(i>length)
          {
            //printf("out of bound\n");
          }
          int ixj=i^j; // Calculate indexing!
          if ((ixj)>i)
          {
            if ((i&k)==0 && data[i]>data[ixj]) exchange(&data[i],&data[ixj]);
            if ((i&k)!=0 && data[i]<data[ixj]) exchange(&data[i],&data[ixj]);
          }
          barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
        }
    }
  }
}
