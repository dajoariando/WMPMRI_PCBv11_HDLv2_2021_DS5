#include "../functions/dac_mcp4728_driver.h"

#include <socal/socal.h>
#include <unistd.h>
#include <stdio.h>

#include "../variables/avalon_i2c.h"
#include "../variables/dac_mcp4728_vars.h"

void mcp4728_i2c_isr_stat(volatile unsigned int * i2c_addr, uint8_t en_mesg) {
	uint32_t isr_status;

	usleep(100);   // this delay is to wait for the i2c to finish its operation

	isr_status = alt_read_word(i2c_addr + ISR_OFST);
	if (isr_status & RX_OVER_MSK) {
		printf("\t[ERROR] Receive data FIFO has overrun condition, new data is lost\n");
		alt_write_word( ( i2c_addr + ISR_OFST ), RX_OVER_MSK);   // clears receive overrun
	}
	else {
		if (en_mesg)
			printf("\t[NORMAL] No receive overrun\n");
	}
	if (isr_status & ARBLOST_DET_MSK) {
		printf("\t[ERROR] Core has lost bus arbitration\n");
		alt_write_word( ( i2c_addr + ISR_OFST ), ARBLOST_DET_MSK);   // clears receive overrun
	}
	else {
		if (en_mesg)
			printf("\t[NORMAL] No arbitration lost\n");
	}
	if (isr_status & NACK_DET_MSK) {
		printf("\t[ERROR] NACK is received by the core\n");
		alt_write_word( ( i2c_addr + ISR_OFST ), NACK_DET_MSK);   // clears receive overrun
	}
	else {
		if (en_mesg)
			printf("\t[NORMAL] ACK has been received\n");
	}
	if (isr_status & RX_READY_MSK) {
		if (en_mesg)
			printf("\t[WARNING] RX_DATA_FIFO level is equal or more than its threshold\n");
	}
	else {
		if (en_mesg)
			printf("\t[NORMAL] RX_DATA_FIFO level is less than its threshold\n");
	}
	if (isr_status & TX_READY_MSK) {
		if (en_mesg)
			printf("\t[WARNING] TFR_CMD level is equal or more than its threshold\n");
	}
	else {
		if (en_mesg)
			printf("\t[NORMAL] TFR_CMD level is less than its threshold\n");
	}

	if (en_mesg)
		printf("\t --- \n");
}

void mcp4728_i2c_sngl_wr(volatile unsigned int * dac_addr, double vout, uint8_t i2c_3bit_addr, uint8_t ch_sel, uint8_t vref_src, double vref_voltage, uint8_t udac_mode, uint8_t pd_mode, uint8_t gain_sel, uint8_t en_mesg) {
	// dac_addr			: dac avalon-mm address
	// voltp			: voltage output
	// i2c_3bit_addr	: i2c 3-bit address for the dac
	// ch_sel			: channel selection (CH_DACA to CH_DACD)
	// vref_src			: vref source (VREF_EXTERN or VREF_INTERN)
	// vref_voltage		: vref voltage (2.048V for internal reference or whatever the VDD is)
	// udac_mode		: UDAC mode (UDAC_DO_UPDT or UDAC_NO_UPDT)
	// pd_mode			: power down mode (PWR_NORM, PWR_1K, PWR_100K, PWR_500K)
	// gain_sel			: gain selector (GAIN_1X, GAIN_2X)
	// en_mesg			: enable message (ENABLE_MESSAGE, DISABLE_MESSAGE)

	uint16_t volt_digit;   // volt value in integer
	uint16_t vmax_digit = BITMASK; // the number of bits on the DAC

	// initial setup
	alt_write_word( ( dac_addr + ISR_OFST ), RX_OVER_MSK | ARBLOST_DET_MSK | NACK_DET_MSK);   // RESET THE I2C FROM PREVIOUS ERRORS
	alt_write_word( ( dac_addr + CTRL_OFST ), 1 << CORE_EN_SHFT);   // enable i2c core
	alt_write_word( ( dac_addr + SCL_LOW_OFST ), 250);   // set the SCL_LOW_OFST to 250 for 100 kHz with 50 MHz clock
	alt_write_word( ( dac_addr + SCL_HIGH_OFST ), 250);   // set the SCL_HIGH_OFST to 250 for 100 kHz with 50 MHz clock
	alt_write_word( ( dac_addr + SDA_HOLD_OFST ), 125);   // set the SDA_HOLD_OFST to 1 as the default (datasheet requires min 0 ns hold time)

	// convert voltage to digit
	if ( (vref_src == VREF_INTERN) && (gain_sel == GAIN_2X)) {
		volt_digit = ( uint16_t )( ( vout / (2*vref_voltage) ) * ((double)(vmax_digit+1)) );
	}
	else { // normal case
		volt_digit = ( uint16_t )( ( vout / vref_voltage ) * ((double)(vmax_digit+1)) );
	}

	// limit the volt_digit only to vmax_digit
	if (volt_digit >= vmax_digit) {
		volt_digit = vmax_digit;
	}
	// printf("digit: %d\n", volt_digit);

	// bytes to sent
	uint8_t byte1_dev_addr = (0b1100 << 3) | (i2c_3bit_addr & 0b111); // 1st byte of i2c: device addressing (7-bit address)
	uint8_t byte2_wr_cmd = (0b01011 << 3) | ((ch_sel << 1) & 0b0110) | (udac_mode & 0b0001); // 2nd byte of i2c: channel select and udac
	uint8_t byte3_dac_msb = (vref_src << 7) | ((pd_mode << 5) & 0b01100000) | ((gain_sel << 4) & 0b00010000) | (((uint8_t) (volt_digit >> 8)) & (0x0F)); // 3rd byte of i2c: setting and msb
	uint8_t byte4_dac_lsb = (uint8_t) (volt_digit & 0xFF); // 4th byte of i2c: lsb

	// write the voltage
	alt_write_word( ( dac_addr + TFR_CMD_OFST ), ( 1 << STA_SHFT ) | ( 0 << STO_SHFT ) | ( byte1_dev_addr << AD_SHFT ) | ( WR_I2C << RW_D_SHFT ));
	alt_write_word( ( dac_addr + TFR_CMD_OFST ), ( 0 << STA_SHFT ) | ( 0 << STO_SHFT ) | ( byte2_wr_cmd & I2C_DATA_MSK ));
	alt_write_word( ( dac_addr + TFR_CMD_OFST ), ( 0 << STA_SHFT ) | ( 0 << STO_SHFT ) | ( byte3_dac_msb & I2C_DATA_MSK ));
	alt_write_word( ( dac_addr + TFR_CMD_OFST ), ( 0 << STA_SHFT ) | ( 1 << STO_SHFT ) | ( byte4_dac_lsb & I2C_DATA_MSK ));

	mcp4728_i2c_isr_stat(dac_addr, en_mesg);

	alt_write_word( ( dac_addr + CTRL_OFST ), 0 << CORE_EN_SHFT);   // disable i2c core

	usleep(40000); // delay to finish i2c operation. Consecutive i2c operation need this delay!

}
