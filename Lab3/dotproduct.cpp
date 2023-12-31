/**************************
** TDDD56 Lab 3
***************************
** Author:
** August Ernstsson
**************************/

#include <iostream>

#include <skepu>

/* SkePU user functions */

/*
float userfunction(...)
{
	// your code here
}

// more user functions...

*/

float mul(float a, float b){
	return a*b;
}

float add(float a, float b){
	return a+b;
}

int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " <input size> <backend>\n";
		exit(1);
	}
	
	const size_t size = std::stoul(argv[1]);
	auto spec = skepu::BackendSpec{argv[2]};
//	spec.setCPUThreads(<integer value>);
	skepu::setGlobalBackendSpec(spec);
	
	
	/* Skeleton instances */
//	auto instance = skepu::Map(userfunction);
// ...
	
	/* SkePU containers */
	skepu::Vector<float> v1(size, 1.0f), v2(size, 2.0f);
	
	
	/* Compute and measure time */
	float resComb, resSep;
	auto dotprod = skepu::MapReduce<2>(mul,add);

	auto timeComb = skepu::benchmark::measureExecTime([&]
	{
		// your code here
		resComb = dotprod(v1,v2);
	});
	
	auto multi = skepu::Map(mul);
	skepu::Vector<float> tempRes(size);
	auto addition = skepu::Reduce(add);

	auto timeSep = skepu::benchmark::measureExecTime([&]
	{
		// your code here		
		multi(tempRes,v1,v2);	
		resSep = addition(tempRes);
	});
	
	std::cout << "Time Combined: " << (timeComb.count() / 10E6) << " seconds.\n";
	std::cout << "Time Separate: " << ( timeSep.count() / 10E6) << " seconds.\n";
	
	
	std::cout << "Result Combined: " << resComb << "\n";
	std::cout << "Result Separate: " << resSep  << "\n";
	
	return 0;
}

