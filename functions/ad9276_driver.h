#ifndef AD9276_DRIVER_H_
#define AD9276_DRIVER_H_

#include <socal/socal.h>
#include <unistd.h>
#include <socal/hps.h>
#include <stdio.h>

#include "../soc_variables/hps_soc_system.h"
#include "./avalon_spi.h"
#include "../soc_variables/ad9276_vars.h"
#include "./AlteraIP/altera_avalon_fifo_regs.h"

// axi bus address
extern void * h2f_lw_axi_master;

// offset for axi bus address
extern volatile unsigned int *h2p_adcspi_addr;   // gpio for dac (spi)

// general variables
extern long i;

// functions
unsigned int write_adc_spi(unsigned int comm);
unsigned int write_ad9276_spi(unsigned char rw, unsigned int addr, unsigned int val);
void init_adc(uint8_t lvds_z, uint8_t lvds_phase);
void read_adc_id();
void adc_wr_testval(uint16_t val1, uint16_t val2);
void read_adc_val(volatile unsigned int *channel_csr_addr, void *channel_data_addr, unsigned int * adc_data);

#endif
