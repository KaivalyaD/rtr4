/* header files */
// standard headers
#include <stdio.h>
#include <math.h>

// OpenCL headers
#include "CL/opencl.h"

// misc.
#include "include/helper_timer.h"

/* macros */
#define BLOCK_WIDTH 1024

/* global variable declarations */
cl_platform_id oclPlatformId;
cl_device_id oclDeviceId;

cl_context oclContext;
cl_command_queue oclCommandQueue;

cl_program oclProgram;
cl_kernel oclKernel;

cl_mem deviceA = NULL;
cl_mem deviceB = NULL;
cl_mem deviceC = NULL;

int *hostA = NULL;
int *hostB = NULL;
int *hostC = NULL;
int *gold = NULL;

/* OpenCL kernel */
const char *oclSource =
"__kernel void multiplyMatrices_GPU(__global int *A, __global int *B, __global int *C, int numRowsA, int numColsA, int numColsB, int numColsC)\n" \
"{\n" \
	"\tint i = get_global_id(0);\n" \
	"\tint j = get_global_id(1);\n" \
	"\tint c_ij = 0;\n" \
	"\t\n"\
	"\tif(i < numRowsA && j < numColsB)\n" \
	"\t{\n" \
		"\t\tfor(int k = 0; k < numColsA; k++)\n" \
		"\t\t{\n" \
			"\t\t\tc_ij = c_ij + *(A + (i * numColsA) + k) * *(B + j + (k * numColsB));\n" \
		"\t\t}\n" \
		"\t\t\n"\
		"\t\t*(C + (i * numColsC) + j) = c_ij;\n" \
	"\t}\n" \
"}\n";

