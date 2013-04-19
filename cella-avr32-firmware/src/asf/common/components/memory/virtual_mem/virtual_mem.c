/**
 * \file
 *
 * \brief Management of the virtual memory.
 *
 * Copyright (c) 2009-2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/**
 * \defgroup group_common_components_memory_virtual_mem Virtual Memory in RAM
 *
 * The component manages a disk on a volatile memory (internal RAM).
 * This can be connected to a File System management or a USB Device
 * Mass Storage Interface via the service Memory Control Access.
 *
 * \{
 */

//_____  I N C L U D E S ___________________________________________________

#include "conf_access.h"
#include "conf_virtual_mem.h"


#if VIRTUAL_MEM == ENABLE

#include "virtual_mem.h"
#include <string.h>


#ifndef VMEM_NB_SECTOR
#  error Define VMEM_NB_SECTOR in conf_virtual_mem.h file
#endif

//_____ M A C R O S ________________________________________________________

//_____ P R I V A T E   D E C L A R A T I O N S ____________________________


//_____ D E F I N I T I O N S ______________________________________________

#ifdef VMEM_ADDRESS
// Virtual disk memory start address is defined in conf_virtual_mem.h
# if (0 != (VMEM_ADDRESS & 0x3))
#   error VMEM_ADDRESS defined in conf_virtual_mem.h must be a WORD address
# endif
# if (VMEM_ADDRESS + (VMEM_NB_SECTOR * VMEM_SECTOR_SIZE) - 1) > UINTPTR_MAX
#  include <hugemem.h>
static hugemem_ptr_t vmem_data = (hugemem_ptr_t)VMEM_ADDRESS;
# else
static uint8_t* vmem_data = (uint8_t*)VMEM_ADDRESS;
# endif
#else
COMPILER_WORD_ALIGNED
static uint8_t vmem_data[VMEM_NB_SECTOR * VMEM_SECTOR_SIZE];
#endif

//_____ D E C L A R A T I O N S ____________________________________________

//! This function tests memory state, and starts memory initialization
//! @return                            Ctrl_status
//!   It is ready                ->    CTRL_GOOD
//!   Memory unplug              ->    CTRL_NO_PRESENT
//!   Not initialized or changed ->    CTRL_BUSY
//!   An error occurred          ->    CTRL_FAIL
Ctrl_status virtual_test_unit_ready(void)
{
	return CTRL_GOOD;
}


//! This function returns the address of the last valid sector
//! @param uint32_t_nb_sector  Pointer to number of sectors (sector=512 bytes)
//! @return                            Ctrl_status
//!   It is ready                ->    CTRL_GOOD
//!   Memory unplug              ->    CTRL_NO_PRESENT
//!   Not initialized or changed ->    CTRL_BUSY
//!   An error occurred          ->    CTRL_FAIL
Ctrl_status virtual_read_capacity(uint32_t *uint32_t_nb_sector)
{
	if (VMEM_NB_SECTOR<8) {
		*uint32_t_nb_sector = 8-1;
	} else {
		*uint32_t_nb_sector = VMEM_NB_SECTOR- 1;
	}
	return CTRL_GOOD;
}


//! This function returns the write-protected mode
//!
//! @return true if the memory is protected
//!
bool virtual_wr_protect(void)
{
	return false;
}


//! This function informs about the memory type
//!
//! @return true if the memory is removable
//!
bool virtual_removal(void)
{
	return false;
}


//------------ SPECIFIC FUNCTIONS FOR TRANSFER BY USB -------------------------

#if ACCESS_USB == true

#include "udi_msc.h"

//! This function transfers the data between memory and USB MSC interface
//!
//! @param addr         Sector address to start read
//! @param nb_sector    Number of sectors to transfer (sector=512 bytes)
//! @param b_read       Memory to USB, if true
//!
//! @return                            Ctrl_status
//!   It is ready                ->    CTRL_GOOD
//!   Memory unplug              ->    CTRL_NO_PRESENT
//!   Not initialized or changed ->    CTRL_BUSY
//!   An error occurred          ->    CTRL_FAIL
//!
static Ctrl_status virtual_usb_trans(uint32_t addr, uint16_t nb_sector,
		bool b_read)
{
/* USB DMA for XMEGA only works on internal RAM, so use a temporal buffer if
 * it's outside this.
 */
#if defined(VMEM_ADDRESS) && ((VMEM_ADDRESS + (VMEM_NB_SECTOR * VMEM_SECTOR_SIZE) - 1) > UINTPTR_MAX)
	uint8_t       buffer[VMEM_SECTOR_SIZE];
	hugemem_ptr_t ptr_cram;

	if ((addr > VMEM_NB_SECTOR) ||  (addr + nb_sector > VMEM_NB_SECTOR)) {
		return CTRL_FAIL;
	}

	while (nb_sector) {
		ptr_cram = (hugemem_ptr_t)((uint32_t)vmem_data
				+ (addr++ * VMEM_SECTOR_SIZE));
		if (b_read) {
			hugemem_read_block(buffer, ptr_cram, VMEM_SECTOR_SIZE);
			if (!udi_msc_trans_block(b_read, buffer,
						VMEM_SECTOR_SIZE, NULL)) {
				return CTRL_FAIL; // transfer aborted
			}
		} else {
			if (!udi_msc_trans_block(b_read, buffer,
						VMEM_SECTOR_SIZE, NULL)) {
				return CTRL_FAIL; // transfer aborted
			}
			hugemem_write_block(ptr_cram, buffer,
					VMEM_SECTOR_SIZE);
		}
		nb_sector -= 1;
	}
#else
	uint8_t *ptr_cram;
	uint8_t nb_sector_trans;

	if ((addr > VMEM_NB_SECTOR) ||  (addr + nb_sector > VMEM_NB_SECTOR)) {
		return CTRL_FAIL;
	}

	while (nb_sector) {
		// udi_msc_trans_block() is limited to 64KB
		nb_sector_trans = min(nb_sector, 64*(1024/VMEM_SECTOR_SIZE));
		ptr_cram = &vmem_data[addr++ * VMEM_SECTOR_SIZE];
		if (!udi_msc_trans_block( b_read, ptr_cram,
				nb_sector_trans*VMEM_SECTOR_SIZE, NULL)) {
			return CTRL_FAIL; // transfer aborted
		}
		nb_sector -= nb_sector_trans;
	}
#endif

	return CTRL_GOOD;
}

