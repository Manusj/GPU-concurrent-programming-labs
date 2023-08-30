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


__kernel void bitonic(__global unsigned int *data, const unsigned int length, const unsigned int k, const unsigned int j)
{ 
  unsigned int pos = 0;
  unsigned int val;
  unsigned int i;
  //printf("piticooooo\n");
  //Something should happen here

  //data[get_global_id(0)]=get_global_id(0);    //for deterministic, it was good
  // int max_offset=1;
  // int offset=0;
  // if(length>512)
  // {
  //   max_offset = length/512;
  // }
  unsigned int id = get_global_id(0);
  //for(offset=max_offset-1;offset>=0;offset--)//simulate multiple kernel runs
  //{
    // unsigned int *workata=data+(offset*512);
    if(id==10)
    {
      //printf("we hold values of: %d and %d \n",data[id],data[id+512]);
      // printf("biboo bibooo bibooo of: %d and %d \n",workata[id],workata[id+512]);
    }
      //for (k=2;k<=length;k=2*k) // Outer loop, double size for each step
      {
        if(id==10&&k==1024)
          {
            //printf("looks like k can be 1024\n");
          }
        //for (j=k>>1;j>0;j=j>>1) // Inner loop, half size for each step
        {
          i = id;
          int ixj=i^j;
          if ((ixj)>i)
            {
              if ((i&k)==0 && data[i]>data[ixj]) exchange(&data[i],&data[ixj]);
              if ((i&k)!=0 && data[i]<data[ixj]) exchange(&data[i],&data[ixj]);
            }
          barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);
        }
      }
  //}
}
