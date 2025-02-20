#ifndef _DAC_MCP4728_VARS_H_
#define _DAC_MCP4728_VARS_H_

// set VREF to be external VDD or internal reference (2.048V).
#define VREF_EXTERN (0) // select VDD as VREF
#define VREF_INTERN (1) // select 2.048V internal reference as VREF

// set the gain
#define GAIN_1X (0) // set amplifier gain to 1x. In the case of external reference, gain is 1x.
#define GAIN_2X (1) // set amplifier gain to 2x. In the case of external reference, gain is 2x.

// select channel
#define CH_DACA (0)
#define CH_DACB (1)
#define CH_DACC (2)
#define CH_DACD (3)

// i2c address of the DAC (this is programmed via SIGRES_GRAD_DAC_PROG_Quartus. Otherwise all channel will be at address 0)
#define DAC_X (1)
#define DAC_Y (2)
#define DAC_Z (3)
#define DAC_Z2 (4)

// select power down mode
#define PWR_NORM (0) // no power down, normal operation
#define PWR_1K (1) // power down with 1K output impedance
#define PWR_100K (2) // power down with 100K output impedance
#define PWR_500K (3) // power down with 500K output impedance

// select UDAC
#define UDAC_NO_UPDT (1) // do not update DAC after I2C routine
#define UDAC_DO_UPDT (0) // update DAC after I2C routine

// number of bits
#define BITMASK (0xFFF) // DAC bitmask (12 bit)

#endif
