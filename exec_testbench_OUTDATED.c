// Created on: August 6th, 2024
// Author: David Ariando
// This is the basic CPMG sequence but taking data from multiple channel ADC

// #define EXEC_ADC_TB
#ifdef EXEC_ADC_TB

#define GET_RAW_DATA
#define GET_CPMG_PARAMS

#include "hps_linux.h"

// bitstream objects
bstream_obj bstream_objs[BSTREAM_COUNT];

void init() {
	printf("START:: EXEC_TESTBENCH\n");

	soc_init();
	bstream__init_all_sram();

	// set polling mode for the main PLL
	Reconfig_Mode(h2p_sys_pll_reconfig_addr, 1);

	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;

	// turn off the ADC (sometimes the ADC is in undefined state during startup and failed to start without turned off first)
	cnt_out_val |= ADC_AD9276_STBY_msk;
	cnt_out_val |= ADC_AD9276_PWDN_msk;// (CAREFUL! SOMETIMES THE ADC CANNOT WAKE UP AFTER PUT TO PWDN)
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	// turn on the ADC
	cnt_out_val &= ~ADC_AD9276_STBY_msk;// turn on the ADC
	cnt_out_val &= ~ADC_AD9276_PWDN_msk;// turn on the ADC
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	// init the DAC
	init_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_PAMP_CLR);
	usleep(100);

}

void leave() {

	// turn off the ADC
	cnt_out_val |= ADC_AD9276_STBY_msk;
	cnt_out_val |= ADC_AD9276_PWDN_msk;// (CAREFUL! SOMETIMES THE ADC CANNOT WAKE UP AFTER PUT TO PWDN)
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	soc_exit();

	printf("STOP:: EXEC_TESTBENCH\n");
}

int main(int argc, char * argv[]) {

	// param defined by user
	double f_larmor = atof(argv[1]);
	double bstrap_pchg_us = atof(argv[2]);// bootstrap circuit precharge by enabling both lower side FETs. Has to be done at the beginning to make sure the high-side circuit is charged. This takes a long time (around 2ms).
	// --- vpc precharging ---
	double lcs_vpc_pchg_us = atof(argv[3]);// precharging the lcs using VDD
	double lcs_recycledump_us = atof(argv[4]);// recycle the energy in lcs to VPC
	double lcs_vpc_pchg_repeat = atof(argv[5]);// repeat VPC precharging for n times
	// --- vpc discharging ---
	double lcs_vpc_dchg_us = atof(argv[6]);// discharging the VPC into lcs
	double lcs_wastedump_us = atof(argv[7]);// waste/dump the lcs energy into the protection diode
	double lcs_vpc_dchg_repeat = atof(argv[8]);// repeat VPC precharging for n times
	// enable lcs initial precharging and discharging
	char en_lcs_pchg = atoi(argv[9]);// enable the vpc precharging via lcs prior to cpmg
	char en_lcs_dchg = atoi(argv[10]);// enable the vpc discharging via lcs post cpmg


	// param defined by Quartus
	unsigned int adc_clk_fact = 4;// the factor of (system_clk_freq / adc_clk_freq)
	unsigned int larmor_clk_fact = 16;// the factor of (system_clk_freq / f_larmor)
	double SYSCLK_MHz = larmor_clk_fact * f_larmor;


	// init
	init();

	// reset
	bstream_rst();


	// program the clock for the ADC
	Set_PLL(h2p_sys_pll_reconfig_addr, 0, f_larmor * adc_clk_fact, 0.5, DISABLE_MESSAGE);
	Reset_PLL(h2p_general_cnt_out_addr, SYS_PLL_RST_ofst, cnt_out_val);
	Set_DPS(h2p_sys_pll_reconfig_addr, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, sys_pll_locked_ofst);
	usleep(100);// wait for the PLL FCO to lock as well

	if (en_lcs_pchg) {
		bstream__tb_vpc_chg_hbridge_left(
				SYSCLK_MHz,
				bstrap_pchg_us,
				lcs_vpc_pchg_us,   // precharging of vpc
				lcs_recycledump_us,// dumping the lcs to the vpc
				lcs_vpc_pchg_repeat// repeat the precharge and dump
		);
		usleep(T_BLANK / ( SYSCLK_MHz ));// wait for T_BLANK as the last bitstream is not being counted in on bitstream code

	}

	if (en_lcs_dchg) {
		bstream__tb_vpc_wastedump(
				SYSCLK_MHz,
				bstrap_pchg_us,
				lcs_vpc_dchg_us,   // discharging of vpc
				lcs_wastedump_us,// dumping the current into RF
				lcs_vpc_dchg_repeat// repeat the precharge and dump
		);
		usleep(T_BLANK / ( SYSCLK_MHz ));// wait for T_BLANK as the last bitstream is not being counted in on bitstream code
	}

	leave();

	return 0;
}

#endif

