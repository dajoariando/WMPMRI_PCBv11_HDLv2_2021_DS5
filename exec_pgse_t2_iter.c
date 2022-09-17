// Created on: April 14th, 2022
// Author: David Ariando

#define EXEC_PGSE_T2_ITER
#ifdef EXEC_PGSE_T2_ITER

#define GET_RAW_DATA

#include "hps_linux.h"

// bitstream objects
bstream_obj bstream_objs[BSTREAM_COUNT];

void init() {
	printf("START::EXEC_PGSE_T2_ITER\n");

	soc_init();
	bstream__init_all_sram();

	// set polling mode for the main PLL
	Reconfig_Mode(h2p_sys_pll_reconfig_addr, 1);

	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;

	// turn off the ADC (sometimes the ADC is in undefined state during startup and failed to start without turned off first)
	cnt_out_val |= ADC_AD9276_STBY_msk;
	cnt_out_val |= ADC_AD9276_PWDN_msk;   // (CAREFUL! SOMETIMES THE ADC CANNOT WAKE UP AFTER PUT TO PWDN)
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	// turn on the ADC
	cnt_out_val &= ~ADC_AD9276_STBY_msk;   // turn on the ADC
	cnt_out_val &= ~ADC_AD9276_PWDN_msk;   // turn on the ADC
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	// init the DAC
	init_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_PAMP_CLR);
	usleep(100000);

}

void leave() {

	// turn off the ADC
	cnt_out_val |= ADC_AD9276_STBY_msk;
	cnt_out_val |= ADC_AD9276_PWDN_msk;   // (CAREFUL! SOMETIMES THE ADC CANNOT WAKE UP AFTER PUT TO PWDN)
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	soc_exit();

	printf("STOP::EXEC_PGSE_T2_ITER\n");
}

