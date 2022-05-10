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

	Reconfig_Mode(h2p_sys_pll_reconfig_addr, 1);   // polling mode for main pll

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

	// default params:

	double f_larmor = atof(argv[1]);

	init();

	// reset
	bstream_rst();

	// set phase increment
	alt_write_word( ( h2p_ph_inc_addr ), 4096);

	// set phase overlap
	alt_write_word( ( h2p_ph_overlap_addr ), (uint16_t) 128);

	// set phase base
	unsigned int ph_base_num = 32;
	alt_write_word( ( h2p_ph0_addr ), ph_base_num);
	alt_write_word( ( h2p_ph1_addr ), 16384 + ph_base_num);
	alt_write_word( ( h2p_ph2_addr ), 32768 + ph_base_num);
	alt_write_word( ( h2p_ph3_addr ), 49152 + ph_base_num);
	alt_write_word( ( h2p_ph4_addr ), ph_base_num);
	alt_write_word( ( h2p_ph5_addr ), ph_base_num);
	alt_write_word( ( h2p_ph6_addr ), ph_base_num);
	alt_write_word( ( h2p_ph7_addr ), ph_base_num);
	alt_write_word( ( h2p_ph8_addr ), ph_base_num);
	alt_write_word( ( h2p_ph9_addr ), ph_base_num);
	alt_write_word( ( h2p_ph10_addr ), ph_base_num);
	alt_write_word( ( h2p_ph11_addr ), ph_base_num);
	alt_write_word( ( h2p_ph12_addr ), ph_base_num);
	alt_write_word( ( h2p_ph13_addr ), ph_base_num);
	alt_write_word( ( h2p_ph14_addr ), ph_base_num);

	// double f_larmor = 4;
	double bstrap_pchg_us = 2000.00;   // bootstrap circuit precharge by enabling both lower side FETs. Has to be done at the beginning to make sure the high-side circuit is charged
	double lcs_pchg_us = 25;
	double lcs_dump_us = 200;
	double p90_pchg_us = 5;
	double p90_pchg_refill_us = 5;
	double p90_us = 5;
	double p90_dchg_us = 10;
	double p90_dtcl = 0.5;
	double p180_pchg_us = 10;
	double p180_pchg_refill_us = 20;
	double p180_us = 5;
	double p180_dchg_us = 20;
	double p180_dtcl = 0.5;
	double echoshift_us = 6;
	double echotime_us = 200;
	long unsigned scanspacing_us = 10000;
	unsigned int samples_per_echo = 64;
	unsigned int echoes_per_scan = 1024;
	unsigned int n_iterate = 2;
	uint8_t ph_cycl_en = 1;
	unsigned int dconv_fact = 1;
	unsigned int echoskip = 1;
	unsigned int echodrop = 0;

	Set_PLL(h2p_sys_pll_reconfig_addr, 0, f_larmor * 16, 0.5, DISABLE_MESSAGE);
	Reset_PLL(h2p_general_cnt_out_addr, SYS_PLL_RST_ofst, cnt_out_val);
	Set_DPS(h2p_sys_pll_reconfig_addr, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, sys_pll_locked_ofst);

	//
	bstream__vpc_chg(
	        bstrap_pchg_us,
	        50.00,   // precharging of vpc
	        1000.00,   // dumping the lcs to the vpc
	        80
	        );
	//

	usleep(100000);

	/*
	 bstream__cpmg(f_larmor,
	 bstrap_pchg_us,
	 lcs_pchg_us,   // precharging of vpc
	 lcs_dump_us,   // dumping the lcs to the vpc
	 p90_pchg_us,
	 // p90_pchg_refill_us,
	 p90_us,
	 p90_dchg_us,   // the discharging length of the current source inductor
	 p90_dtcl,
	 p180_pchg_us,
	 // p180_pchg_refill_us,
	 p180_us,
	 p180_dchg_us,   // the discharging length of the current source inductor
	 p180_dtcl,
	 echoshift_us,   // shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
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
	 */

	bstream__cpmg_refill(f_larmor,
	        bstrap_pchg_us,
	        lcs_pchg_us,   // precharging of vpc
	        lcs_dump_us,   // dumping the lcs to the vpc
	        p90_pchg_us,
	        p90_pchg_refill_us,
	        p90_us,
	        p90_dchg_us,   // the discharging length of the current source inductor
	        p90_dtcl,
	        p180_pchg_us,
	        p180_pchg_refill_us,
	        p180_us,
	        p180_dchg_us,   // the discharging length of the current source inductor
	        p180_dtcl,
	        echoshift_us,   // shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
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
	//

	leave();

	return 0;
}

#endif
