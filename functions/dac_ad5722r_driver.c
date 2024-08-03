#include "../functions/dac_ad5722r_driver.h"

#include <socal/socal.h>
#include <unistd.h>
#include <stdio.h>

#include "../variables/avalon_spi.h"
#include "../variables/dac_ad5722r_vars.h"

// external variables
extern unsigned int cnt_out_val;   // defined at the main() file
extern volatile unsigned int *h2p_general_cnt_out_addr;

void init_dac_ad5722r(volatile unsigned int * dac_addr, unsigned int dac_range, unsigned int DAC_CLR_msk) {
	// read the current cnt_out_val
	cnt_out_val = alt_read_word(h2p_general_cnt_out_addr);

	alt_write_word( ( dac_addr + SPI_SLAVESELECT_offst ), 1);   // set the slave select to 1

	alt_write_word( ( dac_addr + SPI_TXDATA_offst ), WR_DAC | PWR_CNT_REG | DAC_A_PU | DAC_B_PU | REF_PU);   // power up reference voltage, dac A, dac B
	while (! ( alt_read_word(dac_addr + SPI_STATUS_offst) & ( 1 << status_TMT_bit ) ))
		;					// wait for the spi command to finish
	alt_write_word( ( dac_addr + SPI_TXDATA_offst ), WR_DAC | OUT_RANGE_SEL_REG | DAC_AB | dac_range);   // set range to P50, PN50, etc etc
	while (! ( alt_read_word(dac_addr + SPI_STATUS_offst) & ( 1 << status_TMT_bit ) ))
		;					// wait for the spi command to finish
	alt_write_word( ( dac_addr + SPI_TXDATA_offst ), WR_DAC | CNT_REG | Other_opt | Clamp_en);   // enable the current limit clamp
	while (! ( alt_read_word(dac_addr + SPI_STATUS_offst) & ( 1 << status_TMT_bit ) ))
		;					// wait for the spi command to finish

	// clear the DAC output
	cnt_out_val = cnt_out_val & ( ~DAC_CLR_msk );
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(1);

	// release the clear pin
	cnt_out_val = cnt_out_val | DAC_CLR_msk;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(1);

}

void print_warning_ad5722r(volatile unsigned int * dac_addr, uint8_t en_mesg) {
	int dataread;

	alt_write_word( ( dac_addr + SPI_TXDATA_offst ), RD_DAC | PWR_CNT_REG);   // read the power control register
	while (! ( alt_read_word(dac_addr + SPI_STATUS_offst) & ( 1 << status_TMT_bit ) ))
		;			// wait for the spi command to finish
	alt_write_word( ( dac_addr + SPI_TXDATA_offst ), WR_DAC | CNT_REG | NOP);   // no operation (NOP)
	while (! ( alt_read_word(dac_addr + SPI_STATUS_offst) & ( 1 << status_TMT_bit ) ))
		;			// wait for the spi command to finish
	while (! ( alt_read_word(dac_addr + SPI_STATUS_offst) & ( 1 << status_RRDY_bit ) ))
		;			// wait for the data to be ready
	dataread = alt_read_word(dac_addr + SPI_RXDATA_offst);   // read the data
	if (dataread & ( TSD )) {
		printf("\t\nDevice is in thermal shutdown (TSD) mode!\n");
	}
	if (dataread & OC_A) {
		printf("\tDAC A overcurrent alert (OCa)!\n");
		usleep(10);
	}
	if (dataread & OC_B) {
		printf("\tDAC B overcurrent alert (OCb)!\n");
		usleep(10);
	}

	if (en_mesg) {
		if (dataread & DAC_A_PU) {
			printf("\tDAC A is up\n");
		}
		else {
			printf("\tDAC A is down\n");
		}

		if (dataread & DAC_B_PU) {
			printf("\tDAC B is up\n");
		}
		else {
			printf("\tDAC B is down\n");
		}

		if (dataread & REF_PU) {
			printf("\tVREF is up\n");
		}
		else {
			printf("\tVREF is down\n");
		}
	}

}