int main(int argc, char * argv[]) {

	// param defined by user
	double f_larmor = atof(argv[1]);
	double bstrap_pchg_us = atof(argv[2]);   // bootstrap circuit precharge by enabling both lower side FETs. Has to be done at the beginning to make sure the high-side circuit is charged
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
	uint8_t ph_cycl_en = atoi(argv[21]);
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
	double gradlen_us = atof(argv[32]);   // gradient length
	double gradspac_us = atof(argv[33]);   // gradient spacing
	float gradz_volt = atof(argv[34]);   // gradient z dac positive voltage

	// measurement settings
	char wr_indv_scan = 0;   // write individual scan to file
	unsigned char rd_FIFO_or_DMA = RD_DMA;   // data source : RD_FIFO or RD_DMA
	unsigned char wait_til_done;   // wait for done signal from the bitstream
	if (rd_FIFO_or_DMA == RD_DMA) {
		wait_til_done = NOWAIT;
	}
	else if (rd_FIFO_or_DMA == RD_FIFO) {
		wait_til_done = WAIT;
	}

	// param defined by Quartus
	unsigned int adc_clk_fact = 4;   // the factor of (system_clk_freq / adc_clk_freq)
	unsigned int larmor_clk_fact = 16;   // the factor of (system_clk_freq / f_larmor)
	double SYSCLK_MHz = larmor_clk_fact * f_larmor;
	double ADCCLK_MHz = adc_clk_fact * f_larmor;

	// data container
	unsigned int num_of_samples = samples_per_echo * echoes_per_scan;
	uint32_t adc_data_32b[num_of_samples >> 1];   // data for 1 acquisition. Every transfer has 2 data, so the container is divided by 2
	uint16_t adc_data_16b[num_of_samples];
	int32_t adc_data_sum[num_of_samples];   // sum of the data

	float gradz_voltp, gradz_voltn;   // gradient voltage to program dac
	char graddir;   // the gradient direction

	// init
	init();

	// reset
	bstream_rst();

	// set the gradient voltage
	gradz_voltp = ( gradz_volt > 0 ) ? gradz_volt : 0;
	gradz_voltn = ( gradz_volt < 0 ) ? ( -gradz_volt ) : 0;
	graddir = ( gradz_volt > 0 ) ? 1 : 0;   // set the direction to positive if gradz_volt > 0
	dac5571_i2c_wr(h2p_dac_graddrv_addr, gradz_voltp, gradz_voltn, DISABLE_MESSAGE);

	// set phase increment
	alt_write_word( ( h2p_ph_inc_addr ), 1 << ( NCO_PH_RES - 4 ));

	// set phase overlap
	alt_write_word( ( h2p_ph_overlap_addr ), ( uint16_t )(1 << ( NCO_AMP_RES - 4 )));

	// set phase base
	// calculate phase from the phase resolution of the NCO
	unsigned int ph_base_num = 4;
	unsigned int ph0, ph90, ph180, ph270;
	ph0 = ph_base_num;   // phase 0
	ph90 = 1 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;	// phase 90. 1<<(NCO_PH_RES-2) is the bit needs to be changed to get 90 degrees.
	ph180 = 2 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;   // phase 180.
	ph270 = 3 * ( 1 << ( NCO_PH_RES - 2 ) ) + ph_base_num;   // phase 270.
	alt_write_word( ( h2p_ph_0_to_3_addr ), ( ph0 << 24 ) | ( ph90 << 16 ) | ( ph180 << 8 ) | ( ph270 ));	// program phase 0 to phase 3
	alt_write_word( ( h2p_ph_4_to_7_addr ), ( ph0 << 24 ) | ( ph0 << 16 ) | ( ph0 << 8 ) | ( ph0 ));   // program phase 4 to phase 7

	// program the clock for the ADC
	Set_PLL(h2p_sys_pll_reconfig_addr, 0, f_larmor * 4, 0.5, DISABLE_MESSAGE);
	Reset_PLL(h2p_general_cnt_out_addr, SYS_PLL_RST_ofst, cnt_out_val);
	Set_DPS(h2p_sys_pll_reconfig_addr, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_in_addr, sys_pll_locked_ofst);

	// initialize ADC
	// read_adc_id();
	init_adc(AD9276_OUT_ADJ_TERM_100OHM_VAL, AD9276_OUT_PHS_180DEG_VAL, AD9276_OUT_TEST_OFF_VAL, 0, 0);

	// write the preamp dac
	wr_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_B, vvarac, DAC_PAMP_LDAC, DISABLE_MESSAGE);   // set -2.5 for 4 MHz resonant

	usleep(1000);	// wait for the PLL FCO to lock as well

	bstream__vpc_chg(
	        SYSCLK_MHz,
	        bstrap_pchg_us,
	        lcs_vpc_pchg_us,   // precharging of vpc
	        lcs_recycledump_us,   // dumping the lcs to the vpc
	        lcs_vpc_pchg_repeat   // repeat the precharge and dump
	        );
	usleep(T_BLANK / ( SYSCLK_MHz ));	// wait for T_BLANK as the last bitstream is not being counted in on bitstream code

	// flush the adc fifo and check if there's remaining data in the fifo and generate warning message.
	int flushed_data = 0;
	flushed_data = flush_adc_fifo(h2p_fifo_sink_ch_a_csr_addr, h2p_fifo_sink_ch_a_data_addr, DISABLE_MESSAGE);
	if (flushed_data > 0) {
		printf("\tWARNING! Flushed data = %d\n", flushed_data);
	}

	clock_t start, end;
	double net_acq_time, net_elapsed_time, net_scan_time;
	unsigned char p90_ph_sel = 1;	// set phase to 90 degrees
	unsigned int ii;
	char dataname[10];
	cpmg_obj cpmg_params;
	for (ii = 0; ii < n_iterate; ii++) {
		// measure the start time
		start = clock();   // measure time

		cpmg_params = bstream__pgse(f_larmor,
		        larmor_clk_fact,
		        adc_clk_fact,
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
		        samples_per_echo,
		        echoes_per_scan,
		        p90_ph_sel,
		        dconv_fact,
		        echoskip,
		        echodrop,
		        graddir,
		        gradlen_us,
		        gradspac_us,
		        wait_til_done
		        );

		// read data from the ADC into adc_data_32b
		if (rd_FIFO_or_DMA == RD_FIFO) {
			usleep(T_BLANK / ( SYSCLK_MHz ));	// wait for T_BLANK as the last bitstream is not being counted in on bitstream code
			read_adc_fifo(h2p_fifo_sink_ch_a_csr_addr, h2p_fifo_sink_ch_a_data_addr, adc_data_32b, DISABLE_MESSAGE);
		}
		else if (rd_FIFO_or_DMA == RD_DMA) {
			read_adc_dma(h2p_dma_addr, axi_sdram_addr, DMA_READ_MASTER_FIFO_SINK_CH_A_BASE, DMA_WRITE_MASTER_SDRAM_BASE, adc_data_32b, num_of_samples >> 1, DISABLE_MESSAGE);
		}
		buf32_to_buf16(adc_data_32b, adc_data_16b, num_of_samples >> 1);   // convert the 32-bit data format to 16-bit.
		cut_2MSB_and_2LSB(adc_data_16b, num_of_samples);   // cut the 2 MSB and 2 LSB (check signalTap for the details). The data is valid only at bit-2 to bit-13.

		// calculate echosum
		sum_buf(adc_data_sum, adc_data_16b, num_of_samples, p90_ph_sel >> 1);	// if p90_ph_sel == 3, subtract the data. If p90_ph_sel = 1, sum the data.

		// toggle phase cycling
		if (ph_cycl_en) {
			if (p90_ph_sel == 1) {
				p90_ph_sel = 3;   // set phase to 270 degrees
			}
			else {
				p90_ph_sel = 1;   // set phase to 90 degrees
			}
		}

		// write individual scan
		if (wr_indv_scan) {
			sprintf(dataname, "data_%03d.txt", ii);   // create a filename
			wr_File_16b(dataname, num_of_samples, adc_data_16b, SAV_ASCII);   // write the data to the filename
		}

		// measure elapsed time after acquisition
		end = clock();   // measure time
		net_acq_time = ( (double) ( end - start ) ) * 1000000 / CLOCKS_PER_SEC;   // measure time in us
		if (DISABLE_MESSAGE) {
			printf("\t Elapsed time after data acquisition is %ld us.\n", (unsigned long) net_acq_time);
		}

		// add delay according to the given scan_spacing_us
		end = clock();   // measure time
		net_elapsed_time = ( (double) ( end - start ) ) * 1000000 / CLOCKS_PER_SEC;   // measure time in us
		if ((unsigned long) net_elapsed_time < scanspacing_us) {
			while ((unsigned long) net_elapsed_time < scanspacing_us) {
				end = clock();   // measure time
				net_elapsed_time = ( (double) ( end - start ) ) * 1000000 / CLOCKS_PER_SEC;   // measure time in us
			}

			if (DISABLE_MESSAGE) {
				printf("\t Added %0.0f us at the end of the scan to account for scan_spacing_us.\n", net_elapsed_time - net_acq_time);
			}
		}

		else   // scan duration is already longer than the scan_spacing_us parameter
		{
			printf("\t[WARNING] One scan duration is longer than scan_spacing_us parameter (%ld us) and is measured to be approx. %ld us\n", scanspacing_us, (unsigned long) net_acq_time);
		}
	}

	bstream__vpc_wastedump(
	        SYSCLK_MHz,
	        bstrap_pchg_us,
	        lcs_vpc_dchg_us,   // discharging of vpc
	        lcs_wastedump_us,   // dumping the current into RF
	        lcs_vpc_dchg_repeat   // repeat the precharge and dump
	        );
	usleep(T_BLANK / ( SYSCLK_MHz ));   // wait for T_BLANK as the last bitstream is not being counted in on bitstream code

