/* offsets for output control signal
 #define ADC_AD9276_PWDN_OFST				(9)
 #define ADC_AD9276_STBY_OFST				(8)
 #define FSM_RESET_OFST					(7)
 #define sw_off_OFST					(6)
 #define tx_path_en_OFST				(5)
 #define pulser_en_OFST				(4)
 #define lm96570_pin_reset_OFST		(3)
 #define lm96570_tx_en_OFST			(2)
 #define lm96570_spi_reset_OFST		(1)
 #define lm96570_start_OFST			(0)

 // mask for output control signal
 #define ADC_AD9276_PWDN_MSK			(1<<ADC_AD9276_PWDN_OFST			)
 #define ADC_AD9276_STBY_MSK			(1<<ADC_AD9276_STBY_OFST			)
 #define FSM_RESET_MSK				(1<<FSM_RESET_OFST				)
 #define sw_off_MSK					(1<<sw_off_OFST				)
 #define tx_path_en_MSK				(1<<tx_path_en_OFST			)
 #define pulser_en_MSK				(1<<pulser_en_OFST			)
 #define lm96570_pin_reset_MSK		(1<<lm96570_pin_reset_OFST	)
 #define lm96570_tx_en_MSK			(1<<lm96570_tx_en_OFST		)
 #define lm96570_spi_reset_MSK		(1<<lm96570_spi_reset_OFST	)
 #define lm96570_start_MSK			(1<<lm96570_start_OFST		)
 */

// OUTPUT
#define ADC_AD9276_PWDN_ofst		(3)
#define ADC_AD9276_STBY_ofst		(2)
#define FSM_RESET_ofst				(1)
#define BF_TX_EN_ofst				(0)

#define ADC_AD9276_PWDN_msk         (1<<ADC_AD9276_PWDN_ofst     ) // (CAREFUL! SOMETIMES PUTTING THE ADC TO STANDBY WOULD CAUSE IT TO CRASH)
#define ADC_AD9276_STBY_msk         (1<<ADC_AD9276_STBY_ofst     )
#define FSM_RESET_msk               (1<<FSM_RESET_ofst           )
#define BF_TX_EN_msk                (1<<BF_TX_EN_ofst            )

// INPUT
#define adc_pll_locked_ofst	(1)
#define FSM_DONE_ofst          (0)

#define adc_pll_locked_msk	(1<< pulse_pll_locked_msk)
#define FSM_DONE_msk           (1<<FSM_DONE_ofst        )

// default for output control signal
#define CNT_OUT_DEFAULT 0b000000

// general variable
#define ENABLE_MESSAGE	1
#define DISABLE_MESSAGE 0
#define ENABLE 1
#define DISABLE 0

// save format
#define SAV_BINARY		1 // save data in binary format
#define SAV_ASCII		0 // save data in ascii format
