// Created on: Feb 19th, 2025
// Author: David Ariando
// this program is to test the gradient output.

// #define EXEC_TB_GRAD_BIAS_SEARCH
#ifdef EXEC_TB_GRAD_BIAS_SEARCH

#include "hps_linux.h"

// bitstream objects
bstream_obj bstream_objs[BSTREAM_COUNT];

void init() {
	// printf("START::EXEC_PHENC\n");

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

	// turn on the GRAD DAC I2C interface
	cnt_out_val |= GRAD_OE;
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
	// turn on the GRAD DAC I2C interface
	cnt_out_val &= ~GRAD_OE;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	soc_exit();

	// printf("STOP::EXEC_PHENC\n");
}

int main(int argc, char * argv[]) {

	// pulse params
	double grad_len_us = atof(argv[1]);	// gradient length
	double dly_hiside_to_pulse_us = atof(argv[2]); // delay from turning on high-side to the pulse
	double channel = atoi(argv[3]); //  set the channel (A=1, B=2, C=3, D=4)
	double vset = atof(argv[4]); // set bias voltage
	double vbias = atof(argv[5]); // set null voltage

	// fixed params


	// init
	init();

	// reset
	bstream_rst();

	// limit the voltage
	if (vset > 3.5) {
		fprintf(stderr, "\tERROR! Set gradient voltage is too high.\n");
		exit(EXIT_FAILURE);
	}

	// turn on only the necessary voltage
	if (channel == 1) { // channel A
		grad_init_voltage(vset, vbias, 0, 0, DAC_X); // program the DAC_X
		grad_init_voltage(vset, vbias, 0, 0, DAC_Y); // program the DAC_Y
		cnt_out_val |= (GRADX_LO_R_SOC | GRADY_LO_R_SOC);
	}
	else if (channel == 2) { // channel B
		grad_init_voltage(vbias, vset, 0, 0, DAC_X); // program the DAC_X
		grad_init_voltage(vbias, vset, 0, 0, DAC_Y); // program the DAC_Y
		cnt_out_val &= ~(GRADX_LO_R_SOC | GRADY_LO_R_SOC);
	}
	else if (channel == 3) { // channel C
		grad_init_voltage(0, 0, vset, vbias, DAC_X); // program the DAC_X
		grad_init_voltage(0, 0, vset, vbias, DAC_Y); // program the DAC_Y
		cnt_out_val |= (GRADX_LO_L_SOC | GRADY_LO_L_SOC);
	}
	else if (channel == 4) { // channel D
		grad_init_voltage(0, 0, vbias, vset, DAC_X); // program the DAC_X
		grad_init_voltage(0, 0, vbias, vset, DAC_Y); // program the DAC_Y
		cnt_out_val &= ~(GRADX_LO_L_SOC | GRADY_LO_L_SOC);
	}
	else {
		fprintf(stderr, "Error: Invalid input\n");
		return 0;
	}

	// turn the selected hi-side switches on
	if (channel == 1) { // channel A
		cnt_out_val |= GRADX_HI_L_SOC | GRADY_HI_L_SOC;
	}
	else if (channel == 2) { // channel B
		cnt_out_val |= GRADX_HI_L_SOC | GRADY_HI_L_SOC;
	}
	else if (channel == 3) { // channel C
		cnt_out_val |= GRADX_HI_R_SOC | GRADY_HI_R_SOC;
	}
	else if (channel == 4) { // channel D
		cnt_out_val |= GRADX_HI_R_SOC | GRADY_HI_R_SOC;
	}
	else {
		fprintf(stderr, "Error: Invalid input\n");
		return 0;
	}
	alt_write_word((h2p_general_cnt_out_addr), cnt_out_val);
	usleep(dly_hiside_to_pulse_us);

	// turn the selected low-side switches on
	if (channel == 1) { // channel A
		cnt_out_val &= ~(GRADX_LO_R_SOC | GRADY_LO_R_SOC);
	}
	else if (channel == 2) { // channel B
		cnt_out_val |= (GRADX_LO_R_SOC | GRADY_LO_R_SOC);
	}
	else if (channel == 3) { // channel C
		cnt_out_val &= ~(GRADX_LO_L_SOC | GRADY_LO_L_SOC);
	}
	else if (channel == 4) { // channel D
		cnt_out_val |= (GRADX_LO_L_SOC | GRADY_LO_L_SOC);
	}
	else {
		fprintf(stderr, "Error: Invalid input\n");
		return 0;
	}
	cnt_out_val |= GRAD_AUX;
	alt_write_word((h2p_general_cnt_out_addr), cnt_out_val);
	usleep((unsigned int) grad_len_us);
	
	// turn the selected low-side switches on
	if (channel == 1) { // channel A
		cnt_out_val |= (GRADX_LO_R_SOC | GRADY_LO_R_SOC);
	}
	else if (channel == 2) { // channel B
		cnt_out_val &= ~(GRADX_LO_R_SOC | GRADY_LO_R_SOC);
	}
	else if (channel == 3) { // channel C
		cnt_out_val |= (GRADX_LO_L_SOC | GRADY_LO_L_SOC);
	}
	else if (channel == 4) { // channel D
		cnt_out_val &= ~(GRADX_LO_L_SOC | GRADY_LO_L_SOC);
	}
	else {
		fprintf(stderr, "Error: Invalid input\n");
		return 0;
	}
	alt_write_word((h2p_general_cnt_out_addr), cnt_out_val);

	// turn off the high-side
	usleep(dly_hiside_to_pulse_us);
	cnt_out_val &= ~(GRADX_HI_R_SOC | GRADX_HI_L_SOC | GRADY_HI_R_SOC
			| GRADY_HI_L_SOC | GRAD_AUX);
	alt_write_word((h2p_general_cnt_out_addr), cnt_out_val);

	leave();
	return 0;
}

#endif
