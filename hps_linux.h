#ifndef HPS_LINUX_H_
#define HPS_LINUX_H_

#include <stdio.h>
#include <hwlib.h>
#include <socal/socal.h>
#include <stdlib.h>
#include <unistd.h>

#include "functions/adc_ad9276_driver.h"
#include "functions/common_functions.h"
#include "functions/gnrl_calc.h"
#include "functions/pll_param_generator.h"
#include "functions/reconfig_functions.h"
#include "functions/soc_global.h"
#include "variables/adc_ad9276_vars.h"
#include "variables/general.h"

// SOC HPS ADDRESSES (COPY TO THE SOC_GLOBAL.H)
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
volatile unsigned int *h2p_general_cnt_int_addr = NULL;
volatile unsigned int *h2p_general_cnt_out_addr = NULL;
volatile unsigned int *h2p_adc_start_pulselength_addr = NULL;
void *h2p_pulse_adc_reconfig = NULL;

// FUNCTIONS
void leave();	// terminate the program
void init();	// initialize the system with tuned default parameter

// global variables
FILE *fptr;
long i;
long j;
char foldername[50];   // variable to store folder name of the measurement data
char pathname[60];
unsigned int cnt_out_val;

#endif
