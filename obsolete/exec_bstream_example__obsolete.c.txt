// Created on: May 26, 2021
// Author: David Ariando

// #define EXEC_BSTREAM_EXAMPLE
#ifdef EXEC_BSTREAM_EXAMPLE

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

	char RF_mode = atoi(argv[1]);
	double dtcl = atof(argv[2]);
	double bstrap_pchg_us = atof(argv[3]);   // bootstrap circuit precharge by enabling both lower side FETs. Has to be done at the beginning to make sure the high-side circuit is charged
	double ind_pchg_us = atof(argv[4]);// precharging the current source inductor
	unsigned char ind_pchg_mode = atoi(argv[5]);
	double plen_us = atof(argv[6]);
	double tail_us = atof(argv[7]);
	double dump_dly_us = atof(argv[8]);
	double dump_len_us = atof(argv[9]);
	unsigned int en_pchrg = atoi(argv[10]);
	unsigned int repetition = atoi(argv[11]);
	float RFCLK = atof(argv[12]);
	float vvarac = atof(argv[13]);
	double tx_coil_pchg_us = atof(argv[14]);
	unsigned int dump_repetition = atoi(argv[15]);

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

	// alt_write_word(lwaxi_led, 0xFF);
	// alt_write_word(lwaxi_led, 0x00);
	// standard parameters
	float CLK_50 = 50.00;
	//float RFCLK = 1.00;

	// reset
	bstream_rst();

	// test nulling
	bstream__null_everything();

	// test precharging and dump
	// double bstrap_pchg_us = 10;
	bstream__prechrg_n_dump(CLK_50, bstrap_pchg_us, ind_pchg_us, tail_us, dump_dly_us, dump_len_us, en_pchrg);

	// test rf output
	// bstream__prechrg_n_rf_n_dump(RF_mode, CLK_50, RFCLK, dtcl, ind_pchg_us, tx_coil_pchg_us, dump_len_us, en_pchrg, repetition);
	// bstream__prechrg_n_rf_n_dump_180(/*RF_mode,*/CLK_50, RFCLK, dtcl, ind_pchg_us, ind_pchg_mode, tx_coil_pchg_us, dump_len_us, en_pchrg, repetition, dump_repetition);

	// slow toggle output
	// bstream__toggle(&bstream_objs[tx_l1], CLK_50, 3000000, 1000);
	// bstream__toggle(&bstream_objs[tx_l2], CLK_50, 3000000, 1000);
	// bstream__toggle(&bstream_objs[tx_h1], CLK_50, 3000000, 1000);
	// bstream__toggle(&bstream_objs[tx_h2], CLK_50, 3000000, 1000);

	// init_dac_ad5722r (lwaxi_rx_dac);
	// for (vvarac = -5; vvarac < 5; vvarac += 0.5) {
	// wr_dac_ad5722r(lwaxi_rx_dac, DAC_B, vvarac, ENABLE_MESSAGE);
	//usleep(3000000);
	//}
	leave();

	return 0;
}

#endif
