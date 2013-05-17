/*
 * aes_dma.c
 *
 * Created: 5/17/2013 11:07:18 AM
 *  Author: CellaSecure
 */ 

#include "aes_dma.h"

// CipherKey array
// the_key = 256'h603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4;
static const unsigned int      CipherKey[8] = {
	0x603deb10,
	0x15ca71be,
	0x2b73aef0,
	0x857d7781,
	0x1f352c07,
	0x3b6108d7,
	0x2d9810a3,
	0x0914dff4
};

// InitVector array
// Initial Value 128'h000102030405060708090a0b0c0d0e0f
static const unsigned int      InitVector[4] = {
	0x00010203,
	0x04050607,
	0x08090a0b,
	0x0c0d0e0f
};

void ram_aes_ram(bool encrypt, unsigned short int u16BufferSize, unsigned int *pSrcBuf, unsigned int *pDstBuf)
{
	aes_config_t      AesConf;
	//====================
	// Configure the AES.
	//====================
	AesConf.ProcessingMode = encrypt ? AES_PMODE_CIPHER : AES_PMODE_DECIPHER;  // Cipher
	AesConf.ProcessingDelay = 0;                // No delay: best performance
	AesConf.StartMode = AES_START_MODE_DMA;     // DMA mode
	AesConf.KeySize = AES_KEY_SIZE_256;         // 256bit cryptographic key
	AesConf.OpMode = AES_CBC_MODE;              // CBC cipher mode
	AesConf.LodMode = 0;                        // LODMODE == 0 : the end of the
	// encryption is notified by the DMACA transfer complete interrupt. The output
	// is available in the OutputData[] buffer.
	AesConf.CFBSize = 0;                        // Don't-care because we're using the CBC mode.
	AesConf.CounterMeasureMask = 0;             // Disable all counter measures.
	aes_configure(&AVR32_AES, &AesConf);

	//====================
	// Configure the DMACA.
	//====================
	// Enable the DMACA
	AVR32_DMACA.dmacfgreg = 1 << AVR32_DMACA_DMACFGREG_DMA_EN_OFFSET;

	//*
	//* Configure the DMA RAM -> AES channel.
	//*
	// ------------+--------+------+------------+--------+-------+----------------
	//  Transfer   | Source | Dest | Flow       | Width  | Chunk | Buffer
	//  type       |        |      | controller | (bits) | size  | Size
	// ------------+--------+------+------------+--------+-------+----------------
	//             |        |      |            |        |       |
	//  Mem-to-Per |  RAM   | AES  |   DMACA    |   32   |   4   | u16BufferSize
	//             |        |      |            |        |       |
	// ------------+--------+------+------------+--------+-------+----------------
	// NOTE: We arbitrarily choose to use channel 0 for this datapath

	// Src Address: the InputData[] array
	AVR32_DMACA.sar0 = (unsigned long)pSrcBuf;

	// Dst Address: the AES_IDATAXR registers.
	AVR32_DMACA.dar0 = (AVR32_AES_ADDRESS | AVR32_AES_IDATA1R);

	// Linked list ptrs: not used.
	AVR32_DMACA.llp0 = 0x00000000;

	// Channel 0 Ctrl register low
	AVR32_DMACA.ctl0l =
	(0       << AVR32_DMACA_CTL0L_INT_EN_OFFSET)       | // Do not enable interrupts
	(2       << AVR32_DMACA_CTL0L_DST_TR_WIDTH_OFFSET) | // Dst transfer width: 32 bits
	(2       << AVR32_DMACA_CTL0L_SRC_TR_WIDTH_OFFSET) | // Src transfer width: 32 bits
	(2       << AVR32_DMACA_CTL0L_DINC_OFFSET)         | // Dst address increment: none
	(0       << AVR32_DMACA_CTL0L_SINC_OFFSET)         | // Src address increment: increment
	(1       << AVR32_DMACA_CTL0L_DST_MSIZE_OFFSET)    | // Dst burst transaction len: 4 data items
	(1       << AVR32_DMACA_CTL0L_SRC_MSIZE_OFFSET)    | // Src burst transaction len: 4 data items
	(0       << AVR32_DMACA_CTL0L_S_GATH_EN_OFFSET)    | // Source gather: disabled
	(0       << AVR32_DMACA_CTL0L_D_SCAT_EN_OFFSET)    | // Destination scatter: disabled
	(1       << AVR32_DMACA_CTL0L_TT_FC_OFFSET)        | // transfer type:M2P, flow controller: DMACA
	(0       << AVR32_DMACA_CTL0L_DMS_OFFSET)          | // Dest master: HSB master 1
	(1       << AVR32_DMACA_CTL0L_SMS_OFFSET)          | // Source master: HSB master 2
	(0       << AVR32_DMACA_CTL0L_LLP_D_EN_OFFSET)     | // Not used
	(0       << AVR32_DMACA_CTL0L_LLP_S_EN_OFFSET)       // Not used
	;

	// Channel 0 Ctrl register high
	AVR32_DMACA.ctl0h =
	(u16BufferSize << AVR32_DMACA_CTL0H_BLOCK_TS_OFFSET) | // Block transfer size
	(0             << AVR32_DMACA_CTL0H_DONE_OFFSET)       // Not done
	;

	// Channel 0 Config register low
	AVR32_DMACA.cfg0l =
	(0 << AVR32_DMACA_CFG0L_HS_SEL_DST_OFFSET) | // Destination handshaking: hw handshaking
	(0 << AVR32_DMACA_CFG0L_HS_SEL_SRC_OFFSET)   // Source handshaking: ignored because the src is memory.
	; // All other bits set to 0.

	// Channel 0 Config register high
	AVR32_DMACA.cfg0h =
	(AVR32_DMACA_CH_AES_TX << AVR32_DMACA_CFG0H_DEST_PER_OFFSET) | // Dest hw handshaking itf:
	(0                     << AVR32_DMACA_CFG0H_SRC_PER_OFFSET)    // Source hw handshaking itf: ignored because the src is memory.
	; // All other bits set to 0.


	//*
	//* Configure the DMA AES -> RAM channel.
	//*
	// ------------+--------+------+------------+--------+-------+----------------
	//  Transfer   | Source | Dest | Flow       | Width  | Chunk | Buffer
	//  type       |        |      | controller | (bits) | size  | Size
	// ------------+--------+------+------------+--------+-------+----------------
	//             |        |      |            |        |       |
	//  Per-to-Mem |  AES   | RAM  |   DMACA    |   32   |   4   | u16BufferSize
	//             |        |      |            |        |       |
	// ------------+--------+------+------------+--------+-------+----------------
	// NOTE: We arbitrarily choose to use channel 1 for this datapath

	// Src Address: the AES_ODATAXR registers.
	AVR32_DMACA.sar1 = (AVR32_AES_ADDRESS | AVR32_AES_ODATA1R);

	// Dst Address: the OutputData[] array.
	AVR32_DMACA.dar1 = (unsigned long)pDstBuf;

	// Linked list ptrs: not used.
	AVR32_DMACA.llp1 = 0x00000000;

	// Channel 1 Ctrl register low
	AVR32_DMACA.ctl1l =
	(0 << AVR32_DMACA_CTL1L_INT_EN_OFFSET)       | // Do not enable interrupts
	(2 << AVR32_DMACA_CTL1L_DST_TR_WIDTH_OFFSET) | // Dst transfer width: 32 bits
	(2 << AVR32_DMACA_CTL1L_SRC_TR_WIDTH_OFFSET) | // Src transfer width: 32 bits
	(0 << AVR32_DMACA_CTL1L_DINC_OFFSET)         | // Dst address increment: increment
	(2 << AVR32_DMACA_CTL1L_SINC_OFFSET)         | // Src address increment: none
	(1 << AVR32_DMACA_CTL1L_DST_MSIZE_OFFSET)    | // Dst burst transaction len: 4 data items
	(1 << AVR32_DMACA_CTL1L_SRC_MSIZE_OFFSET)    | // Src burst transaction len: 4 data items
	(0 << AVR32_DMACA_CTL1L_S_GATH_EN_OFFSET)    | // Source gather: disabled
	(0 << AVR32_DMACA_CTL1L_D_SCAT_EN_OFFSET)    | // Destination scatter: disabled
	(2 << AVR32_DMACA_CTL1L_TT_FC_OFFSET)        | // transfer type:P2M, flow controller: DMACA
	(1 << AVR32_DMACA_CTL1L_DMS_OFFSET)          | // Dest master: HSB master 2
	(0 << AVR32_DMACA_CTL1L_SMS_OFFSET)          | // Source master: HSB master 1
	(0 << AVR32_DMACA_CTL1L_LLP_D_EN_OFFSET)     | // Not used
	(0 << AVR32_DMACA_CTL1L_LLP_S_EN_OFFSET)       // Not used
	;

	// Channel 1 Ctrl register high
	AVR32_DMACA.ctl1h =
	(u16BufferSize << AVR32_DMACA_CTL1H_BLOCK_TS_OFFSET) | // Block transfer size
	(0             << AVR32_DMACA_CTL1H_DONE_OFFSET)       // Not done
	;

	// Channel 1 Config register low
	AVR32_DMACA.cfg1l =
	(0 << AVR32_DMACA_CFG1L_HS_SEL_DST_OFFSET) | // Destination handshaking: hw handshaking
	(0 << AVR32_DMACA_CFG1L_HS_SEL_SRC_OFFSET)   // Source handshaking: ignored because the src is memory.
	; // All other bits set to 0.

	// Channel 1 Config register high
	AVR32_DMACA.cfg1h =
	(0                     << AVR32_DMACA_CFG1H_DEST_PER_OFFSET) | // Dest hw handshaking itf: ignored because the dst is memory.
	(AVR32_DMACA_CH_AES_RX << AVR32_DMACA_CFG1H_SRC_PER_OFFSET)    // Source hw handshaking itf:
	; // All other bits set to 0.


	//*
	//* Set the AES cryptographic key and init vector.
	//*
	// Set the cryptographic key.
	aes_set_key(&AVR32_AES, CipherKey);

	// Set the initialization vector.
	aes_set_initvector(&AVR32_AES, InitVector);

	// Enable Channel 0 & 1 : start the process.
	AVR32_DMACA.chenreg = ((3<<AVR32_DMACA_CHENREG_CH_EN_OFFSET) | (3<<AVR32_DMACA_CHENREG_CH_EN_WE_OFFSET));

	// Wait for the end of the AES->RAM transfer (channel 1).
	while(AVR32_DMACA.chenreg & (2<<AVR32_DMACA_CHENREG_CH_EN_OFFSET));
}