// Created on: Jan 30 2025
// Author: David Ariando
// This is a test program for mcp4728 dac

// #define EXEC_TB_MCP4728
#ifdef EXEC_TB_MCP4728

#include "hps_linux.h"

// bitstream objects
bstream_obj bstream_objs[BSTREAM_COUNT];

void init() {
	printf("START:: EXEC_NOISE\n");

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
	// init_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_PAMP_CLR);
	usleep(100000);

}

void leave() {

	// turn off the ADC
	cnt_out_val |= ADC_AD9276_STBY_msk;
	cnt_out_val |= ADC_AD9276_PWDN_msk;// (CAREFUL! SOMETIMES THE ADC CANNOT WAKE UP AFTER PUT TO PWDN)
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	soc_exit();

	printf("STOP:: EXEC_NOISE\n");
}

int main(int argc, char * argv[]) {

	// param defined by user
	double vout = atof(argv[1]); // set the output voltage
	uint8_t i2c_3bit_addr = atoi(argv[2]); // select i2c 3-bit address, from 1 to 4. If address 0 works (and 1-4 doesn't), it means the i2c address is still default from factory. Please first program the DAC with FPGA binary from SIGRES_GRAD_DAC_PROG_Quartus.
	uint8_t channel = atoi(argv[3]); // select the DAC output channel, from 0-3.

	// init
	init();

	cnt_out_val |= GRAD_OE;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, vout, i2c_3bit_addr, channel, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);

	leave();

	return 0;
}

#endif
