/* header files */
// standard headers
#include <stdlib.h>
#include <stdio.h>

// OpenCL headers
#include <CL/opencl.h>

// misc. headers
#include "include/helper_timer.h"	// for time

/* global variables */
const int element_count = 11444777;

cl_platform_id oclPlatformId;
cl_device_id oclDeviceId;

cl_context oclContext;
cl_command_queue oclCommandQueue;

cl_program oclProgram;
cl_kernel oclKernel;

float *host_input_1 = NULL;
float *host_input_2 = NULL;
float *host_output = NULL;
float *gold = NULL;

cl_mem device_input_1 = NULL;
cl_mem device_input_2 = NULL;
cl_mem device_output = NULL;

float timeOnGPU = 0.0f;
float timeOnCPU = 0.0f;

const char *oclSourceCode =
"__kernel void largeVectAdd_GPU(__global float *a, __global float *b, __global float *out, int len)" \
"{" \
	"int i = get_global_id(0);" \
								\
	"if (i < len)" \
	"{"
		"*(out + i) = *(a + i) + *(b + i);" \
	"}" \
"}";

/* entry-point function */
int main(void)
{
	// function prototypes
	void cleanup(void);
	void populateWithClampedFloatingPointRandomNumbers(float *const, size_t);
	size_t roundGlobalSizeToNearestLocalSizeMultiple(size_t, int);
	void largeVectAdd_CPU(const float *const, const float *const, float *const, const int);

	// variable declarations
	int size = element_count * sizeof(float);
	cl_int result;

	// code
	// allocate host memory
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
		printf("malloc: failed to allocate memory for host_input_2\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	host_output = (float *)malloc(size);
	if (!host_output)
	{
		printf("malloc: failed to allocate memory for host_output\n");
		cleanup();
		exit(EXIT_FAILURE);
	}

	gold = (float *)malloc(size);
	if (!gold)
	{
		printf("malloc: failed to allocate memory for gold\n");
		cleanup();
		exit(EXIT_FAILURE);
	}
	
	// populate host input arrays
	populateWithClampedFloatingPointRandomNumbers(host_input_1, element_count);
	populateWithClampedFloatingPointRandomNumbers(host_input_2, element_count);

	// obtain the 1st OpenCL supporting platform's ID
	result = clGetPlatformIDs(1, &oclPlatformId, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clGetPlatformIDs: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// obtain the 1st OpenCL supporting device's ID
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

	// create command queue
	oclCommandQueue = clCreateCommandQueue(oclContext, oclDeviceId, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateCommandQueue: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}
	
	// create program from source
	oclProgram = clCreateProgramWithSource(oclContext, 1, &oclSourceCode, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateProgramWithSource: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// build
	result = clBuildProgram(oclProgram, 0, NULL, NULL, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		size_t len;
		char build_log[2048];

		result = clGetProgramBuildInfo(oclProgram, oclDeviceId, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, &len);
		if (result != CL_SUCCESS)
		{
			printf("clGetProgramBuildInfo: failed with code %d\n", result);
			cleanup();
			exit(EXIT_FAILURE);
		}

		printf("build error:\n");
		printf("build log for 'oclSourceCode':\n");
		printf("%s\n", build_log);

		printf("clBuildProgram: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// create OpenCL kernel
	oclKernel = clCreateKernel(oclProgram, "largeVectAdd_GPU", &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateKernel: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// allocate device memory
	device_input_1 = clCreateBuffer(oclContext, CL_MEM_READ_ONLY, size, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateBuffer: failed to allocate device memory for device_input_1 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	device_input_2 = clCreateBuffer(oclContext, CL_MEM_READ_ONLY, size, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateBuffer: failed to allocate device memory for device_input_2 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	device_output = clCreateBuffer(oclContext, CL_MEM_WRITE_ONLY, size, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateBuffer: failed to allocate device memory for device_output with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// set the arguments to pass
	result = clSetKernelArg(oclKernel, 0, sizeof(cl_mem), (void *)&device_input_1);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArgs: failed for argument 0 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 1, sizeof(cl_mem), (void *)&device_input_2);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArgs: failed for argument 1 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 2, sizeof(cl_mem), (void *)&device_output);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArgs: failed for argument 2 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 3, sizeof(cl_int), (void *)&element_count);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArgs: failed for argument 3 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// populate device input buffers
	result = clEnqueueWriteBuffer(oclCommandQueue, device_input_1, CL_FALSE, 0, size, host_input_1, 0, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clEnqueueWriteBuffer: failed for device_input_1 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clEnqueueWriteBuffer(oclCommandQueue, device_input_2, CL_FALSE, 0, size, host_input_2, 0, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clEnqueueWriteBuffer: failed for device_input_2 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// setting up the timer
	StopWatchInterface *timer = NULL;
	sdkCreateTimer(&timer);

	// start timer
	sdkStartTimer(&timer);
	{
		// configure the kernel
		const size_t local_work_size = 256;
		const size_t global_work_size = roundGlobalSizeToNearestLocalSizeMultiple(local_work_size, element_count);

		result = clEnqueueNDRangeKernel(oclCommandQueue, oclKernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);
		if (result != CL_SUCCESS)
		{
			printf("clEnqueueNDRangeKernel: failed with code %d\n", result);

			sdkStopTimer(&timer);
			sdkDeleteTimer(&timer);
			timer = NULL;

			cleanup();
			exit(EXIT_FAILURE);
		}

		// execute
		clFinish(oclCommandQueue);
	}
	// stop timer
	sdkStopTimer(&timer);

	timeOnGPU = sdkGetTimerValue(&timer);

	sdkDeleteTimer(&timer);
	timer = NULL;

	// read results back into host buffer
	result = clEnqueueReadBuffer(oclCommandQueue, device_output, CL_TRUE, 0, size, host_output, 0, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clEnqueueReadBuffer: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// reset timer
	sdkCreateTimer(&timer);
	
	// start timer
	sdkStartTimer(&timer);
	{
		largeVectAdd_CPU(host_input_1, host_input_2, gold, element_count);
	}
	sdkStopTimer(&timer);

	timeOnCPU = sdkGetTimerValue(&timer);

	sdkDeleteTimer(&timer);
	timer = NULL;

	// comparison for accuracy
	const float epsilon = 0.000001f;
	int firstInaccurateIndex = -1;
	bool isAccurate = true;

	for (int i = 0; i < element_count; i++)
	{
		if (fabs(gold[i] - host_output[i]) > epsilon)
		{
			firstInaccurateIndex = i;
			isAccurate = false;
			break;
		}
	}
	
	char statementOfAccuracy[128];
	if (!isAccurate)
	{
		sprintf(statementOfAccuracy, "GPU produced atleast 1 result not within %.6f of that produced by the CPU at index %d", epsilon, firstInaccurateIndex);
	}
	else
	{
		sprintf(statementOfAccuracy, "All GPU results are within %.6f of CPU results", epsilon);
	}

	// comparison for performance
	float timeRatio = timeOnCPU / timeOnGPU;

	char statementOfPerformance[64];
	if (timeRatio < 1.0f)
	{
		sprintf(statementOfPerformance, "CPU performed %.6f times faster than the GPU", 1.0f / timeRatio);
	}
	else
	{
		sprintf(statementOfPerformance, "GPU performed %.6f times faster than the CPU", timeRatio);
	}

	// display results
	printf("\n");
	printf("2 large vectors, %d elements in size, were summed on the CPU and the GPU; the results are as listed:\n\n", element_count);

	printf("Input Vector A began from index 0 with %.6f and ended at index %d with %.6f\n", host_input_1[0], element_count - 1, host_input_1[element_count - 1]);
	printf("Input Vector B began from index 0 with %.6f and ended at index %d with %.6f\n", host_input_2[0], element_count - 1, host_input_2[element_count - 1]);
	printf("Result Vector C began from index 0 with %.6f and ended at index %d with %.6f\n\n", host_output[0], element_count - 1, host_output[element_count - 1]);

	printf("\tTime spent on the CPU: %.6f ms\n\n", timeOnCPU);

	printf("\tTime spent on the GPU: %.6f ms\n\n", timeOnGPU);

	printf("CPU vs GPU with respect to Accuracy:\n");
	printf("\t%s\n", statementOfAccuracy);

	printf("CPU vs GPU with respect to performance:\n");
	printf("\t%s\n", statementOfPerformance);

	// cleanup
	cleanup();

	return 0;
}

void populateWithClampedFloatingPointRandomNumbers(float *const v, size_t len)
{
	// variable declarations
	const float scalar = 1.0f / (float)RAND_MAX;

	for (int i = 0; i < element_count; i++)
		*(v + i) = scalar * rand();
}

size_t roundGlobalSizeToNearestLocalSizeMultiple(size_t local_size, int total_size)
{
	// code
	unsigned int rem = (total_size % local_size);
	
	return (rem > 0) ? (total_size + local_size - rem) : total_size;
}

void largeVectAdd_CPU(const float *const a, const float *const b, float *const out, const int len)
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
		clReleaseMemObject(device_output);
		device_output = NULL;
	}
	if (device_input_2)
	{
		clReleaseMemObject(device_input_2);
		device_input_2 = NULL;
	}
	if (device_input_1)
	{
		clReleaseMemObject(device_input_1);
		device_input_1 = NULL;
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
