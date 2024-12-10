#include "util.h"
#include <sys/mman.h>
#include <unistd.h>    // For sleep()

#define LINE_SIZE 64
#define L1_SIZE 32768
#define L1_ASSOCIATIVITY 8 
#define L2_SIZE 262144
#define L2_ASSOCIATIVITY 4
#define L3_SIZE 6291456
#define L3_ASSOCIATIVITY 12

#define NUM_L1_SETS (L1_SIZE / (LINE_SIZE * L1_ASSOCIATIVITY)) 
#define NUM_L2_SETS (L2_SIZE / (LINE_SIZE * L2_ASSOCIATIVITY))
#define NUM_L3_SETS (L3_SIZE / (LINE_SIZE * L3_ASSOCIATIVITY))

#define NUM_VALUES 256
#define ARRAY_SIZE ((NUM_VALUES + 1) * L1_ASSOCIATIVITY * (LINE_SIZE / sizeof(int)))

int array[ARRAY_SIZE];

#define CACHE_HIT_THRESHOLD 50

int main(int argc, char **argv) {

    printf("Please press enter.\n");
    getchar();
    printf("Receiver now listening.\n");

    // Prime the cache by accessing array elements to load them into the L1 cache
    for (int l2_set = 0; l2_set < NUM_VALUES; ++l2_set) {
        for (int l1_way = 0; l1_way < L1_ASSOCIATIVITY; ++l1_way) {
            int index = (l2_set * L1_ASSOCIATIVITY + l1_way) * (LINE_SIZE / sizeof(int));

            asm volatile(
                "movl (%0), %%eax\n"
                :
                : "r" (&array[index])
                : "%eax"
            );
        }
    }

    sleep(5);

    // Probe the array to measure access time for covert channel
    for (int i = 0; i < NUM_VALUES; i++) {
        // probe array access time starting beyond L1 cache
        CYCLES time = measure_one_block_access_time((uint64_t)(&array[i * LINE_SIZE]));
        if (time > CACHE_HIT_THRESHOLD) {
            printf("Recieved: %d\n", i);
        }
    }
    
    printf("Receiver finished.\n");

    return 0;
}
