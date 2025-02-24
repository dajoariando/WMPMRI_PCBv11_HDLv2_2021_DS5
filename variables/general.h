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
#define GRAD_AUX_ofst			(29)	// GRAD_AUX_ofst				
#define GRADX_LO_L_SOC_ofst		(28)	// GRADX_CNT_N_ofst			
#define GRADX_LO_R_SOC_ofst		(27)	// GRADX_CNT_P_ofst			
#define GRADY_LO_L_SOC_ofst		(26)	// GRADY_CNT_N_ofst			
#define GRADY_LO_R_SOC_ofst		(25)	// GRADY_CNT_P_ofst			
#define GRADZ_LO_L_SOC_ofst		(24)	// GRADZ_CNT_N_ofst			
#define GRADZ_LO_R_SOC_ofst		(23)	// GRADZ_CNT_P_ofst			
#define GRADZ2_LO_L_SOC_ofst	(22)	// GRADZ2_CNT_N_ofst			
#define GRADZ2_LO_R_SOC_ofst	(21)	// GRADZ2_CNT_P_ofst			
#define GRADX_HI_R_SOC_ofst		(20)	// GRADX_HL_ofst				
#define GRADX_HI_L_SOC_ofst		(19)	// GRADX_HR_ofst				
#define GRADY_HI_R_SOC_ofst		(18)	// GRADY_HL_ofst				
#define GRADY_HI_L_SOC_ofst		(17)	// GRADY_HR_ofst				
#define GRADZ_HI_R_SOC_ofst		(16)	// GRADZ_HL_ofst				
#define GRADZ_HI_L_SOC_ofst		(15)	// GRADZ_HR_ofst				
#define GRADZ2_HI_R_SOC_ofst	(14)	// GRADZ2_HL_ofst				
#define GRADZ2_HI_L_SOC_ofst	(13)	// GRADZ2_HR_ofst				
#define GRAD_OE_ofst			(12)	// GRAD_OE_ofst				
#define DAC_PAMP_LDAC_ofst		(11)	// DAC_PAMP_LDAC_ofst			
#define DAC_PAMP_CLR_ofst		(10)	// DAC_PAMP_CLR_ofst			
#define CHG_HBRIDGE_ofst		(9)		// CHG_HBRIDGE_ofst			
#define CHG_BS_ofst				(8)		// CHG_BS_ofst					
#define DCHG_BS_ofst			(7)		// DCHG_BS_ofst				
#define SYS_PLL_RST_ofst		(6)		// SYS_PLL_RST_ofst			
#define BITSTR_ADV_RST_ofst		(5)		// BITSTR_ADV_RST_ofst			
#define BITSTR_ADV_START_ofst	(4)		// BITSTR_ADV_START_ofst		
#define ADC_AD9276_PWDN_ofst	(3)		// ADC_AD9276_PWDN_ofst		
#define ADC_AD9276_STBY_ofst	(2)		// ADC_AD9276_STBY_ofst		
#define FSM_RESET_ofst			(1)		// FSM_RESET_ofst				
#define FSM_START_ofst			(0)		// FSM_START_ofst				

#define GRAD_AUX					(1<<GRAD_AUX_ofst)
#define GRADX_LO_L_SOC				(1<<GRADX_LO_L_SOC_ofst)
#define GRADX_LO_R_SOC				(1<<GRADX_LO_R_SOC_ofst)
#define GRADY_LO_L_SOC				(1<<GRADY_LO_L_SOC_ofst)
#define GRADY_LO_R_SOC				(1<<GRADY_LO_R_SOC_ofst)
#define GRADZ_LO_L_SOC				(1<<GRADZ_LO_L_SOC_ofst)
#define GRADZ_LO_R_SOC				(1<<GRADZ_LO_R_SOC_ofst)
#define GRADZ2_LO_L_SOC				(1<<GRADZ2_LO_L_SOC_ofst)
#define GRADZ2_LO_R_SOC				(1<<GRADZ2_LO_R_SOC_ofst)
#define GRADX_HI_R_SOC				(1<<GRADX_HI_R_SOC_ofst)
#define GRADX_HI_L_SOC				(1<<GRADX_HI_L_SOC_ofst)
#define GRADY_HI_R_SOC				(1<<GRADY_HI_R_SOC_ofst)
#define GRADY_HI_L_SOC				(1<<GRADY_HI_L_SOC_ofst)
#define GRADZ_HI_R_SOC				(1<<GRADZ_HI_R_SOC_ofst)
#define GRADZ_HI_L_SOC				(1<<GRADZ_HI_L_SOC_ofst)
#define GRADZ2_HI_R_SOC				(1<<GRADZ2_HI_R_SOC_ofst)
#define GRADZ2_HI_L_SOC				(1<<GRADZ2_HI_L_SOC_ofst)
#define GRAD_OE						(1<<GRAD_OE_ofst)
#define DAC_PAMP_LDAC				(1<<DAC_PAMP_LDAC_ofst)
#define DAC_PAMP_CLR				(1<<DAC_PAMP_CLR_ofst)
#define CHG_HBRIDGE					(1<<CHG_HBRIDGE_ofst)
#define CHG_BS						(1<<CHG_BS_ofst)
#define DCHG_BS						(1<<DCHG_BS_ofst)
#define SYS_PLL_RST					(1<<SYS_PLL_RST_ofst)
#define BITSTR_ADV_RST				(1<<BITSTR_ADV_RST_ofst)
#define BITSTR_ADV_START			(1<<BITSTR_ADV_START_ofst)
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
#define CNT_OUT_DEFAULT ( ADC_AD9276_PWDN_msk | ADC_AD9276_STBY_msk )

// general variables
#define ENABLE_MESSAGE	1
#define DISABLE_MESSAGE 0
#define ENABLE 1
#define DISABLE 0

// data read variables
#define RD_FIFO 1 // read from FIFO
#define RD_DMA 0 // read from DMA

// wait variables
#define WAIT 1 // wait until done
#define NOWAIT 0 // no wait

// bitstream status
typedef unsigned int error_code;
#define SEQ_ERROR	0
#define SEQ_OK		1

// save format
#define SAV_BINARY		1 // save data in binary format
#define SAV_ASCII		0 // save data in ascii format
