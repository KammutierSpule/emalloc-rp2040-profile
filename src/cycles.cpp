#include "cycles.hpp"
#include <hardware/structs/systick.h>
#include <pico/sync.h>

// SysTick CSR bit definitions (ARMv6-M)
#define SYST_CSR_ENABLE (1u << 0)     // Counter enable
#define SYST_CSR_CLKSOURCE (1u << 2)  // 0: external ref clock, 1: CPU (clk_sys)
#define SYST_CSR_COUNTFLAG \
  (1u << 16)  // Returns 1 if timer counted to 0 since last time this was read

#define SYSTICK_MAX_RELOAD 0x00FFFFFFu  // 24-bit

static critical_section_t s_critical_section_ctx;

void cycles_begin() {
  if (critical_section_is_initialized(&s_critical_section_ctx) == false) {
    critical_section_init(&s_critical_section_ctx);
  }

  critical_section_enter_blocking(&s_critical_section_ctx);

  // Stop counter and clear settings
  systick_hw->csr = 0;

  // Set maximum reload and clear current value
  systick_hw->rvr = SYSTICK_MAX_RELOAD;
  systick_hw->cvr = 0;

  // Clock from CPU, enable counter, no interrupt
  systick_hw->csr = SYST_CSR_CLKSOURCE | SYST_CSR_ENABLE;
}

uint32_t cycles_end() {
  // Read remaining counts (down-counter)
  const uint32_t remaining = systick_hw->cvr;
  const bool is_overflow = (systick_hw->csr & SYST_CSR_COUNTFLAG) != 0;

  // Stop counter
  systick_hw->csr = 0;

  critical_section_exit(&s_critical_section_ctx);
  // critical_section_deinit(&s_critical_section_ctx);

  // Elapsed cycles = reload - remaining
  const uint32_t elapsed_ticks =
      (SYSTICK_MAX_RELOAD - remaining) + (is_overflow ? SYSTICK_MAX_RELOAD : 0);

  return elapsed_ticks;
}
