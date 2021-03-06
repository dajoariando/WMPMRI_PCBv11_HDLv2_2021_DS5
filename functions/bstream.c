/*
 * bstream.c
 *
 *  Created on: April 14th, 2022
 *      Author: David Ariando
 */

#include "bstream.h"

#include <stdio.h>
#include <socal/socal.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

// external memory map peripherals for bitstream
extern volatile unsigned int *axi_ram_tx_h1;
extern volatile unsigned int *axi_ram_tx_l1;
extern volatile unsigned int *axi_ram_tx_aux;
extern volatile unsigned int *axi_ram_tx_h2;
extern volatile unsigned int *axi_ram_tx_l2;
extern volatile unsigned int *axi_ram_tx_charge;
extern volatile unsigned int *axi_ram_tx_charge_bs;
// extern volatile unsigned int *axi_ram_tx_damp;
extern volatile unsigned int *axi_ram_tx_dump;
extern volatile unsigned int *axi_ram_rx_adc_en;
extern volatile unsigned int *axi_ram_rx_inc_damp;
extern volatile unsigned int *axi_ram_rx_in_short;

extern volatile unsigned int *h2p_general_cnt_out_addr;
extern volatile unsigned int *h2p_general_cnt_in_addr;
extern bstream_obj bstream_objs[BSTREAM_COUNT];
extern unsigned int cnt_out_val;

void bstream__push(bstream_obj * obj, char pls_pol, char seq_end, char loop_sta, char loop_sto, char mux_sel, unsigned int dataval) {

	unsigned int loaddata = ( ( ( (unsigned int) pls_pol & 0x01 ) << ( SRAM_DATAWIDTH - 1 ) ) | ( ( (unsigned int) seq_end & 0x01 ) << ( SRAM_DATAWIDTH - 2 ) ) | ( ( (unsigned int) loop_sta & 0x01 ) << ( SRAM_DATAWIDTH - 3 ) ) | ( ( (unsigned int) loop_sto & 0x01 ) << ( SRAM_DATAWIDTH - 4 ) ) | ( ( (unsigned int) mux_sel & 0x0F ) << ( SRAM_DATAWIDTH - 8 ) ) | ( (unsigned int) dataval & 0x0FFFFFFF ) );

	// printf("0x%02x : %d:%d:%d:%d:%d --- loaddata : 0x%08x\n", obj->curr_ofst, pls_pol, seq_end, loop_sta, loop_sto, mux_sel, loaddata);

	// write the sram with the sequence
	alt_write_word(obj->sram_addr + obj->curr_ofst, loaddata);

	// increment addresses
	obj->curr_ofst += 1;

	return;
}

void bstream__init_all_sram() {

	bstream_objs[tx_h1].sram_addr = axi_ram_tx_h1;
	bstream_objs[tx_l1].sram_addr = axi_ram_tx_l1;
	bstream_objs[tx_h2].sram_addr = axi_ram_tx_h2;
	bstream_objs[tx_l2].sram_addr = axi_ram_tx_l2;
	// bstream_objs[tx_damp].sram_addr = axi_ram_tx_damp;
	bstream_objs[tx_dump].sram_addr = axi_ram_tx_dump;
	bstream_objs[tx_charge].sram_addr = axi_ram_tx_charge;
	bstream_objs[tx_charge_bs].sram_addr = axi_ram_tx_charge_bs;
	bstream_objs[tx_aux].sram_addr = axi_ram_tx_aux;
	bstream_objs[rx_adc_en].sram_addr = axi_ram_rx_adc_en;
	bstream_objs[rx_inc_damp].sram_addr = axi_ram_rx_inc_damp;
	bstream_objs[rx_in_short].sram_addr = axi_ram_rx_in_short;

}

void bstream__init(bstream_obj *obj, double freq_MHz) {

	obj->curr_ofst = 0;
	obj->freq_MHz = freq_MHz;
	obj->error_seq = SEQ_OK;   // set the error sequence to be 0

	// check if sram_addr is pre-initialized, otherwise generate error
	if (obj->sram_addr == 0) {
		obj->error_seq = SEQ_ERROR;
		fprintf(stderr, "\tERROR! Bitstream SRAM address is not initialized! Add lines in bstream__init_all_sram() for all bitstream devices!\n");
	}

}

