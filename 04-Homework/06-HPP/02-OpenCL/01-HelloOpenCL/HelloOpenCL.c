/* header files */
// standard headers
#include <stdlib.h>
#include <stdio.h>

// OpenCL headers
#include <CL/opencl.h>

/* global variables */
const int element_count = 5;

cl_platform_id oclPlatformId;
cl_device_id oclDeviceId;

cl_context oclContext;
cl_command_queue oclCommandQueue;

cl_program oclProgram;
cl_kernel oclKernel;

float *host_input_1 = NULL;
float *host_input_2 = NULL;
float *host_output = NULL;

cl_mem device_input_1 = NULL;
cl_mem device_input_2 = NULL;
cl_mem device_output = NULL;

/* OpenCL kernel */
const char *oclSourceCode =
"__kernel void vectAdd_GPU (__global float *a, __global float *b, __global float *out, int len)" \
"{" \
	"int i = get_global_id(0);" \
	"if (i < len)" \
	"{" \
		"out[i] = a[i] + b[i];" \
	"}" \
"}";

/* entry-point function */
int main(void)
{
	// function prototypes
	void cleanup(void);

	// variable declarations
	int size = element_count * sizeof(float);
	cl_int result = CL_SUCCESS;

	// code
	// allocate host memory
	host_input_1 = (float *)malloc(size);
	if (!host_input_1)
	{
		printf("malloc: failed to allocate memory for host_input_1\n");
		exit(EXIT_FAILURE);
	}

	host_input_2 = (float *)malloc(size);
	if (!host_input_2)
	{
		printf("malloc: failed to allocate memory for host_input_2\n");
		exit(EXIT_FAILURE);
	}

	host_output = (float *)malloc(size);
	if (!host_output)
	{
		printf("malloc: failed to allocate memory for host_output\n");
		exit(EXIT_FAILURE);
	}

	// populate host input arrays
	for (int i = 0; i < element_count; i++)
	{
		host_input_1[i] = 101.0f + i;
		host_input_2[i] = 201.0f + i;
	}

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
		printf("clGetDeviceIDs: failed with error code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// create an OpenCL compute context
	oclContext = clCreateContext(NULL, 1, &oclDeviceId, NULL, NULL, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateContext: failed with error code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// create command queue
	oclCommandQueue = clCreateCommandQueue(oclContext, oclDeviceId, 0, &result);
	if (result != CL_SUCCESS)
	{
		printf("clCreateCommandQueue: failed with error code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// create OpenCL program from source.cl
	oclProgram = clCreateProgramWithSource(oclContext, 1, (const char **)&oclSourceCode, NULL, &result);
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
	oclKernel = clCreateKernel(oclProgram, "vectAdd_GPU", &result);
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
		printf("clSetKernelArg: failed for argument 0 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 1, sizeof(cl_mem), (void *)&device_input_2);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArg: failed for argument 1 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 2, sizeof(cl_mem), (void *)&device_output);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArg: failed for argument 2 with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	result = clSetKernelArg(oclKernel, 3, sizeof(cl_int), (void *)&element_count);
	if (result != CL_SUCCESS)
	{
		printf("clSetKernelArg: failed for argument 3 with code %d\n", result);
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

	// configure the kernel
	size_t global_size = element_count;	// 1-D per-element operation

	result = clEnqueueNDRangeKernel(oclCommandQueue, oclKernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clEnqueueNDRangeKernel: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// execute the kernel
	clFinish(oclCommandQueue);

	// read results back into host buffers
	result = clEnqueueReadBuffer(oclCommandQueue, device_output, CL_TRUE, 0, size, (void *)host_output, 0, NULL, NULL);
	if (result != CL_SUCCESS)
	{
		printf("clEnqueueReadBuffer: failed with code %d\n", result);
		cleanup();
		exit(EXIT_FAILURE);
	}

	// display results
	int i;

	printf("\n");
	printf("--------------------------------------\n");
	printf("Output from the OpenCL Compute Device:\n");
	printf("--------------------------------------\n\n");
	for (i = 0; i < element_count; i++)
	{
		printf("%f + %f = %f\n", host_input_1[i], host_input_2[i], host_output[i]);
	}
	printf("--------------------------------------\n");

	cleanup();

	return 0;
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
