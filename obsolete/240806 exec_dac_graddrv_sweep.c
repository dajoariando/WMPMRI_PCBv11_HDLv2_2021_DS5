// PUT ATTENTION to adc_data in read_adc_val function. It really depends on implementation of verilog
// It was found that the data captured by signal tap logic analyzer is delayed by 2 DCO clock cycles, therefore data needs to be shifted by two
// This might not be the case with different FPGA implementation!

// rename to "exec_dac_graddrv"
// default params: 1 2 3 4

// #define EXEC_DAC_GRADDRV_SWEEP // uncomment this line to enable the whole code
#ifdef EXEC_DAC_GRADDRV_SWEEP

#include "hps_linux.h"

// bitstream objects
bstream_obj bstream_objs[BSTREAM_COUNT];

void init() {
	printf("GRADIENT DRIVER DAC WRITE STARTED!\n");

	soc_init();

	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100000);

}

void leave() {

	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100000);

	soc_exit();

	printf("\nGRADIENT WRITE STOPPED!\n");
}

int main(int argc, char * argv[]) {

	// Initialize system
	init();

	float sw_volt = 0.0;// sweep voltage (probe using multimeter)
	for (sw_volt = 0.0; sw_volt < 3.3; sw_volt += 0.1) {
		dac5571_i2c_wr(h2p_dac_gradz_addr, sw_volt, sw_volt, ENABLE_MESSAGE);
		dac5571_i2c_wr(h2p_dac_gradx_addr, sw_volt, sw_volt, ENABLE_MESSAGE);
		printf("voltage : %0.2f\n", sw_volt);
		usleep(5000000);
	}

	// exit program
	leave();

	return 0;
}

#endif
