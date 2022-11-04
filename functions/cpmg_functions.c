#include "cpmg_functions.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int us_to_digit(double val_us, double SYSCLK_MHz) {
	return (int) ( round(val_us * SYSCLK_MHz) );   // conversion from us to clock cycles
}

unsigned int us_to_digit_synced(double val_us, unsigned int round_up_int, double SYSCLK_MHz) {
	int val_int;

	val_int = us_to_digit(val_us, SYSCLK_MHz);   // conversion from us to clock cycles
	return (int) ( lround((double) val_int / (double) round_up_int) ) * round_up_int;   // calculate the closest n*rf_mult to the val_int

}

cpmg_obj cpmg_param_calc(
        double f_larmor,		// nmr RF cpmg frequency (in MHz)
        unsigned int larmor_clk_fact,		// system clock frequency / larmor frequency
        unsigned int adc_clk_fact,		// (system_clock_freq/adc_clock_freq) factor
        double lcs_pchg_us,		// precharging of vpc
        double lcs_dump_us,		// dumping the lcs to the vpc
        double p90_pchg_us,		// the precharging length for the current source inductor
        double p90_pchg_refill_us,		// restore the charge loss from p90 RF
        double p90_us,		// the length of cpmg 90 deg pulse
        double p90_dchg_us,		// the discharging length of the current source inductor
        double p180_pchg_us,		// the precharging length for the current source inductor
        double p180_pchg_refill_us,		// restore the charge loss from p180 RF
        double p180_us,		// the length of cpmg 180 deg pulse
        double p180_dchg_us,		// the discharging length of the current source inductor
        double echoshift_us,		// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
        double echotime_us,		// the length between one echo to the other (equal to p180_us + delay2_us)
        unsigned int echoes_per_scan,		// the number of echoes per scan
        unsigned int samples_per_echo		// the total adc samples captured in one echo
        ) {

	// output variables
	cpmg_obj output;
	int lcs_pchg_int;
	int lcs_dump_int;
	int p90_pchg_int;
	int p90_pchg_refill_int;
	int p90_int;
	int p90_dchg_int;
	int d90_int;
	int p180_pchg_int;
	int p180_pchg_refill_int;
	int p180_int;
	int p180_dchg_int;
	int d180_int;
	int init_adc_delay_int;
	int echoshift_int;
	int adc_en_window_int;
	int echotime_int;

	double SYSCLK_MHz = larmor_clk_fact * f_larmor;

	lcs_pchg_int = us_to_digit_synced(lcs_pchg_us, larmor_clk_fact, SYSCLK_MHz);
	lcs_dump_int = us_to_digit_synced(lcs_dump_us, larmor_clk_fact, SYSCLK_MHz);

	echotime_int = us_to_digit_synced(echotime_us, ( larmor_clk_fact << 1 ), SYSCLK_MHz);		// (larmor_clk_fact<<1) or (2*larmor_clk_fact) is to make sure that the echotime_int is multiplication of (2*SYSCLK_MHz/f_larmor) instead of (SYSCLK_MHz/f_larmor). This is to ensure that if the echotime_int is divided by two, the number is still multiplication of (SYSCLK_MHz/f_larmor). It does not change the absolute length of the echotime.

	p90_pchg_int = us_to_digit_synced(p90_pchg_us, larmor_clk_fact, SYSCLK_MHz);
	p90_pchg_refill_int = us_to_digit_synced(p90_pchg_refill_us, larmor_clk_fact, SYSCLK_MHz);
	p90_int = us_to_digit_synced(p90_us, larmor_clk_fact, SYSCLK_MHz);
	p90_dchg_int = us_to_digit_synced(p90_dchg_us, larmor_clk_fact, SYSCLK_MHz);

	p180_pchg_int = us_to_digit_synced(p180_pchg_us, larmor_clk_fact, SYSCLK_MHz);
	p180_pchg_refill_int = us_to_digit_synced(p180_pchg_refill_us, larmor_clk_fact, SYSCLK_MHz);
	p180_int = us_to_digit_synced(p180_us, larmor_clk_fact, SYSCLK_MHz);
	p180_dchg_int = us_to_digit_synced(p180_dchg_us, larmor_clk_fact, SYSCLK_MHz);

	d90_int = ( echotime_int >> 1 ) - p90_int - p180_pchg_int - p180_pchg_refill_int;
	d180_int = echotime_int - p180_int - p180_pchg_int - p180_pchg_refill_int;

	echoshift_int = us_to_digit(echoshift_us, SYSCLK_MHz);
	adc_en_window_int = samples_per_echo * adc_clk_fact;
	init_adc_delay_int = ( echotime_int >> 1 ) - ( adc_en_window_int >> 1 ) + echoshift_int;

	// put all the numbers into the output
	output.lcs_pchg_int = lcs_pchg_int;
	output.lcs_dump_int = lcs_dump_int;
	output.p90_pchg_int = p90_pchg_int;
	output.p90_pchg_refill_int = p90_pchg_refill_int;
	output.p90_int = p90_int;
	output.p90_dchg_int = p90_dchg_int;
	output.d90_int = d90_int;
	output.p180_pchg_int = p180_pchg_int;
	output.p180_pchg_refill_int = p180_pchg_refill_int;
	output.p180_int = p180_int;
	output.p180_dchg_int = p180_dchg_int;
	output.d180_int = d180_int;
	output.echoes_per_scan_int = echoes_per_scan;
	output.init_adc_delay_int = init_adc_delay_int;
	output.echoshift_int = echoshift_int;
	output.adc_en_window_int = adc_en_window_int;
	output.echotime_int = echotime_int;

	return output;
}

