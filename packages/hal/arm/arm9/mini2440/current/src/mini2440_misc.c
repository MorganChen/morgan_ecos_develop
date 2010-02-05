//==========================================================================
//
//      mini2440_misc.c
//
//      HAL misc board support code for FriendlyARM MINI2440 development board
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2009 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later
// version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with eCos; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// As a special exception, if other files instantiate templates or use
// macros or inline functions from this file, or you compile this file
// and link it with other works to produce a work based on this file,
// this file does not by itself cause the resulting work to be covered by
// the GNU General Public License. However the source code for this file
// must still be made available in accordance with section (3) of the GNU
// General Public License v2.
//
// This exception does not invalidate any other reasons why a work based
// on this file might be covered by the GNU General Public License.
// -------------------------------------------
// ####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):		T.C. Chiu <TCChiu@sq.com.tw>
// Contributors:	T.C. Chiu <TCChiu@sq.com.tw>
// Date:		2009-12-24
// Purpose:		HAL board support
// Description:		Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include CYGBLD_HAL_PLATFORM_H
#include CYGBLD_HAL_BOARD_H		// board specifics

#include <cyg/infra/cyg_type.h>		// base types
#include <cyg/infra/cyg_trac.h>		// tracing macros
#include <cyg/infra/cyg_ass.h>		// assertion macros

#include <cyg/hal/hal_io.h>		// IO macros
#include <cyg/hal/hal_arch.h>		// register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>		// interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_mm.h>		// more MMU definitions

#include CYGBLD_HAL_VAR_H		// platform specifics
#include <cyg/hal/mini2440.h>

#include <cyg/infra/diag.h>		// diag_printf
#include <string.h>			// memset


