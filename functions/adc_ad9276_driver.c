#include "../functions/adc_ad9276_driver.h"
#include "../functions/dma.h"

#include <socal/socal.h>
#include <unistd.h>
#include <socal/hps.h>
#include <stdio.h>
#include <string.h>

#include "../variables/adc_ad9276_vars.h"
#include "../variables/altera_avalon_fifo_regs.h"
#include "../variables/avalon_spi.h"
#include "../variables/hps_soc_system.h"

// offset for axi bus address
extern volatile unsigned int *h2p_adcspi_addr;   // gpio for dac (spi)
// extern volatile unsigned int *h2p_fifo_sink_ch_a_csr_addr;   // chA buffer control

unsigned int write_adc_spi(unsigned int comm) {
	unsigned int data;

	while (! ( alt_read_word(h2p_adcspi_addr + SPI_STATUS_offst) & ( 1 << status_TRDY_bit ) ))
		;
	alt_write_word( ( h2p_adcspi_addr + SPI_TXDATA_offst ), comm);
	while (! ( alt_read_word(h2p_adcspi_addr + SPI_STATUS_offst) & ( 1 << status_TMT_bit ) ))
		;
	data = alt_read_word(h2p_adcspi_addr + SPI_RXDATA_offst);   // wait for the spi command to finish
	return ( data );
}

unsigned int write_ad9276_spi(unsigned char rw, unsigned int addr, unsigned int val) {
	unsigned int command, data;
	unsigned int comm;

	comm = ( rw << 7 ) | AD9276_1BYTE_DATA;   // set command to write a byte data
	if (rw == AD9276_SPI_RD)
		val = 0x00;

	command = ( comm << 16 ) | ( addr << 8 ) | ( val ); 		//
	data = write_adc_spi(command);
	// printf("command = 0x%06x -> spi readback = 0x%06x\n", command, data);
	return data;
}

void init_adc(uint8_t lvds_z, uint8_t lvds_phase, uint32_t adc_mode, uint16_t val1, uint16_t val2) {

	// set default to:
	// lvds_z -> AD9276_OUT_ADJ_TERM_100OHM_VAL
	// lvds_phase -> AD9276_OUT_PHS_000DEG_VAL
	// adc_mode -> AD9276_OUT_TEST_OFF_VAL

	write_ad9276_spi(AD9276_SPI_WR, AD9276_CHIP_PORT_CONF_REG, 0b00111100);   // reset
	write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK);   // update the device
	write_ad9276_spi(AD9276_SPI_WR, AD9276_CHIP_PORT_CONF_REG, 0b00011000);   // reset

	write_ad9276_spi(AD9276_SPI_WR, AD9276_FLEX_GAIN_REG, AD9276_PGA_GAIN_21dB_VAL << AD9276_PGA_GAIN_SHFT | AD9276_LNA_GAIN_21dB_VAL << AD9276_LNA_GAIN_SHFT);   // set PGA Gain to 21 dB, LNA Gain to 15.6 dB
	// write_ad9276_spi(AD9276_SPI_WR, AD9276_OUT_ADJ_REG, AD9276_OUT_ADJ_TERM_200OHM_VAL << AD9276_OUT_ADJ_TERM_SHFT);   // set output driver to 100 ohms
	write_ad9276_spi(AD9276_SPI_WR, AD9276_OUT_ADJ_REG, lvds_z << AD9276_OUT_ADJ_TERM_SHFT);   // set output driver
	// write_ad9276_spi(AD9276_SPI_WR, AD9276_OUT_PHS_REG, AD9276_OUT_PHS_000DEG_VAL);   // set phase to 000 degrees
	write_ad9276_spi(AD9276_SPI_WR, AD9276_OUT_PHS_REG, lvds_phase);   // set phase
	write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK);   // update the device

	// set adc speed
	write_ad9276_spi(AD9276_SPI_WR, AD9276_CHIP_GRADE_REG, 2 << 4);   // set the ADC speed. 0 for 40 MSPS, 1 for 65 MSPS, and 2 for 80 MSPS
	write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK);   // update the device

	// filter setup.
	write_ad9276_spi(AD9276_SPI_WR, AD9276_FLEX_FILT_REG, AD9276_FLEX_FILT_HPF_20PCTG_FLP_VAL);   // set high-pass filter
	write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK);   // update the device

	// io inverse
	//write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_IDX_1_REG, AD9276_DCO_CMD_EN_MSK | AD9276_FCO_CMD_EN_MSK);   // select DCO and FCO
	//write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_IDX_2_REG, 0x00);
	//write_ad9276_spi(AD9276_SPI_WR, AD9276_OUT_MODE_REG, AD9276_OUT_MODE_INVERT_EN_MSK);   // invert DCO and FCO
	//write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK);   // update the device

	//write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_IDX_1_REG, AD9276_CH_A_CMD_EN_MSK | AD9276_CH_B_CMD_EN_MSK | AD9276_CH_C_CMD_EN_MSK | AD9276_CH_D_CMD_EN_MSK);   // select channel A-D
	//write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_IDX_2_REG, AD9276_CH_E_CMD_EN_MSK | AD9276_CH_F_CMD_EN_MSK | AD9276_CH_G_CMD_EN_MSK | AD9276_CH_H_CMD_EN_MSK);   // select channel E-H
	//write_ad9276_spi(AD9276_SPI_WR, AD9276_OUT_MODE_REG, AD9276_OUT_MODE_INVERT_EN_MSK);   // invert all selected channel
	//write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK);   // update the device

	write_ad9276_spi(AD9276_SPI_WR, AD9276_TESTIO_REG, adc_mode);	// set the ADC mode
	write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK);   // update the device

	// write_ad9276_spi(AD9276_SPI_RD, AD9276_OUT_ADJ_REG, 0x00);		// check output driver termination of selected channel
	// write_ad9276_spi(AD9276_SPI_RD, AD9276_FLEX_GAIN_REG, 0x00);		// check flex_gain of selected channel
	// write_ad9276_spi(AD9276_SPI_RD, AD9276_OUT_PHS_REG, 0x00);		// check output_phase

	if (adc_mode == AD9276_OUT_TEST_USR_INPUT_VAL) {
		adc_wr_testval(val1, val2);
	}

	usleep(100);
}