/* entry point function */
int main(void)
{
	// function prototypes
	void initA(int *const data, int, int);
	void initB(int *const data, int, int);
	void multiplyMatrices_CPU(const int *const, const int *const, int *const, const int, const int, const int, const int);
	void cleanup(void);

	// variable declarations

	const int numRowsA = BLOCK_WIDTH;
	const int numColsA = BLOCK_WIDTH;

	const int numRowsB = numColsA;
	const int numColsB = BLOCK_WIDTH;

	const int numRowsC = numRowsA;
	const int numColsC = numColsB;

	const int numRowsGold = numRowsC;
	const int numColsGold = numColsC;

	const int sizeA = numRowsA * numColsA * sizeof(int);
	const int sizeB = numRowsB * numColsB * sizeof(int);
	const int sizeC = numRowsC * numColsC * sizeof(int);
	const int sizeGold = numRowsGold * numColsGold * sizeof(int);

	const void *kernelArgs[] = { &deviceA, &deviceB, &deviceC, &numRowsA, &numColsA, &numColsB, &numColsC };

	float timeOnCPU = 0.0f;
	float timeOnGPU = 0.0f;
	cl_int result;

	// code
	// allocate host memory
	hostA = (int *)malloc(sizeA);
	if (!hostA)
	{
		printf("malloc: failed for hostA\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	hostB = (int *)malloc(sizeB);
	if (!hostB)
	{
		printf("malloc: failed for hostB\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	hostC = (int *)malloc(sizeC);
	if (!hostC)
	{
		printf("malloc: failed for hostC\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	gold = (int *)malloc(sizeGold);
	if (!gold)
	{
		printf("malloc: failed for gold\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	// print pre-operations information
	printf("\n");

	printf("====== Comparison of CPU and GPU Time Performance for Large Matrix Multiplication using OpenCL ======\n\n");

	printf("Dimensions of Matrices:\n");
	printf("\tA:\t%d x %d\n", numRowsA, numColsA);
	printf("\tB:\t%d x %d\n", numRowsB, numColsB);
	printf("\tC:\t%d x %d\n", numRowsC, numColsC);
	printf("\tgold:\t%d x %d\n\n", numRowsGold, numColsGold);

	printf("Sizes (in bytes) of Matrices:\n");
	printf("\tA:\t%d\n", sizeA);
	printf("\tB:\t%d\n", sizeB);
	printf("\tC:\t%d\n", sizeC);
	printf("\tgold:\t%d\n\n", sizeGold);

	// populate host input buffers
	initA(hostA, numRowsA, numColsA);
	initB(hostB, numRowsB, numColsB);

	// get the 1st OpenCL supporting platform's ID
	result = clGetPlatformIDs(1, &oclPlatformId, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clGetPlatformIDs: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// get the 1st OpenCL supporting device's ID
	result = clGetDeviceIDs(oclPlatformId, CL_DEVICE_TYPE_GPU, 1, &oclDeviceId, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clGetDeviceIDs: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// create an OpenCL compute context
	oclContext = clCreateContext(NULL, 1, &oclDeviceId, NULL, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateContext: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// create an OpenCL command queue
	oclCommandQueue = clCreateCommandQueue(oclContext, oclDeviceId, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateCommandQueue: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// create an OpenCL program with source
	oclProgram = clCreateProgramWithSource(oclContext, 1, (const char **)&oclSource, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateProgramWithSource: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// build the program
	result = clBuildProgram(oclProgram, 0, NULL, NULL, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		size_t len;
		char buildLog[2048];
		const cl_int ret = result;
		
		result = clGetProgramBuildInfo(oclProgram, oclDeviceId, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, &len);
		if (result != CL_SUCCESS)
		{
			printf("clGetProgramBuildInfo: failed with code %d\n", result);
			cleanup();
			exit(EXIT_FAILURE);
		}

		printf("\n====== OpenCL Program Build Log ======\n\n");
		printf("%s", buildLog);
		printf("clBuildProgram: failed with code %d\n", ret);

		cleanup();
		exit(EXIT_FAILURE);
	}

	// create an OpenCL kernel from the program
	oclKernel = clCreateKernel(oclProgram, "multiplyMatrices_GPU", &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateKernel: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// by this line, there are no syntactical issues in the OpenCL program,
	// and thus, atleast a point in allocating device memory
	deviceA = clCreateBuffer(oclContext, CL_MEM_READ_ONLY, sizeA, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateBuffer: failed to allocate device memory for deviceA with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	deviceB = clCreateBuffer(oclContext, CL_MEM_READ_ONLY, sizeB, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateBuffer: failed to allocate device memory for deviceB with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	deviceC = clCreateBuffer(oclContext, CL_MEM_WRITE_ONLY, sizeC, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateBuffer: failed to allocate device memory for deviceC with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// set arguments to the kernel
	result = clSetKernelArg(oclKernel, 0, sizeof(cl_mem), (void *)&deviceA);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArg: failed for argument 0 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 1, sizeof(cl_mem), (void *)&deviceB);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArg: failed for argument 1 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 2, sizeof(cl_mem), (void *)&deviceC);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArg: failed for argument 2 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 3, sizeof(int), (void *)&numRowsA);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArg: failed for argument 3 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 4, sizeof(int), (void *)&numColsA);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArg: failed for argument 4 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 5, sizeof(int), (void *)&numColsB);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArg: failed for argument 5 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 6, sizeof(int), (void *)&numColsC);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArg: failed for argument 6 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	/* for (int argIndex = 0; argIndex < 7; argIndex++)
	{
		int argSize = (argIndex < 3) ? sizeof(cl_mem) : sizeof(int);

		result = clSetKernelArg(oclKernel, argIndex, argSize, *(kernelArgs + (argIndex * argSize)));
		if (result != CL_SUCCESS)
		{
			printf("clSetKernelArg: failed for argument %d with code %d\n", argIndex, result);
			cleanup();
			exit(EXIT_FAILURE);
		}
	} */

	// copy content from host buffers to device buffers
	result = clEnqueueWriteBuffer(oclCommandQueue, deviceA, CL_FALSE, 0, sizeA, (void *)hostA, 0, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clEnqueueWriteBuffer: failed for deviceA with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clEnqueueWriteBuffer(oclCommandQueue, deviceB, CL_FALSE, 0, sizeB, (void *)hostB, 0, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clEnqueueWriteBuffer: failed for deviceB with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// configure the kernel
	size_t globalWorkSize[] = { BLOCK_WIDTH, BLOCK_WIDTH };
	
	// start timer
	StopWatchInterface *timer = NULL;
	sdkCreateTimer(&timer);
	sdkStartTimer(&timer);

	// execute kernel on GPU
	result = clEnqueueNDRangeKernel(oclCommandQueue, oclKernel, 2, NULL, globalWorkSize, NULL, 0, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clENqueueNDRangeKernel: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}
	clFinish(oclCommandQueue);

	// stop timer
	sdkStopTimer(&timer);
	timeOnGPU = sdkGetTimerValue(&timer);
	sdkDeleteTimer(&timer);
	timer = NULL;
	
	// read result buffers back from device to host
	result = clEnqueueReadBuffer(oclCommandQueue, deviceC, CL_TRUE, 0, sizeC, (void *)hostC, 0, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clEnqueueReadBuffer: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// start timer to compare with host performance
	sdkCreateTimer(&timer);
	sdkStartTimer(&timer);

	// multiply matrices on CPU
	multiplyMatrices_CPU(hostA, hostB, gold, numRowsA, numColsA, numColsB, numColsGold);

	// stop timer
	sdkStopTimer(&timer);
	timeOnCPU = sdkGetTimerValue(&timer);
	sdkDeleteTimer(&timer);
	timer = NULL;

	// compare performance
	int firstInaccurateIndex = -1;
	bool isAccurate = true;
	char statementOfAccuracy[64];
	for (int i = 0; i < numRowsC * numColsC; i++)
	{
		if (gold[i] != hostC[i])
		{
			firstInaccurateIndex = i;
			isAccurate = false;
			break;
		}
	}
	if (!isAccurate)
		sprintf(statementOfAccuracy, "CPU and GPU calculations mismatched (first inaccuracy at index %d)", firstInaccurateIndex);
	else
		sprintf(statementOfAccuracy, "All CPU and GPU calculations matched");

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

void multiplyMatrices_CPU(const int *const A, const int *const B, int *const C, const int numRowsA, const int numColsA, const int numColsB, const int numColsC)
{
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
		clReleaseMemObject(deviceC);
		deviceC = NULL;
	}
	if (deviceB)
	{
		clReleaseMemObject(deviceB);
		deviceB = NULL;
	}
	if (deviceA)
	{
		clReleaseMemObject(deviceA);
		deviceA = NULL;
	}
	if (oclKernel)
	{
		clReleaseKernel(oclKernel);
		oclKernel = NULL;
	}
	if (oclProgram)
	{
		clReleaseProgram(oclProgram);
		oclProgram = NULL;
	}
	if (oclCommandQueue)
	{
		clReleaseCommandQueue(oclCommandQueue);
		oclCommandQueue = NULL;
	}
	if (oclContext)
	{
		clReleaseContext(oclContext);
		oclContext = NULL;
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
