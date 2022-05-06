// Created on: April 14th, 2022
// Author: David Ariando

// #define EXEC_BSTREAM_VPC_WASTEDUMP
#ifdef EXEC_BSTREAM_VPC_WASTEDUMP

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
	cnt_out_val |= ADC_AD9276_PWDN_msk;// (CAREFUL! SOMETIMES THE ADC CANNOT WAKE UP AFTER PUT TO PWDN)
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	soc_exit();

	printf("\nBITSTREAM EXAMPLE STOPPED!\n");
}

int main(int argc, char * argv[]) {

	// default params:
	// 2000 5 100 3

	double bstrap_pchg_us = atof(argv[1]);// bootstrap circuit precharge by enabling both lower side FETs. Has to be done at the beginning to make sure the high-side circuit is charged
	double lcs_vpc_dchg_us = atof(argv[2]);
	double lcs_wastedump_us = atof(argv[3]);
	unsigned int repeat = atoi(argv[4]);

	if (lcs_vpc_dchg_us > 20) {   // safety, do not let the precharge to be more than 40 us
		printf("lcs discharge length is too long.\n");
		return 0;
	}

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

	double f_larmor = 4;
	Set_PLL(h2p_sys_pll_reconfig_addr, 0, f_larmor * 16, 0.5, DISABLE_MESSAGE);
	Reset_PLL(h2p_general_cnt_out_addr, SYS_PLL_RST_ofst, cnt_out_val);
	Set_DPS(h2p_sys_pll_reconfig_addr, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, sys_pll_locked_ofst);

	bstream__vpc_wastedump(
			bstrap_pchg_us,
			lcs_vpc_dchg_us,// precharging of vpc
			lcs_wastedump_us,// dumping the lcs to the vpc
			repeat
	);

	leave();

	return 0;
}

#endif
