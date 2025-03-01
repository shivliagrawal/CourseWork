#include <stdio.h>

#define MIN_ALLOCATION 16

// The round_up_size function to test
static size_t round_up_size(size_t size) {
    return ((size + MIN_ALLOCATION - 1) / MIN_ALLOCATION) * MIN_ALLOCATION;
}

int main() {
    // Test cases
    size_t test_sizes[] = {1, 15, 16, 17, 31, 32, 33, 48, 64, 100, 128, 255, 256};
    size_t expected_results[] = {16, 16, 16, 32, 32, 32, 48, 48, 64, 112, 128, 256, 256};

    for (int i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); i++) {
        size_t result = round_up_size(test_sizes[i]);
        printf("Input: %zu, Rounded size: %zu, Expected: %zu\n", test_sizes[i], result, expected_results[i]);
        if (result != expected_results[i]) {
            printf("Error: Mismatch for input %zu! Expected %zu but got %zu\n", test_sizes[i], expected_results[i], result);
        }
    }

    return 0;
}

