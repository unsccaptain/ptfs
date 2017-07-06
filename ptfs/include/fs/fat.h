
#pragma once

#include <config.h>
#include <base.h>
#include <err.h>
#include <partition_device.h>
#include <fs/filesystem.h>

#define FAT_BOOT_JUMP	"\xeb\x58\x90"		/* jmp	+5a */

#define FAT_BOOT_CODE	"\x0e"				/* push cs */		\
			"\x1f"							/* pop ds */		\
			"\xbe\x74\x7e"					/* mov si, offset message */ \
											/* write_msg_loop: */		\
			"\xac"							/* lodsb */		\
			"\x22\xc0"						/* and al, al */	\
			"\x74\x06"						/* jz done (+8) */	\
			"\xb4\x0e"						/* mov ah, 0x0e */	\
			"\xcd\x10"						/* int 0x10 */		\
			"\xeb\xf5"						/* jmp write_msg_loop */ \
											/* done: */			\
			"\xb4\x00"						/* mov ah, 0x00 */	\
			"\xcd\x16"						/* int 0x16 */		\
			"\xb4\x00"						/* mov ah, 0x00 */	\
			"\xcd\x19"						/* int 0x19 */		\
			"\xeb\xfe"						/* jmp +0 - in case int 0x19 */ \
											/* doesn't work */	\
											/* message: */			\
			FAT_BOOT_MESSAGE

#define FAT_BOOT_CODE_LENGTH 128

#define BUFFER_SIZE  1024	/* buffer size in sectors (512 bytes) */

#define MAX_FAT12_CLUSTERS	4086
#define MAX_FAT16_CLUSTERS	65526
#define MAX_FAT32_CLUSTERS	2000000

#define READONLY_ATTR		0x01
#define HIDDEN_ATTR		0x02
#define SYSTEM_ATTR		0x04
#define VOLUME_LABEL_ATTR	0x08
#define VFAT_ATTR		0x0f
#define DIRECTORY_ATTR		0x10
#define ARCH_ATTR		0x20

typedef uint32_t		FatCluster;
typedef int32_t			FatFragment;

enum _FatType {
	FAT_TYPE_UNKNOWN,
	FAT_TYPE_FAT12,
	FAT_TYPE_FAT16,
	FAT_TYPE_FAT32
};
typedef enum _FatType		FatType;

#pragma pack(push,1)

struct _FatBootSector {
	uint8_t		boot_jump[3];	/* 00: Boot strap short or near jump */
	uint8_t		system_id[8];	/* 03: system name */
	uint16_t	sector_size;	/* 0b: bytes per logical sector */
	uint8_t		cluster_size;	/* 0d: sectors/cluster */
	uint16_t	reserved;		/* 0e: reserved sectors */
	uint8_t		fats;			/* 10: number of FATs */
	uint16_t	dir_entries;	/* 11: number of root directory entries */
	uint16_t	sectors;		/* 13: if 0, total_sect supersedes */
	uint8_t		media;			/* 15: media code */
	uint16_t	fat_length;		/* 16: sectors/FAT for FAT12/16 */
	uint16_t	secs_track;		/* 18: sectors per track */
	uint16_t	heads;			/* 1a: number of heads */
	uint32_t	hidden;			/* 1c: hidden sectors (partition start) */
	uint32_t	sector_count;	/* 20: no. of sectors (if sectors == 0) */

	union {
		/* FAT16 fields */
		struct /*__attribute__ ((packed))*/ {
			uint8_t		drive_num;			/* 24: */
			uint8_t		empty_1;			/* 25: */
			uint8_t		ext_signature;		/* 26: always 0x29 */
			uint32_t	serial_number;		/* 27: */
			uint8_t		volume_name[11];	/* 2b: */
			uint8_t		fat_name[8];		/* 36: */
			uint8_t		boot_code[448];		/* 3f: Boot code (or message) */
		} fat16;
		/* FAT32 fields */
		struct {
			uint32_t	fat_length;			/* 24: size of FAT in sectors */
			uint16_t	flags;				/* 28: bit8: fat mirroring, low4: active fat */
			uint16_t	version;			/* 2a: minor * 256 + major */
			uint32_t	root_dir_cluster;	/* 2c: */
			uint16_t	info_sector;		/* 30: */
			uint16_t	backup_sector;		/* 32: */
			uint8_t		empty_1[12];		/* 34: */
			uint16_t	drive_num;			/* 40: */
			uint8_t		ext_signature;		/* 42: always 0x29 */
			uint32_t	serial_number;		/* 43: */
			uint8_t		volume_name[11];	/* 47: */
			uint8_t		fat_name[8];		/* 52: */
			uint8_t		boot_code[420];		/* 5a: Boot code (or message) */
		} fat32;
	} u;

	uint16_t	boot_sign;	/* 1fe: always 0xAA55 */
};

struct _FatInfoSector {
	uint32_t	signature_1;	/* should be 0x41615252 */
	uint8_t		unused[480];
	uint32_t	signature_2;	/* should be 0x61417272 */
	uint32_t	free_clusters;
	uint32_t	next_cluster;	/* most recently allocated cluster */
	uint8_t		unused2[0xe];
	uint16_t	signature_3;	/* should be 0xaa55 */
};

