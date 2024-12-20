#include <stdio.h>

__global__ void hello( void ) {
	printf( "hello CUDA %d !\n", threadIdx.x );
}

int main( void ) {
	hello<<<1,8>>>();
#if defined(__linux__)
	cudaDeviceSynchronize();
#endif
	fflush( stdout );
	return 0;
}

/* (c) 2021-2022. biztripcru@gmail.com. All rights reserved. */
