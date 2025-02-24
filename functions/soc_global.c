/*
 * soc_global.c
 *
 *  Created on: Nov 22, 2021
 *      Author: David Ariando
 */

#include <socal/hps.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "../variables/hps_soc_system.h"

// soc hps addresses (copy from the hps_linux.h)
extern void *h2p_fifo_sink_ch_a_data_addr;
extern void *h2p_fifo_sink_ch_b_data_addr;
extern void *h2p_fifo_sink_ch_c_data_addr;
extern void *h2p_fifo_sink_ch_d_data_addr;
extern void *h2p_fifo_sink_ch_e_data_addr;
extern void *h2p_fifo_sink_ch_f_data_addr;
extern void *h2p_fifo_sink_ch_g_data_addr;
extern void *h2p_fifo_sink_ch_h_data_addr;
extern void *h2p_led_addr;
extern void *h2p_sw_addr;
extern void *h2p_button_addr;
extern volatile unsigned int *h2p_adcspi_addr;   // gpio for dac (spi)
extern volatile unsigned int *h2p_fifo_sink_ch_a_csr_addr;   // ADC streaming FIFO status address
extern volatile unsigned int *h2p_fifo_sink_ch_b_csr_addr;   // ADC streaming FIFO status address
extern volatile unsigned int *h2p_fifo_sink_ch_c_csr_addr;   // ADC streaming FIFO status address
extern volatile unsigned int *h2p_fifo_sink_ch_d_csr_addr;   // ADC streaming FIFO status address
extern volatile unsigned int *h2p_fifo_sink_ch_e_csr_addr;   // ADC streaming FIFO status address
extern volatile unsigned int *h2p_fifo_sink_ch_f_csr_addr;   // ADC streaming FIFO status address
extern volatile unsigned int *h2p_fifo_sink_ch_g_csr_addr;   // ADC streaming FIFO status address
extern volatile unsigned int *h2p_fifo_sink_ch_h_csr_addr;   // ADC streaming FIFO status address
extern volatile unsigned int *h2p_adc_samples_addr;
extern volatile unsigned int *h2p_init_delay_addr;
extern volatile unsigned int *h2p_general_cnt_in_addr;
extern volatile unsigned int *h2p_general_cnt_out_addr;
// extern volatile unsigned int *h2p_adc_start_pulselength_addr;
// extern void *h2p_pulse_adc_reconfig;
extern volatile unsigned int *h2p_dac_grad_spi_addr;

extern volatile unsigned int * h2p_dma_addr;   // dma control address
extern volatile unsigned int *axi_sdram_addr;   // sdram access via axi

// memory map peripherals for bitstream codes. Also connect the bitstream object and ram in function bstream__init_all_sram() inside bstream.c
extern volatile unsigned int *axi_ram_tx_h1;
extern volatile unsigned int *axi_ram_tx_l1;
extern volatile unsigned int *axi_ram_tx_clkph;
extern volatile unsigned int *axi_ram_tx_h2;
extern volatile unsigned int *axi_ram_tx_l2;
extern volatile unsigned int *axi_ram_tx_charge;
extern volatile unsigned int *axi_ram_tx_charge_bs;
extern volatile unsigned int *axi_ram_tx_dump;
extern volatile unsigned int *axi_ram_rx_adc_en;
extern volatile unsigned int *axi_ram_rx_in_short;
extern volatile unsigned int *axi_ram_gradY_Lo_R;
extern volatile unsigned int *axi_ram_gradY_Lo_L;
extern volatile unsigned int *axi_ram_gradX_Lo_R;
extern volatile unsigned int *axi_ram_gradX_Lo_L;
extern volatile unsigned int *axi_ram_grad_hside_en;
extern volatile unsigned int *axi_ram_aux;

// pll reconfig address for the bitstream
// extern volatile unsigned int *h2p_bstream_pll_addr;   // bitstream pll reconfig

// pll reconfig address for the system clock
extern volatile unsigned int *h2p_sys_pll_reconfig_addr;

// nco
extern volatile unsigned int *h2p_ph_overlap_addr;   // the nco phase overlap address
extern volatile unsigned int *h2p_ph_inc_addr;   // the nco phase increment address
extern volatile unsigned int *h2p_ph_0_to_3_addr;   // the nco phase modulator
extern volatile unsigned int *h2p_ph_4_to_7_addr;   // the nco phase modulator

// preamplifier
extern volatile unsigned int *h2p_dac_preamp_addr;   // the preamp dac

// gradient driver
extern volatile unsigned int *h2p_dac_grad_addr;

// physical memory file descriptor
int fd_dev_mem = 0;

// memory-mapped peripherals
void *hps_gpio = NULL;
size_t hps_gpio_span = ALT_GPIO1_UB_ADDR - ALT_GPIO1_LB_ADDR + 1;
size_t hps_gpio_ofst = ALT_GPIO1_OFST;

void *h2f_lw_axi_master = NULL;
size_t h2f_lw_axi_master_span = ALT_LWFPGASLVS_UB_ADDR - ALT_LWFPGASLVS_LB_ADDR + 1;
size_t h2f_lw_axi_master_ofst = ALT_LWFPGASLVS_OFST;

