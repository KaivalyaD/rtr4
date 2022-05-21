/* header files */
// standard headers
#include <stdio.h>
#include <stdlib.h>

// OpenCL headers
#include <CL/opencl.h>

/* macros */
#define HandleOCLError(code, msg) \
	if ((code) != CL_SUCCESS) \
	{ \
		printf(#msg": failed at line %d with code %d\n", __LINE__, (code)); \
		exit(EXIT_FAILURE); \
	}

/* entry-point function */
int main(void)
{
	// function prototypes
	void showOpenCLDeviceProperties(void);

	// code
	showOpenCLDeviceProperties();

	return 0;
}

void showOpenCLDeviceProperties(void)
{
	// variable declarations
	cl_int result;
	cl_platform_id oclPlatformId;
	cl_uint oclDevCount;
	cl_device_id *oclDeviceIds;
	char oclPlatformInfo[512], oclDeviceProperty[1024];

	// code
	printf("\n");
	printf("+++++++++++++++++++++ OpenCL Information +++++++++++++++++++++\n");
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

	/* enumerating available andsupported devices */
	// get first supported platform ID
	result = clGetPlatformIDs(1, &oclPlatformId, NULL);
	HandleOCLError(result, clGetPlatformIDs);

	// get GPU device count
	result = clGetDeviceIDs(oclPlatformId, CL_DEVICE_TYPE_GPU, 0, NULL, &oclDevCount);
	HandleOCLError(result, clGetDeviceIDs);
	if (oclDevCount == 0)
	{
		printf("Found 0 OpenCL Supported Devices on this System.\n");
		return;
	}

	// get platform name
	result = clGetPlatformInfo(oclPlatformId, CL_PLATFORM_NAME, sizeof(oclPlatformInfo), oclPlatformInfo, NULL);
	HandleOCLError(result, clGetPlatformInfo);
	printf("Platform Name: %s\n", oclPlatformInfo);

	// get platform version
	clGetPlatformInfo(oclPlatformId, CL_PLATFORM_VERSION, sizeof(oclPlatformInfo), oclPlatformInfo, NULL);
	HandleOCLError(result, clGetPlatformInfo);
	printf("Platform Version: %s\n", oclPlatformInfo);

	// show supported devices' count
	printf("Found %d GPU Device(s) on this System\n", oclDevCount);

	// allocate memory that can occupy these oclDeviceIds device IDs
	oclDeviceIds = (cl_device_id *)malloc(sizeof(cl_device_id) * oclDevCount);

	// get IDs into allocated buffer
	result = clGetDeviceIDs(oclPlatformId, CL_DEVICE_TYPE_GPU, oclDevCount, oclDeviceIds, NULL);
	HandleOCLError(result, clGetDeviceIDs);

	for (int i = 0; i < (int)oclDevCount; i++)
	{
		printf("\n");
		printf("---------- GPU DEVICE GENERAL INFORMATION ----------\n");
		printf("----------------------------------------------------\n\n");

		printf("Device Index\t\t: %d\n", i);

		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_NAME, sizeof(oclDeviceProperty), oclDeviceProperty, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Device Name\t\t: %s\n", oclDeviceProperty);

		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_VENDOR, sizeof(oclDeviceProperty), oclDeviceProperty, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Device Vendor\t\t: %s\n", oclDeviceProperty);

		result = clGetDeviceInfo(oclDeviceIds[i], CL_DRIVER_VERSION, sizeof(oclDeviceProperty), oclDeviceProperty, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Driver Version\t\t: %s\n", oclDeviceProperty);

		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_VERSION, sizeof(oclDeviceProperty), oclDeviceProperty, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Device OpenCL Version\t: %s\n", oclDeviceProperty);

		cl_uint oclDeviceClockFreq;
		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &oclDeviceClockFreq, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Device Max. Clock Rate\t: %u MHz\n", oclDeviceClockFreq);

		printf("\n");
		printf("----------- GPU DEVICE MEMORY INFORMATION ----------\n");
		printf("----------------------------------------------------\n");

		cl_ulong memSize;
		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &memSize, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Device Global Memory\t\t: %llu bytes\n", (unsigned long long)memSize);
		
		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &memSize, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Device Local Memory\t\t: %llu bytes\n", (unsigned long long)memSize);

		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &memSize, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Device Constant Buffer Size\t: %llu bytes\n", (unsigned long long)memSize);

		printf("\n");
		printf("---------- GPU DEVICE COMPUTE INFORMATION ----------\n");
		printf("----------------------------------------------------\n");

		cl_uint oclComputeUnits;
		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &oclComputeUnits, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Count of Parallel Processor Cores\t: %u\n", (unsigned int)oclComputeUnits);

		size_t oclWorkGroupSize;
		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &oclWorkGroupSize, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Device Work Group size\t\t\t: %u\n", (unsigned int)oclWorkGroupSize);

		size_t oclWorkItemDims;
		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &oclWorkItemDims, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Device Work Item Dimensions\t\t: %u\n", (unsigned int)oclWorkItemDims);

		size_t oclWorkItemSize[3];
		result = clGetDeviceInfo(oclDeviceIds[i], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(oclWorkItemSize), &oclWorkItemSize, NULL);
		HandleOCLError(result, clGetDeviceInfo);
		printf("Device Work Item Sizes\t\t\t: %u x %u x %u\n", (unsigned int)oclWorkItemSize[0], (unsigned int)oclWorkItemSize[1], (unsigned int)oclWorkItemSize[2]);
	}

	printf("\n");
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("++++++++++++++++++++++++++++ End +++++++++++++++++++++++++++++\n\n");

	free(oclDeviceIds);
	oclDeviceIds = NULL;
}
