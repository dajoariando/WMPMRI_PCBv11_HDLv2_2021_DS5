// Created on: April 14th, 2022
// Author: David Ariando

// #define EXEC_CPMG_T2
#ifdef EXEC_CPMG_T2

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
	cnt_out_val &= ~ADC_AD9276_STBY_msk;// turn on the ADC
	cnt_out_val &= ~ADC_AD9276_PWDN_msk;// turn on the ADC
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	// init the DAC
	init_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_PAMP_CLR);
	usleep(100000);

}

void leave() {

	// turn off the ADC
	cnt_out_val |= ADC_AD9276_STBY_msk;
	cnt_out_val |= ADC_AD9276_PWDN_msk;// (CAREFUL! SOMETIMES THE ADC CANNOT WAKE UP AFTER PUT TO PWDN)
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	soc_exit();

	printf("\nBITSTREAM EXAMPLE STOPPED!\n");
}

int main(int argc, char * argv[]) {

	/* DEFAULT CONFIGURATION TO EASILY SEE CPMG SEQUENCE RESULTS IN SIGNALTAP
	 // DON'T FORGET TO COMMENT bstrm__vpc_chg
	 // double f_larmor = 4;
	 double bstrap_pchg_us = 2000.00;   // bootstrap circuit precharge by enabling both lower side FETs. Has to be done at the beginning to make sure the high-side circuit is charged
	 double lcs_pchg_us = 10;
	 double lcs_dump_us = 50;
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
	 double echotime_us = 150;
	 long unsigned scanspacing_us = 10000;
	 unsigned int samples_per_echo = 512;
	 unsigned int echoes_per_scan = 2;
	 unsigned int n_iterate = 1;
	 uint8_t p180_ph_sel = 1;
	 unsigned int dconv_fact = 1;
	 unsigned int echoskip = 1;
	 unsigned int echodrop = 0;
	 double vvarac = -2.5;
	 */

	// param defined by user
	double f_larmor = atof(argv[1]);
	double bstrap_pchg_us = atof(argv[2]);// bootstrap circuit precharge by enabling both lower side FETs. Has to be done at the beginning to make sure the high-side circuit is charged
	double lcs_pchg_us = atof(argv[3]);
	double lcs_dump_us = atof(argv[4]);
	double p90_pchg_us = atof(argv[5]);
	double p90_pchg_refill_us = atof(argv[6]);
	double p90_us = atof(argv[7]);
	double p90_dchg_us = atof(argv[8]);
	double p90_dtcl = atof(argv[9]);
	double p180_pchg_us = atof(argv[10]);
	double p180_pchg_refill_us = atof(argv[11]);
	double p180_us = atof(argv[12]);
	double p180_dchg_us = atof(argv[13]);
	double p180_dtcl = atof(argv[14]);
	double echoshift_us = atof(argv[15]);
	double echotime_us = atof(argv[16]);
	long unsigned scanspacing_us = atoi(argv[17]);
	unsigned int samples_per_echo = atoi(argv[18]);
	unsigned int echoes_per_scan = atoi(argv[19]);
	unsigned int n_iterate = atoi(argv[20]);
	uint8_t p90_ph_sel = atoi(argv[21]);// set this to 0 for phase 0, 1 for phase 90, 2 for phase 180, 3 for phase 270.
	unsigned int dconv_fact = atoi(argv[22]);
	unsigned int echoskip = atoi(argv[23]);
	unsigned int echodrop = atoi(argv[24]);
	double vvarac = atof(argv[25]);
	// --- vpc precharging ---
	double lcs_vpc_pchg_us = atof(argv[26]);
	double lcs_recycledump_us = atof(argv[27]);
	double lcs_vpc_pchg_repeat = atof(argv[28]);
	// --- vpc discharging ---
	double lcs_vpc_dchg_us = atof(argv[29]);
	double lcs_wastedump_us = atof(argv[30]);
	double lcs_vpc_dchg_repeat = atof(argv[31]);

	// param defined by Quartus
	unsigned int adc_clk_fact = 4;// the factor of (system_clk_freq / adc_clk_freq)
	unsigned int larmor_clk_fact = 16;// the factor of (system_clk_freq / f_larmor)
	double SYSCLK_MHz = larmor_clk_fact * f_larmor;

	// data container
	unsigned int num_of_samples = samples_per_echo * echoes_per_scan;
	uint32_t adc_data_32b[num_of_samples >> 1];// data for 1 acquisition. Every transfer has 2 data, so the container is divided by 2
	uint16_t adc_data_16b[num_of_samples];

	// init
	init();

	// reset
	bstream_rst();

	// set phase increment
	alt_write_word( ( h2p_ph_inc_addr ), 1 << ( NCO_PH_RES - 4 ));

	// set phase overlap
	alt_write_word( ( h2p_ph_overlap_addr ), ( uint16_t )(1 << ( NCO_AMP_RES - 4 )));

	// set phase base
	// calculate phase from the phase resolution of the NCO
	unsigned int ph_base_num = 4;
	unsigned int ph0, ph90, ph180, ph270;
	ph0 = ph_base_num;// phase 0
	ph90 = 1 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;// phase 90. 1<<(NCO_PH_RES-2) is the bit needs to be changed to get 90 degrees.
	ph180 = 2 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;// phase 180.
	ph270 = 3 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;// phase 270.
	alt_write_word( ( h2p_ph_0_to_3_addr ), ( ph0 << 24 ) | ( ph90 << 16 ) | ( ph180 << 8 ) | ( ph270 ));// program phase 0 to phase 3
	alt_write_word( ( h2p_ph_4_to_7_addr ), ( ph0 << 24 ) | ( ph0 << 16 ) | ( ph0 << 8 ) | ( ph0 ));// program phase 4 to phase 7

	// program the clock for the ADC
	Set_PLL(h2p_sys_pll_reconfig_addr, 0, f_larmor * 4, 0.5, DISABLE_MESSAGE);
	Reset_PLL(h2p_general_cnt_out_addr, SYS_PLL_RST_ofst, cnt_out_val);
	Set_DPS(h2p_sys_pll_reconfig_addr, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, sys_pll_locked_ofst);

	// initialize ADC
	read_adc_id();
	init_adc(AD9276_OUT_ADJ_TERM_100OHM_VAL, AD9276_OUT_PHS_180DEG_VAL, AD9276_OUT_TEST_OFF_VAL, 0, 0);

	// write the preamp dac
	wr_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_B, vvarac, DAC_PAMP_LDAC, ENABLE_MESSAGE);// set -2.5 for 4 MHz resonant

	usleep(1000);// wait for the PLL FCO to lock as well

	bstream__vpc_chg(
			SYSCLK_MHz,
			bstrap_pchg_us,
			lcs_vpc_pchg_us,// precharging of vpc
			lcs_recycledump_us,// dumping the lcs to the vpc
			lcs_vpc_pchg_repeat// repeat the precharge and dump
	);
	usleep(T_BLANK / ( SYSCLK_MHz ));// wait for T_BLANK as the last bitstream is not being counted in on bitstream code

	error_code err = bstream__cpmg(f_larmor,
			larmor_clk_fact,
			adc_clk_fact,
			bstrap_pchg_us,
			lcs_pchg_us,// precharging of vpc
			lcs_dump_us,// dumping the lcs to the vpc
			p90_pchg_us,
			p90_pchg_refill_us,
			p90_us,
			p90_dchg_us,// the discharging length of the current source inductor
			p90_dtcl,
			p180_pchg_us,
			p180_pchg_refill_us,
			p180_us,
			p180_dchg_us,// the discharging length of the current source inductor
			p180_dtcl,
			echoshift_us,// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
			echotime_us,
			samples_per_echo,
			echoes_per_scan,
			p90_ph_sel,
			dconv_fact,
			echoskip,
			echodrop
	);
	if (err == SEQ_ERROR) {
		return 0;
	}
	usleep(T_BLANK / ( SYSCLK_MHz ));   // wait for T_BLANK as the last bitstream is not being counted in on bitstream code

	bstream__vpc_wastedump(
			SYSCLK_MHz,
			bstrap_pchg_us,
			lcs_vpc_dchg_us,// discharging of vpc
			lcs_wastedump_us,// dumping the current into RF
			lcs_vpc_dchg_repeat// repeat the precharge and dump
	);
	usleep(T_BLANK / ( SYSCLK_MHz ));// wait for T_BLANK as the last bitstream is not being counted in on bitstream code

	read_adc_val(h2p_fifo_sink_ch_a_csr_addr, h2p_fifo_sink_ch_a_data_addr, adc_data_32b);
	buf32_to_buf16(adc_data_32b, adc_data_16b, num_of_samples >> 1);// convert the 32-bit data format to 16-bit.
	cut_2MSB_and_2LSB(adc_data_16b, num_of_samples);// cut the 2 MSB and 2 LSB (check signalTap for the details). The data is valid only at bit-2 to bit-13.
	wr_File_16b("data.txt", num_of_samples, adc_data_16b, SAV_ASCII);// write the data to the filename

	leave();

	return 0;
}

#endif
