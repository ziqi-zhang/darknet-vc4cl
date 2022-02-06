#ifdef GPU

#include "darknet.h"

#include "avgpool_layer.h"
#include "opencl.h"

#include "avgpool_layer_kernels.cl"

cl_program* opencl_avgpool_layer_kernel_program;
cl_kernel* opencl_forward_avgpool_layer_kernel;
cl_kernel* opencl_backward_avgpool_layer_kernel;

void avgpool_kernel_init(void)
{
    if (opencl_device_id_t == 0) {
        opencl_avgpool_layer_kernel_program = (cl_program*)calloc(opencl_device_ct_t, sizeof(cl_program));
        opencl_forward_avgpool_layer_kernel = (cl_kernel*)calloc(opencl_device_ct_t, sizeof(cl_kernel));
        opencl_backward_avgpool_layer_kernel = (cl_kernel*)calloc(opencl_device_ct_t, sizeof(cl_kernel));
    }

    char save_path[100];
    strcpy(save_path, cl_binary_dir);
    strcat(save_path, "avgpool_layer_kernel_source.bin");
    opencl_load_buffer_cache(
        avgpool_layer_kernel_source, strlen(avgpool_layer_kernel_source), 
        &opencl_avgpool_layer_kernel_program[opencl_device_id_t],
        save_path
    );

    opencl_create_kernel(&opencl_avgpool_layer_kernel_program[opencl_device_id_t], "forward_avgpool_layer_kernel", &opencl_forward_avgpool_layer_kernel[opencl_device_id_t]);
    opencl_create_kernel(&opencl_avgpool_layer_kernel_program[opencl_device_id_t], "backward_avgpool_layer_kernel", &opencl_backward_avgpool_layer_kernel[opencl_device_id_t]);
}

void avgpool_kernel_release(void)
{
    clReleaseKernel(opencl_forward_avgpool_layer_kernel[opencl_device_id_t]);
    clReleaseKernel(opencl_backward_avgpool_layer_kernel[opencl_device_id_t]);

    clReleaseProgram(opencl_avgpool_layer_kernel_program[opencl_device_id_t]);

    opencl_forward_avgpool_layer_kernel[opencl_device_id_t] = 0;
    opencl_backward_avgpool_layer_kernel[opencl_device_id_t] = 0;
    opencl_avgpool_layer_kernel_program[opencl_device_id_t] = 0;

    if (opencl_device_id_t == opencl_device_ct_t-1) {
        free(opencl_avgpool_layer_kernel_program);
        free(opencl_forward_avgpool_layer_kernel);
        free(opencl_backward_avgpool_layer_kernel);
    }
}

void forward_avgpool_layer_gpu(avgpool_layer layer, network net)
{
    size_t n = layer.c*layer.batch;

    dim2 dimGrid;
    dimGrid = opencl_gridsize(n);

    opencl_kernel(opencl_forward_avgpool_layer_kernel[opencl_device_id_t], dimGrid, 12, &n, sizeof(cl_int), &layer.w, sizeof(cl_int), &layer.h, sizeof(cl_int), &layer.c, sizeof(cl_int), &net.input_gpu.mem, sizeof(cl_mem), &layer.output_gpu.mem, sizeof(cl_mem));
}

void backward_avgpool_layer_gpu(avgpool_layer layer, network net)
{
    size_t n = layer.c*layer.batch;

    dim2 dimGrid;
    dimGrid = opencl_gridsize(n);

    opencl_kernel(opencl_backward_avgpool_layer_kernel[opencl_device_id_t], dimGrid, 12, &n, sizeof(cl_int), &layer.w, sizeof(cl_int), &layer.h, sizeof(cl_int), &layer.c, sizeof(cl_int), &net.delta_gpu.mem, sizeof(cl_mem), &layer.delta_gpu.mem, sizeof(cl_mem));
}

#endif