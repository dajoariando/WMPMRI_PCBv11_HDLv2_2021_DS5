#include "../variables/general.h"

typedef struct cpmg_struct {
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
		int echoes_per_scan_int;
		int init_adc_delay_int;
		int echoshift_int;
		int adc_en_window_int;
		int echotime_int;
} cpmg_obj;

int us_to_digit(double val_us, double SYSCLK_MHz);

unsigned int us_to_digit_synced(double val_us, double f_larmor, double SYSCLK_MHz);

cpmg_obj cpmg_param_calc(
        double SYSCLK_MHz,		// nmr fsm operating frequency (in MHz)
        double f_larmor,		// nmr RF cpmg frequency (in MHz)
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
        unsigned int echoes_per_scan,   // the number of echoes per scan
        unsigned int samples_per_echo,		// the total adc samples captured in one echo
        unsigned int adc_clk_fact		// (system_clock_freq/adc_clock_freq) factor
        );

error_code check_cpmg_param(cpmg_obj obj1);   // check the constraints for a working CPMG params. Return SEQ_ERROR if there's an error, and SEQ_OK if there is no error.
