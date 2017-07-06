
#include "config.h"
#include "base.h"
#include "disk_device.h"
#include "label/gpt.h"
#include "partition_device.h"
#include <algorithm>

namespace ptfs {

	namespace label {

		const EFI_PARTITION_ENTRY* Gpt::GetPartitionEntry(uint8_t PartNumber) {
			assert(PartNumber >= 0);
			return &part_entries[PartNumber - 1];
		}

		device::PartitionDevice* Gpt::GetPartitionDevice(uint8_t PartNumber) {
			assert(PartNumber >= 0);
			for (size_t i = 0;i < part_array.size();i++) {
				if (part_array[i]->GetPartitionNumber() == PartNumber)
					return part_array[i];
			}
			return NULL;
		}

		bool Gpt::MakeLabel() {

			std::for_each(begin(part_array), end(part_array), [](device::PartitionDevice* dev) {
				delete dev;
			});
			part_array.clear();
			
			sec_off_t label_size = ALIGN_DOWN_BY(blk_dev->GetDeviceSize(), blk_dev->GetOptimumAlign());
			if (label_size < GPT_MIN_HEADER_SIZE)
				return false;

			memset(&mbr, 0, sizeof(mbr));
			mbr.magic = 0xAA55;
			mbr.partitions[0].BootIndicator = 0;
			mbr.partitions[0].Type = PARTITION_GPT;
			mbr.partitions[0].Start = 0;
			mbr.partitions[0].Length = 0xFFFFFFFF;

			gpt_header.Header.Signature = 0x5452415020494645;
			gpt_header.Header.HeaderSize = sizeof(EFI_PARTITION_TABLE_HEADER);
			gpt_header.Header.Reserved = 0;
			gpt_header.Header.Revision = 0x10000;

			gpt_header.FirstUsableLBA = 2 + sizeof(EFI_PARTITION_ENTRY) * 128 / 512;
			gpt_header.AlternateLBA = label_size - 1;
			gpt_header.SizeOfPartitionEntry = sizeof(EFI_PARTITION_ENTRY);;
			gpt_header.MyLBA = 1;
			gpt_header.NumberOfPartitionEntries = 0x80;
			gpt_header.LastUsableLBA = label_size - gpt_header.FirstUsableLBA + 1;
			GenerateGuid(&gpt_header.DiskGUID);

			part_entries = (EFI_PARTITION_ENTRY*)malloc(gpt_header.NumberOfPartitionEntries*gpt_header.SizeOfPartitionEntry);
			if (part_entries == NULL)
				return false;
			memset(part_entries, 0,
				gpt_header.NumberOfPartitionEntries*gpt_header.SizeOfPartitionEntry);

			/* 不急着计算CRC，后面可能还要改NumberOfPartitionEntries字段 */

			//创建一个FLAG
			auto part_dev = new device::PartitionDevice(
				blk_dev,
				blk_dev->GetDeviceSize(),
				blk_dev->GetDeviceSize(),
				-1
			);

			part_array.push_back(part_dev);

			return true;

		}

