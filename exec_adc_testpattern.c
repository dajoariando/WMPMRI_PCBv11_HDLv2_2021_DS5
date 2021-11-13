// PUT ATTENTION to adc_data in read_adc_val function. It really depends on implementation of verilog
// It was found that the data captured by signal tap logic analyzer is delayed by 2 DCO clock cycles, therefore data needs to be shifted by two
// This might not be the case with different FPGA implementation!

// #define EXEC_ADC_TESTPATTERN // uncomment this line to enable the whole code
#ifdef EXEC_ADC_TESTPATTERN

#include "hps_linux.h"

// parameters
unsigned int num_of_samples = 1000;
unsigned int cnt_out_val;

void init(double adc_freq) {
	printf("TEST PATTERN STARTED!\n");

	// open device memory
	fd_dev_mem = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd_dev_mem == -1) {
		printf("ERROR: could not open \"/dev/mem\".\n");
		printf("    errno = %s\n", strerror(errno));
		exit (EXIT_FAILURE);
	}

	// mmap hps peripherals
	hps_gpio = mmap(NULL, hps_gpio_span, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_mem, hps_gpio_ofst);
	if (hps_gpio == MAP_FAILED) {
		printf("Error: hps_gpio mmap() failed.\n");
		printf("    errno = %s\n", strerror(errno));
		close (fd_dev_mem);
		exit (EXIT_FAILURE);
	}

	// mmap fpga peripherals
	h2f_lw_axi_master = mmap(NULL, h2f_lw_axi_master_span, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_mem, h2f_lw_axi_master_ofst);
	if (h2f_lw_axi_master == MAP_FAILED) {
		printf("Error: h2f_lw_axi_master mmap() failed.\n");
		printf("    errno = %s\n", strerror(errno));
		close (fd_dev_mem);
		exit (EXIT_FAILURE);
	}

	h2p_adcspi_addr = h2f_lw_axi_master + AD9276_SPI_BASE;
	h2p_fifo_sink_ch_a_csr_addr = h2f_lw_axi_master + FIFO_SINK_CH_A_OUT_CSR_BASE;
	h2p_fifo_sink_ch_a_data_addr = h2f_lw_axi_master + FIFO_SINK_CH_A_OUT_BASE;
	h2p_fifo_sink_ch_b_csr_addr = h2f_lw_axi_master + FIFO_SINK_CH_B_OUT_CSR_BASE;
	h2p_fifo_sink_ch_b_data_addr = h2f_lw_axi_master + FIFO_SINK_CH_B_OUT_BASE;
	h2p_fifo_sink_ch_c_csr_addr = h2f_lw_axi_master + FIFO_SINK_CH_C_OUT_CSR_BASE;
	h2p_fifo_sink_ch_c_data_addr = h2f_lw_axi_master + FIFO_SINK_CH_C_OUT_BASE;
	h2p_fifo_sink_ch_d_csr_addr = h2f_lw_axi_master + FIFO_SINK_CH_D_OUT_CSR_BASE;
	h2p_fifo_sink_ch_d_data_addr = h2f_lw_axi_master + FIFO_SINK_CH_D_OUT_BASE;
	h2p_fifo_sink_ch_e_csr_addr = h2f_lw_axi_master + FIFO_SINK_CH_E_OUT_CSR_BASE;
	h2p_fifo_sink_ch_e_data_addr = h2f_lw_axi_master + FIFO_SINK_CH_E_OUT_BASE;
	h2p_fifo_sink_ch_f_csr_addr = h2f_lw_axi_master + FIFO_SINK_CH_F_OUT_CSR_BASE;
	h2p_fifo_sink_ch_f_data_addr = h2f_lw_axi_master + FIFO_SINK_CH_F_OUT_BASE;
	h2p_fifo_sink_ch_g_csr_addr = h2f_lw_axi_master + FIFO_SINK_CH_G_OUT_CSR_BASE;
	h2p_fifo_sink_ch_g_data_addr = h2f_lw_axi_master + FIFO_SINK_CH_G_OUT_BASE;
	h2p_fifo_sink_ch_h_csr_addr = h2f_lw_axi_master + FIFO_SINK_CH_H_OUT_CSR_BASE;
	h2p_fifo_sink_ch_h_data_addr = h2f_lw_axi_master + FIFO_SINK_CH_H_OUT_BASE;
	h2p_led_addr = h2f_lw_axi_master + LED_PIO_BASE;
	h2p_sw_addr = h2f_lw_axi_master + DIPSW_PIO_BASE;
	h2p_button_addr = h2f_lw_axi_master + BUTTON_PIO_BASE;
	h2p_adc_samples_addr = h2f_lw_axi_master + ADC_SAMPLES_BASE;
	h2p_init_delay_addr = h2f_lw_axi_master + ADC_INIT_DELAY_BASE;
	h2p_general_cnt_int_addr = h2f_lw_axi_master + GENERAL_CNT_IN_BASE;
	h2p_general_cnt_out_addr = h2f_lw_axi_master + GENERAL_CNT_OUT_BASE;
	h2p_adc_start_pulselength_addr = h2f_lw_axi_master + ADC_START_PULSELENGTH_BASE;
	h2p_pulse_adc_reconfig = h2f_lw_axi_master + ADC_PLL_RECONFIG_BASE;

	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;

	// turn on the ADC
	cnt_out_val &= ~ADC_AD9276_STBY_msk;
	cnt_out_val &= ~ADC_AD9276_PWDN_msk;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100000);

	// FSM reset
	cnt_out_val |= ( FSM_RESET_msk );
	alt_write_word(h2p_general_cnt_out_addr, cnt_out_val);
	usleep(10);
	cnt_out_val &= ~ ( FSM_RESET_msk );
	alt_write_word(h2p_general_cnt_out_addr, cnt_out_val);
	usleep(300000);

	// set pll settings for adc pll
	Reconfig_Mode(h2p_pulse_adc_reconfig, 1);// polling mode for main pll
	Set_PLL(h2p_pulse_adc_reconfig, 0, adc_freq, 0.5, DISABLE_MESSAGE);
	// Reset_PLL (h2p_ctrl_out_addr, PLL_NMR_SYS_RST_ofst, ctrl_out);
	Set_DPS(h2p_pulse_adc_reconfig, 0, 0, DISABLE_MESSAGE);
	Wait_PLL_To_Lock(h2p_general_cnt_int_addr, adc_pll_locked_ofst);

}

