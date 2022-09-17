// This algorithm is developed to bridge between low level reconfig_function.h and pll_calculator.h

#include <stdint.h>

#define INPUT_FREQ 50 // 50MHz

void Set_M(volatile unsigned int *addr, uint32_t * pll_param, uint32_t enable_message);
void Set_N(volatile unsigned int *addr, uint32_t * pll_param, uint32_t enable_message);
void Set_C(volatile unsigned int *addr, uint32_t * pll_param, uint32_t counter_select, double duty_cycle, uint32_t enable_message);
void Set_DPS(volatile unsigned int *addr, uint32_t counter_select, uint32_t phase, uint32_t enable_message);   // phase is 0 to 360
void Set_MFrac(volatile unsigned int *addr, uint32_t * pll_param, uint32_t enable_message);
void Set_PLL(volatile unsigned int *addr, uint32_t counter_select, double out_freq, double duty_cycle, uint32_t enable_message);
