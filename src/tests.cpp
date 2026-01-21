#include <FreeRTOS.h>
#include <malloc.h>
#include <pico/stdlib.h>
#include <stdlib.h>
#include <array>
#include <cstddef>
#include <cstdio>

static constexpr uint32_t kMinAllocationSize = 16;

// This is just some arbitary N based on FreeRTOS configTOTAL_HEAP_SIZE.
// Do not take this as an example.
static constexpr uint32_t kMaxNodesAllocated =
    (configTOTAL_HEAP_SIZE / 2) / kMinAllocationSize;

// Use fixed seed for reproducibility
static uint32_t s_random_seed = 42;

void alloc_report_stats();
void alloc_reset();
void alloc_wrappers_switch_to_emalloc();
void alloc_wrappers_switch_to_freertos();
void alloc_wrappers_switch_to_libc();
void* alloc_test(size_t xWantedSize);
void free_test(void* pv);

static std::array<void*, kMaxNodesAllocated> s_allocation_table;
static size_t s_allocated;

static void allocation_table_clear() {
  s_allocated = 0;
  std::fill(s_allocation_table.begin(), s_allocation_table.end(), nullptr);
}

static void free_all() {
  for (uint32_t i = 0; i < s_allocation_table.size(); i++) {
    if (s_allocation_table[i] != nullptr) {
      free_test(s_allocation_table[i]);
      s_allocation_table[i] = nullptr;
    }
  }

  s_allocated = 0;
}

// Test function pointer type
typedef void (*TestFunction)();

// Test entry structure
struct TestEntry {
  const char* name;
  TestFunction function;
};

#define CHECK_NOT_NULL_OR_RETURN(a)  \
  if (a == nullptr) {                \
    printf("ERROR: Alloc failed\n"); \
    return;                          \
  }

#define CHECK_NOT_NULL_OR_BREAK(a) \
  if (a == nullptr) {              \
    break;                         \
  }

static void alloc_all_16b() {
  for (uint32_t i = 0; i < kMaxNodesAllocated; i++) {
    void* address = alloc_test(kMinAllocationSize);

    CHECK_NOT_NULL_OR_RETURN(address);

    s_allocation_table[i] = address;
    s_allocated++;
  }
}

void AllocAll16FreeOdd() {
  alloc_all_16b();

  for (uint32_t i = 1; i < s_allocation_table.size(); i += 2) {
    free_test(s_allocation_table[i]);
    s_allocation_table[i] = nullptr;
    s_allocated--;
  }
}

void AllocAll16FreeEven() {
  alloc_all_16b();

  for (uint32_t i = 0; i < (kMaxNodesAllocated - 1); i += 2) {
    free_test(s_allocation_table[i]);
    s_allocation_table[i] = nullptr;
    s_allocated--;
  }
}

void AllocAll16FreeOddReversed() {
  alloc_all_16b();

  for (uint32_t i = 1; i < kMaxNodesAllocated; i += 2) {
    free_test(s_allocation_table[kMaxNodesAllocated - i]);
    s_allocation_table[i] = nullptr;
    s_allocated--;
  }
}

void AllocAll16FreeEvenReversed() {
  alloc_all_16b();

  for (uint32_t i = 0; i < (kMaxNodesAllocated - 1); i += 2) {
    free_test(s_allocation_table[(kMaxNodesAllocated - 2) - i]);
    s_allocation_table[i] = nullptr;
    s_allocated--;
  }
}

static void alloc_add_to_first_empty(void* a_address) {
  for (uint32_t i = 0; i < s_allocation_table.size(); i++) {
    if (s_allocation_table[i] == nullptr) {
      s_allocation_table[i] = a_address;
      s_allocated++;

      return;
    }
  }
}

static bool alloc_rand(size_t a_n_elements, size_t a_max_alloc_size) {
  for (int i = 0; i < a_n_elements; i++) {

    if (s_allocated >= kMaxNodesAllocated) {
      return false;  // Until memory exaustion
    }

    const uint32_t size =
        8 + (rand_r(reinterpret_cast<unsigned int*>(&s_random_seed)) %
             (a_max_alloc_size));
    void* address = alloc_test(size);

    if (address != nullptr) {
      alloc_add_to_first_empty(address);
    } else {
      return false;  // Until memory exaustion
    }
  }

  return true;
}

static void free_rand(size_t a_n_elements) {
  // Free phase - free random allocations
  for (int i = 0; i < a_n_elements; i++) {
    if (s_allocated == 0) {
      return;
    }

    // Compact the array by moving all non-null elements to the left
    size_t write_idx = 0;
    for (size_t read_idx = 0; read_idx < s_allocation_table.size();
         read_idx++) {
      if (s_allocation_table[read_idx] != nullptr) {
        s_allocation_table[write_idx] = s_allocation_table[read_idx];
        if (write_idx != read_idx) {
          s_allocation_table[read_idx] = nullptr;
        }
        write_idx++;
      }
    }
    if (write_idx != s_allocated) {
      printf("ERROR: write_idx(%u) != s_allocated(%u)\n", write_idx,
             s_allocated);
      return;
    }

    size_t idx =
        rand_r(reinterpret_cast<unsigned int*>(&s_random_seed)) % s_allocated;

    free_test(s_allocation_table[idx]);
    s_allocation_table[idx] = nullptr;
    s_allocated--;
  }
}

