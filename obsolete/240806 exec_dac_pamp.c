// #define EXEC_DAC_PAMP // uncomment this line to enable the whole code
#ifdef EXEC_DAC_PAMP

#include "hps_linux.h"

// bitstream objects
bstream_obj bstream_objs[BSTREAM_COUNT];

void init() {
	printf("PAMP WRITE STARTED!\n");

	soc_init();

	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);
	usleep(100000);

}

void leave() {

	soc_exit();

	printf("\nPAMP WRITE STOPPED!\n");
}

int main(int argc, char * argv[]) {

	double VB = atof(argv[1]);

	// Initialize system
	init();

	init_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_PAMP_CLR);
	wr_dac_ad5722r(h2p_dac_preamp_addr, PN50, DAC_B, VB, DAC_PAMP_LDAC, ENABLE_MESSAGE);

	// exit program
	leave();

	return 0;
}

#endif
