// Created on: April 14th, 2022
// Author: David Ariando

#define EXEC_CPMG_RX
#ifdef EXEC_CPMG_RX

#include "hps_linux.h"

// bitstream objects
bstream_obj bstream_objs[BSTREAM_COUNT];

void init() {
	printf("BITSTREAM EXAMPLE STARTED!\n");

	soc_init();
	bstream__init_all_sram();

	// set polling mode for the main PLL
	Reconfig_Mode(h2p_sys_pll_reconfig_addr, 1);

	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;
	cnt_out_val &= ~ADC_AD9276_STBY_msk;   // turn on the ADC
	cnt_out_val &= ~ADC_AD9276_PWDN_msk;   // turn on the ADC
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100000);

}

void leave() {

	// turn off the ADC
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
	// unsigned int overlap = atoi(argv[2]);

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
	unsigned int samples_per_echo = 512;
	unsigned int echoes_per_scan = 3;
	unsigned int n_iterate = 1;
	uint8_t ph_cycl_en = 1;
	unsigned int dconv_fact = 1;
	unsigned int echoskip = 1;
	unsigned int echodrop = 0;

	unsigned int num_of_samples = samples_per_echo * echoes_per_scan;
	uint32_t adc_data_32b[num_of_samples >> 1];   // data for 1 acquisition. Every transfer has 2 data, so the container is divided by 2
	uint16_t adc_data_16b[num_of_samples];

	init();

	// reset
	bstream_rst();

	// set phase increment
	alt_write_word( ( h2p_ph_inc_addr ), 1 << ( NCO_PH_RES - 4 ));

	// set phase overlap
	alt_write_word( ( h2p_ph_overlap_addr ), ( uint16_t )(1 << ( NCO_AMP_RES - 4 )));
	// alt_write_word( ( h2p_ph_overlap_addr ), overlap);

	// set phase base
	unsigned int ph_base_num = 4;
	unsigned int ph0, ph90, ph180, ph270;

	// calculate phase from the phase resolution of the NCO
	ph0 = ph_base_num;
	ph90 = 1 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;
	ph180 = 2 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;
	ph270 = 3 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;

	alt_write_word( ( h2p_ph_0_to_3_addr ), ( ph0 << 24 ) | ( ph90 << 16 ) | ( ph180 << 8 ) | ( ph270 ));   // program phase 0 to phase 3
	alt_write_word( ( h2p_ph_4_to_7_addr ), ( ph0 << 24 ) | ( ph0 << 16 ) | ( ph0 << 8 ) | ( ph0 ));   // program phase 4 to phase 7

	Set_PLL(h2p_sys_pll_reconfig_addr, 0, f_larmor * 4, 0.5, DISABLE_MESSAGE);
	Reset_PLL(h2p_general_cnt_out_addr, SYS_PLL_RST_ofst, cnt_out_val);
	Set_DPS(h2p_sys_pll_reconfig_addr, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, sys_pll_locked_ofst);

	read_adc_id();
	init_adc(AD9276_OUT_ADJ_TERM_100OHM_VAL, AD9276_OUT_PHS_180DEG_VAL, AD9276_OUT_TEST_OFF_VAL, 0, 0);

	/*
	 bstream__vpc_chg(
	 bstrap_pchg_us,
	 50.00,   // precharging of vpc
	 1000.00,   // dumping the lcs to the vpc
	 80
	 );
	 */

	usleep(100000);

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

	usleep(100000);

	read_adc_val(h2p_fifo_sink_ch_a_csr_addr, h2p_fifo_sink_ch_a_data_addr, adc_data_32b);
	buf32_to_buf16(adc_data_32b, adc_data_16b, num_of_samples >> 1);   // convert the 32-bit data format to 16-bit.
	wr_File("data.txt", num_of_samples, (int*) adc_data_16b, SAV_ASCII);   // write the data to the filename

	leave();

	return 0;
}

#endif
