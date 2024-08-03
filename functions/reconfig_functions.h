/*
 author: David Joseph Ariando, Nov 2016
 for detailed explanation of this code, refer to Altera's paper "Implementing
 Fractional PLL Reconfiguration with Altera PLL and Altera PLL Reconfig IP Cores".
 All abbreviation here are consistent with what are explained in the document.
 */

#include <stdint.h>

// GENERAL WRITE/READ REGISTER OFFSET (BYTE-ADDRESSING, when addressed using void*)
// #define MODE      		0x00
// #define STATUS    		0x04
// #define START     		0x08
// #define N_COUNTER 		0x0C
// #define M_COUNTER 		0x10
// #define C_COUNTER 		0x14
// #define DPS_REG			0x18
// #define FRAC_REG		0x1C
// #define BS_REG			0x20
// #define CPS_REG			0x24
// #define VCO_DIV_REG		0x70
// #define MIF_BASE_ADDR	0x7C
// READ REGISTER OFFSET
// #define C00_COUNTER		0x28
// #define C01_COUNTER		0x2C
// #define C02_COUNTER		0x30
// #define C03_COUNTER		0x34
// #define C04_COUNTER		0x38
// #define C05_COUNTER		0x3C
// #define C06_COUNTER		0x40
// #define C07_COUNTER		0x44
// #define C08_COUNTER		0x48
// #define C09_COUNTER		0x4C
// #define C10_COUNTER		0x50
// #define C11_COUNTER		0x54
// #define C12_COUNTER		0x58
// #define C13_COUNTER		0x5C
// #define C14_COUNTER		0x60
// #define C15_COUNTER		0x64
// #define C16_COUNTER		0x68
// #define C17_COUNTER		0x6C

// GENERAL WRITE/READ REGISTER OFFSET (WORD-ADDRESSING, when addressed using *unsigned int)
#define MODE      		0x00
#define STATUS    		0x01
#define START     		0x02
#define N_COUNTER 		0x03
#define M_COUNTER 		0x04
#define C_COUNTER 		0x05
#define DPS_REG			0x06
#define FRAC_REG		0x07
#define BS_REG			0x08
#define CPS_REG			0x09
#define VCO_DIV_REG		0x1C
#define MIF_BASE_ADDR	0x1F
// READ REGISTER OFFSET
#define C00_COUNTER		0x0A
#define C01_COUNTER		0x0B
#define C02_COUNTER		0x0C
#define C03_COUNTER		0x0D
#define C04_COUNTER		0x0E
#define C05_COUNTER		0x0F
#define C06_COUNTER		0x10
#define C07_COUNTER		0x11
#define C08_COUNTER		0x12
#define C09_COUNTER		0x13
#define C10_COUNTER		0x14
#define C11_COUNTER		0x15
#define C12_COUNTER		0x16
#define C13_COUNTER		0x17
#define C14_COUNTER		0x18
#define C15_COUNTER		0x19
#define C16_COUNTER		0x1A
#define C17_COUNTER		0x1B

void Reconfig_Mode(volatile unsigned int * addr, uint32_t val);
void Reconfig_N(volatile unsigned int * addr, uint32_t low_count, uint32_t high_count, uint32_t bypass_enable, uint32_t odd_division);
void Reconfig_M(volatile unsigned int * addr, uint32_t low_count, uint32_t high_count, uint32_t bypass_enable, uint32_t odd_division);
void Reconfig_C(volatile unsigned int * addr, uint32_t counter_select, uint32_t low_count, uint32_t high_count, uint32_t bypass_enable, uint32_t odd_division);
void Reconfig_DPS(volatile unsigned int * addr, uint32_t DPS_select, uint32_t DPS, uint32_t DPS_direction);
void Reconfig_MFrac(volatile unsigned int * addr, uint32_t MFrac);
void Reconfig_BS(volatile unsigned int * addr, uint32_t BS);
void Reconfig_CPS(volatile unsigned int * addr, uint32_t CPS);
void Reconfig_VCO_DIV(volatile unsigned int * addr, uint32_t VCO_DIV);
void Start_Reconfig(volatile unsigned int * addr, uint32_t enable_message);
void Read_Reconfig_Registers(volatile unsigned int * addr);
uint32_t Read_C_Counter(volatile unsigned int * addr, uint32_t counter_select);
void Reset_PLL(volatile unsigned int *ctl_out_reg, uint32_t rst_ofst, uint32_t ctrl_out_signal);
void Wait_PLL_To_Lock(volatile unsigned int *ctl_in_reg, uint32_t lock_ofst);