// write the data output
	avg_buf(adc_data_sum, num_of_samples, n_iterate);   // divide the sum data by the averaging factor
	wr_File_32b("datasum.txt", num_of_samples, adc_data_sum, SAV_ASCII);   // write the data to the filename

// print general measurement settings
	sprintf(acq_file, "acqu.par");
	fptr = fopen(acq_file, "w");
	fprintf(fptr, "b1Freq = %4.3f\n", f_larmor);

	fprintf(fptr, "p90LengthGiven = %4.3f\n", p90_us);
	fprintf(fptr, "p90LengthRun = %4.3f\n", (double) cpmg_params.p90_int / SYSCLK_MHz);
	fprintf(fptr, "p90LengthCnt = %d @ %4.3f MHz\n", cpmg_params.p90_int, SYSCLK_MHz);
	fprintf(fptr, "d90LengthRun = %4.3f\n", (double) cpmg_params.d90_int / SYSCLK_MHz);
	fprintf(fptr, "d90LengthCnt = %d @ %4.3f MHz\n", cpmg_params.d90_int, SYSCLK_MHz);
	fprintf(fptr, "p180LengthGiven = %4.3f\n", p180_us);
	fprintf(fptr, "p180LengthRun = %4.3f\n", (double) cpmg_params.p180_int / SYSCLK_MHz);
	fprintf(fptr, "p180LengthCnt =  %d @ %4.3f MHz\n", cpmg_params.p180_int, SYSCLK_MHz);
	fprintf(fptr, "d180LengthRun = %4.3f\n", (double) cpmg_params.d180_int / SYSCLK_MHz);
	fprintf(fptr, "d180LengthCnt = %d @ %4.3f MHz\n", cpmg_params.d180_int, SYSCLK_MHz);
