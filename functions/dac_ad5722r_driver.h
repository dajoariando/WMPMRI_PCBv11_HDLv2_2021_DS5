#include <stdint.h>

void init_dac_ad5722r(volatile unsigned int * dac_addr, unsigned int dac_range, unsigned int DAC_CLR_msk);
void print_warning_ad5722r(volatile unsigned int * dac_addr, uint8_t en_mesg);
void wr_dac_ad5722r(volatile unsigned int * dac_addr, unsigned int dac_range, unsigned int dac_id, double volt, unsigned int DAC_LDAC_msk, uint8_t en_mesg);