void *axi_base = NULL;   // the AXI bus mm base address
#define ALT_AXI_FPGASLVS_OFST (0xC0000000) // axi_master
#define HW_FPGA_AXI_SPAN (0x40000000) // Bridge span
#define HW_FPGA_AXI_MASK ( HW_FPGA_AXI_SPAN - 1 )

void soc_init() {

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
		close(fd_dev_mem);
		exit (EXIT_FAILURE);
	}

	// mmap fpga peripherals
	h2f_lw_axi_master = mmap(NULL, h2f_lw_axi_master_span, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_mem, h2f_lw_axi_master_ofst);
	if (h2f_lw_axi_master == MAP_FAILED) {
		printf("Error: h2f_lw_axi_master mmap() failed.\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit (EXIT_FAILURE);
	}

	axi_base = mmap(NULL, HW_FPGA_AXI_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_mem, ALT_AXI_FPGASLVS_OFST);
	if (axi_base == MAP_FAILED) {
		printf("Error: h2f_axi_master mmap() failed.\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit (EXIT_FAILURE);
	}

	h2p_adcspi_addr = h2f_lw_axi_master + AD9276_SPI_BASE;
	h2p_fifo_sink_ch_a_csr_addr = h2f_lw_axi_master + FIFO_SINK_CH_A_OUT_CSR_BASE;
	h2p_fifo_sink_ch_a_data_addr = h2f_lw_axi_master + FIFO_SINK_CH_A_OUT_BASE;
	/*
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
	 */
	h2p_general_cnt_in_addr = h2f_lw_axi_master + GENERAL_CNT_IN_BASE;
	h2p_general_cnt_out_addr = h2f_lw_axi_master + GENERAL_CNT_OUT_BASE;
	/*
	 h2p_adc_start_pulselength_addr = h2f_lw_axi_master + ADC_START_PULSELENGTH_BASE;
	 h2p_pulse_adc_reconfig = h2f_lw_axi_master + ADC_PLL_RECONFIG_BASE;
	 h2p_dac_grad_spi_addr = h2f_lw_axi_master + AD5724_GRAD_SPI_BASE;
	 h2p_bstream_pll_addr = axi_base + BSTREAM_PLL_RECONFIG_BASE;
	 */

	h2p_sys_pll_reconfig_addr = h2f_lw_axi_master + SYS_PLL_RECONFIG_BASE;

	// bitstream ram
	axi_ram_tx_h1 = axi_base + TX_H1_BASE;
	axi_ram_tx_l1 = axi_base + TX_L1_BASE;
	axi_ram_tx_h2 = axi_base + TX_H2_BASE;
	axi_ram_tx_l2 = axi_base + TX_L2_BASE;
	axi_ram_tx_charge = axi_base + TX_CHRG_BASE;
	axi_ram_tx_charge_bs = axi_base + TX_CHRG_BS_BASE;
	// axi_ram_tx_damp = axi_base + TX_DAMP_BASE;
	axi_ram_tx_dump = axi_base + TX_DUMP_BASE;
	axi_ram_tx_clkph = axi_base + TX_CLKPH_BASE;
	axi_ram_rx_adc_en = axi_base + RX_ADC_EN_BASE;
	axi_ram_rx_in_short = axi_base + RX_IN_SHORT_BASE;
	axi_ram_gradY_Lo_R = axi_base + GRADY_LO_R_BASE;
	axi_ram_gradY_Lo_L = axi_base + GRADY_LO_L_BASE;
	axi_ram_gradX_Lo_R = axi_base + GRADX_LO_R_BASE;
	axi_ram_gradX_Lo_L = axi_base + GRADX_LO_L_BASE;
	axi_ram_grad_hside_en = axi_base + GRAD_HSIDE_EN_BASE;
	axi_ram_aux = axi_base + AUX_BASE;
	axi_sdram_addr = axi_base + SDRAM_BASE;

	h2p_dma_addr = h2f_lw_axi_master + DMA_BASE;

	// nco
	h2p_ph_overlap_addr = h2f_lw_axi_master + NCO_PHASE_PH_OVERLAP_BASE;
	h2p_ph_inc_addr = h2f_lw_axi_master + NCO_PHASE_PH_INC_BASE;
	h2p_ph_0_to_3_addr = h2f_lw_axi_master + NCO_PHASE_PHASE_0_TO_3_BASE;
	h2p_ph_4_to_7_addr = h2f_lw_axi_master + NCO_PHASE_PHASE_4_TO_7_BASE;

	// preamp
	h2p_dac_preamp_addr = h2f_lw_axi_master + AD5724_PAMP_SPI_BASE;

	// gradient driver
	h2p_dac_grad_addr = h2f_lw_axi_master + GRAD_I2C_BASE;

}

void soc_exit() {
	close(fd_dev_mem);

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
	h2p_fifo_sink_ch_c_data_addr = NULL;
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
	h2p_general_cnt_in_addr = NULL;
	h2p_general_cnt_out_addr = NULL;
	h2p_adcspi_addr = NULL;
	h2p_dac_grad_spi_addr = NULL;
	// h2p_bstream_pll_addr = NULL;
	h2p_ph_inc_addr = NULL;   // the nco phase increment address
	h2p_dma_addr = NULL;
}
