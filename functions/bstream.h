/*
 * bstream.h
 *
 *  Created on: April 14th, 2022
 *      Author: David Ariando
 */

#ifndef FUNCTIONS_BSTREAM_H_
#define FUNCTIONS_BSTREAM_H_

#define SRAM_DATAWIDTH 32 // memory width of the SRAM, check in the Quartus

typedef struct bstream_struct {
		volatile unsigned int * sram_addr;   // the sram address
		unsigned int curr_ofst;				// the sram current offset
		float freq_MHz;						// frequency
		char error_seq;						// the error flag
} bstream_obj;

// define the variables for the bitstream
enum bstream_gpio {
	tx_h1 = 0, tx_l1, tx_h2, tx_l2, tx_charge, tx_damp, tx_dump, tx_aux, rx_inc_damp, rx_in_short, BSTREAM_COUNT   // BSTREAM_COUNT is a dummy variable to mark the end of the enum
};

void bstream__push(bstream_obj * obj, char pls_pol, char seq_end, char loop_sta, char loop_sto, char mux_sel, unsigned int dataval);

void bstream__init_all_sram();
void bstream__init(bstream_obj *obj, float freq_MHz);

void bstream_rst();

void bstream_start();

char bstream_check(bstream_obj *obj);

void bstream__test(float clk_MHz);

#endif /* FUNCTIONS_BSTREAM_H_ */
