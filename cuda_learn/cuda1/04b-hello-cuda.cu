#include <stdio.h>

__global__ void hello( void ) {
	printf( "hello CUDA!\n" );
}

int main( void ) {
	hello<<<1,1>>>();
	fflush( stdout );
	return 0;
}

/* (c) 2021-2022. biztripcru@gmail.com. All rights reserved. */
