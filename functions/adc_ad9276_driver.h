#ifndef AD9276_DRIVER_H_
#define AD9276_DRIVER_H_

#include <stdint.h>

// functions
unsigned int write_adc_spi(unsigned int comm);
unsigned int write_ad9276_spi(unsigned char rw, unsigned int addr, unsigned int val);
void init_adc(uint8_t lvds_z, uint8_t lvds_phase, uint32_t adc_mode, uint16_t val1, uint16_t val2);
void read_adc_id();
void adc_wr_testval(uint16_t val1, uint16_t val2);
void read_adc_fifo(volatile unsigned int *channel_csr_addr, void *channel_data_addr, unsigned int * adc_data, unsigned char en_mesg);
void read_adc_dma(volatile unsigned int * dma_addr, volatile unsigned int * sdram_addr, uint32_t DMA_SRC_BASE, uint32_t DMA_DEST_BASE, uint32_t* rddata, uint32_t transfer_length, uint8_t en_mesg);
unsigned int flush_adc_fifo(volatile unsigned int *channel_csr_addr, void *channel_data_addr, unsigned char en_mesg);

#endif