void hal_mmu_init(void)
{
	unsigned long	ttb_base = MINI2440_SDRAM_PHYS_BASE + 0x4000;
	unsigned long	i;

	// Set the TTB register
	asm volatile ("mcr	p15, 0, %0, c2, c0, 0" : : "r"(ttb_base) /*:*/);

	// Set the Domain Access Control Register
	i = ARM_ACCESS_DACR_DEFAULT;
	asm volatile ("mcr	p15, 0, %0, c3, c0, 0" : : "r"(i) /*:*/);

	// First clear all TT entries - ie Set them to Faulting
	memset((void *) ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

#define _CACHED		ARM_CACHEABLE
#define _UNCACHED	ARM_UNCACHEABLE
#define _BUF		ARM_BUFFERABLE
#define _NOBUF		ARM_UNBUFFERABLE
#define _RWRW		ARM_ACCESS_PERM_RW_RW
	// Memory layout after MMU is turned on
	//
	//   SDRAM_BASE_ADDRESS:     0x00000000,  64M
	//   SRAM_BASE_ADDRESS:      0x40000000,   4K
	//   SFR_BASE_ADDRESS:       0x48000000, 512M
	//   FLASH_BASE_ADDRESS:     0x80000000,   2M

	//               Actual  Virtual  Size   Attributes             access          Function
	//               Base    Base     MB     cached?     buffered?  permissions
	//             xxx00000  xxx00000
	X_ARM_MMU_SECTION(0x000,  0x800,     2,  _UNCACHED,    _NOBUF,    _RWRW);	// Flash
	X_ARM_MMU_SECTION(0x300,  0x000,    64,  _CACHED,      _BUF,      _RWRW);	// SDRAM
	X_ARM_MMU_SECTION(0x400,  0x400,     1,  _CACHED,      _BUF,      _RWRW);	// SRAM
	X_ARM_MMU_SECTION(0x480,  0x480,   512,  _UNCACHED,    _NOBUF,    _RWRW);	// SFRs
	X_ARM_MMU_SECTION(0x300,  0x300,    64,  _UNCACHED,    _NOBUF,    _RWRW);	// Raw SDRAM
	X_ARM_MMU_SECTION(0x200,  0x200,     2,  _UNCACHED,    _NOBUF,    _RWRW);	// nGCS4
}

//----------------------------------------------------------------------------
// Platform specific initialization
void plf_hardware_init(void)
{
	HAL_WRITE_UINT32(INTMOD, 0x0);			//All=IRQ mode
	HAL_WRITE_UINT32(INTMSK, BIT_ALLMSK);		//All interrupt is masked.
	HAL_WRITE_UINT32(INTSUBMSK, BIT_SUB_ALLMSK);	//All sub-interrupt is masked.

	// Note: Follow the configuration order for setting the ports.
	// 1) Set data register (GPnDAT)
	// 2) Set control register (GPnCON)
	// 3) Configure pull-up's (GPnUP)

	// *** PORT A GROUP
	// Ports  : GPA22 GPA21  GPA20 GPA19 GPA18 GPA17 GPA16 GPA15 GPA14 GPA13 GPA12
	// Signal : nFCE nRSTOUT nFRE   nFWE  ALE   CLE  nGCS5 nGCS4 nGCS3 nGCS2 nGCS1
	// Binary :  1      1      1      1    1     1     1     1     1     1     1
	// -----------------------------------------------------------------------------------------
	// Ports  : GPA11   GPA10  GPA9   GPA8   GPA7   GPA6   GPA5   GPA4   GPA3   GPA2   GPA1  GPA0
	// Signal : ADDR26 ADDR25 ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0
	// Binary :  1       1      1      1      1      1      1      1      1      1      1      1
	HAL_WRITE_UINT32(GPACON, 0x007FFFFF);

	// **** PORT B GROUP
	// Ports  : GPB10    GPB9    GPB8    GPB7    GPB6     GPB5    GPB4   GPB3   GPB2     GPB1      GPB0
	// Signal : nXDREQ0 nXDACK0 nXDREQ1 nXDACK1 nSS_KBD nDIS_OFF L3CLOCK L3DATA L3MODE nIrDATXDEN Keyboard
	// Setting: INPUT  OUTPUT   INPUT  OUTPUT   INPUT   OUTPUT   OUTPUT OUTPUT OUTPUT   OUTPUT    OUTPUT
	// Binary :   00     01       00     01       00      01       01     01     01       01        01
	HAL_WRITE_UINT32(GPBCON, 0x00044555);
	HAL_WRITE_UINT32(GPBUP,  0x000007FF);	// The pull up function is disabled GPB[10:0]

	// *** PORT C GROUP
	// Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8 GPC7   GPC6   GPC5 GPC4 GPC3  GPC2  GPC1 GPC0
	// Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND
	// Binary : 10    10    10    10    10    10    10   10    10     10     10   10   10     10   10   10
	HAL_WRITE_UINT32(GPCCON, 0xAAAAAAAA);
	HAL_WRITE_UINT32(GPCUP,  0x0000FFFF);	// The pull up function is disabled GPC[15:0]

	// *** PORT D GROUP
	// Ports  : GPD15 GPD14 GPD13 GPD12 GPD11 GPD10 GPD9 GPD8 GPD7 GPD6 GPD5 GPD4 GPD3 GPD2 GPD1 GPD0
	// Signal : VD23  VD22  VD21  VD20  VD19  VD18  VD17 VD16 VD15 VD14 VD13 VD12 VD11 VD10 VD9  VD8
	// Binary :  10    10    10    10    10    10    10   10   10   10   10   10   10   10  10   10
	HAL_WRITE_UINT32(GPDCON, 0xAAAAAAAA);
	HAL_WRITE_UINT32(GPDUP,  0x0000FFFF);	// The pull up function is disabled GPD[15:0]

	// *** PORT E GROUP
	// Ports  : GPE15  GPE14 GPE13   GPE12   GPE11   GPE10   GPE9    GPE8     GPE7   GPE6  GPE5   GPE4
	// Signal : IICSDA IICSCL SPICLK SPIMOSI SPIMISO SDDATA3 SDDATA2 SDDATA1 SDDATA0 SDCMD SDCLK I2SSDO
	// Binary :  10     10     10      10      10      10      10      10      10     10    10     10
	// ------------------------------------------------------------------------------------------------
	// Ports  :  GPE3   GPE2  GPE1    GPE0
	// Signal : I2SSDI CDCLK I2SSCLK I2SLRCK
	// Binary :  10     10     10      10
	HAL_WRITE_UINT32(GPECON, 0xAAAAAAAA);
	HAL_WRITE_UINT32(GPEUP,  0x0000FFFF);	// The pull up function is disabled GPE[15:0]

	// *** PORT F GROUP
	// Ports  : GPF7   GPF6   GPF5   GPF4      GPF3     GPF2  GPF1   GPF0
	// Signal : nLED_8 nLED_4 nLED_2 nLED_1 nIRQ_PCMCIA EINT2 KBDINT EINT0
	// Setting: Output Output Output Output    EINT3    EINT2 EINT1  EINT0
	// Binary :  01      01     01     01       10       10    10     10
	HAL_WRITE_UINT32(GPFCON, 0x000055AA);
	HAL_WRITE_UINT32(GPFUP,  0x000000FF);	// The pull up function is disabled GPF[7:0]

	// *** PORT G GROUP
	// Ports  : GPG15 GPG14 GPG13 GPG12 GPG11    GPG10    GPG9     GPG8     GPG7      GPG6
	// Signal : nYPON  YMON nXPON XMON  EINT19 DMAMODE1 DMAMODE0 DMASTART KBDSPICLK KBDSPIMOSI
	// Setting: nYPON  YMON nXPON XMON  EINT19  Output   Output   Output   SPICLK1    SPIMOSI1
	// Binary :   11    11   11    11    10      01        01       01       11         11
	// -----------------------------------------------------------------------------------------
	// Ports  :    GPG5       GPG4    GPG3    GPG2    GPG1    GPG0
	// Signal : KBDSPIMISO LCD_PWREN EINT11 nSS_SPI IRQ_LAN IRQ_PCMCIA
	// Setting:  SPIMISO1  LCD_PWRDN EINT11   nSS0   EINT9    EINT8
	// Binary :     11         11      10      11     10       10
	HAL_WRITE_UINT32(GPGCON, 0xFF95FFBA);
	HAL_WRITE_UINT32(GPGUP,  0x0000FFFF);	// The pull up function is disabled GPG[15:0]

	// *** PORT H GROUP
	// Ports  :  GPH10    GPH9  GPH8 GPH7  GPH6  GPH5 GPH4 GPH3 GPH2 GPH1  GPH0
	// Signal : CLKOUT1 CLKOUT0 UCLK nCTS1 nRTS1 RXD1 TXD1 RXD0 TXD0 nRTS0 nCTS0
	// Binary :   10      10     10   11    11    10   10   10   10   10    10
	HAL_WRITE_UINT32(GPHCON, 0x002AFAAA);
	HAL_WRITE_UINT32(GPHUP,  0x000007FF);	// The pull up function is disabled GPH[10:0]

	// External interrupts will be falling edge triggered.
	HAL_WRITE_UINT32(EXTINT0, 0x22222222);	// EINT[7:0]
	HAL_WRITE_UINT32(EXTINT1, 0x22222222);	// EINT[15:8]
	HAL_WRITE_UINT32(EXTINT2, 0x22222222);	// EINT[23:16]

	// Initialize real-time clock (for delays, etc, even if kernel doesn't use it)
	hal_clock_initialize(CYGNUM_HAL_RTC_PERIOD);
}

//----------------------------------------------------------------------------
// Is DM9000 present?
int cyg_hal_dm9000_present(void) {
	return (!strcmp(HAL_PLATFORM_BOARD, "FriendlyARM mini2440") ||
		!strcmp(HAL_PLATFORM_CPU, "ARM9"));
}

//-----------------------------------------------------------------------------
// End of mini2440_misc.c