cpmg_obj cpmg_cmode_param_calc(
        double f_larmor,		// nmr RF cpmg frequency (in MHz)
        unsigned int larmor_clk_fact,		// system clock frequency / larmor frequency
        unsigned int adc_clk_fact,		// (system_clock_freq/adc_clock_freq) factor
        double lcs_pchg_us,		// precharging of vpc
        double lcs_dump_us,		// dumping the lcs to the vpc
        double p90_pchg_us,		// the precharging length for the current source inductor
        double p90_us,		// the length of cpmg 90 deg pulse
        double p90_dchg_us,		// the discharging length of the current source inductor
        double p180_pchg_us,		// the precharging length for the current source inductor
        double p180_us,		// the length of cpmg 180 deg pulse
        double p180_dchg_us,		// the discharging length of the current source inductor
        double echoshift_us,		// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
        double echotime_us,		// the length between one echo to the other (equal to p180_us + delay2_us)
        unsigned int echoes_per_scan,		// the number of echoes per scan
        unsigned int samples_per_echo		// the total adc samples captured in one echo
        ) {

	// output variables
	cpmg_obj output;
	int lcs_pchg_int;
	int lcs_dump_int;
	int p90_pchg_int;
	int p90_int;
	int p90_dchg_int;
	int d90_int;
	int p180_pchg_int;
	int p180_int;
	int p180_dchg_int;
	int d180_int;
	int init_adc_delay_int;
	int echoshift_int;
	int adc_en_window_int;
	int echotime_int;

	double SYSCLK_MHz = larmor_clk_fact * f_larmor;

	lcs_pchg_int = us_to_digit_synced(lcs_pchg_us, larmor_clk_fact, SYSCLK_MHz);
	lcs_dump_int = us_to_digit_synced(lcs_dump_us, larmor_clk_fact, SYSCLK_MHz);

	echotime_int = us_to_digit_synced(echotime_us, 0.5 * larmor_clk_fact, SYSCLK_MHz);		// 0.5 * f_larmor is to make sure that the echotime_int is multiplication of (2*SYSCLK_MHz/f_larmor) instead of (SYSCLK_MHz/f_larmor). This is to ensure that if the echotime_int is divided by two, the number is still multiplication of (SYSCLK_MHz/f_larmor). It does not change the absolute length of the echotime.

	p90_pchg_int = us_to_digit_synced(p90_pchg_us, larmor_clk_fact, SYSCLK_MHz);
	p90_int = us_to_digit_synced(p90_us, larmor_clk_fact, SYSCLK_MHz);
	p90_dchg_int = us_to_digit_synced(p90_dchg_us, larmor_clk_fact, SYSCLK_MHz);

	p180_pchg_int = us_to_digit_synced(p180_pchg_us, larmor_clk_fact, SYSCLK_MHz);
	p180_int = us_to_digit_synced(p180_us, larmor_clk_fact, SYSCLK_MHz);
	p180_dchg_int = us_to_digit_synced(p180_dchg_us, larmor_clk_fact, SYSCLK_MHz);

	d90_int = ( echotime_int >> 1 ) - p90_int - p180_pchg_int;
	d180_int = echotime_int - p180_int - p180_pchg_int;

	echoshift_int = us_to_digit(echoshift_us, SYSCLK_MHz);
	adc_en_window_int = samples_per_echo * adc_clk_fact;
	init_adc_delay_int = ( echotime_int >> 1 ) - ( adc_en_window_int >> 1 ) + echoshift_int;

	// put all the numbers into the output
	output.lcs_pchg_int = lcs_pchg_int;
	output.lcs_dump_int = lcs_dump_int;
	output.p90_pchg_int = p90_pchg_int;
	output.p90_pchg_refill_int = 100;   // put to any number more than 10 to supress error
	output.p90_int = p90_int;
	output.p90_dchg_int = p90_dchg_int;
	output.d90_int = d90_int;
	output.p180_pchg_int = p180_pchg_int;
	output.p180_pchg_refill_int = 100;   // put to any number more than 10 to supress error
	output.p180_int = p180_int;
	output.p180_dchg_int = p180_dchg_int;
	output.d180_int = d180_int;
	output.echoes_per_scan_int = echoes_per_scan;
	output.init_adc_delay_int = init_adc_delay_int;
	output.echoshift_int = echoshift_int;
	output.adc_en_window_int = adc_en_window_int;
	output.echotime_int = echotime_int;

	return output;
}

