#include "utility.h"

#ifndef VISUAL
#define PRINT_FUNC print_results_plaintext
#else
#define PRINT_FUNC print_results_for_visualization
#endif

#define LINE_SIZE 64
#define L1_SIZE 32768
#define L1_ASSOCIATIVITY 8 
#define L2_SIZE 262144
#define L2_ASSOCIATIVITY 4
#define L3_SIZE 6291456
#define L3_ASSOCIATIVITY 12
#define BUFF_SIZE (L3_SIZE * 2)
 
#define NUM_L1_SETS (L1_SIZE / (LINE_SIZE * L1_ASSOCIATIVITY)) 
#define NUM_L2_SETS (L2_SIZE / (LINE_SIZE * L2_ASSOCIATIVITY))
#define NUM_L3_SETS (L3_SIZE / (LINE_SIZE * L3_ASSOCIATIVITY))

int main (int ac, char **av) {

    // create 4 arrays to store the latency numbers
    // the arrays are initialized to 0
    uint64_t dram_latency[SAMPLES] = {0};
    uint64_t l1_latency[SAMPLES] = {0};
    uint64_t l2_latency[SAMPLES] = {0};
    uint64_t l3_latency[SAMPLES] = {0};

    // A temporary variable we can use to load addresses
    uint8_t tmp;

    /*
    Used as the target address for measuring cache and memory access latencies
    Allocate a buffer of LINE_SIZE Bytes
    */
    // The volatile keyword tells the compiler to not put this variable into a
    // register -- it should always try to be loaded from memory / cache.
    volatile uint8_t *target_buffer = (uint8_t *)malloc(LINE_SIZE);

    if (NULL == target_buffer) {
        perror("Unable to malloc");
        return EXIT_FAILURE;
    }

    volatile uint8_t *eviction_buffer = (uint8_t *)malloc(BUFF_SIZE);


    uint64_t start, end, latency;

    // Example: Measure L1 access latency, store results in l1_latency array
    for (int i=0; i<SAMPLES; i++){
        tmp = target_buffer[0];
        lfence();
        l1_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // Measure DRAM Latency
    for (int i = 0; i < SAMPLES; i++) {
        clflush((void *)target_buffer);
        lfence();
        dram_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // Measure L2 Latency
    for (int i = 0; i < SAMPLES; i++) {
        tmp = target_buffer[0];
        lfence();
        for (int j = 0; j < NUM_L2_SETS * L2_ASSOCIATIVITY * 2; j++) {
            tmp += eviction_buffer[j * LINE_SIZE];
            lfence();
        }
        lfence();
        l2_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // Measure L3 Latency
    for (int i = 0; i < SAMPLES; i++) {
        tmp = target_buffer[0];
        lfence();
        for (int j = 0; j < NUM_L3_SETS * L3_ASSOCIATIVITY * 1.5; j++) {
            tmp += eviction_buffer[j * LINE_SIZE];
            lfence();
        }
        lfence();
        l3_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }


    // Print the results to the screen
    // When compile to main and used by `make run`,
    // it uses print_results_plaintext
    // When compile to main-visual and used by `run.py`,
    // it uses print_results_for_visualization
    PRINT_FUNC(dram_latency, l1_latency, l2_latency, l3_latency);

    free((uint8_t *)target_buffer);

    // [1.2] TODO: Uncomment this line once you uncomment the eviction_buffer
    //             creation line
    free((uint8_t *)eviction_buffer);
    return 0;
}

