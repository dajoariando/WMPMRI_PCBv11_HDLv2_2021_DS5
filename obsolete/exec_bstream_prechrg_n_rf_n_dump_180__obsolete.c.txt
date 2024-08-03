// Created on: May 26, 2021
// Author: David Ariando

// #define EXEC_BSTREAM_PRECHRG_N_RF_N_DUMP_180
#ifdef EXEC_BSTREAM_PRECHRG_N_RF_N_DUMP_180

#include "hps_linux.h"

// bitstream objects
bstream_obj bstream_objs[BSTREAM_COUNT];

void init() {
	printf("BITSTREAM EXAMPLE STARTED!\n");

	soc_init();
	bstream__init_all_sram();

	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	// set the bitstream pll frequency
	Reconfig_Mode(h2p_bstream_pll_addr, 1);// polling mode for main pll

}

void leave() {
	// turn of the ADC
	cnt_out_val |= ADC_AD9276_STBY_msk;
	cnt_out_val |= ADC_AD9276_PWDN_msk;// (CAREFUL! SOMETIMES THE ADC CANNOT WAKE UP AFTER PUT TO PWDN)
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	soc_exit();

	printf("\nBITSTREAM EXAMPLE STOPPED!\n");
}

int main(int argc, char * argv[]) {

	// try:  1 0.55 12 0 5 0 34 2.00 10 1

	char RF_mode = atoi(argv[1]);// put 1 for high-pulse low-pulse same duty-cycle. 2 and 3 is for different duty-cycle. check the function
	double dtcl = atof(argv[2]);// duty cycle
	double ind_pchg_us = atof(argv[3]);// precharging the current source inductor
	unsigned char ind_pchg_mode = atoi(argv[4]);
	double dump_len_us = atof(argv[5]);
	unsigned int en_pchrg = atoi(argv[6]);
	unsigned int repetition = atoi(argv[7]);
	float RFCLK = atof(argv[8]);
	double tx_coil_pchg_us = atof(argv[9]);
	unsigned int dump_repetition = atoi(argv[10]);

	double max_plen = 3000;// set the maximum plen
	if (ind_pchg_us > max_plen) {
		printf("\t ERROR! Pulse is too long.\n");
		return 0;
	}

	init();

	/*
	 *  set pll for CPMG
	 *
	 *  double clk_freq = 12;
	 Set_PLL(lwaxi_sys_pll, 0, clk_freq, 0.5, DISABLE_MESSAGE);
	 Reset_PLL(lwaxi_cnt_out, SYS_PLL_RST_ofst, ctrl_out);
	 Set_DPS(lwaxi_sys_pll, 0, 0, DISABLE_MESSAGE);
	 Wait_PLL_To_Lock(lwaxi_cnt_in, PLL_SYS_lock_ofst);
	 */

	// standard parameters
	float BSTREAM_CLK_IN = 100.00;

	// set pll for bitstream
	Set_PLL(h2p_bstream_pll_addr, 0, BSTREAM_CLK_IN, 0.5, DISABLE_MESSAGE);
	// Reset_PLL(h2p_ctrl_out_addr, PLL_NMR_SYS_RST_ofst, ctrl_out);
	Set_DPS(h2p_bstream_pll_addr, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_int_addr, bstream_pll_locked_ofst);

	// reset
	bstream_rst();

	// test nulling
	// bstream__null_everything();

	// test rf output
	bstream__prechrg_n_rf_n_dump_180(/*RF_mode,*/BSTREAM_CLK_IN, RFCLK, dtcl, ind_pchg_us, ind_pchg_mode, tx_coil_pchg_us, dump_len_us, en_pchrg, repetition, dump_repetition);

	leave();

	return 0;
}

#endif
