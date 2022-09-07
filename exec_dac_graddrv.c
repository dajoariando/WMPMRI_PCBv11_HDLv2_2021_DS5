// PUT ATTENTION to adc_data in read_adc_val function. It really depends on implementation of verilog
// It was found that the data captured by signal tap logic analyzer is delayed by 2 DCO clock cycles, therefore data needs to be shifted by two
// This might not be the case with different FPGA implementation!

// rename to "dac_graddrv"
// default params: 1 2 3 4

// #define EXEC_DAC_GRADDRV // uncomment this line to enable the whole code
#ifdef EXEC_DAC_GRADDRV

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

	float voltp = atof(argv[1]);
	float voltn = atof(argv[2]);
	float plen_us = atof(argv[3]);

	// Initialize system
	init();// make sure that all the big discharging capacitors are charged before connecting the
	usleep(1000000);

	dac5571_i2c_wr(h2p_dac_graddrv_addr, voltp, voltn, ENABLE_MESSAGE);

	// enable output current
	cnt_out_val = cnt_out_val | GRADDRV_CNT_P;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(plen_us);
	// disable output current
	cnt_out_val = cnt_out_val & ( ~GRADDRV_CNT_P );
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(1);

	// enable output current
	cnt_out_val = cnt_out_val | GRADDRV_CNT_N;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(plen_us);
	// disable output current
	cnt_out_val = cnt_out_val & ( ~GRADDRV_CNT_N );
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(1);

	// exit program
	leave();

	return 0;
}

#endif
