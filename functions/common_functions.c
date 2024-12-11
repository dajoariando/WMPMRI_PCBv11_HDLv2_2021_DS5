#include <socal/socal.h>
#include <stdint.h>
#include <stdio.h>

#include "../variables/altera_avalon_fifo_regs.h"

void buf32_to_buf16(uint32_t * buf32, uint16_t * buf16, unsigned int length) {
	unsigned int i, j;

	j = 0;
	for (i = 0; i < ( length ); i++) {
		buf16[j++] = ( (unsigned int) buf32[i] );   // 12 significant bit
		buf16[j++] = ( (unsigned int) ( buf32[i] >> 16 ) );   // 12 significant bit
	}

}

void cut_2MSB_and_2LSB(uint16_t * buf16, unsigned int length) {
	int ii;
	for (ii = 0; ii < length; ii++) {
		buf16[ii] = ( buf16[ii] >> 2 ) & 0xFFF;
	}
}

void sum_buf(int32_t * buf32, uint16_t * buf16, unsigned int length, unsigned char subtract) {
	int ii;
	int32_t temp;   // temporary file to convert uint16_t to int32_t

	for (ii = 0; ii < length; ii++) {
		temp = 0;
		temp = temp | buf16[ii];
		if (subtract)
			buf32[ii] -= temp;
		else
			buf32[ii] += temp;
	}
}

void sum_buf_to_float(float * buf32, uint16_t * buf16, unsigned int length, unsigned char subtract) {
	int ii;
	int32_t temp;   // temporary file to convert uint16_t to int32_t

	for (ii = 0; ii < length; ii++) {
		temp = 0;
		temp = temp | buf16[ii];

		if (subtract)
			buf32[ii] -= (float) temp;
		else
			buf32[ii] += (float) temp;

	}
}

void sum_buf_sq_to_float(float * buf32, uint16_t * buf16, unsigned int length) {
	int ii;
	int32_t temp;   // temporary file to convert uint16_t to int32_t

	for (ii = 0; ii < length; ii++) {
		temp = 0;
		temp = temp | buf16[ii];

		buf32[ii] += (float) temp * (float) temp;

	}
}

void avg_buf(int32_t * buf32, unsigned int length, unsigned int div_fact) {
	int ii;

	for (ii = 0; ii < length; ii++) {
		buf32[ii] /= div_fact;
	}
}

void avg_buf_float(float * buf32, unsigned int length, unsigned int div_fact) {
	int ii;

	for (ii = 0; ii < length; ii++) {
		buf32[ii] /= (float) div_fact;
	}
}

void wr_File_16b(char * pathname, unsigned int length, int16_t * buf, char binary_OR_ascii) {

	// binary_OR_ascii = 1 save binary output into the text file (1). Otherwise, it'll be ASCII output (0)

	FILE *fptr;

	fptr = fopen(pathname, "w");
	if (fptr == NULL) {
		printf("File does not exists \n");
	}

	if (binary_OR_ascii) {   // binary output
		fwrite(buf, sizeof(int16_t), length, fptr);
	}

	else {   // ascii output
		long i;

		for (i = 0; i < ( ( length ) ); i++) {
			fprintf(fptr, "%d\n", buf[i]);
		}

	}

	fclose(fptr);

}

void wr_File_32b(char * pathname, unsigned int length, int32_t * buf, char binary_OR_ascii) {

	// binary_OR_ascii = 1 save binary output into the text file (1). Otherwise, it'll be ASCII output (0)

	FILE *fptr;

	fptr = fopen(pathname, "w");
	if (fptr == NULL) {
		printf("File does not exists \n");
	}

	if (binary_OR_ascii) {   // binary output
		fwrite(buf, sizeof(int32_t), length, fptr);
	}

	else {   // ascii output
		long i;

		for (i = 0; i < ( ( length ) ); i++) {
			fprintf(fptr, "%d\n", buf[i]);
		}

	}

	fclose(fptr);

}

void wr_File_float(char * pathname, unsigned int length, float * buf, char binary_OR_ascii) {

	// binary_OR_ascii = 1 save binary output into the text file (1). Otherwise, it'll be ASCII output (0)

	FILE *fptr;

	fptr = fopen(pathname, "w");
	if (fptr == NULL) {
		printf("File does not exists \n");
	}

	if (binary_OR_ascii) {   // binary output
		fwrite(buf, sizeof(float), length, fptr);
	}

	else {   // ascii output
		long i;

		for (i = 0; i < ( ( length ) ); i++) {
			fprintf(fptr, "%f\n", buf[i]);
		}

	}

	fclose(fptr);

}

void wr_File_64b(char * pathname, unsigned int length, long long * buf, char binary_OR_ascii) {

	// binary_OR_ascii = 1 save binary output into the text file (1). Otherwise, it'll be ASCII output (0)

	FILE *fptr;

	fptr = fopen(pathname, "w");
	if (fptr == NULL) {
		printf("File does not exists \n");
	}

	if (binary_OR_ascii) {   // binary output
		fwrite(buf, sizeof(long long), length, fptr);
	}

	else {   // ascii output
		long i;

		for (i = 0; i < ( ( length ) ); i++) {
			fprintf(fptr, "%lld\n", buf[i]);
		}

	}

	fclose(fptr);

}

void print_progress(int iterate, int number_of_iteration) {
	float static old_progress = 0;
	float new_progress = (float) iterate / (float) number_of_iteration * 100;

	if (new_progress >= ( old_progress + 10 )) {
		printf("\t %0.1f%%\n", new_progress);
		old_progress = new_progress;
	}
	new_progress = (float) iterate / (float) number_of_iteration * 100;
}
