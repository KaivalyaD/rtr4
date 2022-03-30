/* header files */
// standard headers
#include<stdio.h>

// CUDA headers
#include<cuda.h>
#include"include/helper_timer.h"

/* global variable declarations */
const size_t element_count = 11444777;

float *host_input_1 = NULL;
float *host_input_2 = NULL;
float *host_output = NULL;
float *gold = NULL;

float *device_input_1 = NULL;
float *device_input_2 = NULL;
float *device_output = NULL;

float time_on_CPU = 0.0f;
float time_on_GPU = 0.0f;

/* CUDA kernel */
__global__ void largeVectAdd_GPU(float *a, float *b, float *out, size_t len)
{
	// code
	int i = blockIdx.x * blockDim.x + threadIdx.x;

	if (i < len)
	{
		*(out + i) = *(a + i) + *(b + i);
	}
}

/* entry-point function */
int main(void)
{
	// function prototypes
	void populateWithClampedFloatingPointRandomNumbers(float *, size_t);
	void largeVectAdd_CPU(const float *, const float *, float *, const size_t);
	void cleanup(void);

	// variable declarations
	size_t size = element_count * sizeof(float);
	cudaError_t result = cudaSuccess;

	// code
	// host memory allocation
	host_input_1 = (float *)malloc(size);
	if (!host_input_1)
	{
		printf("malloc: failed to allocate memory for host buffer host_input_1\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	host_input_2 = (float *)malloc(size);
	if (!host_input_2)
	{
		printf("malloc: failed to allocate memory for host buffer host_input_2\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	host_output = (float *)malloc(size);
	if (!host_output)
	{
		printf("malloc: failed to allocate memory for host buffer host_output\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	gold = (float *)malloc(size);
	if (!gold)
	{
		printf("malloc: failed to allocate memory for host buffer gold\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// populate input host buffers
	populateWithClampedFloatingPointRandomNumbers(host_input_1, element_count);
	populateWithClampedFloatingPointRandomNumbers(host_input_2, element_count);

	// device memory allocation
	result = cudaMalloc((void **)&device_input_1, size);
	if (result != cudaSuccess)
	{
		printf("cudaMalloc: failed to allocate memory for device buffer device_input_1\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = cudaMalloc((void **)&device_input_2, size);
	if (result != cudaSuccess)
	{
		printf("cudaMalloc: failed to allocate memory for device buffer device_input_2\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = cudaMalloc((void **)&device_output, size);
	if (result != cudaSuccess)
	{
		printf("cudaMalloc: failed to allocate memory for device buffer device_output\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// copy host input buffers into device input buffers
	result = cudaMemcpy(device_input_1, host_input_1, size, cudaMemcpyHostToDevice);
	if (result != cudaSuccess)
	{
		printf("cudaMemcpy: failed to copy host buffer host_input_1 into device buffer device_input_1\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = cudaMemcpy(device_input_2, host_input_2, size, cudaMemcpyHostToDevice);
	if (result != cudaSuccess)
	{
		printf("cudaMemcpy: failed to copy host buffer host_input_2 into device buffer device_input_2\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// configuring kernel dimensions
	dim3 dimGrid = dim3((size_t)ceil((float)element_count / 256.0f), 1, 1);
	dim3 dimBlock = dim3(256, 1, 1);

	// setup timer
	StopWatchInterface *timer = NULL;
	sdkCreateTimer(&timer);

	// start timer
	sdkStartTimer(&timer);
	{
		// launch the kernel
		largeVectAdd_GPU<<< dimGrid, dimBlock >>>(device_input_1, device_input_2, device_output, size);
	}
	sdkStopTimer(&timer);
	
	// get time spent on GPU
	time_on_GPU = sdkGetTimerValue(&timer);
	
	// delete timer
	sdkDeleteTimer(&timer);
	timer = NULL;

	// copy device output buffer into host output buffer
	result = cudaMemcpy(host_output, device_output, size, cudaMemcpyDeviceToHost);
	if (result != cudaSuccess)
	{
		printf("cudaMemcpy: failed to copy device buffer device_output into host buffer host_output\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// perform the same addition on host
	// setup timer
	sdkCreateTimer(&timer);

	// start timer
	sdkStartTimer(&timer);
	{
		largeVectAdd_CPU(host_input_1, host_input_2, gold, element_count);
	}
	sdkStopTimer(&timer);

	// get time spent on CPU
	time_on_CPU = sdkGetTimerValue(&timer);

	// delete timer
	sdkDeleteTimer(&timer);
	timer = NULL;

	// measuring GPU accuracy
	const float epsilon = 0.000001;
	int firstInaccurateIndex = -1;
	bool isAccurate = true;

	for (int i = 0; i < element_count; i++)
	{
		float g = gold[i];
		float v = host_output[i];

		if (fabs(g - v) > epsilon)
		{
			isAccurate = false;
			firstInaccurateIndex = i;
			break;
		}
	}

	char statementOfAccuracy[128];
	if (!isAccurate)
	{
		sprintf(statementOfAccuracy, "GPU produced atleast 1 result not within %f of that produced by the CPU at index %d", epsilon, firstInaccurateIndex);
	}
	else
	{
		sprintf(statementOfAccuracy, "All GPU results are within %.6f of CPU results", epsilon);
	}

	char statementOfPerformance[128];
	float timeRatio = time_on_CPU / time_on_GPU;
	if (timeRatio < 1.0f)
	{
		sprintf(statementOfPerformance, "CPU outperformed the GPU by %.6f times", 1.0f / timeRatio);
	}
	else
	{
		sprintf(statementOfPerformance, "GPU outperformed the CPU by %.6f times", timeRatio);
	}

	// display output of the benchmark
	printf("\n");
	printf("+------------------------------------------------------------------------------+\n");
	printf("+                      CPU vs GPU on Large Vector Addition                     +\n");
	printf("+                                                                              +\n");
	printf("+                                                                              +\n");
	printf("+ vectorA begins at 0 with %.6f and ends at %lld with %.6f         +\n", host_input_1[0], element_count - 1, host_input_1[element_count - 1]);
	printf("+ vectorB begins at 0 with %.6f and ends at %lld with %.6f         +\n", host_input_2[0], element_count - 1, host_input_2[element_count - 1]);
	printf("+                                                                              +\n");
	printf("+ on CPU:                                                                      +\n");
	printf("+ \tvectorOut begins at 0 with %.6f and ends at %lld with %.6f +\n", gold[0], element_count - 1, gold[element_count - 1]);
	printf("+                                                                              +\n");
	printf("+ \ttime taken on the CPU: %.6f ms                                    +\n", time_on_CPU);
	printf("+                                                                              +\n");
	printf("+------------------------------------------------------------------------------+\n");
	printf("+                                                                              +\n");
	printf("+ on GPU:                                                                      +\n");
	printf("+ \tvectorOut begins at 0 with %.6f and ends at %lld with %.6f +\n", host_output[0], element_count - 1, host_output[element_count - 1]);
	printf("+                                                                              +\n");
	printf("+ \tCUDA kernel grid dimensions: %d x %d x %d                             +\n", dimGrid.x, dimGrid.y, dimGrid.z);
	printf("+ \tCUDA kernel block dimensions: %d x %d x %d                              +\n", dimBlock.x, dimBlock.y, dimBlock.z);
	printf("+                                                                              +\n");
	printf("+ \ttime taken on the GPU: %.6f ms                                     +\n", time_on_GPU);
	printf("+                                                                              +\n");
	printf("+ Comparison of CPU and GPU Accuracy in Large Vector Addition:                 +\n");
	printf("+ %s                           +\n", statementOfAccuracy);
	printf("+                                                                              +\n");
	printf("+ Comparison of CPU and GPU Performance in Large Vector Addition:              +\n");
	printf("+ %s                                 +\n", statementOfPerformance);
	printf("+------------------------------------------------------------------------------+\n");

	// cleanup
	cleanup();

	return 0;
}

void populateWithClampedFloatingPointRandomNumbers(float *v, size_t len)
{
	// code
	const float scalar = 1.0f / (float)RAND_MAX;

	for (int i = 0; i < len; i++)
	{
		*(v + i) = scalar * rand();
	}
}

void largeVectAdd_CPU(const float *a, const float *b, float *out, const size_t len)
{
	// code
	for (int i = 0; i < len; i++)
	{
		*(out + i) = *(a + i) + *(b + i);
	}
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
	if (gold)
	{
		free(gold);
		gold = NULL;
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
