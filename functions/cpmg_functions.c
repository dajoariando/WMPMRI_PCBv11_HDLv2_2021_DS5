#include "cpmg_functions.h"
#include <math.h>

int us_to_digit(double val_us, double SYSCLK_MHz) {
	return (int) ( round(val_us * SYSCLK_MHz) );   // conversion from us to clock cycles
}

unsigned int us_to_digit_synced(double val_us, double f_larmor, double SYSCLK_MHz) {
	int val_int, rf_mult;

	val_int = us_to_digit(val_us, SYSCLK_MHz);   // conversion from us to clock cycles
	rf_mult = (int) ( lround(SYSCLK_MHz / f_larmor) );   // find multiplication factor from SYSCLK_MHz to f_larmor
	return (int) ( lround((double) val_int / (double) rf_mult) ) * rf_mult;   // calculate closest the closest n*rf_mult to the val_int

}

cpmg_obj cpmg_param_calc(
        double SYSCLK_MHz,		// nmr fsm operating frequency (in MHz)
        double f_larmor,		// nmr RF cpmg frequency (in MHz)
        double lcs_pchg_us,		// precharging of vpc
        double lcs_dump_us,		// dumping the lcs to the vpc
        double p90_pchg_us,		// the precharging length for the current source inductor
        double p90_pchg_refill_us,   // restore the charge loss from p90 RF
        double p90_us,			// the length of cpmg 90 deg pulse
        double p90_dchg_us,		// the discharging length of the current source inductor
        double p180_pchg_us,		// the precharging length for the current source inductor
        double p180_pchg_refill_us,   // restore the charge loss from p180 RF
        double p180_us,			// the length of cpmg 180 deg pulse
        double p180_dchg_us,	// the discharging length of the current source inductor
        double echoshift_us,	// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
        double echotime_us,			// the length between one echo to the other (equal to p180_us + delay2_us)
        unsigned int samples_per_echo	// the total adc samples captured in one echo
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
	int echotime_int;

	lcs_pchg_int = us_to_digit_synced(lcs_pchg_us, f_larmor, SYSCLK_MHz);
	lcs_dump_int = us_to_digit_synced(lcs_dump_us, f_larmor, SYSCLK_MHz);

	echotime_int = us_to_digit_synced(echotime_us, 0.5 * f_larmor, SYSCLK_MHz);   // 0.5 * f_larmor is to make sure that the echotime_int is multiplication of (2*SYSCLK_MHz/f_larmor) instead of (SYSCLK_MHz/f_larmor). This is to ensure that if the echotime_int is divided by two, the number is still multiplication of (SYSCLK_MHz/f_larmor)

	p90_pchg_int = us_to_digit_synced(p90_pchg_us, f_larmor, SYSCLK_MHz);
	p90_pchg_refill_int = us_to_digit_synced(p90_pchg_refill_us, f_larmor, SYSCLK_MHz);
	p90_int = us_to_digit_synced(p90_us, f_larmor, SYSCLK_MHz);
	p90_dchg_int = us_to_digit_synced(p90_dchg_us, f_larmor, SYSCLK_MHz);

	p180_pchg_int = us_to_digit_synced(p180_pchg_us, f_larmor, SYSCLK_MHz);
	p180_pchg_refill_int = us_to_digit_synced(p180_pchg_refill_us, f_larmor, SYSCLK_MHz);
	p180_int = us_to_digit_synced(p180_us, f_larmor, SYSCLK_MHz);
	p180_dchg_int = us_to_digit_synced(p180_dchg_us, f_larmor, SYSCLK_MHz);

	d90_int = ( echotime_int >> 1 ) - p90_int - p180_pchg_int;
	d180_int = echotime_int - p180_int - p180_pchg_int;

	echoshift_int = us_to_digit(echoshift_us, SYSCLK_MHz);
	init_adc_delay_int = ( echotime_int >> 1 ) - ( samples_per_echo >> 1 ) + echoshift_int;

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
	output.init_adc_delay_int = init_adc_delay_int;
	output.echoshift_int = echoshift_int;
	output.echotime_int = echotime_int;
	return output;
}
