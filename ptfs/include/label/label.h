#pragma once

#include <config.h>
#include <base.h>
#include <disk_device.h>
#include <partition_device.h>

#define MSDOS_MAGIC				0xAA55
#define PARTITION_MAGIC_MAGIC	0xf6f6

/* The maximum number of DOS primary partitions.  */
#define DOS_N_PRI_PARTITIONS	4

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

///
/// Data structure that precedes all of the standard EFI table types.
///
typedef struct {
	///
	/// A 64-bit signature that identifies the type of table that follows.
	/// Unique signatures have been generated for the EFI System Table,
	/// the EFI Boot Services Table, and the EFI Runtime Services Table.
	///
	uint64_t	Signature;
	///
	/// The revision of the EFI Specification to which this table
	/// conforms. The upper 16 bits of this field contain the major
	/// revision value, and the lower 16 bits contain the minor revision
	/// value. The minor revision values are limited to the range of 00..99.
	///
	uint32_t	Revision;
	///
	/// The size, in bytes, of the entire table including the EFI_TABLE_HEADER.
	///	
	uint32_t	HeaderSize;
	///
	/// The 32-bit CRC for the entire table. This value is computed by
	/// setting this field to 0, and computing the 32-bit CRC for HeaderSize bytes.
	///
	uint32_t	CRC32;
	///
	/// Reserved field that must be set to 0.
	///
	uint32_t	Reserved;
} EFI_TABLE_HEADER;

///
/// GPT Partition Table Header.
///
typedef struct {
	///
	/// The table header for the GPT partition Table.
	/// This header contains EFI_PTAB_HEADER_ID.
	///
	EFI_TABLE_HEADER	Header;
	///
	/// The LBA that contains this data structure.
	///
	sec_off_t			MyLBA;
	///
	/// LBA address of the alternate GUID Partition Table Header.
	///
	sec_off_t			AlternateLBA;
	///
	/// The first usable logical block that may be used
	/// by a partition described by a GUID Partition Entry.
	///
	sec_off_t           FirstUsableLBA;
	///
	/// The last usable logical block that may be used
	/// by a partition described by a GUID Partition Entry.
	///
	sec_off_t           LastUsableLBA;
	///
	/// GUID that can be used to uniquely identify the disk.
	///
	guid_s				DiskGUID;
	///
	/// The starting LBA of the GUID Partition Entry array.
	///
	sec_off_t			PartitionEntryLBA;
	///
	/// The number of Partition Entries in the GUID Partition Entry array.
	///
	uint32_t            NumberOfPartitionEntries;
	///
	/// The size, in bytes, of each the GUID Partition
	/// Entry structures in the GUID Partition Entry
	/// array. This field shall be set to a value of 128 x 2^n where n is
	/// an integer greater than or equal to zero (e.g., 128, 256, 512, etc.).
	///
	uint32_t            SizeOfPartitionEntry;
	///
	/// The CRC32 of the GUID Partition Entry array.
	/// Starts at PartitionEntryLBA and is
	/// computed over a byte length of
	/// NumberOfPartitionEntries * SizeOfPartitionEntry.
	///
	uint32_t            PartitionEntryArrayCRC32;
} EFI_PARTITION_TABLE_HEADER;

///
/// GPT Partition Entry.
///
typedef struct {
	///
	/// Unique ID that defines the purpose and type of this Partition. A value of
	/// zero defines that this partition entry is not being used.
	///
	guid_s			PartitionTypeGUID;
	///
	/// GUID that is unique for every partition entry. Every partition ever
	/// created will have a unique GUID.
	/// This GUID must be assigned when the GUID Partition Entry is created.
	///
	guid_s			UniquePartitionGUID;
	///
	/// Starting LBA of the partition defined by this entry
	///
	sec_off_t		StartingLBA;
	///
	/// Ending LBA of the partition defined by this entry.
	///
	sec_off_t		EndingLBA;
	///
	/// Attribute bits, all bits reserved by UEFI
	/// Bit 0:      If this bit is set, the partition is required for the platform to function. The owner/creator of the
	///             partition indicates that deletion or modification of the contents can result in loss of platform
	///             features or failure for the platform to boot or operate. The system cannot function normally if
	///             this partition is removed, and it should be considered part of the hardware of the system.
	///             Actions such as running diagnostics, system recovery, or even OS install or boot, could
	///             potentially stop working if this partition is removed. Unless OS software or firmware
	///             recognizes this partition, it should never be removed or modified as the UEFI firmware or
	///             platform hardware may become non-functional.
	/// Bit 1:      If this bit is set, then firmware must not produce an EFI_BLOCK_IO_PROTOCOL device for
	///             this partition. By not producing an EFI_BLOCK_IO_PROTOCOL partition, file system
	///             mappings will not be created for this partition in UEFI.
	/// Bit 2:      This bit is set aside to let systems with traditional PC-AT BIOS firmware implementations
	///             inform certain limited, special-purpose software running on these systems that a GPT 
	///             partition may be bootable. The UEFI boot manager must ignore this bit when selecting
	///             a UEFI-compliant application, e.g., an OS loader.
	/// Bits 3-47:  Undefined and must be zero. Reserved for expansion by future versions of the UEFI
	///             specification.
	/// Bits 48-63: Reserved for GUID specific use. The use of these bits will vary depending on the
	///             PartitionTypeGUID. Only the owner of the PartitionTypeGUID is allowed
	///             to modify these bits. They must be preserved if Bits 0-47 are modified..
	///
	sec_off_t		Attributes;
	///
	/// Null-terminated name of the partition.
	///
	uint16_t		PartitionName[36];
} EFI_PARTITION_ENTRY;

#pragma pack(pop,1)

namespace ptfs {

	namespace label {


		class Label :public Base {

		protected:

			char label_name[16];
			
			device::Device* blk_dev;

			bool valid_format;

		public:

			Label(char* Name,device::Device* Device):blk_dev(Device) {
				strcpy(label_name, Name);
			}

			//创建磁盘格式
			virtual bool MakeLabel() = 0;
		
			//创建分区
			virtual device::PartitionDevice* MakePart(sec_off_t Size, sec_off_t Start) = 0;

			//获取分区设备
			virtual device::PartitionDevice* GetPartitionDevice(uint8_t PartNumber) = 0;

			//设置分区类型
			virtual void SetPartitionType(uint8_t PartNumber, uint16_t PartType) = 0;

			//将磁盘格式同步至硬盘
			virtual bool Sync() = 0;

		};

	}

}