struct /*__attribute__ ((packed))*/ _FatDirEntry {
	char		name[8];
	uint8_t		extension[3];
	uint8_t		attributes;
	uint8_t		is_upper_case_name;
	uint8_t		creation_time_low;      /* milliseconds */
	uint16_t	creation_time_high;
	uint16_t	creation_date;
	uint16_t	access_date;
	uint16_t	first_cluster_high;     /* for FAT32 */
	uint16_t	time;
	uint16_t	date;
	uint16_t	first_cluster;
	uint32_t	length;
};

typedef struct _FatTable		FatTable;
typedef struct _FatBootSector	FatBootSector;
typedef struct _FatInfoSector	FatInfoSector;
typedef struct _FatDirEntry		FatDirEntry;

#pragma pack(pop)

namespace ptfs {

	namespace filesystem {

		class Fat :public FileSystem {

		private:

			//typedef struct {

			//	sec_off_t FatDirSec;
			//	sec_off_t FatS

			//}FAT_PRIVATE, *PFAT_PRIVATE;

			/* DBR */
			FatBootSector*		BootSector = NULL;
			
			/* FAT32特有的INFOSECTOR */
			FatInfoSector*		InfoSector = NULL;
			
			/* 有效扇区大小 */
			sec_off_t			ValidSecSize = 0;
			
			/* 有效簇数 */
			sec_off_t			ValidClusterSize = 0;
			
			/* 第一个FAT表的扇区号 */
			sec_off_t			FirstFat;

			/* 第一个簇的扇区号 */
			sec_off_t			FirstClusterSector;

			/* 每个FAT表项字节数 */
			uint8_t				TbEntrySize;

			/* FAT表项数 */
			uint32_t			TbEntryCount;

			/* 填充Infosector */
			void FillInfoSector();

			/* 填充FAT32的DBR */
			void FillBootSectorFat32();

			/* 填充FAT16的DBR */
			//void FillBootSectorFat16();


		public:

			Fat() {

			}
			
			Fat(volume::Volume* Device)
				:FileSystem(Device) {

				Device->INC_REF();

				ClusterSizeInByte = 4096;
				LogicalSizeInByte = 512;

				BootSector = (FatBootSector*)malloc(sizeof(FatBootSector));

				if (Device->ReadVolumeSector(0, (uint8_t*)BootSector, 1) != 1)
					goto EXIT;

				Type = Probe(BootSector, Device->GetVolumeSize());
				if (Type == FAT_TYPE_UNKNOWN)
					goto EXIT;

				if (Type == FAT_TYPE_FAT32) {
					InfoSector = (FatInfoSector*)malloc(sizeof(FatInfoSector));
					if (Device->ReadVolumeSector(BootSector->u.fat32.info_sector, (uint8_t*)InfoSector, 1) != 1)
						goto EXIT;
				}

				FirstFat = BootSector->reserved;
				if (Type == FAT_TYPE_FAT32)
					FirstClusterSector = BootSector->reserved + BootSector->fats*BootSector->u.fat32.fat_length;

				ValidSecSize = BootSector->sector_count;
				ValidClusterSize = (ValidSecSize - FirstClusterSector) / BootSector->cluster_size;

				return;

			EXIT:

				if (BootSector) {
					free(BootSector);
					BootSector = NULL;
				}

				if (InfoSector) {
					free(InfoSector);
					InfoSector = NULL;
				}

				return;

			}

			/* 判断FAT类型 */
			
			FatType Probe(FatBootSector* BootSector, sec_off_t DevSize) {

				if (BootSector->boot_sign != 0xAA55)
					return FAT_TYPE_UNKNOWN;

				if (BootSector->dir_entries == 0) {

					if (BootSector->fats > 2)
						return FAT_TYPE_UNKNOWN;
					
					if (BootSector->reserved == 0 || BootSector->reserved >= DevSize)
						return FAT_TYPE_UNKNOWN;

					return FAT_TYPE_FAT32;

				}

				/* 计算出目录表之后的扇区数 */
				sec_off_t first_cluster_sec =
					(BootSector->reserved + BootSector->fats*BootSector->fat_length + BootSector->dir_entries / (512 / sizeof(FatDirEntry)));

				/* 计算后面的簇数 */
				sec_off_t cluster_count = (DevSize - first_cluster_sec) / BootSector->cluster_size;

				if (cluster_count > MAX_FAT12_CLUSTERS)
					return FAT_TYPE_FAT16;
				else
					return FAT_TYPE_FAT12;

			}

			/* 简单的判断是不是FAT分区 */

			//static bool Probe(device::Device* Dev) {

			//	FatBootSector BootSector;
			//	if (Dev->ReadDeviceSector(0, (uint8_t*)&BootSector, 1) != 1)
			//		return FALSE;

			//	if (BootSector.boot_sign != 0xAA55)
			//		return false;

			//	if ((BootSector.cluster_size == 0) ||
			//		(BootSector.fats == 0))
			//		return false;

			//	return true;

			//}

			/* 生成分区 */
			virtual bool MakeFs(uint32_t Type, uint32_t ClusterSize) override;

			/* 写入硬盘 */
			virtual bool Sync() override;

			/* 将文件系统信息同步至分区表 */
			virtual bool SyncPartitionTable(label::Label* Label) override;

			/* 释放本对象 */
			void ReleaseObject() {

				if (!NoMoreRef()) {
					Status = ERR_REFED;
					return;
				}

				if (BootSector)
					free(BootSector);

				if (InfoSector)
					free(InfoSector);

			}

		};

	}

}