void read_adc_id() {
	unsigned int command, data;
	unsigned int comm, addr;

	comm = ( 1 << 7 ) | ( 0 << 6 ) | ( 0 << 5 );   // read chip settings
	addr = 0x00;
	command = ( comm << 16 ) | ( addr << 8 ) | 0x00;
	data = write_adc_spi(command);
	printf("command = 0x%06x -> spi readback = 0x%06x", command, data);
	if ( ( data & 0xFF ) == 0x18) {
		printf(" (ok. chip_port config is 0x18)\n");
	}
	else {
		printf(" (error. chip_port_config is wrong)\n");
	}

	comm = ( 1 << 7 ) | ( 0 << 6 ) | ( 0 << 5 );   // read chip ID
	addr = 0x01;
	command = ( comm << 16 ) | ( addr << 8 ) | 0x00;
	data = write_adc_spi(command);
	printf("command = 0x%06x -> spi readback = 0x%06x", command, data);
	if ( ( data & 0xFF ) == 0x72) {
		printf(" (ok. Chip ID for AD9276 is found)\n");
	}
	else {
		printf(" (error. Chip ID is incorrect = 0x%x)\n", data & 0xFF);
	}

	comm = ( 1 << 7 ) | ( 0 << 6 ) | ( 0 << 5 );   // read chip grade
	addr = 0x02;
	command = ( comm << 16 ) | ( addr << 8 ) | 0x00;
	data = write_adc_spi(command);
	printf("command = 0x%06x -> spi readback = 0x%06x", command, data);
	switch (data >> 4 & 0x03) {
		case 0x00:
			printf(" (ok. Mode 1 is activated -- 40MSPS)\n");
			break;
		case 0x01:
			printf(" (ok. Mode 2 is activated -- 65MSPS)\n");
			break;
		case 0x02:
			printf(" (ok. Mode 3 is activated -- 80MSPS)\n");
			break;
		default:
			printf(" (error. Mode is incorrect)\n");
			break;
	}

}

void adc_wr_testval(uint16_t val1, uint16_t val2) {   // write test value for the ADC output
	write_ad9276_spi(AD9276_SPI_WR, AD9276_TESTIO_REG, AD9276_OUT_TEST_USR_INPUT_VAL);   // user input test

	write_ad9276_spi(AD9276_SPI_WR, AD9276_USR_PATT1_LSB_REG, ( uint8_t )(val1 & 0xFF));   // user input values
	write_ad9276_spi(AD9276_SPI_WR, AD9276_USR_PATT1_MSB_REG, ( uint8_t )( ( val1 >> 8 ) & 0xFF));   // user input values

	write_ad9276_spi(AD9276_SPI_WR, AD9276_USR_PATT2_LSB_REG, ( uint8_t )(val2 & 0xFF));   // user input values
	write_ad9276_spi(AD9276_SPI_WR, AD9276_USR_PATT2_MSB_REG, ( uint8_t )( ( val2 >> 8 ) & 0xFF));   // user input values

	write_ad9276_spi(AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK);   // update the device
	usleep(10000);
}

