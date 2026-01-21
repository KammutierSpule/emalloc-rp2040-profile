#include <hardware/clocks.h>
#include <hardware/structs/clocks.h>
#include <pico/stdlib.h>
#include <pico/types.h>
#include <cstdint>
#include <iostream>
#include "cycles.hpp"

void RunAllTests();

int main() {
  stdio_init_all();

  while (true) {
    for (uint8_t i = 0; i < 10; i++) {
      printf("About to restart testing ");
      printf("%u\n", 10 - i);
      sleep_ms(1000);
    }

    cycles_begin();
    const uint32_t empty_number_of_cycles = cycles_end();

    printf("empty number of cycles:%u\n", empty_number_of_cycles);

    uint32_t f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);

    printf("f_clk_sys:%u khz\n", f_clk_sys);

    RunAllTests();
  }
}