void bstream_rst() {

	// reset
	cnt_out_val |= BITSTR_ADV_RST;
	*h2p_general_cnt_out_addr = cnt_out_val;
	cnt_out_val &= ~BITSTR_ADV_RST;
	*h2p_general_cnt_out_addr = cnt_out_val;

}

void bstream_start() {
	unsigned char ii;
	char stop = SEQ_OK;   // do not start the sequence if there's any error
	for (ii = 0; ii < BSTREAM_COUNT; ii++) {
		// printf("bitstream %d : ", ii);
		stop = bstream_check(&bstream_objs[ii]);
		if (stop == SEQ_ERROR) {
			fprintf(stderr, "\tERROR! Bitstream sequence %d has an error.\n", ii);
			return;
		};
	}

	cnt_out_val |= BITSTR_ADV_START;
	*h2p_general_cnt_out_addr = cnt_out_val;
	cnt_out_val &= ~BITSTR_ADV_START;
	*h2p_general_cnt_out_addr = cnt_out_val;

	bstream_wait_for_done();   // wait for the done signal from h1. Remember that the last bitstream chunk is still running when done is asserted.
}

void bstream_wait_for_done() {   // wait for done signal from h1. Remember that the last bitstream chunk is still running when done is asserted.
	while (! ( alt_read_word(h2p_general_cnt_in_addr) & ( tx_h1_done ) ))
		;
}

char bstream_check(bstream_obj *obj) {
	if (obj->error_seq == SEQ_ERROR) {
		return SEQ_ERROR;
	}
	else
		return SEQ_OK;
}

void bstream__test(float clk_MHz) {
	bstream__init(&bstream_objs[tx_h1], clk_MHz);   // initialize the object along with the SRAM address
	bstream_rst();

	usleep(100);

// tx_H1
// pls_pol, seq_end, loop_sta, loop_sto, mux_sel, dataval
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 100/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 200/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 300/*dataval*/);

	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);

	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);

	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_H2
// pls_pol, seq_end, loop_sta, loop_sto, mux_sel, dataval
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 100/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 200/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 300/*dataval*/);

	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);

	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);

	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_l1
// pls_pol, seq_end, loop_sta, loop_sto, mux_sel, dataval
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 100/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 200/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 300/*dataval*/);

	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);

	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);

	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_l2
// pls_pol, seq_end, loop_sta, loop_sto, mux_sel, dataval
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 100/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 200/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 300/*dataval*/);

	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);

	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);

	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_AUX
// pls_pol, seq_end, loop_sta, loop_sto, mux_sel, dataval
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 100/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 200/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 300/*dataval*/);

	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 20/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, 10/*dataval*/);

	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, 500/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 500/*dataval*/);

	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

	bstream_start();
}

void bstream__vpc_chg(
        double SYSCLK_MHz,
        double bstrap_pchg_us,
        double lcs_pchg_us,		// precharging of vpc
        double lcs_recycledump_us,		// dumping the lcs to the vpc
        unsigned int repeat		// repeat the precharge and dump
        ) {

// take a look at the diagram of cpmg in the one note to understand how the values for different parameters are derived to cpmg_params.

// initialize all objects
	bstream__init(&bstream_objs[tx_h1], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_l1], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_h2], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_l2], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_dump], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_charge], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_charge_bs], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_aux], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_adc_en], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_inc_damp], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_in_short], SYSCLK_MHz);

	bstream_rst();
	usleep(100);

	int lcs_pchg_int;
	int lcs_recycledump_int;

	lcs_pchg_int = us_to_digit_synced(lcs_pchg_us, 1, SYSCLK_MHz);
	lcs_recycledump_int = us_to_digit_synced(lcs_recycledump_us, 1, SYSCLK_MHz);

