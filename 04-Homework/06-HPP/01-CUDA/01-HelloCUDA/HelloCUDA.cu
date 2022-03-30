/* header files */
// standard
#include<stdio.h>	// for printf()

// CUDA
#include<cuda.h>

/* global variables */
const int element_count = 5;

float *host_input_1 = NULL;
float *host_input_2 = NULL;
float *host_output = NULL;

float *device_input_1 = NULL;
float *device_input_2 = NULL;
float *device_output = NULL;

/* CUDA kernel */
__global__ void vectAdd_GPU(float *a, float *b, float *out, int len)
{
	// code
	int i = blockIdx.x * blockDim.x + threadIdx.x;	// i represents ID of this hardware thread

	if (i < len)
	{
		*(out + i) = *(a + i) + *(b + i);
	}
}

/* entry-point function */
int main(void)
{
	// function declarations
	void cleanup(void);

	// variable declarations
	int size = element_count * sizeof(float), i;
	cudaError_t result = cudaSuccess;

	// code
	// host memory allocation
	host_input_1 = (float *)malloc(size);
	if (!host_input_1)
	{
		printf("malloc: failed to allocate memory for host_input_1\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	host_input_2 = (float *)malloc(size);
	if (!host_input_2)
	{
		printf("malloc: failed to allocate memory on host for host_input_2\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	host_output = (float *)malloc(size);
	if (!host_output)
	{
		printf("malloc: failed to allocate memory on host for host_output\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// filling the arrays
	for (i = 0; i < element_count; i++)
	{
		host_input_1[i] = 101.0f + i;
		host_input_2[i] = 201.0f + i;
	}

	// device memory allocation
	result = cudaMalloc((void **)&device_input_1, size);
	if (result != cudaSuccess)
	{
		printf("cudaMalloc: failed to allocate memory on device for device_input_1\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = cudaMalloc((void **)&device_input_2, size);
	if (result != cudaSuccess)
	{
		printf("cudaMalloc: failed to allocate memory on device for device_input_2\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = cudaMalloc((void **)&device_output, size);
	if (result != cudaSuccess)
	{
		printf("cudaMalloc: failed to allocate memory on device for device_output\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// copy data from host input buffers to device input buffers
	result = cudaMemcpy(device_input_1, host_input_1, size, cudaMemcpyHostToDevice);
	if (result != cudaSuccess)
	{
		printf("cudaMemcpy: failed to copy data from host buffer host_input_1 to device buffer device_input_1\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = cudaMemcpy(device_input_2, host_input_2, size, cudaMemcpyHostToDevice);
	if (result != cudaSuccess)
	{
		printf("cudaMemcpy: failed to copy data from host buffer host_input_2 to device buffer device_buffer_2\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// configure kernel grid dimensions
	dim3 dimGrid = dim3(element_count, 1, 1);	// 5 blocks in x direction for this grid
	dim3 dimBlock = dim3(1, 1, 1);	// 1 thread per block

	// launch the kernel
	vectAdd_GPU<<< dimGrid, dimBlock >>>(device_input_1, device_input_2, device_output, size);

	// copy data from device output buffer into host output buffer
	result = cudaMemcpy(host_output, device_output, size, cudaMemcpyDeviceToHost);
	if (result != cudaSuccess)
	{
		printf("cudaMemcpy: failed to copy data from device buffer device_output to host buffer host_output\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// display the output from CUDA compute device
	printf("\n------------------------------------\n");
	printf("Output from the CUDA Compute Device:\n");
	printf("------------------------------------\n");
	for (i = 0; i < element_count; i++)
		printf("%f + %f = %f\n", host_input_1[i], host_input_2[i], host_output[i]);
	printf("\n------------------------------------\n");

	// return all allocated resources
	cleanup();

	return 0;
}

void cleanup(void)
{
	// code
	if (device_output)
	{
		cudaFree(device_output);
		device_output = NULL;
	}
	if (device_input_2)
	{
		cudaFree(device_input_2);
		device_input_2 = NULL;
	}
	if (device_input_1)
	{
		cudaFree(device_input_1);
		device_input_1 = NULL;
	}
	if (host_output)
	{
		free(host_output);
		host_output = NULL;
	}
	if (host_input_2)
	{
		free(host_input_2);
		host_input_2 = NULL;
	}
	if (host_input_1)
	{
		free(host_input_1);
		host_input_1 = NULL;
	}
}
