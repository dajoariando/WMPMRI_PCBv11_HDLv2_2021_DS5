#ifndef GRAD_DRV_H
#define GRAD_DRV_H

// grad init current programs the 4 voltage of the DAC corresponds to the current as an input.
// it also disables the high-side switches of the DAC, and select the default DAC to drive the gradient gate.
// as DAC_A and DAC_C is the default DAC, DAC_A and DAC_C can be used to select bias current of the DAC.
// however, as the high-side switches are disabled, means the feedback circuitry will always sense 0A of
// current, the feedback circuit will cause the voltage to be VDD at the gate, fully turning-on the low-side
// FETs, and thus, charging the high-side switch capacitor.
void grad_init_current (double i_ChA, double i_ChB, double i_ChC, double i_ChD, uint8_t DAC_SEL);




#endif