		device::PartitionDevice* Gpt::MakePart(sec_off_t Size, sec_off_t Start) {

			//获取对齐量
			sec_off_t sec_align = blk_dev->GetOptimumAlign();
			/* 对齐后的理想大小 */
			sec_off_t real_size = ALIGN_DOWN_BY(Size, sec_align);
			/* 可以接受的容量损失 */
			sec_off_t size_loss = real_size * 0.1;
			/* 真实起始地址 */
			sec_off_t real_start = 0;
			/* 可用大小,可用大小满足的情况下再看对齐后的大小能否满足可接受容量损失 */
			sec_off_t usable_size = 0;

			if (part_array.size() >= 128)
				return false;

			if (Start >= blk_dev->GetDeviceSize() || Size >= blk_dev->GetDeviceSize()) {
				Status = ERR_INVALID_POS;
				return false;
			}

			//本来没有分区
			if (part_array.size() == 0) {

				real_start = Start;
				if (real_size > blk_dev->GetDeviceSize()) {
					real_size = blk_dev->GetDeviceSize() - Start;
				}
				if (Size - real_size > size_loss) {
					printf("device space limited!");
					return NULL;
				}

			}
			//本来只有一个分区
			else if (Start < part_array[0]->GetDeviceStartLBA()) {

				Start = MBR_HEADER_SIZE + GPT_HEADER_SIZE + GPT_TABLE_SIZE(&gpt_header);
				real_start = ALIGN_UP_BY(Start, sec_align);

				if (part_array[0]->GetDeviceStartLBA() - real_start > Size)
					real_size = Size;
				else
					real_size = part_array[0]->GetDeviceStartLBA() - real_start;

				real_size = ALIGN_DOWN_BY(real_size, sec_align);

				if (Size - real_size > size_loss) {
					printf("device space limited!");
					return NULL;
				}

			}
			else {

				for (int i = 0;i < part_array.size();i++) {
					if (part_array[i]->GetDeviceStartLBA() > Start) {

						real_start = ALIGN_UP_BY(part_array[i - 1]->GetDeviceStartLBA(), sec_align);
						real_size = ALIGN_DOWN_BY(part_array[i - 1]->GetDeviceStartLBA() - real_start, sec_align);

						if (Size - real_size > size_loss) {
							real_start = -1;
							if (Start != 0)break;
							continue;
						}

					}
				}

				if (real_start = -1) {
					printf("device space limited!");
					return NULL;
				}

			}

			//生成设备类并入栈
			auto part_dev = new device::PartitionDevice(blk_dev,
				real_start,
				real_start + real_size);
			part_array.push_back(part_dev);

			//根据其实LBA排序所有设备
			std::sort(begin(part_array), end(part_array), device::PartitionComp);

			//重新分配分区号
			std::for_each(std::begin(part_array), std::end(part_array) - 1, 
				[](device::PartitionDevice* part_dev) {
				static int i = 1;
				part_dev->SetPartitionNumber(i++);
			});

			return part_dev;

		}

		//设置分区类型和名称
		void Gpt::SetPartitionType(uint8_t PartNumber, uint16_t PartType) {

			switch(PartType){
			case PARTITION_ESP:
			{
				GUID FsGuid = EFI_PARTITION_TYPE;
				memcpy(&part_entries[PartNumber - 1].PartitionTypeGUID, &FsGuid, sizeof(GUID));
				wcscpy((wchar_t*)part_entries[PartNumber - 1].PartitionName, L"EFI system partition");
				break;
			}
			default: 
			{
				GUID FsGuid = COMMON_PARTITION_TYPE;
				memcpy(&part_entries[PartNumber - 1].PartitionTypeGUID, &FsGuid, sizeof(GUID));
				wcscpy((wchar_t*)part_entries[PartNumber - 1].PartitionName, L"Basic data partition");
				break;
			}
			}

		}

		//把磁盘格式同步至硬盘
		bool Gpt::Sync() {

			Gpt* my = this;

			//迭代填充分区表
			std::for_each(begin(part_array), end(part_array)-1, [my](device::PartitionDevice* part_dev) {

				EFI_PARTITION_ENTRY* part_entry = (EFI_PARTITION_ENTRY*)my->GetPartitionEntry(part_dev->GetPartitionNumber());
				part_entry->StartingLBA = part_dev->GetDeviceStartLBA();
				part_entry->EndingLBA = part_dev->GetDeviceStartLBA() + part_dev->GetDeviceSize();
				part_entry->Attributes = 0;

#ifdef _WIN32
				CoCreateGuid((GUID*)&part_entry->UniquePartitionGUID);
#endif
			});
			
			if (blk_dev->WriteDeviceSector(0, (uint8_t*)&mbr, 1) != 1)
				return false;

			//计算分区表的CRC32
			uint32_t loc_crc32 = RuntimeDriverCalculateCrc32((unsigned char*)part_entries, 
				gpt_header.SizeOfPartitionEntry*gpt_header.NumberOfPartitionEntries
			);
			loc_crc32 = loc_crc32;
			gpt_header.PartitionEntryArrayCRC32 = loc_crc32;

			gpt_header.Header.CRC32 = 0;
			//计算GPT头的CRC32
			loc_crc32 = RuntimeDriverCalculateCrc32((unsigned char*)&gpt_header,
				sizeof(EFI_PARTITION_TABLE_HEADER)
			);
			loc_crc32 = loc_crc32;
			gpt_header.Header.CRC32 = loc_crc32;

			//分别写GPT头和分区表
			if (blk_dev->WriteDeviceSector(1, (uint8_t*)&gpt_header, 1) != 1)
				return false;

			sec_off_t part_entry_lbs = (gpt_header.NumberOfPartitionEntries*gpt_header.SizeOfPartitionEntry) / 512;
			if (blk_dev->WriteDeviceSector(2, (uint8_t*)part_entries, part_entry_lbs) != part_entry_lbs)
				return false;
			
			return true;

		}

	}

}