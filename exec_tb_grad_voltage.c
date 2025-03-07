// Created on: Feb 19th, 2025
// Author: David Ariando
// this program is to test the gradient output.

// #define EXEC_TB_GRAD_VOLTAGE
#ifdef EXEC_TB_GRAD_VOLTAGE

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

	// param defined by user
	double SYSCLK_MHz = atof(argv[1]);
	double bstrap_pchg_us = atof(argv[2]);// bootstrap circuit precharge by enabling both lower side FETs. Has to be done at the beginning to make sure the high-side circuit is charged. This takes a long time (around 2ms).
	// pulse param
	double front_porch_us = atof(argv[3]);
	double grad_len_us = atof(argv[4]);	// gradient length
	double grad_blanking_us = atof(argv[5]);
	double back_tail_us = atof(argv[6]);
	// --- gradient strength
	double voltx_A = atof(argv[7]); // gradient x, ch A
	double voltx_B = atof(argv[8]); // gradient x, ch B
	double voltx_C = atof(argv[9]); // gradient x, ch C
	double voltx_D = atof(argv[10]); // gradient x, ch D
	double volty_A = atof(argv[11]); // gradient y, ch A
	double volty_B = atof(argv[12]); // gradient y, ch B
	double volty_C = atof(argv[13]); // gradient y, ch C
	double volty_D = atof(argv[14]); // gradient y, ch D
	// -- encoding period
	char grad_refocus = atoi(argv[15]); // the gradient refocusing enable that's present in PGSE sequence. When it's off, it's purely phase encoding.
	char flip_grad_refocus_sign = atoi(argv[16]); // flip the gradient refocus sign for phase encoding, and don't flip it for pgse
	//
	char gradx_dir = atoi(argv[17]); // the gradient direction
	char grady_dir = atoi(argv[18]);


	// init
	init();

	// reset
	bstream_rst();

	// limit the voltage
	if ((voltx_A > 3.5) || (voltx_B > 3.5) || (voltx_C > 3.5) || (voltx_D > 3.5)
			|| (volty_A > 3.5) || (volty_B > 3.5) || (volty_C > 3.5)
			|| (volty_D > 3.5)) {
		fprintf(stderr, "\tERROR! Set gradient voltage is too high.\n");
		exit(EXIT_FAILURE);
	}


	// program the dac
	grad_init_voltage(voltx_A, voltx_B, voltx_C, voltx_D, DAC_X); // program the DAC_X
	grad_init_voltage(volty_A, volty_B, volty_C, volty_D, DAC_Y); // program the DAC_Y
	usleep(200000);

	//  program the clock for the ADC
	Set_PLL(h2p_sys_pll_reconfig_addr, 0, SYSCLK_MHz, 0.5, DISABLE_MESSAGE);
	Reset_PLL(h2p_general_cnt_out_addr, SYS_PLL_RST_ofst, cnt_out_val);
	Set_DPS(h2p_sys_pll_reconfig_addr, 0, 0, DISABLE_MESSAGE);
	// Wait_PLL_To_Lock(h2p_general_cnt_in_addr, sys_pll_locked_ofst);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, fco_locked_ofst);

	// initialize ADC
	// read_adc_id();
	init_adc(AD9276_OUT_ADJ_TERM_100OHM_VAL, AD9276_OUT_PHS_180DEG_VAL, AD9276_OUT_TEST_OFF_VAL, 0, 0);

	// write the preamp dac
	// wr_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_B, vvarac, DAC_PAMP_LDAC, DISABLE_MESSAGE);// set -2.5 for 4 MHz resonant

	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, fco_locked_ofst);
	usleep(100);	// wait for the PLL FCO to lock as well


	bstream__tb_grad(
			SYSCLK_MHz * 4,	// base clock. *4 is due to the clock being used by bistream is 4x the system clock
			bstrap_pchg_us,			// bootstrap precharging
			front_porch_us,			// front porch before gradient
		grad_len_us,			// gradient length (for both output)
			grad_blanking_us,	// gradient blanking between two gradient output
			back_tail_us,			// back tail after gradient train
		grady_dir,				// gradient z direction
		gradx_dir,				// gradient x direction
		grad_refocus,			// gradient refocus
			flip_grad_refocus_sign	// flip gradient refocus
	);

	leave();

	/*
	TX BITSTREAM HAS TO BE THE LAST ONE TO FINISH BECAUSE THE FINISH, OTHERWISE THE BITSTREAM DONE SIGNAL WILL
	TELL THE WHOLE PROGRAM TO CONTINUE, POSSIBLY ALTERING THE ADC THAT GENERATES THE CLOCK FOR THE ADC, CAUSING CHAOS.
	THATS THE REASON WHY I HAD T_BLANK AT THE END OF EVERY BITSTREAM SIGNAL, IN ORDER TO MAKE SURE THAT THE FINAL
	BITSTREAM KEEPS ALL OUTPUTS ZERO, BEFORE SENDING "DONE" SIGNAL THROUGH TX_HI. SENDING ONLY THROUGH TX-HI IS ALSO
	A PROBLEM, CAUSE IT DOES NOT GUARANTEE THAT THE SIGNAL IS DONE FOR ALL BITSTREAM. WHAT ABOUT USING ALL SIGNAL "DONE"
	INSTEAD????
	ALSO REMEMBER THAT THE "DONE" SIGNAL IS GENERATED BEFORE THE FINAL BITSTREAM CHUNK IS FINISHED. IT ACTUALLY BEING
	GENERATED WHEN THE FINAL BITSTREAM CHUNK IS STARTED.

	ALSO MAKE SURE TO CLEAN UP PARAMETER SOC_INPUT AND SOC_OUTPUT NAME CAUSE I JUST CHANGED A BUNCH.*/

	return 0;
}

#endif
