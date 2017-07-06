/** @file
This file implements CalculateCrc32 Boot Services as defined in
Platform Initialization specification 1.0 VOLUME 2 DXE Core Interface.

This Boot Services is in the Runtime Driver because this service is
also required by SetVirtualAddressMap() when the EFI System Table and
EFI Runtime Services Table are converted from physical address to
virtual addresses.  This requires that the 32-bit CRC be recomputed.

Copyright (c) 2006 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include <base.h>
#include <crypto/crc32.h>

uint32_t  mCrcTable[256];

/**
Calculate CRC32 for target data.

@param  Data                  The target data.
@param  DataSize              The target data size.
@param  CrcOut                The CRC32 for target data.

@retval EFI_SUCCESS           The CRC32 for target data is calculated successfully.
@retval EFI_INVALID_PARAMETER Some parameter is not valid, so the CRC32 is not
calculated.

**/
uint32_t
RuntimeDriverCalculateCrc32(
	IN  unsigned char	*Data,
	IN  uintptr_t		DataSize
)
{
	uint32_t	Crc;
	uintptr_t   Index;
	uint8_t		*Ptr;

	if (Data == NULL || DataSize == 0 ) {
		return 0;
	}

	Crc = 0xffffffff;
	for (Index = 0, Ptr = Data; Index < DataSize; Index++, Ptr++) {
		Crc = (Crc >> 8) ^ mCrcTable[(uint8_t)Crc ^ *Ptr];
	}

	return Crc;
}


/**
This internal function reverses bits for 32bit data.

@param  Value                 The data to be reversed.

@return                       Data reversed.

**/
uint32_t
ReverseBits(
	uint32_t  Value
)
{
	uintptr_t   Index;
	uint32_t  NewValue;

	NewValue = 0;
	for (Index = 0; Index < 32; Index++) {
		if ((Value & (1 << Index)) != 0) {
			NewValue = NewValue | (1 << (31 - Index));
		}
	}

	return NewValue;
}

/**
Initialize CRC32 table.

**/
void
RuntimeDriverInitializeCrc32Table(
	void
)
{
	uintptr_t   TableEntry;
	uintptr_t   Index;
	uint32_t  Value;

	for (TableEntry = 0; TableEntry < 256; TableEntry++) {
		Value = ReverseBits((uint32_t)TableEntry);
		for (Index = 0; Index < 8; Index++) {
			if ((Value & 0x80000000) != 0) {
				Value = (Value << 1) ^ 0x04c11db7;
			}
			else {
				Value = Value << 1;
			}
		}

		mCrcTable[TableEntry] = ReverseBits(Value);
	}
}