//fprintf(fptr,"p90_dtcl = %4.3f\n", pulse1_dtcl);
//fprintf(fptr,"p180_dtcl = %4.3f\n", pulse2_dtcl);
	fprintf(fptr, "echoTimeGiven = %4.3f\n", echotime_us);
	fprintf(fptr, "echoTimeRun = %4.3f\n", (double) cpmg_params.echotime_int / SYSCLK_MHz);
	fprintf(fptr, "ieTime = %lu\n", scanspacing_us / 1000);
	fprintf(fptr, "nrPnts = %d\n", samples_per_echo);
	fprintf(fptr, "nrEchoes = %d\n", echoes_per_scan);
	fprintf(fptr, "echoShift = %4.3f\n", echoshift_us);
	fprintf(fptr, "nrIterations = %d\n", n_iterate);
	fprintf(fptr, "dummyEchoes = 0\n");
	fprintf(fptr, "adcFreq = %4.3f\n", ADCCLK_MHz);
	fprintf(fptr, "usePhaseCycle = %d\n", ph_cycl_en);
	fprintf(fptr, "echoSkipHw = %d\n", 1);
	fprintf(fptr, "gradZ_Volt = %4.3f\n", gradz_volt);
	fprintf(fptr, "gradLen = %4.3f\n", gradlen_us);
	fprintf(fptr, "gradSpac = %4.3f\n", gradspac_us);
#ifdef GET_RAW_DATA
	fprintf(fptr, "dwellTime = %4.3f\n", 1 / ADCCLK_MHz);
	fprintf(fptr, "fpgaDconv = 0\n");
	fprintf(fptr, "dconvFact = 1\n");
#endif
#ifdef GET_DCONV_DATA
	fprintf(fptr, "dwellTime = %4.3f\n", 1 / ADCCLK_MHz*dconv_fact);
	fprintf(fptr, "fpgaDconv = 1\n");
	fprintf(fptr,"dconvFact = %d\n", dconv_fact);
#endif
	fclose (fptr);

	leave();

	return 0;
}

#endif
