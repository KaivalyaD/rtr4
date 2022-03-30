/* header files */
// standard headers
#include<stdio.h>

// CUDA headers
#include<cuda.h>
#include"include/helper_timer.h"

/* macros */
#define BLOCK_WIDTH 1024

/* global variable declarations */
int *hostA = NULL;
int *hostB = NULL;
int *hostC = NULL;
int *gold = NULL;

int *deviceA = NULL;
int *deviceB = NULL;
int *deviceC = NULL;

/* CUDA kernel */
__global__
void
multiplyMatrices_GPU (
	const int *const A,
	const int *const B,
	int *const C,
	const int numRowsA,
	const int numColsA,
	const int numColsB,
	const int numColsC
) {
	// variable declarations
	int row, column;

	// code
	row = blockIdx.y * blockDim.y + threadIdx.y;
	column = blockIdx.x * blockDim.x + threadIdx.x;

	if (row < numRowsA && column < numColsB)
	{
		int e, f, acc = 0;
		for (int k = 0; k < numColsA; k++)
		{
			e = *(A + (row * numColsA) + k);	 // (row * numColsA) is a constant for each SP that freezes a row
			f = *(B + column + (k * numColsB)); // column is also a constant for that SP which freezes a column
			
			/*
			 * multiply corresponding elements in the fixed row-column pair
			 * and accumulate their sum (an O(n) operation)
			 * Each row-column pair is therefore processed parallely
			 */
			acc += (e * f);
		}

		// finally, assign the accumulate to its appropriate position in the resultant matrix
		*(C + (row * numColsC) + column) = acc;
	}
}

