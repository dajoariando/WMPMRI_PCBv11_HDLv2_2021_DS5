// Created on: Jan 30 2025
// Author: David Ariando
// This is a test program for mcp4728 dac

//#define EXEC_TB_MCP4728_2
#ifdef EXEC_TB_MCP4728_2

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
	// init_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_PAMP_CLR);
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

	// init
	init();

	cnt_out_val |= GRAD_OE;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_X, CH_DACA, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_X, CH_DACB, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_X, CH_DACC, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_X, CH_DACD, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Y, CH_DACA, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Y, CH_DACB, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Y, CH_DACC, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Y, CH_DACD, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Z, CH_DACA, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Z, CH_DACB, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Z, CH_DACC, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Z, CH_DACD, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Z2, CH_DACA, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Z2, CH_DACB, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Z2, CH_DACC, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.00, DAC_Z2, CH_DACD, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);

	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.25, DAC_X, CH_DACA, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.50, DAC_X, CH_DACB, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 0.75, DAC_X, CH_DACC, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 1.00, DAC_X, CH_DACD, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 1.25, DAC_Y, CH_DACA, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 1.50, DAC_Y, CH_DACB, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 1.75, DAC_Y, CH_DACC, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 2.00, DAC_Y, CH_DACD, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 2.25, DAC_Z, CH_DACA, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 2.50, DAC_Z, CH_DACB, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 2.75, DAC_Z, CH_DACC, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 3.00, DAC_Z, CH_DACD, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 3.25, DAC_Z2, CH_DACA, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 3.50, DAC_Z2, CH_DACB, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 3.75, DAC_Z2, CH_DACC, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, 4.00, DAC_Z2, CH_DACD, VREF_INTERN, (float)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);

	// Test X
	cnt_out_val |= GRADX_CNT_N;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	cnt_out_val &= ~GRADX_CNT_N;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	cnt_out_val |= GRADX_CNT_P;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	cnt_out_val &= ~GRADX_CNT_P;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	// Test Y
	cnt_out_val |= GRADY_CNT_N;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	cnt_out_val &= ~GRADY_CNT_N;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	cnt_out_val |= GRADY_CNT_P;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	cnt_out_val &= ~GRADY_CNT_P;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	// Test Z
	cnt_out_val |= GRADZ_CNT_N;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	cnt_out_val &= ~GRADZ_CNT_N;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	cnt_out_val |= GRADZ_CNT_P;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	cnt_out_val &= ~GRADZ_CNT_P;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);


	// Test Z2
	cnt_out_val |= GRADZ2_CNT_N;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	cnt_out_val &= ~GRADZ2_CNT_N;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	cnt_out_val |= GRADZ2_CNT_P;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	cnt_out_val &= ~GRADZ2_CNT_P;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	leave();

	return 0;
}

#endif
