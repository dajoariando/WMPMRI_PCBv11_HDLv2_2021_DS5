#include <stdio.h>
#include <unistd.h>
#include <socal/socal.h>

#include "../variables/avalon_dma.h"
#include "../variables/hps_soc_system.h"
#include "../variables/general.h"
#include "./dma.h"

void check_dma(volatile unsigned int * dma_addr, uint8_t en_mesg) {
	// this function waits until the dma addressed finishes its operation
	unsigned int dma_status;
	do {
		dma_status = alt_read_word(dma_addr + DMA_STATUS_OFST);
		if (en_mesg) {
			printf("\tDMA Status reg: 0x%x\n", dma_status);
			if (! ( dma_status & DMA_STAT_DONE_MSK )) {
				printf("\tDMA transaction is not done.\n");
			}
			if (dma_status & DMA_STAT_BUSY_MSK) {
				printf("\tDMA is busy.\n");

				// print length register
				printf("\t--> DMA length register: %d\n", alt_read_word(dma_addr + DMA_LENGTH_OFST));   // set transfer length (in byte, so multiply by 4 to get word-addressing));

				// wait time in case of DMA busy. Only valid under ENABLE_MESSAGE because the loop needs to be fast otherwise
				unsigned int wait_time_ms = 500;
				usleep(wait_time_ms * 1000);   // wait time to prevent overloading the DMA bus arbitration request only if the dma is busy
				printf("\t---> waiting for %d ms ...\n", wait_time_ms);
			}
		}
	}
	while (! ( dma_status & DMA_STAT_DONE_MSK ) || ( dma_status & DMA_STAT_BUSY_MSK ));   // keep in the loop when the 'DONE' bit is '0' and 'BUSY' bit is '1'
	if (en_mesg) {
		if (dma_status & DMA_STAT_REOP_MSK) {
			printf("\tDMA transaction completed due to end-of-packet on read side.\n");
		}
		if (dma_status & DMA_STAT_WEOP_MSK) {
			printf("\tDMA transaction completed due to end-of-packet on write side.\n");
		}
		if (dma_status & DMA_STAT_LEN_MSK) {
			printf("\tDMA transaction completed due to length-register decrements to 0.\n");
		}
	}
}

void fifo_to_sdram_dma_trf(volatile unsigned int * dma_addr, uint32_t rd_addr, uint32_t wr_addr, uint32_t transfer_length) {
	// the original conventional code
	alt_write_word(dma_addr + DMA_CONTROL_OFST, DMA_CTRL_SWRST_MSK);   // write twice to do software reset
	alt_write_word(dma_addr + DMA_CONTROL_OFST, DMA_CTRL_SWRST_MSK);   // software resetted
	alt_write_word(dma_addr + DMA_STATUS_OFST, 0x0); 	// clear the DONE bit
	alt_write_word(dma_addr + DMA_READADDR_OFST, rd_addr);   // set DMA read address
	alt_write_word(dma_addr + DMA_WRITEADDR_OFST, wr_addr);   // set DMA write address
	alt_write_word(dma_addr + DMA_LENGTH_OFST, transfer_length * 4);   // set transfer length (in byte, so multiply by 4 to get word-addressing)
	alt_write_word(dma_addr + DMA_CONTROL_OFST, ( DMA_CTRL_WORD_MSK | DMA_CTRL_LEEN_MSK | DMA_CTRL_RCON_MSK ));   // set settings for transfer
	alt_write_word(dma_addr + DMA_CONTROL_OFST, ( DMA_CTRL_WORD_MSK | DMA_CTRL_LEEN_MSK | DMA_CTRL_RCON_MSK | DMA_CTRL_GO_MSK ));   // set settings & also enable transfer
	//

	/* burst method (tested. Doesn't really work with large data due to slow reinitialization. Generally burst mode only works and faster when the amount of data is less than max burst size. Otherwise It doesn't work)
	 uint32_t burst_size = 1024; // the max burst size is set in QSYS in DMA section
	 uint32_t transfer_remain = transfer_length*4; // multiplied by 4 to get word-addressing
	 alt_write_word(dma_addr+DMA_CONTROL_OFST,	DMA_CTRL_SWRST_MSK); 	// write twice to do software reset
	 alt_write_word(dma_addr+DMA_CONTROL_OFST,	DMA_CTRL_SWRST_MSK); 	// software resetted
	 alt_write_word(dma_addr+DMA_READADDR_OFST,	rd_addr); 				// set DMA read address (fifo)
	 uint32_t iaddr = 0;
	 while (transfer_remain>burst_size) { // transfer with burst_size when remaining data is larger than burst_size
	 alt_write_word(dma_addr+DMA_WRITEADDR_OFST,	wr_addr + iaddr);		// set DMA write address
	 alt_write_word(dma_addr+DMA_LENGTH_OFST, burst_size);				// set transfer length (in byte, so multiply by 4 to get word-addressing)
	 alt_write_word(dma_addr+DMA_CONTROL_OFST,	(DMA_CTRL_WORD_MSK|DMA_CTRL_LEEN_MSK|DMA_CTRL_RCON_MSK)); // set settings for transfer
	 alt_write_word(dma_addr+DMA_CONTROL_OFST,	(DMA_CTRL_WORD_MSK|DMA_CTRL_LEEN_MSK|DMA_CTRL_RCON_MSK|DMA_CTRL_GO_MSK)); // set settings & also enable transfer
	 transfer_remain -= burst_size;
	 iaddr += (burst_size/4);
	 check_dma(dma_addr, DISABLE_MESSAGE); // wait for the dma operation to complete
	 }
	 if (transfer_remain>0) { // transfer with the remaining data
	 alt_write_word(dma_addr+DMA_WRITEADDR_OFST,	wr_addr + iaddr);		// set DMA write address
	 alt_write_word(dma_addr+DMA_LENGTH_OFST, transfer_remain);			// set transfer length (in byte, so multiply by 4 to get word-addressing)
	 alt_write_word(dma_addr+DMA_CONTROL_OFST,	(DMA_CTRL_WORD_MSK|DMA_CTRL_LEEN_MSK|DMA_CTRL_RCON_MSK)); // set settings for transfer
	 alt_write_word(dma_addr+DMA_CONTROL_OFST,	(DMA_CTRL_WORD_MSK|DMA_CTRL_LEEN_MSK|DMA_CTRL_RCON_MSK|DMA_CTRL_GO_MSK)); // set settings & also enable transfer
	 check_dma(dma_addr, DISABLE_MESSAGE); // wait for the dma operation to complete
	 }
	 */
}

void reset_dma(volatile unsigned int * dma_addr) {
	alt_write_word(dma_addr + DMA_CONTROL_OFST, DMA_CTRL_SWRST_MSK);   // write twice to do software reset
	alt_write_word(dma_addr + DMA_CONTROL_OFST, DMA_CTRL_SWRST_MSK);   // software resetted
}
