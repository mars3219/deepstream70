#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cuda_runtime.h>

// main program for the CPU: compiled by MS-VC++
int main(void) {
	// host-side data
	const int SIZE = 8;
	const float a[SIZE] = { 1., 2., 3., 4., 5., 6., 7., 8. };
	float b[SIZE] = { 0., 0., 0., 0., 0., 0., 0., 0. };
	// print source
	printf("a = {%f,%f,%f,%f,%f,%f,%f,%f}\n", a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
	fflush( stdout );
	// device-side data
	float* dev_a = nullptr;
	float* dev_b = nullptr;
	// allocate device memory
	cudaMalloc( (void**)&dev_a, SIZE * sizeof(float) );
	cudaMalloc( (void**)&dev_b, SIZE * sizeof(float) );
	// 3 copies
	cudaMemcpy( dev_a, a, SIZE * sizeof(float), cudaMemcpyHostToDevice); // dev_a = a;
	cudaMemcpy( dev_b, dev_a, SIZE * sizeof(float), cudaMemcpyDeviceToDevice); // dev_b = dev_a;
	cudaMemcpy( b, dev_b, SIZE * sizeof(float), cudaMemcpyDeviceToHost); // b = dev_b;
	// free device memory
	cudaFree( dev_a );
	cudaFree( dev_b );
	// print the result
	printf("b = {%f,%f,%f,%f,%f,%f,%f,%f}\n", b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
	fflush( stdout );
	// done
	return 0;
}

/* (c) 2021-2022. biztripcru@gmail.com. All rights reserved. */