//! This function transfers the memory data to the USB MSC interface
//!
//! @param addr         Sector address to start read
//! @param nb_sector    Number of sectors to transfer (sector=512 bytes)
//!
//! @return                            Ctrl_status
//!   It is ready                ->    CTRL_GOOD
//!   Memory unplug              ->    CTRL_NO_PRESENT
//!   Not initialized or changed ->    CTRL_BUSY
//!   An error occurred          ->    CTRL_FAIL
//!
Ctrl_status virtual_usb_read_10(uint32_t addr, uint16_t nb_sector)
{
	return virtual_usb_trans(addr, nb_sector, true);
}


//! This function transfers the USB MSC data to the memory
//!
//! @param addr         Sector address to start write
//! @param nb_sector    Number of sectors to transfer (sector=512 bytes)
//!
//! @return                            Ctrl_status
//!   It is ready                ->    CTRL_GOOD
//!   Memory unplug              ->    CTRL_NO_PRESENT
//!   Not initialized or changed ->    CTRL_BUSY
//!   An error occurred          ->    CTRL_FAIL
//!
Ctrl_status virtual_usb_write_10(uint32_t addr, uint16_t nb_sector)
{
	return virtual_usb_trans(addr, nb_sector, false);
}

#endif  // ACCESS_USB == true


//------------ SPECIFIC FUNCTIONS FOR TRANSFER BY RAM --------------------------

#if ACCESS_MEM_TO_RAM == true

#include <string.h>

//! This function transfers 1 data sector from memory to RAM
//! sector = 512 bytes
//! @param addr         Sector address to start read
//! @param ram          Address of RAM buffer
//! @return                            Ctrl_status
//!   It is ready                ->    CTRL_GOOD
//!   Memory unplug              ->    CTRL_NO_PRESENT
//!   Not initialized or changed ->    CTRL_BUSY
//!   An error occurred          ->    CTRL_FAIL
Ctrl_status virtual_mem_2_ram(uint32_t addr, void *ram)
{
	if (addr + 1 > Max(VMEM_NB_SECTOR, 8)) {
		return CTRL_FAIL;
	}

	// If overflow (possible with size virtual mem < 8 sectors) then read the last sector
	addr = min(addr, VMEM_NB_SECTOR - 1);
#if defined(VMEM_ADDRESS) && ((VMEM_ADDRESS + (VMEM_NB_SECTOR * VMEM_SECTOR_SIZE) - 1) > UINTPTR_MAX)
	hugemem_read_block(ram, (hugemem_ptr_t)((uint32_t)vmem_data + addr
				* VMEM_SECTOR_SIZE), VMEM_SECTOR_SIZE);
#else
	memcpy(ram, &vmem_data[addr * VMEM_SECTOR_SIZE], VMEM_SECTOR_SIZE);
#endif

	return CTRL_GOOD;
}


//! This function transfers 1 data sector from memory to RAM
//! sector = 512 bytes
//! @param addr         Sector address to start write
//! @param ram          Address of RAM buffer
//! @return                            Ctrl_status
//!   It is ready                ->    CTRL_GOOD
//!   Memory unplug              ->    CTRL_NO_PRESENT
//!   Not initialized or changed ->    CTRL_BUSY
//!   An error occurred          ->    CTRL_FAIL
Ctrl_status virtual_ram_2_mem(uint32_t addr, const void *ram)
{
	if (addr + 1 > VMEM_NB_SECTOR) {
		return CTRL_FAIL;
	}

#if defined(VMEM_ADDRESS) && ((VMEM_ADDRESS + (VMEM_NB_SECTOR * VMEM_SECTOR_SIZE) - 1) > UINTPTR_MAX)
	hugemem_write_block((hugemem_ptr_t)((uint32_t)vmem_data + addr
				* VMEM_SECTOR_SIZE), ram, VMEM_SECTOR_SIZE);
#else
	memcpy(&vmem_data[addr * VMEM_SECTOR_SIZE], ram, VMEM_SECTOR_SIZE);
#endif
	return CTRL_GOOD;
}

#endif  // ACCESS_MEM_TO_RAM == true


#endif  // VIRTUAL_MEM == ENABLE

/**
 * \}
 */
