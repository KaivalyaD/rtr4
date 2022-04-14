/* header files */
// standard headers
#include<stdio.h>

// CUDA headers
#include<cuda.h>

/* entry-point function */
int main(void)
{
	// function prototypes
	void showCudaDeviceProperties(void);

	// code
	showCudaDeviceProperties();

	return 0;
}

void showCudaDeviceProperties(void)
{
	// variable declarations
	cudaError_t retCudaRT;
	int devCount;

	// code
	printf("\n================================================== CUDA INFORMATION ===============================================\n\n");
	
	retCudaRT = cudaGetDeviceCount(&devCount);
	if (retCudaRT != cudaSuccess)
	{
		printf("in %s at line %d: %s\n", __FILE__, __LINE__, cudaGetErrorString(retCudaRT));
		exit(EXIT_FAILURE);
	}
	else if (devCount == 0)
	{
		printf("There is no CUDA-supported device on this system\n");
		return;
	}
	else
	{
		printf("Total number of CUDA-supported devices on this system : %d\n", devCount);

		for (int i = 0; i < devCount; i++)
		{
			cudaDeviceProp devProp;
			int driverVersion = 0, runtimeVersion = 0;

			retCudaRT = cudaGetDeviceProperties(&devProp, i);
			if (retCudaRT != cudaSuccess)
			{
				printf("in %s at line %d : %s\n", __FILE__, __LINE__, cudaGetErrorString(retCudaRT));
				exit(EXIT_FAILURE);
			}

			cudaDriverGetVersion(&driverVersion);
			cudaRuntimeGetVersion(&runtimeVersion);
			printf("\n======================================= CUDA DRIVER AND RUNTIME INFORMATION =======================================\n\n");

			printf("CUDA Driver Version  : %d.%d\n", driverVersion / 1000, (driverVersion % 100) / 10);
			printf("CUDA Runtime Version : %d.%d\n", runtimeVersion / 1000, (runtimeVersion % 100) / 10);
			
			printf("\n========================================== GPU DEVICE GENERAL INFORMATION =========================================\n\n");

			printf("GPU Device Number             : %d\n", i);
			printf("GPU Device Name               : %s\n", devProp.name);
			printf("GPU Device Compute Capability : %d.%d\n", devProp.major, devProp.minor);
			printf("GPU Device Clock Rate         : %lu\n", (unsigned long)devProp.clockRate);
			printf("GPU Device Type               : ");
			if (devProp.integrated)
				printf("Integrated (On-Board)\n");
			else
				printf("Discrete\n");

			printf("\n=========================================== GPU DEVICE MEMORY INFORMATION =========================================\n\n");

			printf("Total Memory                  : %.0fGiB = %.0fMiB = %llu Bytes\n",
				((float)devProp.totalGlobalMem / (1024.0f * 1024.0f * 1024.0f)),
				((float)devProp.totalGlobalMem / (1024.0f * 1024.0f)),
				(unsigned long long)devProp.totalGlobalMem);
			printf("Constant Memory               : %lu Bytes\n", (unsigned long)devProp.totalConstMem);
			printf("Shared Memory Per SMProcessor : %lu Bytes\n", (unsigned long)devProp.sharedMemPerBlock);

			printf("\n======================================= GPU DEVICE MULTIPROCESSOR INFORMATION =====================================\n\n");

			printf("Number of SMProcessors              : %d\n", devProp.multiProcessorCount);
			printf("Number of Registers per SMProcessor : %d\n", devProp.regsPerBlock);
			
			printf("\n=========================================== GPU DEVICE THREAD INFORMATION =========================================\n\n");

			printf("Max. number of Threads per SMProcessor : %d\n", devProp.maxThreadsPerMultiProcessor);
			printf("Max. number of Threads per Block       : %d\n", devProp.maxThreadsPerBlock);
			printf("Threads in Warp                        : %d\n", devProp.warpSize);
			printf("Max. Thread Dimensions                 : %d x %d x %d\n", devProp.maxThreadsDim[0], devProp.maxThreadsDim[1], devProp.maxThreadsDim[2]);
			printf("Max. Grid Dimensions                   : %d x %d x %d\n", devProp.maxGridSize[0], devProp.maxGridSize[1], devProp.maxGridSize[2]);

			printf("\n=========================================== GPU DEVICE DRIVER INFORMATION =========================================\n\n");
			
			printf("ECC Support                    : %s\n", devProp.ECCEnabled ? "Enabled" : "Disabled");

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			printf("CUDA Driver Mode (TCC or WDDM) : %s\n", devProp.tccDriver ? "TCC (Tesla Compute Cluster)" : "WDDM (Windows Display Driver Model)");
#endif

			printf("\n*******************************************************************************************************************\n\n");
		}
	}
}
