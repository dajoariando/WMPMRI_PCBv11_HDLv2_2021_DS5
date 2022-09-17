#include <stdint.h>

void dac5571_i2c_isr_stat(volatile unsigned int * i2c_addr, uint8_t en_mesg);
void dac5571_i2c_wr(volatile unsigned int * dac_addr, float voltp, float voltn, uint8_t en_mesg);
