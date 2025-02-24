extern unsigned int cnt_out_val;
extern volatile unsigned int *h2p_general_cnt_out_addr;
extern volatile unsigned int *h2p_dac_grad_addr;
#include "../functions/dac_mcp4728_driver.h"
#include "../variables/general.h"
#include "../variables/dac_mcp4728_vars.h"
#include <socal/socal.h>

////////////////////// DAC CONFIG /////////////////////////////////////
// GRADX_HL -------------------|    |------------------GRADX_HR
//
// DAC A -----------\                         /-------- DAC C (default)
// GRADX_CNT_P        ---------|    |---------    GRADX_CNT_N
// DAC B -----------/                         \-------- DAC D

void grad_init_current (double i_ChA, double i_ChB, double i_ChC, double i_ChD, uint8_t DAC_SEL) {
	// i_ChA - i_ChD are in mA
	// DAC_SEL is DAC_X, DAC_Y, DAC_Z, and DAC_Z2

	// conversion from current to voltage
	double v_A, v_B, v_C, v_D;
	double conv_i_to_v = 0.2; // 1A is equal to 0.2V.
	double vref = 2.5; // vref of the system is 2.5V.
	double A_to_mA = 1000; // conversion from A to mA.
	unsigned int grad_cnt_ch, grad_hi_side_ch; // variable to select gradient channel

	// current to voltage conversion
	v_A = (i_ChA / A_to_mA) * conv_i_to_v + vref;
	v_B = (i_ChB / A_to_mA) * conv_i_to_v + vref;
	v_C = (i_ChC / A_to_mA) * conv_i_to_v + vref;
	v_D = (i_ChD / A_to_mA) * conv_i_to_v + vref;

	// select DAC
	switch (DAC_SEL) {
		case DAC_X:
			grad_cnt_ch = GRADX_LO_L_SOC | GRADX_LO_R_SOC;
			grad_hi_side_ch = GRADX_HI_R_SOC | GRADX_HI_L_SOC;
			break;
		case DAC_Y:
			grad_cnt_ch = GRADY_LO_L_SOC | GRADY_LO_R_SOC;
			grad_hi_side_ch = GRADY_HI_R_SOC | GRADY_HI_L_SOC;
			break;
		case DAC_Z:
			grad_cnt_ch = GRADZ_LO_L_SOC | GRADZ_LO_R_SOC;
			grad_hi_side_ch = GRADZ_HI_R_SOC | GRADZ_HI_L_SOC;
			break;
		case DAC_Z2:
			grad_cnt_ch = GRADZ2_LO_L_SOC | GRADZ2_LO_R_SOC;
			grad_hi_side_ch = GRADZ2_HI_R_SOC | GRADZ2_HI_L_SOC;
			break;
		default:
			grad_cnt_ch = 0;
			grad_hi_side_ch = 0;
	}

	// enable the I/O and set gradient to a default state.
	cnt_out_val |= GRAD_OE;
	cnt_out_val &= ~( grad_cnt_ch | grad_hi_side_ch ); // turn off all the high-side, and set selected channel to B and D.
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);

	// program the DAC
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, v_A, DAC_SEL, CH_DACA, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, v_B, DAC_SEL, CH_DACB, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, v_C, DAC_SEL, CH_DACC, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);
	mcp4728_i2c_sngl_wr (h2p_dac_grad_addr, v_D, DAC_SEL, CH_DACD, VREF_INTERN, (double)2.048, UDAC_DO_UPDT, PWR_NORM, GAIN_2X, DISABLE_MESSAGE);

}