// tx_aux
	int alltime = repeat * ( lcs_pchg_int + lcs_recycledump_int );
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_h1
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_recycledump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_h2
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_recycledump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_l1
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_recycledump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_l2
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_recycledump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_dump
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_recycledump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_charge
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_charge_bs
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_adc_en
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_in_short
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_inc_damp
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// precharge the high-side bootstrap circuits
	cnt_out_val |= ( CHG_BS | DCHG_BS | CHG_HBRIDGE );
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);   // start
	usleep(bstrap_pchg_us);
	cnt_out_val &= ~ ( CHG_BS | DCHG_BS | CHG_HBRIDGE );
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);   // start

	bstream_start();

}

void bstream__en_adc(
        double SYSCLK_MHz,
        unsigned int adc_clk_fact,   // the factor of (system_clk_freq / adc_clk_freq)
        unsigned int num_of_samples
        // repeat the precharge and dump
        ) {

	unsigned int adc_en_window = num_of_samples * adc_clk_fact;   // as the adc_en_window is referenced to SYSCLK_MHz, it needs to be multiplied by the

// initialize all objects
	bstream__init(&bstream_objs[tx_h1], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_l1], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_h2], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_l2], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_dump], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_charge], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_charge_bs], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_aux], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_adc_en], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_inc_damp], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_in_short], SYSCLK_MHz);

	bstream_rst();
	usleep(100);

