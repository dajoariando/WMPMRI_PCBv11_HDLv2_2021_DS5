// Created on: April 14th, 2022
// Author: David Ariando

// #define EXEC_BSTREAM_VPC_CHARGE
#ifdef EXEC_BSTREAM_VPC_CHARGE

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
	// 2000 20 40 3

	double bstrap_pchg_us = atof(argv[1]);// bootstrap circuit precharge by enabling both lower side FETs. Has to be done at the beginning to make sure the high-side circuit is charged
	double lcs_pchg_us = atof(argv[2]);
	double lcs_recycledump_us = atof(argv[3]);
	unsigned int repeat = atoi(argv[4]);

	if (lcs_pchg_us > 100) {   // safety, do not let the precharge to be more than 40 us
		printf("precharge length is too long.\n");
		return 0;
	}

	init();

	// reset
	bstream_rst();

	// set phase increment
	alt_write_word( ( h2p_ph_inc_addr ), 1 << ( NCO_PH_RES - 4 ));

	// set phase overlap
	alt_write_word( ( h2p_ph_overlap_addr ), (uint16_t) 4);

	// set phase base
	unsigned int ph_base_num = 4;
	unsigned int ph0, ph90, ph180, ph270;

	// calculate phase from the phase resolution of the NCO
	ph0 = ph_base_num;
	ph90 = 1 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;
	ph180 = 2 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;
	ph270 = 3 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;

	alt_write_word( ( h2p_ph_0_to_3_addr ), ( ph0 << 24 ) | ( ph90 << 16 ) | ( ph180 << 8 ) | ( ph270 ));// program phase 0 to phase 3
	alt_write_word( ( h2p_ph_4_to_7_addr ), ( ph0 << 24 ) | ( ph0 << 16 ) | ( ph0 << 8 ) | ( ph0 ));// program phase 4 to phase 7

	double f_larmor = 4;
	Set_PLL(h2p_sys_pll_reconfig_addr, 0, f_larmor * 16, 0.5, DISABLE_MESSAGE);
	Reset_PLL(h2p_general_cnt_out_addr, SYS_PLL_RST_ofst, cnt_out_val);
	Set_DPS(h2p_sys_pll_reconfig_addr, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, sys_pll_locked_ofst);

	bstream__vpc_chg(
			bstrap_pchg_us,
			lcs_pchg_us,// precharging of vpc
			lcs_recycledump_us,// dumping the lcs to the vpc
			repeat
	);

	leave();

	return 0;
}

#endif
