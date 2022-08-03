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
#define GRADDRV_CNT_N_ofst			(17)
#define GRADDRV_CNT_P_ofst			(16)
#define GRADDRV_3P3_N_ofst			(15)
#define GRADDRV_3P3_P_ofst			(14)
#define DAC_PAMP_LDAC_ofst			(13)
#define DAC_PAMP_CLR_ofst			(12)
#define CHG_HBRIDGE_ofst			(11)
#define CHG_BS_ofst					(10)
#define DCHG_BS_ofst				(9)
#define SYS_PLL_RST_ofst			(8)
#define BITSTR_ADV_RST_ofst			(7)
#define BITSTR_ADV_START_ofst		(6)
#define DAC_GRAD_LDAC_ofst			(5)
#define DAC_GRAD_CLR_ofst			(4)
#define ADC_AD9276_PWDN_ofst		(3)
#define ADC_AD9276_STBY_ofst		(2)
#define FSM_RESET_ofst				(1)
#define FSM_START_ofst				(0)

#define GRADDRV_CNT_N			(1<<GRADDRV_CNT_N_ofst)
#define GRADDRV_CNT_P			(1<<GRADDRV_CNT_P_ofst)
#define GRADDRV_3P3_N			(1<<GRADDRV_3P3_N_ofst)
#define GRADDRV_3P3_P			(1<<GRADDRV_3P3_P_ofst)
#define DAC_PAMP_LDAC				(1<<DAC_PAMP_LDAC_ofst)
#define DAC_PAMP_CLR				(1<<DAC_PAMP_CLR_ofst)
#define CHG_HBRIDGE					(1<<CHG_HBRIDGE_ofst)
#define CHG_BS						(1<<CHG_BS_ofst)
#define DCHG_BS						(1<<DCHG_BS_ofst)
#define SYS_PLL_RST					(1<<SYS_PLL_RST_ofst)
#define BITSTR_ADV_RST				(1<<BITSTR_ADV_RST_ofst)
#define BITSTR_ADV_START			(1<<BITSTR_ADV_START_ofst)
#define DAC_GRAD_LDAC_msk			(1<<DAC_GRAD_LDAC_ofst)
#define DAC_GRAD_CLR_msk			(1<<DAC_GRAD_CLR_ofst)
#define ADC_AD9276_PWDN_msk         (1<<ADC_AD9276_PWDN_ofst) // (CAREFUL! SOMETIMES PUTTING THE ADC TO STANDBY WOULD CAUSE IT TO CRASH)
#define ADC_AD9276_STBY_msk         (1<<ADC_AD9276_STBY_ofst)
#define FSM_RESET_msk               (1<<FSM_RESET_ofst)
#define FSM_START_msk				(1<<FSM_START_ofst)

// INPUT
#define fco_locked_ofst			(4)
#define bitstr_adv_done_ofst	(3)
#define tx_h1_done_ofst			(2)
#define sys_pll_locked_ofst		(1)
#define FSM_DONE_ofst			(0)

#define fco_locked				(1<<fco_locked_ofst)
#define bitstr_adv_done			(1<<bitstr_adv_done_ofst)
#define tx_h1_done				(1<<tx_h1_done_ofst)
#define sys_pll_locked			(1<<sys_pll_locked_ofst)
#define FSM_DONE_msk			(1<<FSM_DONE_ofst)

// default for output control signal
#define CNT_OUT_DEFAULT (DAC_GRAD_CLR_msk | DAC_GRAD_LDAC_msk | ADC_AD9276_PWDN_msk | ADC_AD9276_STBY_msk | GRADDRV_3P3_P | GRADDRV_3P3_N)

// general variable
#define ENABLE_MESSAGE	1
#define DISABLE_MESSAGE 0
#define ENABLE 1
#define DISABLE 0

// bitstream status
typedef unsigned int error_code;
#define SEQ_ERROR	1
#define SEQ_OK		0

// save format
#define SAV_BINARY		1 // save data in binary format
#define SAV_ASCII		0 // save data in ascii format