void wr_dac_ad5722r(volatile unsigned int * dac_addr, unsigned int dac_range, unsigned int dac_id, double volt, unsigned int DAC_LDAC_msk, uint8_t en_mesg) {
	// dac_range must be the same with the one at the initialization

	int16_t volt_int;

	uint8_t ldac_is_wired = 1;   // if LDAC pin is wired to the FPGA
	uint8_t sdo_is_wired = 0;   // if the SDO pin is wired to the FPGA

	double REFIN = 2.5;   //  the default reference voltage inside AD5724R
	double volt_ana;   // the analog voltage calculated after digital conversion

	switch (dac_range) {
		case P50:
			volt_int = ( int16_t )(volt / ( 2 * REFIN ) * 4096);
			volt_ana = (double) ( volt_int ) * 2 * REFIN / 4096;
			break;
		case P100:
			volt_int = ( int16_t )(volt / ( 4 * REFIN ) * 4096);
			volt_ana = (double) ( volt_int ) * 4 * REFIN / 4096;
			break;
		case P108:
			volt_int = ( int16_t )(volt / ( 4.32 * REFIN ) * 4096);
			volt_ana = (double) ( volt_int ) * 4.32 * REFIN / 4096;
			break;
		case PN50:
			volt_int = ( int16_t )(volt / ( 2 * REFIN ) * 2048);
			volt_ana = (double) ( volt_int ) * 2 * REFIN / 2048;
			break;
		case PN100:
			volt_int = ( int16_t )(volt / ( 4 * REFIN ) * 2048);
			volt_ana = (double) ( volt_int ) * 4 * REFIN / 2048;
			break;
		case PN108:
			volt_int = ( int16_t )(volt / ( 4.32 * REFIN ) * 2048);
			volt_ana = (double) ( volt_int ) * 4.32 * REFIN / 2048;
			break;
		default:
			volt_int = ( int16_t )(volt / ( 2 * REFIN ) * 4096);
			volt_ana = (double) ( volt_int ) * 2 * REFIN / 4096;
			break;

	}

// read the current cnt_out_val
	cnt_out_val = alt_read_word(h2p_general_cnt_out_addr);

	alt_write_word( ( dac_addr + SPI_TXDATA_offst ), WR_DAC | DAC_REG | dac_id | ( ( volt_int & 0x0FFF ) << 4 ));   // set the voltage
	while (! ( alt_read_word(dac_addr + SPI_STATUS_offst) & ( 1 << status_TMT_bit ) ))
		;						// wait for the spi command to finish

// use this only if SDO pin is connected to the FPGA
// read the value of the DAC, check warning, and redo the writing if necessary
	if (sdo_is_wired) {
		alt_write_word( ( dac_addr + SPI_TXDATA_offst ), RD_DAC | DAC_REG | dac_id | 0x00);   // read DAC value
		while (! ( alt_read_word(dac_addr + SPI_STATUS_offst) & ( 1 << status_TMT_bit ) ))
			;			// wait for the spi command to finish
		alt_write_word( ( dac_addr + SPI_TXDATA_offst ), WR_DAC | CNT_REG | NOP);   // no operation (NOP)
		while (! ( alt_read_word(dac_addr + SPI_STATUS_offst) & ( 1 << status_TMT_bit ) ))
			;			// wait for the spi command to finish
		while (! ( alt_read_word(dac_addr + SPI_STATUS_offst) & ( 1 << status_RRDY_bit ) ))
			;			// wait for read data to be ready

		int dataread;
		dataread = alt_read_word(dac_addr + SPI_RXDATA_offst);   // read the data at the dac register
		if (en_mesg) {
			printf("\tV_in: %4.3f V ", volt_ana);   // print the voltage desired
			printf("\t(w:0x%04x)", ( volt_int & 0x0FFF ));   // print the integer dac_varac value, truncate to 12-bit signed integer value
			printf("\t(r:0x%04x)\n", dataread >> 4);	// print the read value
		}
		usleep(100);
		print_warning_ad5722r(dac_addr, en_mesg);   // find out if warning has been detected

		// recursion to make sure it works
		if ( ( volt_int & 0x0FFF ) != ( dataread >> 4 )) {
			usleep(100);
			wr_dac_ad5722r(dac_addr, dac_id, dac_range, volt, DAC_LDAC_msk, en_mesg);
		}
	}

// write data register to DAC output (ONLY IF LDAC IS WIRED)
	if (ldac_is_wired) {
		cnt_out_val = cnt_out_val & ~DAC_LDAC_msk;
		alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
		usleep(50);

		// disable LDAC one more time
		cnt_out_val = cnt_out_val | DAC_LDAC_msk;
		alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
		usleep(50);
	}
}
