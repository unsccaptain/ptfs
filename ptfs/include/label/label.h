#pragma once

#include <config.h>
#include <base.h>
#include <disk_device.h>
#include <fs/filesystem.h>

#define MSDOS_MAGIC				0xAA55
#define PARTITION_MAGIC_MAGIC	0xf6f6

/* The maximum number of DOS primary partitions.  */
#define DOS_N_PRI_PARTITIONS	4

#define PARTITION_EMPTY			0x00
#define PARTITION_FAT12			0x01
#define PARTITION_FAT16_SM		0x04
#define PARTITION_DOS_EXT		0x05
#define PARTITION_FAT16			0x06
#define PARTITION_NTFS			0x07
#define PARTITION_HPFS			0x07
#define PARTITION_FAT32			0x0b
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

	namespace label {

		typedef struct _DosRawPartition		DosRawPartition;
		typedef struct _DosRawTable			DosRawTable;

		/* note: lots of bit-bashing here, thus, you shouldn't look inside it.
		* Use chs_to_sector() and sector_to_chs() instead.
		*/

#pragma pack(push,1)

		typedef struct {
			uint8_t		Head;
			uint8_t		Sector;
			uint8_t		Cylinder;
		} RawCHS;

		/* ripped from Linux source */
		struct _DosRawPartition {
			uint8_t		BootIndicator;		/* 00:  0x80 - active */
			RawCHS		CHS_Start;			/* 01: */
			uint8_t		Type;				/* 04: partition type */
			RawCHS		CHS_END;			/* 05: */
			uint32_t	Start;				/* 08: starting sector counting from 0 */
			uint32_t	Length;				/* 0c: nr of sectors in partition */
		};

		struct _DosRawTable {
			char			boot_code[440];
			uint32_t		mbr_signature;	/* really a unique ID */
			uint16_t		Unknown;
			DosRawPartition	partitions[DOS_N_PRI_PARTITIONS];
			uint16_t		magic;
		};

#pragma pack(pop,1)

		class Label :public Base {

		protected:

			char LabelName[16];
			
			device::Device* Dev;

		public:

			Label(char* Name,device::Device* Device):Dev(Device) {
				strcpy(LabelName, Name);
			}

			virtual bool MakeLabel() = 0;
		
			virtual bool MakePart(sec_off_t Size, sec_off_t Start, filesystem::FileSystem*Fs) = 0;

		};

	}

}