error_code check_cpmg_param(cpmg_obj obj1) {
	if (obj1.lcs_pchg_int <= 10) {
		fprintf(stderr, "\tERROR! lcs_pchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.lcs_dump_int <= 10) {
		fprintf(stderr, "\tERROR! lcs_dump_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p90_pchg_int <= 10) {
		fprintf(stderr, "\tERROR! p90_pchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p90_pchg_refill_int <= 10) {
		fprintf(stderr, "\tERROR! p90_pchg_refill_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p90_int <= 10) {
		fprintf(stderr, "\tERROR! p90_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p90_dchg_int <= 10) {
		fprintf(stderr, "\tERROR! p90_dchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.d90_int <= 10) {
		fprintf(stderr, "\tERROR! d90_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p180_pchg_int <= 10) {
		fprintf(stderr, "\tERROR! p180_pchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p180_pchg_refill_int <= 10) {
		fprintf(stderr, "\tERROR! p180_pchg_refill_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p180_int <= 10) {
		fprintf(stderr, "\tERROR! p180_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p180_dchg_int <= 10) {
		fprintf(stderr, "\tERROR! p180_dchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.d180_int <= 10) {
		fprintf(stderr, "\tERROR! d180_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.init_adc_delay_int <= 10) {
		fprintf(stderr, "\tERROR! init_adc_delay_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.echoes_per_scan_int < 2) {
		fprintf(stderr, "\tERROR! echoes_per_scan_int is less than 2!\n");
		return SEQ_ERROR;
	}
	if (obj1.adc_en_window_int <= 10) {
		fprintf(stderr, "\tERROR! adc_en_window_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.adc_en_window_int > obj1.echotime_int + obj1.echoshift_int - 10) {   // subtraction factor of 10 is due to the minimum init_adc_delay_int to be valid.
		fprintf(stderr, "\tERROR! adc_en_window_int is larger than the echotime_int!\n");
		return SEQ_ERROR;
	}

	return SEQ_OK;

}

phenc_obj phenc_param_calc(
        double f_larmor,		// nmr RF cpmg frequency (in MHz)
        unsigned int larmor_clk_fact,		// system clock frequency / larmor frequency
        unsigned int adc_clk_fact,   // (system_clock_freq/adc_clock_freq) factor
        double lcs_pchg_us,   // precharging of vpc
        double lcs_dump_us,   // dumping the lcs to the vpc
        double p90_pchg_us,   // the precharging length for the current source inductor
        double p90_pchg_refill_us,   // restore the charge loss from p90 RF
        double p90_us,   // the length of cpmg 90 deg pulse
        double p90_dchg_us,   // the discharging length of the current source inductor
        double p180_pchg_us,   // the precharging length for the current source inductor
        double p180_pchg_refill_us,   // restore the charge loss from p180 RF
        double p180_us,   // the length of cpmg 180 deg pulse
        double p180_dchg_us,   // the discharging length of the current source inductor
        double echoshift_us,   // shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
        double echotime_us,   // the length between one echo to the other (equal to p180_us + delay2_us)
        unsigned int echoes_per_scan,   // the number of echoes per scan
        unsigned int samples_per_echo,   // the total adc samples captured in one echo
        double gradz_len_us,   // the gradient pulse length
        double gradx_len_us,   // the gradient pulse length
        double enc_tao_us,   // the encoding time tao. Spacing from p90 to first echo is 2*tao with p180 in the middle of the spacing.
        unsigned char en_mesg   // print out the values generated by the function
        ) {

	// output variables
	phenc_obj output;
	int lcs_pchg_int;
	int lcs_dump_int;
	int p90_pchg_int;
	int p90_pchg_refill_int;
	int p90_int;
	int p90_dchg_int;
	int d90_enc_int;
	int p180_pchg_int;
	int p180_pchg_refill_int;
	int p180_int;
	int p180_dchg_int;
	int d180_enc_int;
	int d180_int;
	int init_adc_delay_int;
	int echoshift_int;
	int adc_en_window_int;
	int echotime_int;
	int gradz_len_int;
	int gradx_len_int;
	int enc_tao_int;

	double SYSCLK_MHz = larmor_clk_fact * f_larmor;
	if (larmor_clk_fact % 2) {
		printf("larmor_clk_fact should be multiplication of 2.\n");
		exit (EXIT_FAILURE);
	}

	// read the data from the input
	lcs_pchg_int = us_to_digit_synced(lcs_pchg_us, larmor_clk_fact, SYSCLK_MHz);
	lcs_dump_int = us_to_digit_synced(lcs_dump_us, larmor_clk_fact, SYSCLK_MHz);
	echotime_int = us_to_digit_synced(echotime_us, ( larmor_clk_fact << 1 ), SYSCLK_MHz);		// (larmor_clk_fact<<1) or (2*larmor_clk_fact) is to make sure that the echotime_int is multiplication of (2*SYSCLK_MHz/f_larmor) instead of (SYSCLK_MHz/f_larmor). This is to ensure that if the echotime_int is divided by two, the number is still multiplication of (SYSCLK_MHz/f_larmor). It does not change the absolute length of the echotime.
	p90_pchg_int = us_to_digit_synced(p90_pchg_us, larmor_clk_fact, SYSCLK_MHz);
	p90_pchg_refill_int = us_to_digit_synced(p90_pchg_refill_us, larmor_clk_fact, SYSCLK_MHz);
	p90_int = us_to_digit_synced(p90_us, larmor_clk_fact, SYSCLK_MHz);
	p90_dchg_int = us_to_digit_synced(p90_dchg_us, larmor_clk_fact, SYSCLK_MHz);
	p180_pchg_int = us_to_digit_synced(p180_pchg_us, larmor_clk_fact, SYSCLK_MHz);
	p180_pchg_refill_int = us_to_digit_synced(p180_pchg_refill_us, larmor_clk_fact, SYSCLK_MHz);
	p180_int = us_to_digit_synced(p180_us, larmor_clk_fact, SYSCLK_MHz);
	p180_dchg_int = us_to_digit_synced(p180_dchg_us, larmor_clk_fact, SYSCLK_MHz);
	gradz_len_int = us_to_digit_synced(gradz_len_us, larmor_clk_fact, SYSCLK_MHz);
	gradx_len_int = us_to_digit_synced(gradx_len_us, larmor_clk_fact, SYSCLK_MHz);
	enc_tao_int = us_to_digit_synced(enc_tao_us, larmor_clk_fact, SYSCLK_MHz);

	// compute parameters
	d90_enc_int = enc_tao_int - p90_int - p180_pchg_int - p180_pchg_refill_int;
	d180_enc_int = enc_tao_int + ( echotime_int >> 1 ) - p180_int - p180_pchg_int - p180_pchg_refill_int;
	d180_int = echotime_int - p180_int - p180_pchg_int - p180_pchg_refill_int;
	echoshift_int = us_to_digit_synced(echoshift_us, larmor_clk_fact, SYSCLK_MHz);
	adc_en_window_int = samples_per_echo * adc_clk_fact;
	init_adc_delay_int = ( echotime_int >> 1 ) - ( adc_en_window_int >> 1 ) + echoshift_int;

	// put all the numbers into the output
	output.lcs_pchg_int = lcs_pchg_int;
	output.lcs_dump_int = lcs_dump_int;
	output.p90_pchg_int = p90_pchg_int;
	output.p90_pchg_refill_int = p90_pchg_refill_int;
	output.p90_int = p90_int;
	output.p90_dchg_int = p90_dchg_int;
	output.d90_enc_int = d90_enc_int;
	output.p180_pchg_int = p180_pchg_int;
	output.p180_pchg_refill_int = p180_pchg_refill_int;
	output.p180_int = p180_int;
	output.p180_dchg_int = p180_dchg_int;
	output.d180_enc_int = d180_enc_int;
	output.d180_int = d180_int;
	output.echoes_per_scan_int = echoes_per_scan;
	output.init_adc_delay_int = init_adc_delay_int;
	output.echoshift_int = echoshift_int;
	output.adc_en_window_int = adc_en_window_int;
	output.echotime_int = echotime_int;
	output.gradz_len_int = gradz_len_int;
	output.gradx_len_int = gradx_len_int;
	output.enc_tao_int = enc_tao_int;

	if (en_mesg) {
		printf("lcs_pchg_int : %d \n", output.lcs_pchg_int);
		printf("lcs_dump_int : %d \n", output.lcs_dump_int);
		printf("p90_pchg_int : %d \n", output.p90_pchg_int);
		printf("p90_pchg_refill_int : %d \n", output.p90_pchg_refill_int);
		printf("p90_int : %d \n", output.p90_int);
		printf("p90_dchg_int : %d \n", output.p90_dchg_int);
		printf("d90_enc_int : %d \n", output.d90_enc_int);
		printf("p180_pchg_int : %d \n", output.p180_pchg_int);
		printf("p180_pchg_refill_int : %d \n", output.p180_pchg_refill_int);
		printf("p180_int : %d \n", output.p180_int);
		printf("p180_dchg_int : %d \n", output.p180_dchg_int);
		printf("d180_enc_int : %d \n", output.d180_enc_int);
		printf("d180_int : %d \n", output.d180_int);
		printf("echoes_per_scan_int : %d \n", output.echoes_per_scan_int);
		printf("init_adc_delay_int : %d \n", output.init_adc_delay_int);
		printf("echoshift_int : %d \n", output.echoshift_int);
		printf("adc_en_window_int : %d \n", output.adc_en_window_int);
		printf("echotime_int : %d \n", output.echotime_int);
		printf("gradz_len_int : %d \n", output.gradz_len_int);
		printf("gradx_len_int : %d \n", output.gradx_len_int);
		printf("enc_tao_int : %d \n", output.enc_tao_int);
	}

	return output;
}

error_code check_phenc_param(phenc_obj obj1) {
	if (obj1.lcs_pchg_int <= 10) {
		fprintf(stderr, "\tERROR! lcs_pchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.lcs_dump_int <= 10) {
		fprintf(stderr, "\tERROR! lcs_dump_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p90_pchg_int <= 10) {
		fprintf(stderr, "\tERROR! p90_pchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p90_pchg_refill_int <= 10) {
		fprintf(stderr, "\tERROR! p90_pchg_refill_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p90_int <= 10) {
		fprintf(stderr, "\tERROR! p90_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p90_dchg_int <= 10) {
		fprintf(stderr, "\tERROR! p90_dchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.d90_enc_int <= 10) {
		fprintf(stderr, "\tERROR! d90_enc_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p180_pchg_int <= 10) {
		fprintf(stderr, "\tERROR! p180_pchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p180_pchg_refill_int <= 10) {
		fprintf(stderr, "\tERROR! p180_pchg_refill_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p180_int <= 10) {
		fprintf(stderr, "\tERROR! p180_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.p180_dchg_int <= 10) {
		fprintf(stderr, "\tERROR! p180_dchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.d180_enc_int <= 10) {
		fprintf(stderr, "\tERROR! p180_pchg_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.d180_int <= 10) {
		fprintf(stderr, "\tERROR! d180_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.init_adc_delay_int <= 10) {
		fprintf(stderr, "\tERROR! init_adc_delay_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.echoes_per_scan_int < 2) {
		fprintf(stderr, "\tERROR! echoes_per_scan_int is less than 2!\n");
		return SEQ_ERROR;
	}
	if (obj1.adc_en_window_int <= 10) {
		fprintf(stderr, "\tERROR! adc_en_window_int is less than 10!\n");
		return SEQ_ERROR;
	}
	if (obj1.adc_en_window_int > obj1.echotime_int + obj1.echoshift_int - 10) {   // subtraction factor of 10 is due to the minimum init_adc_delay_int to be valid.
		fprintf(stderr, "\tERROR! adc_en_window_int is larger than the echotime_int!\n");
		return SEQ_ERROR;
	}
	if (obj1.gradz_len_int > ( obj1.d90_enc_int + obj1.p180_pchg_int + obj1.p180_pchg_refill_int )) {
		fprintf(stderr, "\tERROR! gradz_len is intersecting with the p180 pulse! Prolong the encoding period or reduce gradz_len.\n");
		return SEQ_ERROR;
	}
	if (obj1.gradx_len_int > ( obj1.d90_enc_int + obj1.p180_pchg_int + obj1.p180_pchg_refill_int )) {
		fprintf(stderr, "\tERROR! gradx_len is intersecting with the p180 pulse! Prolong the encoding period or reduce gradx_len.\n");
		return SEQ_ERROR;
	}

	return SEQ_OK;

}
