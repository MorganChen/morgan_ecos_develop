#ifndef CYGONCE_HAL_MM_H
#define CYGONCE_HAL_MM_H

//=============================================================================
//
//      hal_mm.h
//
//      ARM920T MM common definitions 
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):		T.C. Chiu <TCChiu@sq.com.tw>
// Contributors:	T.C. Chiu <TCChiu@sq.com.tw>
// Date:		2009-12-22
// Purpose:		ARM920T MM common definitions
// Description:		The macros defined here provide common definitions for
//			memory management initialization.
// Usage:
//			#include <cyg/hal/hal_mm.h>
//			...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

// -------------------------------------------------------------------------
// MMU initialization:
// 
// These structures are laid down in memory to define the translation
// table.
// 

// ARM920T Translation Table Base Bit Masks
#define ARM_TRANSLATION_TABLE_MASK		0xFFFFC000

// ARM920T Domain Access Control Bit Masks
#define ARM_ACCESS_TYPE_NO_ACCESS(domain_num)	(0x0 << (domain_num)*2)
#define ARM_ACCESS_TYPE_CLIENT(domain_num)	(0x1 << (domain_num)*2)
#define ARM_ACCESS_TYPE_MANAGER(domain_num)	(0x3 << (domain_num)*2)

struct ARM_MMU_FIRST_LEVEL_FAULT {
	int	id		: 2;
	int	sbz		: 30;
};
#define ARM_MMU_FIRST_LEVEL_FAULT_ID		0x0

struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE {
	int	id		: 2;
	int	imp		: 2;
	int	domain		: 4;
	int	sbz		: 1;
	int	base_address	: 23;
};
#define ARM_MMU_FIRST_LEVEL_PAGE_TABLE_ID	0x1

struct ARM_MMU_FIRST_LEVEL_SECTION {
	int	id		: 2;
	int	b		: 1;
	int	c		: 1;
	int	imp		: 1;
	int	domain		: 4;
	int	sbz0		: 1;
	int	ap		: 2;
	int	sbz1		: 8;
	int	base_address	: 12;
};
#define ARM_MMU_FIRST_LEVEL_SECTION_ID		0x2

struct ARM_MMU_FIRST_LEVEL_RESERVED {
	int	id		: 2;
	int	sbz		: 30;
};
#define ARM_MMU_FIRST_LEVEL_RESERVED_ID		0x3


#define ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, table_index) \
	(unsigned long *) ((unsigned long) (ttb_base) + ((table_index) << 2))

#define ARM_FIRST_LEVEL_PAGE_TABLE_SIZE		0x4000

#define ARM_MMU_SECTION(ttb_base, actual_base, virtual_base, cacheable, bufferable, perm) \
	CYG_MACRO_START \
		register union ARM_MMU_FIRST_LEVEL_DESCRIPTOR	desc; \
 \
		desc.word			= 0; \
		desc.section.id			= ARM_MMU_FIRST_LEVEL_SECTION_ID; \
		desc.section.imp		= 1; \
		desc.section.domain		= 0; \
		desc.section.c			= (cacheable); \
		desc.section.b			= (bufferable); \
		desc.section.ap			= (perm); \
		desc.section.base_address	= (actual_base); \
		*ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, (virtual_base)) = desc.word; \
	CYG_MACRO_END

#define X_ARM_MMU_SECTION(abase, vbase, size, cache, buff, access) \
{ \
	int	i, j, k; \
	for (i=size, j=abase, k=vbase; i>0 ; i--, j++, k++) { \
		ARM_MMU_SECTION(ttb_base, j, k, cache, buff, access); \
	} \
}


union ARM_MMU_FIRST_LEVEL_DESCRIPTOR {
	unsigned long				word;
	struct ARM_MMU_FIRST_LEVEL_FAULT	fault;
	struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE	page_table;
	struct ARM_MMU_FIRST_LEVEL_SECTION	section;
	struct ARM_MMU_FIRST_LEVEL_RESERVED	reserved;
};

#define ARM_UNCACHEABLE				0
#define ARM_CACHEABLE				1
#define ARM_UNBUFFERABLE			0
#define ARM_BUFFERABLE				1

#define ARM_ACCESS_PERM_NONE_NONE		0
#define ARM_ACCESS_PERM_RO_NONE			0
#define ARM_ACCESS_PERM_RO_RO			0
#define ARM_ACCESS_PERM_RW_NONE			1
#define ARM_ACCESS_PERM_RW_RO			2
#define ARM_ACCESS_PERM_RW_RW			3

// Initialization for the Domain Access Control Register
#define ARM_ACCESS_DACR_DEFAULT \
( \
	ARM_ACCESS_TYPE_MANAGER(0)    | \
	ARM_ACCESS_TYPE_NO_ACCESS(1)  | \
	ARM_ACCESS_TYPE_NO_ACCESS(2)  | \
	ARM_ACCESS_TYPE_NO_ACCESS(3)  | \
	ARM_ACCESS_TYPE_NO_ACCESS(4)  | \
	ARM_ACCESS_TYPE_NO_ACCESS(5)  | \
	ARM_ACCESS_TYPE_NO_ACCESS(6)  | \
	ARM_ACCESS_TYPE_NO_ACCESS(7)  | \
	ARM_ACCESS_TYPE_NO_ACCESS(8)  | \
	ARM_ACCESS_TYPE_NO_ACCESS(9)  | \
	ARM_ACCESS_TYPE_NO_ACCESS(10) | \
	ARM_ACCESS_TYPE_NO_ACCESS(11) | \
	ARM_ACCESS_TYPE_NO_ACCESS(12) | \
	ARM_ACCESS_TYPE_NO_ACCESS(13) | \
	ARM_ACCESS_TYPE_NO_ACCESS(14) | \
	ARM_ACCESS_TYPE_NO_ACCESS(15) \
)


#endif // ifndef CYGONCE_HAL_MM_H

//-----------------------------------------------------------------------------
// End of hal_mm.h





