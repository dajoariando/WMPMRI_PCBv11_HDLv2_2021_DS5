// PUT ATTENTION to adc_data in read_adc_val function. It really depends on implementation of verilog
// It was found that the data captured by signal tap logic analyzer is delayed by 2 DCO clock cycles, therefore data needs to be shifted by two
// This might not be the case with different FPGA implementation!

// rename to "exec_dac_gradwrite"
// default params: 1 2 3 4

// #define EXEC_DAC_GRADWRITE // uncomment this line to enable the whole code
#ifdef EXEC_DAC_GRADWRITE

#include "hps_linux.h"

void init() {
	printf("GRADIENT WRITE STARTED!\n");

	soc_init();

	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100000);

}

void leave() {

	soc_exit();

	printf("\nGRADIENT WRITE STOPPED!\n");
}

int main(int argc, char * argv[]) {

	double VA = atof(argv[1]);
	double VB = atof(argv[2]);
	double VC = atof(argv[3]);
	double VD = atof(argv[4]);

	// Initialize system
	init();

	init_dac_ad5724r(h2p_dac_grad_spi_addr, PN100, DAC_GRAD_CLR_msk);
	wr_dac_ad5724r(h2p_dac_grad_spi_addr, PN100, DAC_A, VA, DAC_GRAD_LDAC_msk, ENABLE_MESSAGE);
	wr_dac_ad5724r(h2p_dac_grad_spi_addr, PN100, DAC_B, VB, DAC_GRAD_LDAC_msk, ENABLE_MESSAGE);
	wr_dac_ad5724r(h2p_dac_grad_spi_addr, PN100, DAC_C, VC, DAC_GRAD_LDAC_msk, ENABLE_MESSAGE);
	wr_dac_ad5724r(h2p_dac_grad_spi_addr, PN100, DAC_D, VD, DAC_GRAD_LDAC_msk, ENABLE_MESSAGE);

	// exit program
	leave();

	return 0;
}

#endif
