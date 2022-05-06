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
		int init_adc_delay_int;
		int echoshift_int;
		int echotime_int;
} cpmg_obj;

int us_to_digit(double val_us, double SYSCLK_MHz);

unsigned int us_to_digit_synced(double val_us, double f_larmor, double SYSCLK_MHz);

cpmg_obj cpmg_param_calc(
        SYSCLK_MHz,		// nmr fsm operating frequency (in MHz)
        f_larmor,		// nmr RF cpmg frequency (in MHz)
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
        samples_per_echo		// the total adc samples captured in one echo
        );
