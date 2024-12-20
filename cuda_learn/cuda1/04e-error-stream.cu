#include <iostream>
using namespace std;

__global__ void hello( void ) {
	std::cout << "hello world!" << std::endl;
}

int main( void ) {
	hello<<<1,1>>>();
	return 0;
}

/* (c) 2021-2022. biztripcru@gmail.com. All rights reserved. */