void leave() {

	// turn of the ADC
	cnt_out_val |= ADC_AD9276_STBY_msk;
	cnt_out_val |= ADC_AD9276_PWDN_msk;// (CAREFUL! SOMETIMES THE ADC CANNOT WAKE UP AFTER PUT TO PWDN)
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100);

	close (fd_dev_mem);

	// munmap hps peripherals
	if (munmap(hps_gpio, hps_gpio_span) != 0) {
		printf("Error: hps_gpio munmap() failed\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit (EXIT_FAILURE);
	}

	hps_gpio = NULL;

	if (munmap(h2f_lw_axi_master, h2f_lw_axi_master_span) != 0) {
		printf("Error: h2f_lw_axi_master munmap() failed\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit (EXIT_FAILURE);
	}

	h2f_lw_axi_master = NULL;
	h2p_led_addr = NULL;
	h2p_sw_addr = NULL;
	h2p_fifo_sink_ch_a_csr_addr = NULL;
	h2p_fifo_sink_ch_a_data_addr = NULL;
	h2p_fifo_sink_ch_b_csr_addr = NULL;
	h2p_fifo_sink_ch_b_data_addr = NULL;
	h2p_fifo_sink_ch_c_csr_addr = NULL;
	h2p_fifo_sink_ch_d_data_addr = NULL;
	h2p_fifo_sink_ch_d_csr_addr = NULL;
	h2p_fifo_sink_ch_d_data_addr = NULL;
	h2p_fifo_sink_ch_e_csr_addr = NULL;
	h2p_fifo_sink_ch_e_data_addr = NULL;
	h2p_fifo_sink_ch_f_csr_addr = NULL;
	h2p_fifo_sink_ch_f_data_addr = NULL;
	h2p_fifo_sink_ch_g_csr_addr = NULL;
	h2p_fifo_sink_ch_g_data_addr = NULL;
	h2p_fifo_sink_ch_h_csr_addr = NULL;
	h2p_fifo_sink_ch_h_data_addr = NULL;
	h2p_led_addr = NULL;
	h2p_sw_addr = NULL;
	h2p_button_addr = NULL;
	h2p_adc_samples_addr = NULL;
	h2p_init_delay_addr = NULL;
	h2p_general_cnt_int_addr = NULL;
	h2p_general_cnt_out_addr = NULL;
	h2p_adcspi_addr = NULL;

	printf("\nTEST PATTERN STOPPED!\n");
}

int main(int argc, char * argv[]) {

	// example parameter: 100 40 2 10 1 0

	double adc_init_delay_us = atof(argv[1]);
	double adc_freq = atof(argv[2]);
	uint8_t lvds_z = atoi(argv[3]);// set the lvds output impedance (number 0 to 3)
	uint8_t lvds_phase = atoi(argv[4]);// set the lvds phase (number 0 to 15)
	unsigned int val1 = atoi(argv[5]);// write value for pattern 1 (being used by default)
	unsigned int val2 = atoi(argv[6]);// write value for pattern 2 (usually is not being used)

	uint32_t adc_data_32b[num_of_samples >> 1];// data for 1 acquisition. Every transfer has 2 data, so the container is divided by 2
	uint16_t adc_data_16b[num_of_samples];
	usleep(200000);

// Initialize system
	init(adc_freq);

	read_adc_id();
	init_adc(lvds_z, lvds_phase);
	adc_wr_testval(val1, val2);

// adc parameter
	alt_write_word(h2p_adc_start_pulselength_addr, 10);// the length of ADC_START pulse
	alt_write_word(h2p_adc_samples_addr, num_of_samples);// number of ADC samples
	alt_write_word(h2p_init_delay_addr, us_to_clk_cycles(adc_init_delay_us, adc_freq));// the ADC init delay

// tx_enable fire
	cnt_out_val |= BF_TX_EN_msk;
	alt_write_word(h2p_general_cnt_out_addr, cnt_out_val);// start the sequence
	cnt_out_val &= ( ~BF_TX_EN_msk );
	alt_write_word(h2p_general_cnt_out_addr, cnt_out_val);// stop the sequence

	while (! ( alt_read_word(h2p_general_cnt_int_addr) & FSM_DONE_msk ))
	;
	usleep(100000);// this delay is important, otherwise the data will be read before it's ready

	read_adc_val(h2p_fifo_sink_ch_a_csr_addr, h2p_fifo_sink_ch_a_data_addr, adc_data_32b);
// read_adc_val(h2p_fifo_sink_ch_b_csr_addr, h2p_fifo_sink_ch_b_data_addr, adc_data);
// read_adc_val(h2p_fifo_sink_ch_c_csr_addr, h2p_fifo_sink_ch_c_data_addr, adc_data);
// read_adc_val(h2p_fifo_sink_ch_d_csr_addr, h2p_fifo_sink_ch_d_data_addr, adc_data);
// read_adc_val(h2p_fifo_sink_ch_e_csr_addr, h2p_fifo_sink_ch_e_data_addr, adc_data);
// read_adc_val(h2p_fifo_sink_ch_f_csr_addr, h2p_fifo_sink_ch_f_data_addr, adc_data);
// read_adc_val(h2p_fifo_sink_ch_g_csr_addr, h2p_fifo_sink_ch_g_data_addr, adc_data);
// read_adc_val(h2p_fifo_sink_ch_h_csr_addr, h2p_fifo_sink_ch_h_data_addr, adc_data);

	buf32_to_buf16(adc_data_32b, adc_data_16b, num_of_samples >> 1);// convert the 32-bit data format to 16-bit.
	wr_File("data.txt", num_of_samples, adc_data_16b, SAV_ASCII);// write the data to the filename

// exit program
	leave();

	return 0;
}

#endif
