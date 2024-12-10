
#include"util.h"
// mman library to be used for hugepage allocations (e.g. mmap or posix_memalign only)
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
#define BUFF_SIZE (L1_SIZE + NUM_VALUES * L2_ASSOCIATIVITY * LINE_SIZE)

#define CACHE_HIT_THRESHOLD 100

int main(int argc, char **argv) {

    printf("Please press enter.\n");
    getchar();
    printf("Receiver now listening.\n");

    // [Bonus] TODO: prime cache
    volatile uint8_t *buf= mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE, MAP_POPULATE |
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB, -1, 0);
    
    if (buf == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }

    // The first access to a page triggers overhead associated with
    // page allocation, TLB insertion, etc.
    // Thus, we use a dummy write here to trigger page allocation
    // so later access will not suffer from such overhead.
    *((char *)buf) = 1; // dummy write to trigger page allocation

    for (size_t i = 0; i < BUFF_SIZE; i += LINE_SIZE) {
        asm volatile(
            "mov (%0), %%eax\n"
            :
            : "r"(buf + i)
            : "%eax"
        );
    }

    sleep(5);

    // [Bonus] TODO: Put your covert channel code here
    for (int i = 0; i < NUM_VALUES; i++) {
        CYCLES time = measure_one_block_access_time((uint64_t)(buf + L1_SIZE + i * LINE_SIZE)); // probe
        if (time > CACHE_HIT_THRESHOLD) {
            // print address found
            printf("Address found: %p\n", buf + L1_SIZE + i * LINE_SIZE);
            printf("Received: %d with time %d\n", i, time);
            printf("\n");
            //break;
        }
    }
    
    printf("Receiver finished.\n");

    return 0;
}