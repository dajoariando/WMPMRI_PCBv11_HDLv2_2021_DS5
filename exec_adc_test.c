// Created on: April 14th, 2022
// Author: David Ariando

// #define EXEC_ADC_TEST
#ifdef EXEC_ADC_TEST

#include "hps_linux.h"

// bitstream objects
bstream_obj bstream_objs[BSTREAM_COUNT];

void init() {
	printf("ADC TEST STARTED!\n");

	soc_init();
	bstream__init_all_sram();

	// set polling mode for the main PLL
	Reconfig_Mode(h2p_sys_pll_reconfig_addr, 1);

	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;
	cnt_out_val &= ~ADC_AD9276_STBY_msk;// turn on the ADC
	cnt_out_val &= ~ADC_AD9276_PWDN_msk;// turn on the ADC
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
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

	// default params:

	double f_ADC = atof(argv[1]);// set the ADC frequency
	uint8_t lvds_z = atoi(argv[2]);// set the lvds output impedance (number 0 to 3). Default is 2.
	uint8_t lvds_phase = atoi(argv[3]);// set the lvds phase (number 0 to 15). Default is 3.
	uint32_t adc_mode = atoi(argv[4]);// set adc mode of operation (number 0 to 12). 0 is the default where the ADC takes data from the input. Set 8 for user mode.
	uint32_t num_of_samples = atoi(argv[5]);// the number of samples taken by the adc

	unsigned int val1;
	unsigned int val2;
	if (adc_mode == AD9276_OUT_TEST_USR_INPUT_VAL) {
		val1 = atoi(argv[6]);   // write value for pattern 1 (being used by default)
		val2 = atoi(argv[7]);// write value for pattern 2 (usually is not being used)
	}

	uint32_t adc_data_32b[num_of_samples >> 1];   // data for 1 acquisition. Every transfer has 2 data, so the container is divided by 2
	uint16_t adc_data_16b[num_of_samples];

	init();

	// program the nco
	alt_write_word( ( h2p_ph_inc_addr ), 1 << ( NCO_PH_RES - 4 ));// set phase increment
	alt_write_word( ( h2p_ph_overlap_addr ), (uint16_t) 16);// set phase overlap
	// set phase base
	unsigned int ph_base_num = 4;// the phase base so that it never go to phase accumulator of 0, otherwise the sin output will go to zero twice in a period giving higher duty cycle output as 0 is considered positive (MSB=0)
	unsigned int ph0, ph90, ph180, ph270;
	// calculate phase from the phase resolution of the NCO
	ph0 = ph_base_num;
	ph90 = 1 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;
	ph180 = 2 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;
	ph270 = 3 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;
	// write the phase output
	alt_write_word( ( h2p_ph_0_to_3_addr ), ( ph0 << 24 ) | ( ph90 << 16 ) | ( ph180 << 8 ) | ( ph270 ));// program phase 0 to phase 3
	alt_write_word( ( h2p_ph_4_to_7_addr ), ( ph0 << 24 ) | ( ph0 << 16 ) | ( ph0 << 8 ) | ( ph0 ));// program phase 4 to phase 7

	// reset
	bstream_rst();

	Set_PLL(h2p_sys_pll_reconfig_addr, 0, f_ADC, 0.5, DISABLE_MESSAGE);
	Reset_PLL(h2p_general_cnt_out_addr, SYS_PLL_RST_ofst, cnt_out_val);
	Set_DPS(h2p_sys_pll_reconfig_addr, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, sys_pll_locked_ofst);

	read_adc_id();
	// init_adc(AD9276_OUT_ADJ_TERM_100OHM_VAL, AD9276_OUT_PHS_600DEG_VAL, AD9276_OUT_TEST_OFF_VAL, 0, 0);
	init_adc(lvds_z, lvds_phase, adc_mode, val1, val2);
	read_adc_id();

	bstream__en_adc(f_ADC, num_of_samples);

	while (! ( alt_read_word(h2p_general_cnt_in_addr) & tx_h1_done ))
	;
	// usleep(100000);

	read_adc_val(h2p_fifo_sink_ch_a_csr_addr, h2p_fifo_sink_ch_a_data_addr, adc_data_32b);
	buf32_to_buf16(adc_data_32b, adc_data_16b, num_of_samples >> 1);// convert the 32-bit data format to 16-bit.
	wr_File("data.txt", num_of_samples, (int*) adc_data_16b, SAV_ASCII);// write the data to the filename

	leave();

	return 0;
}

#endif
