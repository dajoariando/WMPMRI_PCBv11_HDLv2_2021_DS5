// Created on: August 6th 2024
// Author: David Ariando
// This sequence takes multichannel ADC data and interleaves it in this order (example with 3 channel):
// d0_ch0 - d0_ch1 - d0_ch2 - d1_ch0 - d1_ch1 - d1_ch2

// #define EXEC_NOISE_MULTCH
#ifdef EXEC_NOISE_MULTCH

#define GET_RAW_DATA

#include "hps_linux.h"

// bitstream objects
bstream_obj bstream_objs[BSTREAM_COUNT];

void init() {
	printf("START:: EXEC_NOISE\n");

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

	printf("STOP:: EXEC_NOISE\n");
}

int main(int argc, char * argv[]) {

	// param defined by user
	double f_adc = atof(argv[1]);
	unsigned int samples_per_channel = atoi(argv[2]);
	double vvarac = atof(argv[3]);
	unsigned int adc_mode = atoi(argv[4]);
		// OFF_VAL				0
		// MIDSCL_SHRT_VAL		1
		// FSP_SHRT_VAL			2
		// FSN_SHRT_VAL			3
		// CHCKBOARD_VAL		4
		// PNSEQ_LONG_VAL		5
		// PNSEQ_SHRT_VAL		6
		// 10_WORDTOG_VAL		7
		// USR_INPUT_VAL		8
		// 10_BITTOG_VAL		9
		// 1xSYNC_VAL			10
		// 1BIT_HIGH_VAL		11
		// MIX_BITFREQ_VAL		12
	unsigned int val1 = atoi(argv[5]);
	unsigned int val2 = atoi(argv[6]);

	// measurement settings
	char rd_FIFO_or_DMA = RD_FIFO; // read data via FIFO or via DMA
	char adc_channel = 2; // the number of ADC channels

	// param defined by Quartus
	unsigned int adc_clk_fact = 4; // the factor of (system_clk_freq / adc_clk_freq)
	double SYSCLK_MHz = adc_clk_fact * f_adc;
	double ADCCLK_MHz = f_adc;

	// data container
	unsigned int samples_captured = samples_per_channel*adc_channel; // multiply by 8 because now there's 8 channels
	uint32_t adc_data_32b[samples_captured>>1]; // data for 1 acquisition. Every transfer has 2 data, so the container is divided by 2
	uint16_t adc_data_16b[samples_captured];

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
	Set_PLL(h2p_sys_pll_reconfig_addr, 0, ADCCLK_MHz, 0.5, DISABLE_MESSAGE);
	Reset_PLL(h2p_general_cnt_out_addr, SYS_PLL_RST_ofst, cnt_out_val);
	Set_DPS(h2p_sys_pll_reconfig_addr, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, sys_pll_locked_ofst);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, fco_locked_ofst);

	// initialize ADC
	// read_adc_id();
	// init_adc(AD9276_OUT_ADJ_TERM_100OHM_VAL, AD9276_OUT_PHS_180DEG_VAL, AD9276_OUT_TEST_OFF_VAL, 0, 0);
	init_adc(AD9276_OUT_ADJ_TERM_100OHM_VAL, AD9276_OUT_PHS_180DEG_VAL, adc_mode, val1, val2);

	// write the preamp dac
	wr_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_B, vvarac, DAC_PAMP_LDAC, ENABLE_MESSAGE);// set -2.5 for 4 MHz resonant

	usleep(1000);// wait for the PLL FCO to lock as well

	flush_adc_fifo(h2p_fifo_sink_ch_a_csr_addr, h2p_fifo_sink_ch_a_data_addr, ENABLE_MESSAGE);

	bstream__noise(f_adc, adc_clk_fact, samples_per_channel, NOWAIT); // use NOWAIT if read data from DMA

	usleep(T_BLANK / ( SYSCLK_MHz ));// wait for T_BLANK as the last bitstream is not being counted in on bitstream code

	// read data from the ADC into adc_data_32b
	if (rd_FIFO_or_DMA == RD_FIFO) {
		usleep(T_BLANK / ( SYSCLK_MHz ));	// wait for T_BLANK as the last bitstream is not being counted in on bitstream code
		read_adc_fifo(h2p_fifo_sink_ch_a_csr_addr, h2p_fifo_sink_ch_a_data_addr, adc_data_32b, DISABLE_MESSAGE);
		// flush_adc_fifo(h2p_fifo_sink_ch_a_csr_addr, h2p_fifo_sink_ch_a_data_addr, ENABLE_MESSAGE);
	}
	else if (rd_FIFO_or_DMA == RD_DMA) {
		read_adc_dma(h2p_dma_addr, axi_sdram_addr, DMA_READ_MASTER_FIFO_SINK_CH_A_BASE, DMA_WRITE_MASTER_SDRAM_BASE, adc_data_32b, samples_captured >> 1, DISABLE_MESSAGE);
	}
	buf32_to_buf16(adc_data_32b, adc_data_16b, samples_captured >> 1);// convert the 32-bit data format to 16-bit.
	cut_2MSB_and_2LSB(adc_data_16b, samples_captured);// cut the 2 MSB and 2 LSB (check signalTap for the details). The data is valid only at bit-2 to bit-13.

	// write noise acquisition
	wr_File_16b("noise.txt", samples_captured, adc_data_16b, SAV_ASCII);// write the data to the filename

	// print general measurement settings
	sprintf(acq_file, "acqu.par");
	fptr = fopen(acq_file, "w");
	fprintf(fptr, "adcFreq = %4.3f\n", ADCCLK_MHz);
	fprintf(fptr, "samples = %d\n", samples_per_channel);
	fprintf(fptr, "vvarac = %4.3f\n", vvarac);
	fprintf(fptr, "adc_channel = %d\n", adc_channel);
	fclose (fptr);
	//

	leave();

	return 0;
}

#endif
