#ifndef AD9276_DRIVER_H_
#define AD9276_DRIVER_H_

#include <stdint.h>

// functions
unsigned int write_adc_spi(unsigned int comm);
unsigned int write_ad9276_spi(unsigned char rw, unsigned int addr, unsigned int val);
void init_adc(uint8_t lvds_z, uint8_t lvds_phase, uint32_t adc_mode, uint16_t val1, uint16_t val2);
void read_adc_id();
void adc_wr_testval(uint16_t val1, uint16_t val2);
void read_adc_val(volatile unsigned int *channel_csr_addr, void *channel_data_addr, unsigned int * adc_data);

#endif
