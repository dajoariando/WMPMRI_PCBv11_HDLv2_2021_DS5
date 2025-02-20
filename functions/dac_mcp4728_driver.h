#include <stdint.h>

void mcp4728_i2c_isr_stat(volatile unsigned int * i2c_addr, uint8_t en_mesg);


void mcp4728_i2c_sngl_wr(volatile unsigned int * dac_addr, double vout, uint8_t i2c_3bit_addr, uint8_t ch_sel, uint8_t vref_src, double vref_voltage, uint8_t udac_mode, uint8_t pd_mode, uint8_t gain_sel, uint8_t en_mesg);
// dac_addr			: dac avalon-mm address
// voltp			: voltage output
// i2c_3bit_addr	: i2c 3-bit address for the dac
// ch_sel			: channel selection (CH_DACA to CH_DACD)
// vref_src			: vref source (VREF_EXTERN or VREF_INTERN)
// udac_mode		: UDAC mode (UDAC_DO_UPDT or UDAC_NO_UPDT)
// pd_mode			: power down mode (PWR_NORM, PWR_1K, PWR_100K, PWR_500K)
// gain_sel			: gain selector (GAIN_1X, GAIN_2X)
// en_mesg			: enable message (ENABLE_MESSAGE, DISABLE_MESSAGE)
