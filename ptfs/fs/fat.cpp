
#include <config.h>
#include <base.h>
#include <err.h>
#include <partition_device.h>
#include <fs/filesystem.h>
#include <fs/fat.h>
#include <sys/timeb.h>

namespace ptfs {

	namespace filesystem {

		bool Fat::MakeFs(uint32_t Type, uint32_t ClusterSize) {

			struct _timeb timebuffer;

			if (BootSector) {
				free(BootSector);
				BootSector = (FatBootSector*)malloc(sizeof(*BootSector));
			}

			memset(BootSector, 0, sizeof(*BootSector));
			BootSector->boot_sign = 0xAA55;
			BootSector->cluster_size = ClusterSize / 512;
			BootSector->hidden = 0;
			BootSector->sector_size = 0xFF;
			BootSector->secs_track = 0x3F;
			BootSector->reserved = 64;
			BootSector->sector_count = Dev->GetSize();

			if (Type == PARTITION_FAT32) {

				if (InfoSector) {
					free(InfoSector);
					InfoSector = (FatInfoSector*)malloc(sizeof(*InfoSector));
				}
				memset(InfoSector, 0, sizeof(*InfoSector));

				memcpy(BootSector->boot_jump, FAT_BOOT_JUMP, 3);

				BootSector->fats = 2;
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
				(size-r)/(fc*4*512+c_s)=2*x  
				*/

				/* 计算出FAT表项数并进一步计算簇起始地址和可用簇数 */
				TbEntryCount = 
					((Dev->GetSize() - BootSector->reserved) / (BootSector->fats * 4 / 512 + BootSector->cluster_size)) / 2;
				TbEntrySize = 4;
				FirstFat = BootSector->reserved;
				FirstClusterSector = BootSector->reserved + BootSector->fats*(TbEntryCount * 4 / 512);

				ValidSecSize = Dev->GetSize();
				ValidClusterSize = (Dev->GetSize() - FirstClusterSector) / BootSector->cluster_size;

				BootSector->u.fat32.fat_length = (TbEntryCount*TbEntrySize / 512);

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

		}

	}

}