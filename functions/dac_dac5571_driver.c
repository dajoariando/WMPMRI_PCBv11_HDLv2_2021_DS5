#include "../functions/dac_dac5571_driver.h"

#include <socal/socal.h>
#include <unistd.h>
#include <stdio.h>

#include "../variables/avalon_i2c.h"
#include "../variables/dac_dac5571_vars.h"

// external variables
extern unsigned int cnt_out_val;   // defined at the main() file
extern volatile unsigned int *h2p_general_cnt_out_addr;

void dac5571_i2c_isr_stat(volatile unsigned int * i2c_addr, uint8_t en_mesg) {
	uint32_t isr_status;

	usleep(300);   // this delay is to wait for the i2c to finish its operation

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

void dac5571_i2c_wr(volatile unsigned long * dac_addr, float voltp, float voltn, uint8_t en_mesg) {

	uint8_t i2c_addr_cntp, i2c_addr_cntn;   // the chip i2c addresses
	uint8_t voltp_digit, voltn_digit;   // volt value in integer
	float vmaxp = 3.3;   // set this with the VDD of the positive regulator of the DAC
	float vmaxn = 3.299;   // set this with the VDD of the negative regulator of the DAC
	uint8_t vmax_digit = 255;

	// fix addresses for the i2c
	i2c_addr_cntp = 0x4D;	// 7-bit address for the positive DAC
	i2c_addr_cntn = 0x4C;	// 7-bit address for the negative DAC

	// initial setup
	alt_write_word( ( dac_addr + ISR_OFST ), RX_OVER_MSK | ARBLOST_DET_MSK | NACK_DET_MSK);   // RESET THE I2C FROM PREVIOUS ERRORS
	alt_write_word( ( dac_addr + CTRL_OFST ), 1 << CORE_EN_SHFT);   // enable i2c core
	alt_write_word( ( dac_addr + SCL_LOW_OFST ), 250);   // set the SCL_LOW_OFST to 250 for 100 kHz with 50 MHz clock
	alt_write_word( ( dac_addr + SCL_HIGH_OFST ), 250);   // set the SCL_HIGH_OFST to 250 for 100 kHz with 50 MHz clock
	alt_write_word( ( dac_addr + SDA_HOLD_OFST ), 125);   // set the SDA_HOLD_OFST to 1 as the default (datasheet requires min 0 ns hold time)

	// convert voltage to digit
	voltp_digit = ( uint8_t )( ( voltp / vmaxp ) * vmax_digit);
	// printf("digit: %d\n", voltp_digit);
	voltn_digit = ( uint8_t )( ( voltn / vmaxn ) * vmax_digit);

	// write the voltage
	alt_write_word( ( dac_addr + TFR_CMD_OFST ), ( 1 << STA_SHFT ) | ( 0 << STO_SHFT ) | ( i2c_addr_cntp << AD_SHFT ) | ( WR_I2C << RW_D_SHFT ));
	// dac5571_i2c_isr_stat(dac_addr, en_mesg);
	alt_write_word( ( dac_addr + TFR_CMD_OFST ), ( 0 << STA_SHFT ) | ( 0 << STO_SHFT ) | ( ( ( PD_NORMAL << PD_SHIFT ) | ( voltp_digit >> 4 ) ) & I2C_DATA_MSK ));
	// dac5571_i2c_isr_stat(dac_addr, en_mesg);
	alt_write_word( ( dac_addr + TFR_CMD_OFST ), ( 0 << STA_SHFT ) | ( 1 << STO_SHFT ) | ( ( ( voltp_digit << 4 ) & 0xF0 ) & I2C_DATA_MSK ));
	dac5571_i2c_isr_stat(dac_addr, en_mesg);

	// write the voltage
	alt_write_word( ( dac_addr + TFR_CMD_OFST ), ( 1 << STA_SHFT ) | ( 0 << STO_SHFT ) | ( i2c_addr_cntn << AD_SHFT ) | ( WR_I2C << RW_D_SHFT ));
	//dac5571_i2c_isr_stat(dac_addr, en_mesg);
	alt_write_word( ( dac_addr + TFR_CMD_OFST ), ( 0 << STA_SHFT ) | ( 0 << STO_SHFT ) | ( ( ( PD_NORMAL << PD_SHIFT ) | ( voltn_digit >> 4 ) ) & I2C_DATA_MSK ));
	//dac5571_i2c_isr_stat(dac_addr, en_mesg);
	alt_write_word( ( dac_addr + TFR_CMD_OFST ), ( 0 << STA_SHFT ) | ( 1 << STO_SHFT ) | ( ( ( voltn_digit << 4 ) & 0xF0 ) & I2C_DATA_MSK ));
	dac5571_i2c_isr_stat(dac_addr, en_mesg);

	//if (en_mesg) {
	//	printf("Status for i2c transactions:\n");
	//	printf("\ti2c0_port0 : %x\n", i2c0_port0);
	//	printf("\ti2c0_port1 : %x\n", i2c0_port1);
	//}

	alt_write_word( ( dac_addr + CTRL_OFST ), 0 << CORE_EN_SHFT);   // disable i2c core

	// usleep(10000); // delay to finish i2c operation

}
