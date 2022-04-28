// Created on: April 14th, 2022
// Author: David Ariando

#define EXEC_BSTREAM_PCHARGE_N_DUMP
#ifdef EXEC_BSTREAM_PCHARGE_N_DUMP

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
	cnt_out_val |= ADC_AD9276_PWDN_msk;   // (CAREFUL! SOMETIMES THE ADC CANNOT WAKE UP AFTER PUT TO PWDN)
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	soc_exit();

	printf("\nBITSTREAM EXAMPLE STOPPED!\n");
}

int main(int argc, char * argv[]) {

	// default param for test : 500 100 50 0 30 1
	// to completely show the waveform in SignalTap, change the first parameter from 500 to 100, but this is not enough for precharging the high-side FETs

	/*
	 double bstrap_pchg_us = atof(argv[1]);   // bootstrap circuit precharge by enabling both lower side FETs. Has to be done at the beginning to make sure the high-side circuit is charged
	 double ind_pchg_us = atof(argv[2]);   // precharging the current source inductor
	 double tail_us = atof(argv[3]);   // tx_l1 and tx_l2 length after dump
	 double dump_dly_us = atof(argv[4]);   // added delay before dump all the current after precharging
	 double dump_len_us = atof(argv[5]);   // current dump length
	 unsigned int en_pchrg = atoi(argv[6]);   // enable precharging via higher voltage VPC

	 double max_plen = 3000;   // set the maximum plen
	 if (ind_pchg_us > max_plen) {
	 printf("\t ERROR! Pulse is too long.\n");
	 return 0;
	 }
	 */

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
	float CLK_50 = 50.00;

	// reset
	bstream_rst();

	// set phase increment
	alt_write_word( ( h2p_ph_inc_addr ), 4096);
	alt_write_word( ( h2p_ph_overlap_addr ), (uint16_t) 32);

	// set phase base
	alt_write_word( ( h2p_ph0_addr ), 1);
	alt_write_word( ( h2p_ph1_addr ), 1);
	alt_write_word( ( h2p_ph2_addr ), 1);
	alt_write_word( ( h2p_ph3_addr ), 1);
	alt_write_word( ( h2p_ph4_addr ), 1);
	alt_write_word( ( h2p_ph5_addr ), 1);
	alt_write_word( ( h2p_ph6_addr ), 1);
	alt_write_word( ( h2p_ph7_addr ), 1);
	alt_write_word( ( h2p_ph8_addr ), 1);
	alt_write_word( ( h2p_ph9_addr ), 1);
	alt_write_word( ( h2p_ph10_addr ), 1);
	alt_write_word( ( h2p_ph11_addr ), 1);
	alt_write_word( ( h2p_ph12_addr ), 1);
	alt_write_word( ( h2p_ph13_addr ), 1);
	alt_write_word( ( h2p_ph14_addr ), 1);

	double f_larmor = 50 / 16;
	double lcs_pchg_us = 20;
	double lcs_dump_us = 10;
	double p90_pchg_us = 5;
	double p90_us = 5;
	double p90_dchg_us = 10;
	double p90_dtcl = 0.5;
	double p180_pchg_us = 10;
	double p180_us = 10;
	double p180_dchg_us = 20;
	double p180_dtcl = 0.5;
	double echoshift_us = 6;
	double echotime_us = 100;
	long unsigned scanspacing_us = 10000;
	unsigned int samples_per_echo = 64;
	unsigned int echoes_per_scan = 3;
	unsigned int n_iterate = 2;
	uint8_t ph_cycl_en = 1;
	unsigned int dconv_fact = 1;
	unsigned int echoskip = 1;
	unsigned int echodrop = 0;

	// start
	// sbstream__test(CLK_50);
	bstream__cpmg(f_larmor,
	        lcs_pchg_us,   // precharging of vpc
	        lcs_dump_us,   // dumping the lcs to the vpc
	        p90_pchg_us,
	        p90_us,
	        p90_dchg_us,   // the discharging length of the current source inductor
	        p90_dtcl,
	        p180_pchg_us,
	        p180_us,
	        p180_dchg_us,	// the discharging length of the current source inductor
	        p180_dtcl,
	        echoshift_us,	// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
	        echotime_us,
	        scanspacing_us,
	        samples_per_echo,
	        echoes_per_scan,
	        n_iterate,
	        ph_cycl_en,
	        dconv_fact,
	        echoskip,
	        echodrop
	        );

	// test nulling
	// bstream__null_everything();

	// test precharging and dump
	// bstream__prechrg_n_dump(CLK_50, bstrap_pchg_us, ind_pchg_us, tail_us, dump_dly_us, dump_len_us, en_pchrg);

	leave();

	return 0;
}

#endif
