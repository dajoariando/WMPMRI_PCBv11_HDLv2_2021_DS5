
void check_dma(volatile unsigned int * dma_addr, uint8_t en_mesg);

void fifo_to_sdram_dma_trf(volatile unsigned int * dma_addr, uint32_t rd_addr, uint32_t wr_addr, uint32_t transfer_length);

void reset_dma(volatile unsigned int * dma_addr);
