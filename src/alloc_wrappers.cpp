#include <FreeRTOS.h>
#include <emalloc/emalloc.h>
#include <malloc.h>
#include <pico/malloc.h>
#include <portable.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdint>
#include "cycles.hpp"

typedef struct s_stats {
  uint32_t min;
  uint32_t max;
  uint32_t counter;
  uint64_t sum;

  void Add(uint32_t c) {
    if (c < min) {
      min = c;
    }

    if (c > max) {
      max = c;
    }

    counter++;

    sum += c;
  };
} sStats;

typedef struct s_alloc_free_stats {
  sStats alloc;
  sStats free;
} sMallocFreeStats;

static sMallocFreeStats s_emalloc;
static sMallocFreeStats s_freertos_heap4;
static sMallocFreeStats s_libc;

static sEMALLOC_ctx s_emalloc_ctx;

#define EMALLOC_MIN_MEMORY_SIZE (16)
#define EMALLOC_MAX_NUM_BLOCKS (configTOTAL_HEAP_SIZE / EMALLOC_MIN_MEMORY_SIZE)

static uint64_t s_emalloc_blocks[EMALLOC_MAX_NUM_BLOCKS];

typedef void* (*alloc_test_fn)(size_t xWantedSize);
typedef void (*free_test_fn)(void* pv);

static alloc_test_fn s_p_alloc;
static free_test_fn s_p_free;

void* alloc_test(size_t xWantedSize) {
  return (*s_p_alloc)(xWantedSize);
}

void free_test(void* pv) {
  return (*s_p_free)(pv);
}

// emalloc wrapers to be used on test profile
// This does mean nothing, and address wont be used.
// It is just used so if it returns 0 offset (which is valid on emalloc)
// it will later can be checked against null
static constexpr uint32_t kEmallocDummyOffset = 1;

void* emalloc_alloc_test(size_t xWantedSize) {
  cycles_begin();

  const uint32_t ret = emalloc_alloc(&s_emalloc_ctx, xWantedSize);

  s_emalloc.alloc.Add(cycles_end());

  if ((ret & EMALLOC_ERR_MASK) != 0) {
    return nullptr;
  }

  return reinterpret_cast<void*>(ret + kEmallocDummyOffset);
}

void emalloc_free_test(void* pv) {
  if (pv == nullptr) {
    return;
  }

  cycles_begin();
  const uint32_t offset = reinterpret_cast<uint32_t>(pv) - kEmallocDummyOffset;

  const uint32_t ret = emalloc_free(&s_emalloc_ctx, offset);

  if ((ret & ~EMALLOC_ERR_MASK) != 0) {
    printf("ERROR: emalloc_free_test 0x%08X\n", ret);

    return;
  }

  s_emalloc.free.Add(cycles_end());
}

// FreeRTOS allocs wrapers to be used on test profile
void* freertos_alloc_test(size_t xWantedSize) {
  cycles_begin();

  void* ret = pvPortMalloc(xWantedSize);

  s_freertos_heap4.alloc.Add(cycles_end());

  return ret;
}

void freertos_free_test(void* pv) {
  if (pv == nullptr) {
    return;
  }

  cycles_begin();

  vPortFree(pv);

  s_freertos_heap4.free.Add(cycles_end());
}

// libc allocs wrapers to be used on test profile
void* libc_alloc_test(size_t xWantedSize) {
  cycles_begin();

  void* ret = malloc(xWantedSize);

  s_libc.alloc.Add(cycles_end());

  return ret;
}

void libc_free_test(void* pv) {
  if (pv == nullptr) {
    return;
  }

  cycles_begin();

  free(pv);

  s_libc.free.Add(cycles_end());
}

void alloc_wrappers_switch_to_emalloc() {
  s_p_alloc = &emalloc_alloc_test;
  s_p_free = &emalloc_free_test;
}

void alloc_wrappers_switch_to_freertos() {
  s_p_alloc = &freertos_alloc_test;
  s_p_free = &freertos_free_test;
}

void alloc_wrappers_switch_to_libc() {
  s_p_alloc = &libc_alloc_test;
  s_p_free = &libc_free_test;
}

void alloc_reset() {
  memset(&s_emalloc, 0, sizeof(s_emalloc));
  memset(&s_freertos_heap4, 0, sizeof(s_freertos_heap4));
  memset(&s_libc, 0, sizeof(s_freertos_heap4));

  s_emalloc.alloc.min = 0xFFFFFFFF;
  s_emalloc.free.min = 0xFFFFFFFF;

  s_freertos_heap4.alloc.min = 0xFFFFFFFF;
  s_freertos_heap4.free.min = 0xFFFFFFFF;

  s_libc.alloc.min = 0xFFFFFFFF;
  s_libc.free.min = 0xFFFFFFFF;

  sEMALLOC_cfg emalloc_cfg;
  emalloc_cfg.nodes_poll = s_emalloc_blocks;
  emalloc_cfg.nodes_poll_length = EMALLOC_MAX_NUM_BLOCKS;
  emalloc_cfg.external_memory_size_bytes = configTOTAL_HEAP_SIZE;

  emalloc_init(&s_emalloc_ctx, &emalloc_cfg);
}

void alloc_report_stats() {
  printf("emalloc.alloc ticks min=%u avg=%.1f max=%u (n=%zu)\n",
         s_emalloc.alloc.min,
         s_emalloc.alloc.sum / (double)s_emalloc.alloc.counter,
         s_emalloc.alloc.max, s_emalloc.alloc.counter);

  printf("emalloc.free ticks min=%u avg=%.1f max=%u (n=%zu)\n",
         s_emalloc.free.min,
         s_emalloc.free.sum / (double)s_emalloc.free.counter,
         s_emalloc.free.max, s_emalloc.free.counter);

  printf("freertos.alloc ticks min=%u avg=%.1f max=%u (n=%zu)\n",
         s_freertos_heap4.alloc.min,
         s_freertos_heap4.alloc.sum / (double)s_freertos_heap4.alloc.counter,
         s_freertos_heap4.alloc.max, s_freertos_heap4.alloc.counter);

  printf("freertos.free ticks min=%u avg=%.1f max=%u (n=%zu)\n",
         s_freertos_heap4.free.min,
         s_freertos_heap4.free.sum / (double)s_freertos_heap4.free.counter,
         s_freertos_heap4.free.max, s_freertos_heap4.free.counter);

  printf("libc.alloc ticks min=%u avg=%.1f max=%u (n=%zu)\n", s_libc.alloc.min,
         s_libc.alloc.sum / (double)s_libc.alloc.counter, s_libc.alloc.max,
         s_libc.alloc.counter);

  printf("libc.free ticks min=%u avg=%.1f max=%u (n=%zu)\n", s_libc.free.min,
         s_libc.free.sum / (double)s_libc.free.counter, s_libc.free.max,
         s_libc.free.counter);

  printf("\n");
}
