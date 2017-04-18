
#include <config.h>
#include <base.h>
#include <err.h>
#include <partition_device.h>
#include <fs/filesystem.h>
#include <fs/fat.h>
#include <sys/timeb.h>
#include <time.h>
#include <stdio.h>

namespace ptfs {

	namespace filesystem {

		bool Fat::MakeFs(uint32_t Type, uint32_t ClusterSize) {

			struct _timeb timebuffer;

			if ((Type != PARTITION_FAT_16) && (Type != PARTITION_FAT32)) {
				Status = ERR_UNSUPPORT;
				return false;
			}

			if (Dev->GetSize() <= 0x400) {
				Status = ERR_INVALID_DEV;
				return false;
			}

			if (BootSector) {
				free(BootSector);
			}

			/* 填写DBR */
			BootSector = (FatBootSector*)malloc(sizeof(*BootSector));
			memset(BootSector, 0, sizeof(*BootSector));
			BootSector->boot_sign = 0xAA55;
			BootSector->cluster_size = ClusterSize / LogicalSizeInByte;
			BootSector->hidden = 0;
			BootSector->sector_size = LogicalSizeInByte;
			BootSector->secs_track = 0x3F;
			BootSector->reserved = 64;
			BootSector->sector_count = Dev->GetSize();
			BootSector->fats = 2;
			BootSector->heads = 255;
			BootSector->media = 0xF8;

			memcpy(BootSector->system_id, "PTFS FAT", 8);
			memcpy(BootSector->boot_jump, FAT_BOOT_JUMP, 3);

			this->Type = Type;

			if (Type == PARTITION_FAT32) {

				if (InfoSector) {
					free(InfoSector);
				}

				InfoSector = (FatInfoSector*)malloc(sizeof(*InfoSector));
				memset(InfoSector, 0, sizeof(*InfoSector));

				BootSector->dir_entries = 0;

				BootSector->u.fat32.root_dir_cluster = 2;
				BootSector->u.fat32.info_sector = 1;
				BootSector->u.fat32.drive_num = 0x80;
				BootSector->u.fat32.ext_signature = 0x29;
				BootSector->u.fat32.backup_sector = 6;
				
				strcpy((char*)BootSector->u.fat32.fat_name, "FAT32");
				strcpy((char*)BootSector->u.fat32.volume_name, "NO NAME");

				/*
				FAT表项数计算公式
				reserved+fat_count*(x*4/512)+x*cluster_size=dev_size
				(size-r)/(fc*4*512+c_s)=x  
				*/

				/* 计算出FAT表项数并进一步计算簇起始地址和可用簇数 */
				TbEntryCount = 
					((Dev->GetSize() - BootSector->reserved) / (BootSector->fats * 4 / LogicalSizeInByte + BootSector->cluster_size));
				TbEntrySize = 4;
				FirstFat = BootSector->reserved;
				FirstClusterSector = BootSector->reserved + BootSector->fats*(TbEntryCount * 4 / LogicalSizeInByte);

				ValidSecSize = Dev->GetSize();
				ValidClusterSize = (Dev->GetSize() - FirstClusterSector) / BootSector->cluster_size;

				BootSector->u.fat32.fat_length = (TbEntryCount*TbEntrySize / LogicalSizeInByte);

				/* 利用时间生成一个随机数作为签名 */
				_ftime_s(&timebuffer);
				srand(timebuffer.millitm);

				BootSector->u.fat32.serial_number = rand();

				/* 签名是个固定值 */
				InfoSector->signature_1 = 0x41615252;
				InfoSector->signature_2 = 0x61417272;
				InfoSector->signature_3 = 0xAA55;

				InfoSector->free_clusters = ValidClusterSize;
				InfoSector->next_cluster = 2;

			}
			else if (Type == PARTITION_FAT_16) {

				/*
				BootSector->dir_entries = 32;

				/* 利用时间生成一个随机数作为签名 */
				/*_ftime_s(&timebuffer);
				srand(timebuffer.millitm);

				BootSector->u.fat16.serial_number = rand();

				strcpy((char*)BootSector->u.fat32.fat_name, "FAT16");
				strcpy((char*)BootSector->u.fat32.volume_name, "NO NAME");
				*/

				return false;

			}

			return true;

		}

		bool Fat::Sync() {

			uint8_t* ThirdSector = (uint8_t*)malloc(512);
			uint32_t* FatEntries = (uint32_t*)ThirdSector;

			/* 第三个扇区结束标志 */
			memset(ThirdSector, 0, 512);
			ThirdSector[510] = 0x55;
			ThirdSector[511] = 0xAA;

			/* 写入头三个扇区 */
			Dev->WriteDeviceSector(0, (uint8_t*)BootSector, 1);
			Dev->WriteDeviceSector(BootSector->u.fat32.info_sector, (uint8_t*)InfoSector, 1);
			Dev->WriteDeviceSector(2, (uint8_t*)ThirdSector, 1);

			/* 写入备份扇区 */
			Dev->WriteDeviceSector(BootSector->u.fat32.backup_sector, (uint8_t*)BootSector, 1);
			Dev->WriteDeviceSector(BootSector->u.fat32.backup_sector + 1, (uint8_t*)InfoSector, 1);
			Dev->WriteDeviceSector(BootSector->u.fat32.backup_sector + 2, (uint8_t*)ThirdSector, 1);

			/* 写FAT表 */
			memset(FatEntries, 0, 512);
			FatEntries[0] = 0xF8FFFF0F;
			FatEntries[1] = 0xFFFFFFFF;
			FatEntries[2] = 0xFFFFFFFF;
			Dev->WriteDeviceSector(BootSector->reserved, (uint8_t*)FatEntries, 1);
			Dev->WriteDeviceSector(BootSector->reserved + BootSector->u.fat32.fat_length, (uint8_t*)FatEntries, 1);
			
			memset(FatEntries, 0, 512);
			FatDirEntry* RootEntry = (FatDirEntry*)ThirdSector;
			memcpy(RootEntry->name, LabelName, 8);
			RootEntry->attributes = VOLUME_LABEL_ATTR;
			RootEntry->length = 0;

			Dev->WriteDeviceSector(FirstClusterSector, (uint8_t*)FatEntries, 1);

			return true;

		}

	}

}