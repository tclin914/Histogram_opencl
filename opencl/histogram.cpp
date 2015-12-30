#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <fstream>
#include <iostream>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include <vector>

cl_program load_program(cl_context context, cl_device_id device, const char* filename) {
    FILE *fp = fopen(filename, "rt");
    size_t length;
    char *data;
    char *build_log;
    size_t ret_val_size;
    cl_program program = 0;
    cl_int status = 0;

    if(!fp) return 0;

    // get file length
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // read program source
    data = (char *)malloc(length + 1);
    fread(data, sizeof(char), length, fp);
    data[length] = '\0';

    // create and build program 
    program = clCreateProgramWithSource(context, 1, (const char **)&data, 0, 0);
    if (program == 0) return 0;

    status = clBuildProgram(program, 0, 0, 0, 0, 0);
    if (status != CL_SUCCESS) {
        printf("Error:  Building Program from file %s\n", filename);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
        build_log = (char *)malloc(ret_val_size + 1);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
        build_log[ret_val_size] = '\0';
        printf("Building Log:\n%s", build_log);
        return 0;
    }

    return program;
}

unsigned int * histogram(unsigned int *image_data, unsigned int _size) {

	unsigned int *img = image_data;
	unsigned int *ref_histogram_results;
	unsigned int *ptr;

	ref_histogram_results = (unsigned int *)malloc(256 * 3 * sizeof(unsigned int));
	ptr = ref_histogram_results;
	memset (ref_histogram_results, 0x0, 256 * 3 * sizeof(unsigned int));

	// histogram of R
	for (unsigned int i = 0; i < _size; i += 3)
	{
		unsigned int index = img[i];
		ptr[index]++;
	}

	// histogram of G
	ptr += 256;
	for (unsigned int i = 1; i < _size; i += 3)
	{
		unsigned int index = img[i];
		ptr[index]++;
	}

	// histogram of B
	ptr += 256;
	for (unsigned int i = 2; i < _size; i += 3)
	{
		unsigned int index = img[i];
		ptr[index]++;
	}

	return ref_histogram_results;
}

int main(int argc, char const *argv[])
{

	unsigned int * histogram_results;
	unsigned int i=0, a, input_size;
	std::fstream inFile("input", std::ios_base::in);
	std::ofstream outFile("0356100.out", std::ios_base::out);

	inFile >> input_size;
	unsigned int *image = new unsigned int[input_size];
	while( inFile >> a ) {
		image[i++] = a;
	}
    
	unsigned int* results = (unsigned int*)malloc(256 * 3 * sizeof(unsigned int));
	memset(results, 0, 256 * 3 * sizeof(unsigned int));
    
    // get platforms
    cl_int err;
    cl_uint num;
    err = clGetPlatformIDs(0, 0, &num);
    if (err != CL_SUCCESS) {
        std::cerr << "Unable to get platforms\n";
        exit(1);
    }

    std::vector<cl_platform_id> platforms(num);
    err = clGetPlatformIDs(num, &platforms[0], &num);
    if (err != CL_SUCCESS) {
        std::cerr << "Unable to get platform ID\n";
        exit(1);
    }

    // get context
    cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[0]), 0};
    cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);
    if (context == 0) {
        std::cerr << "Can't create OpenCL context\n";
        exit(1);
    }

    size_t cb;
    clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
    std::vector<cl_device_id> devices(cb / sizeof(cl_device_id));
    clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, &devices[0], 0);

    clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &cb);
    std::string devname;
    devname.resize(cb);
    clGetDeviceInfo(devices[0], CL_DEVICE_NAME, cb, &devname[0], 0);
    std::cout << "Device: " << devname.c_str() << "\n";

    clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, 0, NULL, &cb);
    std::vector<size_t> max_work_item_sizes(cb / sizeof(size_t));
    clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, cb, &max_work_item_sizes[0], 0);
    std::cout << "Max work item sizes " << max_work_item_sizes[0] << " " << 
            max_work_item_sizes[1] << " " <<
            max_work_item_sizes[2] << "\n";

    // commnad queue
    cl_command_queue queue = clCreateCommandQueue(context, devices[0], 0, 0);
    if (queue == 0) {
        std::cerr << "Can't create command queue\n";
        clReleaseContext(context);
        exit(1);
    }

    // memory
    cl_mem cl_image = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(cl_uint) * input_size, image, NULL);
    if (cl_image == 0) { 
        std::cerr << "Can't create image memory\n";
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        exit(1);
    }
    cl_mem cl_results = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            sizeof(cl_uint) * 256 * 3, results, NULL);
    if (cl_results == 0) {
        std::cerr << "Can't create results memory\n";
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        clReleaseMemObject(cl_results);
        exit(1);
    }

    // program
    cl_program program = load_program(context, devices[0], "histogram.cl");
    if (program == 0) {
        std::cerr << "Can't create program\n";
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        clReleaseMemObject(cl_image);
        clReleaseMemObject(cl_results);
        exit(1);
    }

    // kernel
    cl_kernel kernel = clCreateKernel(program, "histogram", 0);
    if (kernel == 0) {
        std::cerr << "Can't create kernel\n";
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        clReleaseMemObject(cl_image);
        clReleaseMemObject(cl_results);
        exit(1); 
    }

    clSetKernelArg(kernel, 0, sizeof(cl_image), &cl_image);
    clSetKernelArg(kernel, 1, sizeof(cl_results), &cl_results);

    size_t lSize = 48;
    size_t gSize = input_size / 48;
    if (input_size % lSize != 0) {
        gSize++;
    }
    std::cout << "lSize = " << lSize << " gSize =  " << gSize << "\n";
    err = clEnqueueNDRangeKernel(queue, kernel, 1, 0, &gSize, &lSize, 0, NULL, NULL);
    if (err = CL_SUCCESS) {
        std::cout << "SUCCESS\n";

    } else {
        std::cout << "FAIL\n";
    }
    
	histogram_results = histogram(image, input_size);
	for(unsigned int i = 0; i < 256 * 3; ++i) {
		if (i % 256 == 0 && i != 0)
			outFile << std::endl;
		outFile << histogram_results[i]<< ' ';
	}

	inFile.close();
	outFile.close();

	return 0;
}
