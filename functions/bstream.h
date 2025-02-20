/*
 * bstream.h
 *
 *  Created on: April 14th, 2022
 *      Author: David Ariando
 */

#include "cpmg_functions.h"

#ifndef FUNCTIONS_BSTREAM_H_
#define FUNCTIONS_BSTREAM_H_

#define SRAM_DATAWIDTH 32 // memory width of the SRAM, check in the Quartus

#define T_BLANK 1000 // the amount of zero blanking period during the first and last bitstream sequence. The length of T_BLANK

typedef struct bstream_struct {
		volatile unsigned int * sram_addr;   // the sram address
		unsigned int curr_ofst;				// the sram current offset
		double freq_MHz;						// frequency
		char error_seq;						// the error flag
} bstream_obj;

// define the variables for the bitstream
enum bstream_gpio {
	tx_h1 = 0, tx_l1, tx_h2, tx_l2, tx_charge, tx_charge_bs, tx_dump, tx_clkph, rx_adc_en, rx_in_short, gradZ_p, gradZ_n, gradX_p, gradX_n, aux, BSTREAM_COUNT   // BSTREAM_COUNT is a dummy variable to mark the end of the enum
};

void bstream__push(bstream_obj * obj, char pls_pol, char seq_end, char loop_sta, char loop_sto, char mux_sel, unsigned int dataval);

void bstream__init_all_sram();
void bstream__init(bstream_obj *obj, double freq_MHz);

void bstream_rst();

void bstream_start(unsigned char wait_til_done);   // start the bitstream

void bstream_wait_for_done();   // wait for done signal from h1

char bstream_check(bstream_obj *obj);

void bstream__test(float clk_MHz);

void bstream__en_adc(
        double SYSCLK_MHz,
        unsigned int adc_clk_fact,   // the factor of (system_clk_freq / adc_clk_freq)
        unsigned int num_of_samples		// repeat the precharge and dump
        );

void bstream__vpc_chg(
        double SYSCLK_MHz,
        double bstrap_pchg_us,
        double lcs_pchg_us,		// precharging of vpc
        double lcs_recycledump_us,		// dumping the lcs to the vpc
        unsigned int repeat		// repeat the precharge and dump
        );

void bstream__vpc_wastedump(
        double SYSCLK_MHz,
        double bstrap_pchg_us,
        double lcs_vpc_dchg_us,		// discharging of vpc
        double lcs_wastedump_us,	// dumping the current into RF
        unsigned int repeat			// repeat the precharge and dump
        );

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
        unsigned int echodrop,
        unsigned char wait_til_done
        );

cpmg_cmode_obj bstream__cpmg_cmode(
        double f_larmor,
        unsigned int larmor_clk_fact,
        unsigned int adc_clk_fact,
        double bstrap_pchg_us,
        double lcs_pchg_us,		// precharging of vpc
        double lcs_dump_us,		// dumping the lcs to the vpc
        double p90_pchg_us,
        double p90_us,
        double p90_dchg_us,		// the discharging length of the current source inductor
        double p90_dtcl,
        double p180_1st_pchg_us,   // the first p180 precharging time
        double p180_pchg_us,   // the other p180 precharging time
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
        unsigned int echodrop,
        unsigned char wait_til_done
        );

cpmg_obj bstream__pgse(
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
        unsigned int echodrop,
        char gradz_dir,   // gradient direction
        double gradz_len_us,   // gradient length
        double gradz_spac_us,   // gradient spacing
        char gradx_dir,
        double gradx_len_us,
        double gradx_spac_us,
        unsigned char wait_til_done   // wait for the bitstream to be done (for FIFO read), or don't wait for bitstream to be done (for SDRAM DMA read)
        );

phenc_obj bstream__phenc(
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
        unsigned int echodrop,
        char gradz_dir,
        double gradz_len_us,
        char gradx_dir,
        double gradx_len_us,
        char grad_refocus,
        char flip_grad_refocus_sign,
        double enc_tao_us,
        char p180_xy_angle,
        unsigned char wait_til_done
        );

void bstream__noise(
        double f_adc,
        unsigned int adc_clk_fact,
        unsigned int samples,
		char wait_til_done
        );

void bstream__toggle(
        double f_adc,
        double bstrap_pchg_us,
        unsigned int adc_clk_fact,
        double toggle_len_us
        );

void bstream__tb_vpc_chg_hbridge_left(
        double SYSCLK_MHz,
        double bstrap_pchg_us,
        double lcs_pchg_us,		// precharging of vpc
        double lcs_recycledump_us,		// dumping the lcs to the vpc
        unsigned int repeat		// repeat the precharge and dump
        );

void bstream__tb_vpc_wastedump(
        double SYSCLK_MHz,
        double bstrap_pchg_us,
        double lcs_vpc_dchg_us,		// discharging of vpc
        double lcs_wastedump_us,		// dumping the current into RF
        unsigned int repeat		// repeat the precharge and dump
        );

#endif /* FUNCTIONS_BSTREAM_H_ */
