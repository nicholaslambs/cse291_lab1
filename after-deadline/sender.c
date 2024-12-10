
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

#define CACHE_HIT_THRESHOLD 45
// [Bonus] TODO: define your own buffer size
#define BUFF_SIZE L1_SIZE

int main(int argc, char **argv){
    // Allocate a buffer using huge page
    // See the handout for details about hugepage management
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

    // [Bonus] TODO:
    // Put your covert channel setup code here

    printf("Please enter a value to send (0-255):\n");

    int value;
    if (scanf("%d", &value) != 1) {
        fprintf(stderr, "Failed to read input.\n");
        exit(EXIT_FAILURE);
    }

    if (value < 0 || value > 255) {
        printf("Invalid input. Please enter a value between 0 and 255.\n");
        exit(EXIT_FAILURE);
    }

    // Access the line in memory to bring it into the cache
    asm volatile(
        "mov (%0), %%eax\n"
        :
        : "r"(buf + L1_SIZE + value * LINE_SIZE)
        : "%eax"
    );

    printf("Sender finished.\n");
    return 0;
}