void read_adc_fifo(volatile unsigned int *channel_csr_addr, void *channel_data_addr, unsigned int *rddata, unsigned char en_mesg) {
	unsigned int fifo_mem_level;
	unsigned int i;

	// PRINT # of DATAS in FIFO
	fifo_mem_level = alt_read_word(channel_csr_addr + ALTERA_AVALON_FIFO_LEVEL_REG);   // the fill level of FIFO memory
	if (en_mesg) {
		printf("fifo data before reading: %d ---", fifo_mem_level);
	}

	// READING DATA FROM FIFO
	fifo_mem_level = alt_read_word(channel_csr_addr + ALTERA_AVALON_FIFO_LEVEL_REG);   // the fill level of FIFO memory
	for (i = 0; fifo_mem_level > 0; i++) {
		rddata[i] = alt_read_word(channel_data_addr);
		// printf("rddata: %x\n", rddata[i]);

		fifo_mem_level--;
		if (fifo_mem_level == 0) {
			fifo_mem_level = alt_read_word(channel_csr_addr + ALTERA_AVALON_FIFO_LEVEL_REG);
		}
	}
	usleep(100);

	fifo_mem_level = alt_read_word(channel_csr_addr + ALTERA_AVALON_FIFO_LEVEL_REG);   // the fill level of FIFO memory
	if (en_mesg) {
		printf("fifo data after reading: %d\n", fifo_mem_level);
	}

}

unsigned int flush_adc_fifo(volatile unsigned int *channel_csr_addr, void *channel_data_addr, unsigned char en_mesg) {
	unsigned int fifo_mem_level;
	unsigned int rddata;
	unsigned int i;

	// PRINT # of DATAS in FIFO
	fifo_mem_level = alt_read_word(channel_csr_addr + ALTERA_AVALON_FIFO_LEVEL_REG);   // the fill level of FIFO memory
	if (en_mesg) {
		printf("fifo before flush: %5d -", fifo_mem_level);
	}

	// READING DATA FROM FIFO
	fifo_mem_level = alt_read_word(channel_csr_addr + ALTERA_AVALON_FIFO_LEVEL_REG);   // the fill level of FIFO memory
	for (i = 0; fifo_mem_level > 0; i++) {
		rddata = alt_read_word(channel_data_addr);
		if (0) {   // dummy code to remove warning for not using "rddata"
			printf("%d\n", rddata);
		}

		fifo_mem_level--;
		if (fifo_mem_level == 0) {
			fifo_mem_level = alt_read_word(channel_csr_addr + ALTERA_AVALON_FIFO_LEVEL_REG);
		}
	}
	// usleep(100);

	fifo_mem_level = alt_read_word(channel_csr_addr + ALTERA_AVALON_FIFO_LEVEL_REG);   // the fill level of FIFO memory
	if (en_mesg) {
		printf("data captured: %5d -", i);
		printf("fifo after flush: %5d\n", fifo_mem_level);
	}

	return i;

}

void read_adc_dma(volatile unsigned int * dma_addr, volatile unsigned int * sdram_addr, uint32_t DMA_SRC_BASE, uint32_t DMA_DEST_BASE, uint32_t* rddata, uint32_t transfer_length, uint8_t en_mesg) {

	// printf("fifo data before dma transfer: %d\n", alt_read_word(h2p_fifo_sink_ch_a_csr_addr + ALTERA_AVALON_FIFO_LEVEL_REG));   // check fifo fill value before dma
	fifo_to_sdram_dma_trf(dma_addr, DMA_SRC_BASE, DMA_DEST_BASE, transfer_length);
	// printf("fifo data after dma transfer: %d\n", alt_read_word(h2p_fifo_sink_ch_a_csr_addr + ALTERA_AVALON_FIFO_LEVEL_REG));   // // check fifo fill value after dma
	check_dma(dma_addr, en_mesg);   // wait for the dma operation to complete. POSSIBLE_ISSUES: DEPENDING ON THE LENGTH OF CPMG, THIS SCRIPT MIGHT BREAK IF ENABLE_MESSAGE
	memcpy(rddata, (int*) sdram_addr, transfer_length * sizeof(int));
}

