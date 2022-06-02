/*
 * common_functions.h
 *
 *  Created on: May 9, 2020
 *      Author: David Ariando
 */

#ifndef FUNCTIONS_COMMON_FUNCTIONS_H_
#define FUNCTIONS_COMMON_FUNCTIONS_H_

unsigned int rd_FIFO(volatile unsigned int *FIFO_status_addr, void *FIFO_data_addr, int * buf32);
void buf32_to_buf16(uint32_t * buf32, uint16_t * buf16, unsigned int length);
void cut_2MSB_and_2LSB(uint16_t * buf16, unsigned int length);
void sum_buf(int32_t * buf32, uint16_t * buf16, unsigned int length, unsigned char subtract);
void avg_buf(int32_t * buf32, unsigned int length, unsigned char div_fact);
void wr_File_16b(char * pathname, unsigned int length, uint16_t * buf, char binary_OR_ascii);
void wr_File_32b(char * pathname, unsigned int length, int32_t * buf, char binary_OR_ascii);
void wr_File_64b(char * pathname, unsigned int length, long long * buf, char binary_OR_ascii);
void print_progress(int iterate, int number_of_iteration);

#endif /* FUNCTIONS_COMMON_FUNCTIONS_H_ */