// Alloc 5, release 3, 8 + 16 bytes
void Alloc5Release3_16b() {
  while (true) {
    if (alloc_rand(5, 16) == false) {
      return;
    }

    free_rand(3);
  }
}

// Alloc 5, release 3, 8 + 64 bytes
void Alloc5Release3_64b() {
  while (true) {
    if (alloc_rand(5, 64) == false) {
      return;
    }

    free_rand(3);
  }
}

// Alloc 5, release 3, 8 + 128 bytes
void Alloc5Release3_128b() {
  while (true) {
    if (alloc_rand(5, 128) == false) {
      return;
    }

    free_rand(3);
  }
}

// Alloc 5, release 3, 8 + 256 bytes
void Alloc5Release3_256b() {
  while (true) {
    if (alloc_rand(5, 256) == false) {
      return;
    }

    free_rand(3);
  }
}

// Alloc 5, release 3, 8 + 512 bytes
void Alloc5Release3_512b() {
  while (true) {
    if (alloc_rand(5, 512) == false) {
      return;
    }

    free_rand(3);
  }
}

// Alloc 6, release 2, 8 + 16 bytes
void Alloc6Release2_16b() {
  while (true) {
    if (alloc_rand(6, 16) == false) {
      return;
    }

    free_rand(2);
  }
}

// Alloc 6, release 2, 8 + 32 bytes
void Alloc6Release2_32b() {
  while (true) {
    if (alloc_rand(6, 32) == false) {
      return;
    }

    free_rand(2);
  }
}

// Alloc 6, release 2, 8 + 64 bytes
void Alloc6Release2_64b() {
  while (true) {
    if (alloc_rand(6, 64) == false) {
      return;
    }

    free_rand(2);
  }
}

// Alloc 6, release 2, 8 + 128 bytes
void Alloc6Release2_128b() {
  while (true) {
    if (alloc_rand(6, 128) == false) {
      return;
    }

    free_rand(2);
  }
}

// Alloc 6, release 2, 8 + 256 bytes
void Alloc6Release2_256b() {
  while (true) {
    if (alloc_rand(6, 256) == false) {
      return;
    }

    free_rand(2);
  }
}

// Alloc 6, release 2, 8 + 512 bytes
void Alloc6Release2_512b() {
  while (true) {
    if (alloc_rand(6, 512) == false) {
      return;
    }

    free_rand(2);
  }
}

// Static test list
static TestEntry s_test_list[] = {
    {"AllocAll16FreeOdd", AllocAll16FreeOdd},
    {"AllocAll16FreeEven", AllocAll16FreeEven},
    {"AllocAll16FreeOddReversed", AllocAll16FreeOddReversed},
    {"AllocAll16FreeEvenReversed", AllocAll16FreeEvenReversed},
    {"Alloc5Release3_16b", Alloc5Release3_16b},
    {"Alloc5Release3_64b", Alloc5Release3_64b},
    {"Alloc5Release3_128b", Alloc5Release3_128b},
    {"Alloc5Release3_256b", Alloc5Release3_256b},
    {"Alloc5Release3_512b", Alloc5Release3_512b},
    {"Alloc6Release2_16b", Alloc6Release2_16b},
    {"Alloc6Release2_32b", Alloc6Release2_32b},
    {"Alloc6Release2_64b", Alloc6Release2_64b},
    {"Alloc6Release2_128b", Alloc6Release2_128b},
    {"Alloc6Release2_256b", Alloc6Release2_256b},
    {"Alloc6Release2_512b", Alloc6Release2_512b},
};

// Get the number of tests
static constexpr size_t s_test_count =
    sizeof(s_test_list) / sizeof(s_test_list[0]);

// Function to run all tests
void RunAllTests() {
  printf("Running %zu tests...\n", s_test_count);

  allocation_table_clear();

  for (size_t i = 0; i < s_test_count; i++) {
    printf("[%zu/%zu] Running test: %s\n", i + 1, s_test_count,
           s_test_list[i].name);

    sleep_ms(100);

    alloc_reset();

    alloc_wrappers_switch_to_emalloc();
    s_test_list[i].function();
    free_all();

    alloc_wrappers_switch_to_freertos();
    s_test_list[i].function();
    free_all();

    struct mallinfo m = mallinfo();
    printf("libc heap: free chunk=%u, free fastbin=%u, free=%u alloc=%u\n",
           m.ordblks, m.smblks, m.fordblks, m.uordblks);

    alloc_wrappers_switch_to_libc();
    s_test_list[i].function();
    free_all();

    alloc_report_stats();
  }

  printf("\nAll tests completed: %zu/%zu\n", s_test_count, s_test_count);
}
