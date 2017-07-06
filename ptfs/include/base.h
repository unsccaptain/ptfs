
#pragma once

#include "config.h"
#include <stdint.h>
#include <stdio.h>

#ifdef CFG_WIN

#include <windows.h>
#include <assert.h>

typedef UINT8		uint8_t;
typedef UINT16		uint16_t;
typedef UINT32		uint32_t;
typedef UINT64		uint64_t;

typedef INT8		int8_t;
typedef INT16		int16_t;
typedef INT32		int32_t;
typedef INT64		int64_t;
typedef UINT16		wchar;

typedef UINT32		err_t;

typedef UINT64		sec_off_t;
typedef UINT32		ssize_t;

typedef HANDLE		device_t;

#endif

//#ifdef CFG_ARCH_X86
//typedef uint32_t	uint_ptr;
//#elif defined(CFG_ARCH_X64)
//typedef uint64_t	uint_
//#endif

typedef struct _guid_s {
	uint32_t	Data1;
	uint16_t	Data2;
	uint16_t	Data3;
	uint8_t		Data4[8];
} guid_s;

typedef guid_s*	guid_t;

/* µÿ÷∑∂‘∆Î∫Í */

#define ALIGN_DOWN_BY(length, alignment) \
    ((uintptr_t)(length) & ~(alignment - 1))

#define ALIGN_UP_BY(length, alignment) \
    (ALIGN_DOWN_BY(((uintptr_t)(length) + alignment - 1), alignment))

#define ALIGN_DOWN_POINTER_BY(address, alignment) \
    ((void*)((uintptr_t)(address) & ~((uintptr_t)alignment - 1)))

#define ALIGN_UP_POINTER_BY(address, alignment) \
    (ALIGN_DOWN_POINTER_BY(((uintptr_t)(address) + alignment - 1), alignment))

#define ALIGN_DOWN(length, type) \
    ALIGN_DOWN_BY(length, sizeof(type))

#define ALIGN_UP(length, type) \
    ALIGN_UP_BY(length, sizeof(type))

#define ALIGN_DOWN_POINTER(address, type) \
    ALIGN_DOWN_POINTER_BY(address, sizeof(type))

#define ALIGN_UP_POINTER(address, type) \
    ALIGN_UP_POINTER_BY(address, sizeof(type))

#define PARTITION_EMPTY			0x00
#define PARTITION_FAT12			0x01
#define PARTITION_FAT16_SM		0x04
#define PARTITION_DOS_EXT		0x05
#define PARTITION_FAT16			0x06
#define PARTITION_NTFS			0x07
#define PARTITION_HPFS			0x07
//#define PARTITION_FAT32			0x0b
#define PARTITION_FAT32_LBA		0x0c
#define PARTITION_FAT16_LBA		0x0e
#define PARTITION_EXT_LBA		0x0f

#define PART_FLAG_HIDDEN		0x10	/* Valid for FAT/NTFS only */
#define PARTITION_FAT12_H		(PARTITION_FAT12		| PART_FLAG_HIDDEN)
#define PARTITION_FAT16_SM_H	(PARTITION_FAT16_SM		| PART_FLAG_HIDDEN)
#define PARTITION_DOS_EXT_H		(PARTITION_DOS_EXT		| PART_FLAG_HIDDEN)
#define PARTITION_FAT16_H		(PARTITION_FAT16		| PART_FLAG_HIDDEN)
#define PARTITION_NTFS_H		(PARTITION_NTFS			| PART_FLAG_HIDDEN)
#define PARTITION_FAT32_H		(PARTITION_FAT32		| PART_FLAG_HIDDEN)
#define PARTITION_FAT32_LBA_H	(PARTITION_FAT32_LBA	| PART_FLAG_HIDDEN)
#define PARTITION_FAT16_LBA_H	(PARTITION_FAT16_LBA	| PART_FLAG_HIDDEN)

#define PARTITION_COMPAQ_DIAG	0x12
#define PARTITION_MSFT_RECOVERY	0x27
#define PARTITION_LDM			0x42
#define PARTITION_LINUX_SWAP	0x82
#define PARTITION_LINUX			0x83
#define PARTITION_IRST			0x84
#define PARTITION_LINUX_EXT		0x85
#define PARTITION_LINUX_LVM		0x8e
#define PARTITION_HFS			0xaf
#define PARTITION_SUN_UFS		0xbf
#define PARTITION_DELL_DIAG		0xde
#define PARTITION_GPT			0xee
#define PARTITION_ESP			0xef
#define PARTITION_PALO			0xf0
#define PARTITION_PREP			0x41
#define PARTITION_LINUX_RAID	0xfd
#define PARTITION_LINUX_LVM_OLD 0xfe

namespace ptfs {

	void GenerateGuid(guid_t Guid);

	namespace device {

		typedef struct {
			uint64_t TransferLength;
			uint64_t PhysicalPageSize;
		}adapter_parameter;

		typedef struct {
			uint16_t LogicalSectorInByte;
			uint16_t PhysicalSectorInByte;
			uint16_t CacheLineInByte;
		}alignment_parameter;

	}

	class Base {

	protected:

		err_t Status;

		uint32_t Refs;

	public:

		err_t GetStatus() { return Status; }

		void INC_REF() { Refs++; }

		void DEC_REF() { Refs--; }

		bool NoMoreRef() { return Refs == 0; }

		void ReleaseObject() {};

	};

}