
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

		void Fat::FillInfoSector() {
			/* 签名是个固定值 */
			InfoSector->signature_1 = 0x41615252;
			InfoSector->signature_2 = 0x61417272;
			InfoSector->signature_3 = 0xAA55;
		}

		void Fat::FillBootSectorFat32() {

			BootSector->boot_sign = 0xAA55;
			BootSector->hidden = 0;
			BootSector->secs_track = 0x3F;
			BootSector->reserved = 64;
			BootSector->fats = 2;
			BootSector->heads = 255;
			BootSector->media = 0xF8;
			BootSector->dir_entries = 0;

			memcpy(BootSector->system_id, "PTFS FAT", 8);
			memcpy(BootSector->boot_jump, FAT_BOOT_JUMP, 3);

			BootSector->u.fat32.root_dir_cluster = 2;
			BootSector->u.fat32.info_sector = 1;
			BootSector->u.fat32.drive_num = 0x80;
			BootSector->u.fat32.ext_signature = 0x29;
			BootSector->u.fat32.backup_sector = 6;

			strcpy((char*)BootSector->u.fat32.fat_name, "FAT32");
			strcpy((char*)BootSector->u.fat32.volume_name, "NO NAME");
		}

		bool Fat::MakeFs(uint32_t Type, uint32_t ClusterSize) {
			
			sec_off_t size_in_sec = ALIGN_DOWN_BY(Volume->GetVolumeSize(), 8);

			if ((Type != PARTITION_FAT_16) && (Type != PARTITION_FAT32)) {
				Status = ERR_UNSUPPORT;
				return false;
			}

			if (Volume->GetVolumeSize() < 0x400) {
				Status = ERR_INVALID_DEV;
				return false;
			}

			if (BootSector) {
				free(BootSector);
			}

			/* 填写DBR */
			BootSector = (FatBootSector*)malloc(sizeof(FatBootSector));
			memset(BootSector, 0, sizeof(FatBootSector));
			FillBootSectorFat32();

			BootSector->cluster_size = ClusterSize / LogicalSizeInByte;
			BootSector->sector_size = LogicalSizeInByte;
			BootSector->sector_count = size_in_sec;

			this->Type = Type;

			if (Type == PARTITION_FAT32) {
				if (InfoSector) {
					free(InfoSector);
				}

				InfoSector = (FatInfoSector*)malloc(sizeof(*InfoSector));
				memset(InfoSector, 0, sizeof(*InfoSector));
				FillInfoSector();

				/*
				FAT表项数计算公式
				reserved+fat_count*(x*4/512)+x*cluster_size=dev_size
				(size-r)/(fc*4*512+c_s)=x  
				*/

				/* 计算出FAT表项数并进一步计算簇起始地址和可用簇数 */
				TbEntryCount = 
					((size_in_sec - BootSector->reserved) / (BootSector->fats * 4 / LogicalSizeInByte + BootSector->cluster_size));
				TbEntrySize = 4;
				FirstFat = BootSector->reserved;
				FirstClusterSector = BootSector->reserved + BootSector->fats*(TbEntryCount * 4 / LogicalSizeInByte);

				ValidSecSize = size_in_sec;
				ValidClusterSize = (size_in_sec - FirstClusterSector) / BootSector->cluster_size;

				BootSector->u.fat32.fat_length = (TbEntryCount*TbEntrySize / LogicalSizeInByte);

				struct _timeb timebuffer;
				/* 利用时间生成一个随机数作为签名 */
				_ftime_s(&timebuffer);
				srand(timebuffer.millitm);

				BootSector->u.fat32.serial_number = rand();

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

		bool Fat::SyncPartitionTable(label::Label* Label) {

			Volume->SyncPartitionTable(Label, GetType());

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
			if (Volume->WriteVolumeSector(0, (uint8_t*)BootSector, 1) != 1)
				return false;
			if (Volume->WriteVolumeSector(BootSector->u.fat32.info_sector, (uint8_t*)InfoSector, 1) != 1)
				return false;
			if (Volume->WriteVolumeSector(2, (uint8_t*)ThirdSector, 1) != 1)
				return false;

			/* 写入备份扇区 */
			if (Volume->WriteVolumeSector(BootSector->u.fat32.backup_sector, (uint8_t*)BootSector, 1) != 1)
				return false;
			if (Volume->WriteVolumeSector(BootSector->u.fat32.backup_sector + 1, (uint8_t*)InfoSector, 1) != 1)
				return false;
			if (Volume->WriteVolumeSector(BootSector->u.fat32.backup_sector + 2, (uint8_t*)ThirdSector, 1) != 1)
				return false;

			/* 写FAT表 */
			memset(FatEntries, 0, 512);
			FatEntries[0] = 0xFFFFFF8;
			FatEntries[1] = 0x7FFFFFFF;
			FatEntries[2] = 0xFFFFFFFF;
			
			if (Volume->WriteVolumeSector(BootSector->reserved, (uint8_t*)FatEntries, 1) != 1)
				return false;
			if (Volume->WriteVolumeSector(BootSector->reserved + BootSector->u.fat32.fat_length, (uint8_t*)FatEntries, 1) != 1)
				return false;
			
			memset(FatEntries, 0, 512);
			FatDirEntry* RootEntry = (FatDirEntry*)ThirdSector;
			memcpy(RootEntry->name, LabelName, 8);
			RootEntry->attributes = VOLUME_LABEL_ATTR;
			RootEntry->length = 0;

			if (Volume->WriteVolumeSector(FirstClusterSector, (uint8_t*)FatEntries, 1) != 1)
				return false;

			return true;

		}	

	}

}