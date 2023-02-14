#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <math.h>
#include <getopt.h>
#include <sched.h>
#include <hip/hip_runtime.h>

/* ---------------------------------------------------------------------------------
Macro for checking errors in HIP API calls
----------------------------------------------------------------------------------*/
#define hipErrorCheck(call)                                                                \
do{                                                                                        \
    hipError_t hipErr = call;                                                              \
    if(hipSuccess != hipErr){                                                              \
      printf("HIP Error - %s:%d: '%s'\n", __FILE__, __LINE__, hipGetErrorString(hipErr)); \
      exit(0);                                                                             \
    }                                                                                      \
}while(0)

/* ---------------------------------------------------------------------------------
Define default options
----------------------------------------------------------------------------------*/
int loop_count = 50;

/* ---------------------------------------------------------------------------------
Parse command line arguments
----------------------------------------------------------------------------------*/
void print_help(){

    printf(
        "----------------------------------------------------------------\n"
        "Usage: ./bandwidth [OPTIONS]\n\n"
        "--loop_count=<value>,  -l:       Iterations of timed bw loop\n"
        "                                 <value> should be an integer > 1\n"
        "                                 (default is 50)\n"
        "\n"
        "--help,                -h:       Show help\n"
        "----------------------------------------------------------------\n"
    );

    exit(1);
}

void process_arguments(int argc, char *argv[]){

    const char* const short_options = "l:h";

    const option long_options[] = {
        {"loop_count", optional_argument, nullptr, 'l'},
        {"help",       no_argument,       nullptr, 'h'},
        {nullptr,      no_argument,       nullptr,   0}
    };

    while(true){

        const auto opts = getopt_long(argc, argv, short_options, long_options, nullptr);

        if(-1 == opts){ break; }

        switch(opts){
            case 'l':
                loop_count = std::stoi(optarg);
                break;
            case 'h':
            default:
                print_help();
                break;
        }
    }
}


void host_device_transfer(std::string direction){

    for(int i=10; i<=27; i++){

        long int N = 1 << i;

        size_t buffer_size = N * sizeof(double);

        float milliseconds = 0.0;

        double *h_A;
        hipErrorCheck( hipHostMalloc(&h_A, buffer_size) );

        double *d_A;
        hipErrorCheck( hipMalloc(&d_A, buffer_size) );

        hipEvent_t start, stop;
        hipErrorCheck( hipEventCreate(&start) );
        hipErrorCheck( hipEventCreate(&stop) );

        for(int j=0; j<N; j++){
            h_A[j] = (double)rand()/(double)RAND_MAX;
        }

        // Warm-up loop
        if(direction == "H2D"){

            for(int iteration=0; iteration<5; iteration++){
                hipErrorCheck( hipMemcpy(d_A, h_A, buffer_size, hipMemcpyHostToDevice) );
            }

        }
        else if(direction == "D2H"){

            for(int iteration=0; iteration<5; iteration++){
                hipErrorCheck( hipMemcpy(d_A, h_A, buffer_size, hipMemcpyHostToDevice) );
            }        
        }
        else{
            std::cout << "Error! direction unknown. Exiting..." << std::endl;
            exit(1);
        }

        hipErrorCheck( hipDeviceSynchronize() );
        hipErrorCheck( hipEventRecord(start, NULL) );

        // Timed loop
        if(direction == "H2D"){

            for(int iteration=0; iteration<loop_count; iteration++){
            hipErrorCheck( hipMemcpy(d_A, h_A, buffer_size, hipMemcpyHostToDevice) );
            }
        }
        else if(direction == "D2H"){

            for(int iteration=0; iteration<loop_count; iteration++){
                hipErrorCheck( hipMemcpy(d_A, h_A, buffer_size, hipMemcpyHostToDevice) );
            }
        }
        else{
            std::cout << "Error! direction unknown. Exiting..." << std::endl;
            exit(1);
        }

        hipErrorCheck( hipEventRecord(stop, NULL) );
        hipErrorCheck( hipEventSynchronize(stop) );
        hipErrorCheck( hipEventElapsedTime(&milliseconds, start, stop) );

        double bandwidth = ( 1000.0 * (double)loop_count * (double)buffer_size ) /
                           ( (double)milliseconds * 1000.0 * 1000.0 * 1000.0);

        double buffer_size_mb = (double)buffer_size / (1024.0 * 1024.0);

        std::cout << std::fixed     << std::setprecision(4) << std::right
                  << "Buffer = "    << std::setw(10)        << buffer_size_mb << " MiB, "
                  << "Time = "      << std::setw(10)        << milliseconds   << " ms, "
                  << "Bandwidth = " << std::setw(8)         << bandwidth      << " GB/s"
                  << std::endl;
    }
}

/* ---------------------------------------------------------------------------------
Main program
----------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    process_arguments(argc, argv);

    int hwthread = sched_getcpu();

    int num_devices;
    hipErrorCheck( hipGetDeviceCount(&num_devices) );

    // If ROCR_VISIBLE_DEVICES is set, capture visible GPUs
    const char* gpu_id;
    const char* rocr_visible_devices = getenv("ROCR_VISIBLE_DEVICES");
    if(rocr_visible_devices == NULL){
        gpu_id = "N/A";
    }
    else{
        gpu_id = rocr_visible_devices;
    }

    char busid[64];

    // Measure H2D and D2H bandwidth for each GPU
    for(int device=0; device<num_devices; device++){

        hipErrorCheck( hipSetDevice(device) );
        hipErrorCheck( hipDeviceGetPCIBusId(busid, 64, device) );

        std::cout << "\n=======================================================================" << std::endl;
        printf("Running on HWT %03d and GPU %c (RT GPU ID: %01d - GPU BusID %s)\n", hwthread, gpu_id[0+2*device], device, busid);
        std::cout << "=======================================================================" << std::endl;    

        std::cout << "----- H2D -----" << std::endl;
        host_device_transfer("H2D");

        std::cout << "----- D2H -----" << std::endl;
        host_device_transfer("D2H");
    }

    std::cout << "\n__SUCCESS__\n" << std::endl;

    return 0;
}
