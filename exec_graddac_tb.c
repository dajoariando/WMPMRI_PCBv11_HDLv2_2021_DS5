// Created on: Jan 30 2025
// Author: David Ariando
// This is a test program for mcp4728 dac

#define EXEC_TB_MCP4728_3
#ifdef EXEC_TB_MCP4728_3

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
	double t_on = atoi(argv[1]);
	double iin = atof(argv[2]); // set the output current in mA
	double ibias = atof(argv[3]); // set the bias current in mA

	// limit the bias current
	if (ibias<0) {
		ibias = fabs(ibias);
		printf("ibias parameter has to be positive.\n");
	}
	if (ibias>20) { // limit bias current to 20 mA
		ibias = 20;
		printf("ibias is over limit. Maximum is 20mA.\n");
	}

	// conversion from current to voltage
	double vout = (fabs(iin/1000)*0.2) + 2.5;
	double vbias = (ibias/1000*0.2) + 2.5; // ibias is already positive at this point

	// find the direction of the current
	int dir = 0;
	if (iin>=0) {
		dir = 1;
	}
	else {
		dir = 0;
	}



	// init
	init();

	cnt_out_val |= GRAD_OE;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 3.00, DAC_Y, CH_DACC, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 3.00, DAC_Y, CH_DACA, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);
	cnt_out_val |= ( GRADY_CNT_N | GRADY_CNT_P );
	usleep(500000);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 3.00, DAC_Y, CH_DACC, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 3.00, DAC_Y, CH_DACA, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);
	cnt_out_val &= ~( GRADY_CNT_N | GRADY_CNT_P );

	// use DAC A and DAC C for biasing, and use DAC B and DAC D for on-voltage gradient.
	// DAC C and DAC D are on the same side with GRADY_CNT_N (low-side FET) and GRADY_HR (high-side FETs).
	// DAC A and DAC B are on the same side with GRADY_CNT_P (low-side FET) and GRADY_HL (high-side FETs).

	// set bias voltage
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, vbias, DAC_Y, CH_DACA, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, vbias, DAC_Y, CH_DACC, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);

	// set voltage output
	if (dir) {
		mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, vout, DAC_Y, CH_DACB, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);
		mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, vbias, DAC_Y, CH_DACD, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);

		usleep(1000);

		// turn on the current output for a short time
		cnt_out_val |= GRADY_HR;
		alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
		usleep(100);

		cnt_out_val |= GRAD_AUX | GRADY_CNT_P;
		alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
		usleep(t_on);
		cnt_out_val &= ~(GRAD_AUX | GRADY_CNT_P );
		alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
		cnt_out_val &= ~( GRADY_HR);
		alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	}
	else {
		mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, vbias, DAC_Y, CH_DACB, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);
		mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, vout, DAC_Y, CH_DACD, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, ENABLE_MESSAGE);

		usleep(1000);

		// turn on the current output for a short time
		cnt_out_val |= GRADY_HL;
		alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
		usleep(100);

		cnt_out_val |= GRAD_AUX | GRADY_CNT_N;
		alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
		usleep(t_on);
		cnt_out_val &= ~(GRAD_AUX | GRADY_CNT_N );
		alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
		cnt_out_val &= ~( GRADY_HL);
		alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);


	}


	// GRADY_HL -------------------|    |------------------GRADY_HR
	//
	// DAC A -----------\                         /-------- DAC C (default)
	// GRADY_CNT_P        ---------|    |---------    GRADY_CNT_N
	// DAC B -----------/                         \-------- DAC D



	leave();

	return 0;
}

#endif
