#include <stdint.h>

void init_dac_ad5724r(volatile unsigned int * dac_addr, unsigned int DAC_CLR);
void print_warning_ad5724r(volatile unsigned int * dac_addr, uint8_t en_mesg);
void wr_dac_ad5724r(volatile unsigned int * dac_addr, unsigned int dac_id, double volt, unsigned int DAC_LDAC_msk, uint8_t en_mesg);
