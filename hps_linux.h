#ifndef HPS_LINUX_H_
#define HPS_LINUX_H_

#include <stdio.h>
#include <hwlib.h>
#include <socal/socal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "functions/adc_ad9276_driver.h"
// #include "functions/dac_ad5724r_driver.h" // careful on having both ad5724 and ad5722 driver
#include "functions/dac_ad5722r_driver.h"
#include "functions/common_functions.h"
#include "functions/gnrl_calc.h"
#include "functions/pll_param_generator.h"
#include "functions/reconfig_functions.h"
#include "functions/soc_global.h"
#include "functions/bstream.h"

#include "variables/adc_ad9276_vars.h"
// #include "variables/dac_ad5724r_vars.h"
#include "variables/dac_ad5722r_vars.h"
#include "variables/general.h"

#define NCO_PH_RES 8 // the resolution for the NCO phase. Check it at the NCO platform designer
#define NCO_AMP_RES 16 // the resolution for the NCO output amplitude. Check it at the NCO platform designer

// SOC HPS ADDRESSES (COPY TO THE SOC_GLOBAL.C)
void *h2p_fifo_sink_ch_a_data_addr = NULL;
void *h2p_fifo_sink_ch_b_data_addr = NULL;
void *h2p_fifo_sink_ch_c_data_addr = NULL;
void *h2p_fifo_sink_ch_d_data_addr = NULL;
void *h2p_fifo_sink_ch_e_data_addr = NULL;
void *h2p_fifo_sink_ch_f_data_addr = NULL;
void *h2p_fifo_sink_ch_g_data_addr = NULL;
void *h2p_fifo_sink_ch_h_data_addr = NULL;
void *h2p_led_addr = NULL;
void *h2p_sw_addr = NULL;
void *h2p_button_addr = NULL;

// nco
volatile unsigned int *h2p_ph_overlap_addr = NULL;   // the nco phase overlap address
volatile unsigned int *h2p_ph_inc_addr = NULL;   // the nco phase increment address
volatile unsigned int *h2p_ph_0_to_3_addr = NULL;   // the nco phase modulator
volatile unsigned int *h2p_ph_4_to_7_addr = NULL;   // the nco phase modulator

volatile unsigned int *h2p_adcspi_addr;   // gpio for dac (spi)
volatile unsigned int *h2p_fifo_sink_ch_a_csr_addr = NULL;   // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_b_csr_addr = NULL;   // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_c_csr_addr = NULL;   // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_d_csr_addr = NULL;   // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_e_csr_addr = NULL;   // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_f_csr_addr = NULL;   // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_g_csr_addr = NULL;   // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_h_csr_addr = NULL;   // ADC streaming FIFO status address
volatile unsigned int *h2p_adc_samples_addr = NULL;
volatile unsigned int *h2p_init_delay_addr = NULL;
volatile unsigned int *h2p_general_cnt_in_addr = NULL;
volatile unsigned int *h2p_general_cnt_out_addr = NULL;

// volatile unsigned int *h2p_adc_start_pulselength_addr = NULL;
// void *h2p_pulse_adc_reconfig = NULL;
volatile unsigned int *h2p_dac_grad_spi_addr = NULL;

// memory map peripherals for bitstream codes. Also connect the bitstream object and ram in function bstream__init_all_sram() inside bstream.c
volatile unsigned int *axi_ram_tx_h1 = NULL;
volatile unsigned int *axi_ram_tx_l1 = NULL;
volatile unsigned int *axi_ram_tx_aux = NULL;
volatile unsigned int *axi_ram_tx_h2 = NULL;
volatile unsigned int *axi_ram_tx_l2 = NULL;
volatile unsigned int *axi_ram_tx_charge = NULL;
volatile unsigned int *axi_ram_tx_charge_bs = NULL;
// volatile unsigned int *axi_ram_tx_damp = NULL;
volatile unsigned int *axi_ram_tx_dump = NULL;
volatile unsigned int *axi_ram_rx_adc_en = NULL;
volatile unsigned int *axi_ram_rx_inc_damp = NULL;
volatile unsigned int *axi_ram_rx_in_short = NULL;

// pll reconfig address for the system clock
volatile unsigned int *h2p_sys_pll_reconfig_addr = NULL;   // bitstream pll reconfig

// preamp
volatile unsigned int *h2p_dac_preamp_addr = NULL;

// FUNCTIONS
void leave();   // terminate the program
void init();	// initialize the system with tuned default parameter

// global variables
FILE *fptr;
long i;
long j;
char acq_file[60];
unsigned int cnt_out_val;

#endif