/* entry-point function */
int main(void)
{
	// function prototypes
	void initA(int *const, const int, const int);
	void initB(int *const, const int, const int);
	void multiplyMatrices_CPU(
		const int *const,
		const int *const,
		int *const,
		const int,
		const int,
		const int,
		const int
	);
	void cleanup(void);

	// variable declarations
	int numRowsA = BLOCK_WIDTH;
	int numColsA = BLOCK_WIDTH;
	int numRowsB = BLOCK_WIDTH;
	int numColsB = BLOCK_WIDTH;
	
	int numRowsC = numRowsA;
	int numColsC = numColsB;
	int numRowsGold = numRowsC;
	int numColsGold = numColsC;

	size_t sizeA = numRowsA * numColsA * sizeof(int);
	size_t sizeB = numRowsB * numColsB * sizeof(int);
	size_t sizeC = numRowsC * numColsC * sizeof(int);
	size_t sizeGold = numRowsGold * numColsGold * sizeof(int);

	float timeOnGPU, timeOnCPU;

	cudaError_t result = cudaSuccess;

	// code
	// host memory allocation
	hostA = (int *)malloc(sizeA);
	if (!hostA)
	{
		printf("malloc: failed to allocate memory for host buffer hostA\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	hostB = (int *)malloc(sizeB);
	if (!hostB)
	{
		printf("malloc: failed to allocate memory for host buffer hostB\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	hostC = (int *)malloc(sizeC);
	if (!hostC)
	{
		printf("malloc: failed to allocate memory for host buffer hostC\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	gold = (int *)malloc(sizeGold);
	if (!gold)
	{
		printf("malloc: failed to allocate memory for host buffer gold\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// display pre-operations information
	printf("\n");

	printf("Dimensions of Matrices:\n");
	printf("\tA:\t%d x %d\n", numRowsA, numColsA);
	printf("\tB:\t%d x %d\n", numRowsB, numColsB);
	printf("\tC:\t%d x %d\n", numRowsC, numColsC);
	printf("\tgold:\t%d x %d\n\n", numRowsGold, numColsGold);

	printf("Sizes (in bytes) of Matrices:\n");
	printf("\tA:\t%llu\n", sizeA);
	printf("\tB:\t%llu\n", sizeB);
	printf("\tC:\t%llu\n", sizeC);
	printf("\tgold:\t%llu\n\n", sizeGold);

	// populate host input buffers
	initA(hostA, numRowsA, numColsA);
	initB(hostB, numRowsB, numColsB);

	// device memory allocation
	result = cudaMalloc((void **)&deviceA, sizeA);
	if (result != cudaSuccess)
	{
		printf("cudaMalloc: failed to allocate memory for device buffer deviceA\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = cudaMalloc((void **)&deviceB, sizeB);
	if (result != cudaSuccess)
	{
		printf("cudaMalloc: failed to allocate memory for device buffer deviceB\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = cudaMalloc((void **)&deviceC, sizeC);
	if (result != cudaSuccess)
	{
		printf("cudaMalloc: failed to allocate memory for device buffer deviceC\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// populate device input buffers
	result = cudaMemcpy(deviceA, hostA, sizeA, cudaMemcpyHostToDevice);
	if (result != cudaSuccess)
	{
		printf("cudaMemcpy: failed to copy host buffer hostA into device buffer deviceA\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = cudaMemcpy(deviceB, hostB, sizeB, cudaMemcpyHostToDevice);
	if (result != cudaSuccess)
	{
		printf("cudaMemcpy: failed to copy host buffer hostB into device buffer deviceB\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// configure the kernel
	dim3 dimGrid = dim3(ceil((int)numColsB / (int)BLOCK_WIDTH), ceil((int)numRowsA / (int)BLOCK_WIDTH));
	dim3 dimBlock = dim3(BLOCK_WIDTH, BLOCK_WIDTH, 1);

	// configure the timer
	StopWatchInterface *timer = NULL;
	sdkCreateTimer(&timer);
	
	// start the timer
	sdkStartTimer(&timer);
	{
		// launch the kernel
		multiplyMatrices_GPU<<< dimGrid, dimBlock >>>(
			deviceA,
			deviceB,
			deviceC,
			numRowsA,
			numColsA,
			numColsB,
			numColsGold
		);
	}
	// stop the timer
	sdkStopTimer(&timer);

	// extract time spent on GPU
	timeOnGPU = sdkGetTimerValue(&timer);
	
	// delete the timer
	sdkDeleteTimer(&timer);
	timer = NULL;

	// copy resultant matrix from device output buffer to host output buffer
	result = cudaMemcpy(hostC, deviceC, sizeC, cudaMemcpyDeviceToHost);
	if (result != cudaSuccess)
	{
		printf("cudaMemcpy: failed to copy device buffer deviceC into host buffer hostC\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// reconfigure timer
	sdkCreateTimer(&timer);

	// restart the timer
	sdkStartTimer(&timer);
	{
		// operate on the CPU
		multiplyMatrices_CPU(
			hostA,
			hostB,
			gold,
			numRowsA,
			numColsA,
			numColsB,
			numColsC
		);
	}
	// stop the timer
	sdkStopTimer(&timer);

	// extract time spent on CPU
	timeOnCPU = sdkGetTimerValue(&timer);

	// delete the timer
	sdkDeleteTimer(&timer);
	timer = NULL;

	// compare accuracies
	const float epsilon = 0.000001f;
	bool isAccurate = true;
	int firstInaccurateIndex = -1;
	char statementOfAccuracy[128];

	for (int i = 0; i < numRowsC * numColsC; i++)
	{
		if (fabs(hostC[i] - gold[i]) > epsilon)
		{
			isAccurate = false;
			firstInaccurateIndex = i;
			break;
		}
	}
	if (!isAccurate)
	{
		sprintf(
			statementOfAccuracy,
			"The GPU produced atleast 1 inaccurate result at %d such that its deviation from the actual result is >%.6f",
			firstInaccurateIndex,
			epsilon);
	}
	else
	{
		sprintf(
			statementOfAccuracy,
			"All GPU results are accurate atleast up to %.6f",
			epsilon
		);
	}

	// compare performance
	float performanceRatio = timeOnCPU / timeOnGPU;
	char statementOfPerformance[128];

	if (performanceRatio < 1.0f)
		sprintf(statementOfPerformance, "CPU outperformed the GPU by a factor of %.6f", 1 / performanceRatio);
	else
		sprintf(statementOfPerformance, "GPU outperformed the CPU by a factor of %.6f", performanceRatio);

	// display results of the benchmark
	printf("Results:\n\n");

	printf("\tTime spent on the CPU for Matrix Multiplication A * B = C: %.6f ms\n\n", timeOnCPU);
	
	printf("\tTime spent on the GPU for Matrix Multiplication A * B = C: %.6f ms\n\n", timeOnGPU);
	
	printf("CPU vs GPU with respect to Accuracy:\n");
	printf("\t%s\n", statementOfAccuracy);

	printf("CPU vs GPU with respect to performance:\n");
	printf("\t%s\n", statementOfPerformance);

	// cleanup
	cleanup();

	return 0;
}

void initA(int *const buffer, const int countRows, const int countCols)
{
	// variable declarations
	int v = 1;

	// code
	for (int i = 0; i < countRows; i++)
	{
		for (int j = 0; j < countCols; j++)
		{
			*(buffer + (i * countCols) + j) = v;
			v++;
		}
	}
}

void initB(int *const buffer, const int countRows, const int countCols)
{
	// variable declarations
	int v = BLOCK_WIDTH;

	// code
	for (int i = 0; i < countRows; i++)
	{
		for (int j = 0; j < countCols; j++)
		{
			*(buffer + (i * countCols) + j) = v;
			v--;
		}
	}
}

void
multiplyMatrices_CPU(
	const int *const A,
	const int *const B,
	int *const C,
	const int numRowsA,
	const int numColsA,
	const int numColsB,
	const int numColsC
) {
	// variable declarations
	int row, column;

	for (row = 0; row < numRowsA; row++)
	{
		for (column = 0; column < numColsB; column++)
		{
			int e1, e2, acc = 0;

			for (int k = 0; k < numColsA; k++)
			{
				e1 = *(A + (row * numColsA) + k);
				e2 = *(B + column + (k * numColsB));

				acc += e1 * e2;
			}

			*(C + (row * numColsC) + column) = acc;
		}
	}
}

void cleanup(void)
{
	// code
	if (deviceC)
	{
		cudaFree(deviceC);
		deviceC = NULL;
	}
	if (deviceB)
	{
		cudaFree(deviceB);
		deviceB = NULL;
	}
	if (deviceA)
	{
		cudaFree(deviceA);
		deviceA = NULL;
	}
	if (gold)
	{
		free(gold);
		gold = NULL;
	}
	if (hostC)
	{
		free(hostC);
		hostC = NULL;
	}
	if (hostB)
	{
		free(hostB);
		hostB = NULL;
	}
	if (hostA)
	{
		free(hostA);
		hostA = NULL;
	}
}