// tx_aux
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_h1
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_h2
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_l1
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_l2
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_dump
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_charge
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_charge_bs
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_adc_en
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_in_short
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_inc_damp
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, adc_en_window/*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

	bstream_start();

}

void bstream__vpc_wastedump(
        double SYSCLK_MHz,
        double bstrap_pchg_us,
        double lcs_vpc_dchg_us,		// discharging of vpc
        double lcs_wastedump_us,		// dumping the current into RF
        unsigned int repeat		// repeat the precharge and dump
        ) {

// take a look at the diagram of cpmg in the one note to understand how the values for different parameters are derived to cpmg_params.

	// program the SYSCLK to 16x f_larmor. The maximum frequency for f_larmor is 16x5MHz = 80MHz and it's limited by the ADC acquisition frequency.
//
//

// initialize all objects
	bstream__init(&bstream_objs[tx_h1], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_l1], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_h2], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_l2], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_dump], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_charge], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_charge_bs], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_aux], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_adc_en], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_inc_damp], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_in_short], SYSCLK_MHz);

	bstream_rst();
	usleep(100);

	int lcs_vpc_pchg_int;
	int lcs_wastedump_int;

	lcs_vpc_pchg_int = us_to_digit_synced(lcs_vpc_dchg_us, 1, SYSCLK_MHz);
	lcs_wastedump_int = us_to_digit_synced(lcs_wastedump_us, 1, SYSCLK_MHz);

// tx_aux
	int alltime = repeat * ( lcs_vpc_pchg_int + lcs_wastedump_int );
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, alltime/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_h1
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, lcs_vpc_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_wastedump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_h2
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, lcs_vpc_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_wastedump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_l1
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, lcs_vpc_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_wastedump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_l2
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, lcs_vpc_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_wastedump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_dump
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, lcs_vpc_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_wastedump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_charge
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, lcs_vpc_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_wastedump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_charge_bs
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, repeat /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, lcs_vpc_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, lcs_wastedump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_adc_en
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_in_short
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, alltime/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_inc_damp
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// precharge the high-side bootstrap circuits
	cnt_out_val |= ( CHG_BS | DCHG_BS | CHG_HBRIDGE );
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);		// start
	usleep(bstrap_pchg_us);
	cnt_out_val &= ~ ( CHG_BS | DCHG_BS | CHG_HBRIDGE );
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);		// start

	bstream_start();

}

cpmg_obj bstream__cpmg(
        double f_larmor,
        unsigned int larmor_clk_fact,
        unsigned int adc_clk_fact,
        double bstrap_pchg_us,
        double lcs_pchg_us,		// precharging of vpc
        double lcs_dump_us,		// dumping the lcs to the vpc
        double p90_pchg_us,
        double p90_pchg_refill_us,   // restore the charge loss from RF
        double p90_us,
        double p90_dchg_us,		// the discharging length of the current source inductor
        double p90_dtcl,
        double p180_pchg_us,
        double p180_pchg_refill_us,   // restore the charge loss from RF
        double p180_us,
        double p180_dchg_us,	// the discharging length of the current source inductor
        double p180_dtcl,
        double echoshift_us,	// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
        double echotime_us,
        unsigned int samples_per_echo,
        unsigned int echoes_per_scan,
        unsigned char p90_ph_sel,
        unsigned int dconv_fact,
        unsigned int echoskip,
        unsigned int echodrop
        ) {

	// take a look at the diagram of cpmg in the one note to understand how the values for different parameters are derived to cpmg_params.

	// program the SYSCLK to 16x f_larmor. The maximum frequency for f_larmor is 16x5MHz = 80MHz and it's limited by the ADC acquisition frequency.
	// float SYSCLK_MHz = f_larmor * 16;
	// unsigned int adc_clk_fact = 4;   // the factor of (system_clk_freq / adc_clk_freq)

	double SYSCLK_MHz = f_larmor * larmor_clk_fact;

// initialize all objects
	bstream__init(&bstream_objs[tx_h1], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_l1], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_h2], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_l2], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_dump], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_charge], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_charge_bs], SYSCLK_MHz);
	bstream__init(&bstream_objs[tx_aux], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_adc_en], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_inc_damp], SYSCLK_MHz);
	bstream__init(&bstream_objs[rx_in_short], SYSCLK_MHz);

	bstream_rst();
	usleep(100);

// calculate the pulse length
	cpmg_obj cpmg_params;

	cpmg_params = cpmg_param_calc(
	        f_larmor,		// nmr RF cpmg frequency (in MHz)
	        larmor_clk_fact,		// system clock frequency / larmor frequency
	        adc_clk_fact,		// (system_clock_freq/adc_clock_freq) factor
	        lcs_pchg_us,		// precharging of vpc
	        lcs_dump_us,		// dumping the lcs to the vpc
	        p90_pchg_us,		// the precharging length for the current source inductor
	        p90_pchg_refill_us,		// restore the charge loss from p90 RF
	        p90_us,		// the length of cpmg 90 deg pulse
	        p90_dchg_us,		// the discharging length of the current source inductor
	        p180_pchg_us,		// the precharging length for the current source inductor
	        p180_pchg_refill_us,		// restore the charge loss from p180 RF
	        p180_us,		// the length of cpmg 180 deg pulse
	        p180_dchg_us,		// the discharging length of the current source inductor
	        echoshift_us,		// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
	        echotime_us,		// the length between one echo to the other (equal to p180_us + delay2_us)
	        echoes_per_scan,		// the number of echoes per scan
	        samples_per_echo		// the total adc samples captured in one echo
	        );

	int alltime = cpmg_params.lcs_pchg_int + cpmg_params.lcs_dump_int + cpmg_params.p90_pchg_int + cpmg_params.p90_pchg_refill_int +
	        cpmg_params.p90_int + cpmg_params.d90_int + cpmg_params.p180_pchg_int + cpmg_params.p180_pchg_refill_int +
	        ( echoes_per_scan - 1 ) * ( cpmg_params.p180_int + cpmg_params.d180_int + cpmg_params.p180_pchg_int + cpmg_params.p180_pchg_refill_int ) +
	        cpmg_params.p180_int + cpmg_params.d180_int;

	if (check_cpmg_param(cpmg_params) == SEQ_ERROR) {   // check if the sequence has error
		exit (EXIT_FAILURE);
	}

// tx_aux ( the mux sel selects phase cycle)
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, p90_ph_sel/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, p90_ph_sel/*mux_sel*/, cpmg_params.p90_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, p90_ph_sel/*mux_sel*/, cpmg_params.p90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_h1
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.d90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_h2
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.d90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_l1
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.d90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_l2
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.d90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_dump
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK + cpmg_params.lcs_pchg_int/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int + cpmg_params.p90_pchg_refill_int + cpmg_params.p90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d90_int + cpmg_params.p180_pchg_int + cpmg_params.p180_pchg_refill_int - cpmg_params.p90_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int + cpmg_params.p180_pchg_int + cpmg_params.p180_pchg_refill_int - cpmg_params.p180_dchg_int/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_charge
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK + cpmg_params.lcs_pchg_int + cpmg_params.lcs_dump_int/*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_int + cpmg_params.d90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_int + cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// tx_charge_bs
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d90_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_adc_en
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK + cpmg_params.lcs_pchg_int/*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int + cpmg_params.p90_pchg_refill_int + cpmg_params.p90_int /*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d90_int + cpmg_params.p180_pchg_int + cpmg_params.p180_pchg_refill_int - cpmg_params.p90_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan /*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.init_adc_delay_int /*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.adc_en_window_int /*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_int + cpmg_params.d180_int + cpmg_params.p180_pchg_int + cpmg_params.p180_pchg_refill_int - cpmg_params.init_adc_delay_int - cpmg_params.adc_en_window_int /*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_adc_en], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_short
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK /*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int + cpmg_params.p90_pchg_refill_int + cpmg_params.p90_int /*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d90_int + cpmg_params.p180_pchg_int + cpmg_params.p180_pchg_refill_int - cpmg_params.p90_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan /*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int - cpmg_params.p180_dchg_int/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int + cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_in_short], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// rx_damp
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK /*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int + cpmg_params.p90_pchg_refill_int + cpmg_params.p90_int /*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d90_int + cpmg_params.p180_pchg_int + cpmg_params.p180_pchg_refill_int - cpmg_params.p90_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan /*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_dchg_int /*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int - cpmg_params.p180_dchg_int/*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int + cpmg_params.p180_pchg_refill_int /*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
	bstream__push(&bstream_objs[rx_inc_damp], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);

// precharge the high-side bootstrap circuits
	cnt_out_val |= ( CHG_BS | DCHG_BS | CHG_HBRIDGE );
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);   // start
	usleep(bstrap_pchg_us);
	cnt_out_val &= ~ ( CHG_BS | DCHG_BS | CHG_HBRIDGE );
	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);   // start

	bstream_start();

	return cpmg_params;

}

// void OBSOLETE_bstream__cpmg(
//         // do not use this simple CPMG sequence. it doesn't deal with re-charging the CHG bootstrap circuit and doesn't deal with the recharging the loss of VPC voltage due to RF.
//         double f_larmor,
//         double bstrap_pchg_us,
//         double lcs_pchg_us,		// precharging of vpc
//         double lcs_dump_us,		// dumping the lcs to the vpc
//         double p90_pchg_us,
//         double p90_us,
//         double p90_dchg_us,		// the discharging length of the current source inductor
//         double p90_dtcl,
//         double p180_pchg_us,
//         double p180_us,
//         double p180_dchg_us,	// the discharging length of the current source inductor
//         double p180_dtcl,
//         double echoshift_us,	// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
//         double echotime_us,
//         long unsigned scanspacing_us,
//         unsigned int samples_per_echo,
//         unsigned int echoes_per_scan,
//         unsigned int n_iterate,
//         uint8_t p180_ph_sel,
//         unsigned int dconv_fact,
//         unsigned int echoskip,
//         unsigned int echodrop
//         ) {
//
// 	// take a look at the diagram of cpmg in the one note to understand how the values for different parameters are derived to cpmg_params.
//
// 	// program the SYSCLK to 16x f_larmor. The maximum frequency for f_larmor is 16x5MHz = 80MHz and it's limited by the ADC acquisition frequency.
// 	//
// 	//
// 	float SYSCLK_MHz = f_larmor * 16;
//
// 	// initialize all objects
// 	bstream__init(&bstream_objs[tx_h1], SYSCLK_MHz);
// 	bstream__init(&bstream_objs[tx_l1], SYSCLK_MHz);
// 	bstream__init(&bstream_objs[tx_h2], SYSCLK_MHz);
// 	bstream__init(&bstream_objs[tx_l2], SYSCLK_MHz);
// 	bstream__init(&bstream_objs[tx_dump], SYSCLK_MHz);
// 	bstream__init(&bstream_objs[tx_charge], SYSCLK_MHz);
// 	bstream__init(&bstream_objs[tx_charge_bs], SYSCLK_MHz);
// 	bstream__init(&bstream_objs[tx_aux], SYSCLK_MHz);
// 	// bstream__init(&bstream_objs[rx_inc_damp], SYSCLK_MHz);
// 	// bstream__init(&bstream_objs[rx_in_short], SYSCLK_MHz);
//
// 	bstream_rst();
// 	usleep(100);
//
// 	// calculate the pulse length
// 	cpmg_obj cpmg_params;
//
// 	cpmg_params = cpmg_param_calc(
// 	        SYSCLK_MHz,		// nmr fsm operating frequency (in MHz)
// 	        f_larmor,		// nmr RF cpmg frequency (in MHz)
// 	        lcs_pchg_us,		// precharging of vpc
// 	        lcs_dump_us,		// dumping the lcs to the vpc
// 	        p90_pchg_us,		// the precharging length for the current source inductor
// 	        0.00,
// 	        p90_us,		// the length of cpmg 90 deg pulse
// 	        p90_dchg_us,		// the discharging length of the current source inductor
// 	        p180_pchg_us,		// the precharging length for the current source inductor
// 	        0.00,
// 	        p180_us,		// the length of cpmg 180 deg pulse
// 	        p180_dchg_us,		// the discharging length of the current source inductor
// 	        echoshift_us,		// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
// 	        echotime_us,		// the length between one echo to the other (equal to p180_us + delay2_us)
// 	        samples_per_echo		// the total adc samples captured in one echo
//
// 	        );
//
// 	int T_BLANK = 100;
//
// 	// tx_aux
// 	int alltime = cpmg_params.lcs_pchg_int + cpmg_params.lcs_dump_int + cpmg_params.p90_pchg_int + cpmg_params.p90_int + cpmg_params.d90_int + cpmg_params.p180_pchg_int + ( echoes_per_scan - 1 ) * ( cpmg_params.p180_int + cpmg_params.d180_int + cpmg_params.p180_pchg_int ) + cpmg_params.p180_int + cpmg_params.d180_int;
// 	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_aux], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, alltime/*dataval*/);
// 	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_aux], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);
//
// 	// tx_h1
// 	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p90_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.d90_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_h1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);
//
// 	// tx_h2
// 	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p90_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.d90_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_h2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);
//
// 	// tx_l1
// 	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p90_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.d90_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 2/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_l1], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);
//
// 	// tx_l2
// 	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p90_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.d90_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 1/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_l2], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);
//
// 	// tx_dump
// 	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK + cpmg_params.lcs_pchg_int/*dataval*/);
// 	bstream__push(&bstream_objs[tx_dump], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.lcs_dump_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int + cpmg_params.p90_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_dump], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_dchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d90_int + cpmg_params.p180_pchg_int - cpmg_params.p90_dchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan /*dataval*/);
// 	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_dump], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_dchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.d180_int + cpmg_params.p180_pchg_int - cpmg_params.p180_dchg_int/*dataval*/);
// 	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_dump], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);
//
// 	// tx_charge
// 	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK + cpmg_params.lcs_pchg_int + cpmg_params.lcs_dump_int/*dataval*/);
// 	bstream__push(&bstream_objs[tx_charge], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p90_int + cpmg_params.d90_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_charge], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 1/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, echoes_per_scan - 1 /*dataval*/);
// 	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_int + cpmg_params.d180_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_charge], 1/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 1/*loop_sto*/, 0/*mux_sel*/, cpmg_params.p180_pchg_int /*dataval*/);
// 	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_charge], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);
//
// 	// tx_charge_bs
// 	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 0/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, T_BLANK/*dataval*/);
// 	bstream__push(&bstream_objs[tx_charge_bs], 0/*pls_pol*/, 1/*seq_end*/, 0/*loop_sta*/, 0/*loop_sto*/, 0/*mux_sel*/, 0/*dataval*/);
//
// 	// precharge the high-side bootstrap circuits
// 	cnt_out_val |= ( CHG_BS | DCHG_BS | CHG_HBRIDGE );
// 	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);   // start
// 	usleep(bstrap_pchg_us);
// 	cnt_out_val &= ~ ( CHG_BS | DCHG_BS | CHG_HBRIDGE );
// 	alt_write_word( ( h2p_general_cnt_out_addr ), cnt_out_val);   // start
//
// 	bstream_start();
//
// }
