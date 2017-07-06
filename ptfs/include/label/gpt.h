#pragma once

#include <config.h>
#include <base.h>
#include <disk_device.h>
#include <partition_device.h>
#include <fs/filesystem.h>
#include <crypto/crc32.h>
#include <vector>

#include "label.h"

#define IS_ZERO_GUID(guid) ((((uint64_t*)&(guid))[0]==0)&&(((uint64_t*)&(guid))[1])==0)

#define EFI_PARTITION_TYPE		{0xC12A7328,0xF81F,0x11D2,0xBA4B,0x00,0xA0,0xC9,0x3E,0xC9,0x3B}
#define COMMON_PARTITION_TYPE	{0xEBD0A0A2,0xB9E5,0x4433,0x87C0,0x68,0xB6,0xB7,0x26,0x99,0xC7}

#define MBR_HEADER_SIZE				sizeof(DosRawTable)
#define GPT_HEADER_SIZE				512
#define GPT_TABLE_SIZE(_header_)	((_header_)->SizeOfPartitionEntry*(_header_)->NumberOfPartitionEntries)
#define GPT_MIN_HEADER_SIZE			MBR_HEADER_SIZE+GPT_HEADER_SIZE+128*128

namespace ptfs {

	namespace label {

		class Gpt :public Label {

		private:

			DosRawTable mbr;

			EFI_PARTITION_TABLE_HEADER gpt_header;

			EFI_PARTITION_ENTRY* part_entries;

		protected:

			std::vector<device::PartitionDevice*> part_array;

		public:

			static bool Probe(device::Device* blk_dev) {

				DosRawTable Mbr;
				if (blk_dev->ReadDeviceSector(0, (uint8_t*)&Mbr, 1) != 1)
					return false;

				if (Mbr.magic != MSDOS_MAGIC)
					return false;

				if (Mbr.partitions[0].Type != PARTITION_GPT)
					return false;

				unsigned char SectorBuffer[512];
				EFI_PARTITION_TABLE_HEADER* GptHeader = (EFI_PARTITION_TABLE_HEADER*)SectorBuffer;
				if (blk_dev->ReadDeviceSector(1, (uint8_t*)SectorBuffer, 1) != 1)
					return false;

				/* UEFI SPEC 5.3.2 */

				/* EFI PART */
				if (GptHeader->Header.Signature != 0x5452415020494645)
					return false;

				/* 必须比结构体大 */
				if (GptHeader->Header.HeaderSize < 0x5c)
					return false;

				uint32_t oldCrc32, Crc32;
				oldCrc32 = GptHeader->Header.CRC32;
				GptHeader->Header.CRC32 = 0;

				/* 计算头部校验和 */
				Crc32 = RuntimeDriverCalculateCrc32((unsigned char*)GptHeader, GptHeader->Header.HeaderSize);
				Crc32 = ~Crc32;
				if (oldCrc32 != Crc32)
					return false;

				/* 计算分区表校验和 */
				uint32_t PartitionEntrySize = GptHeader->SizeOfPartitionEntry*GptHeader->NumberOfPartitionEntries;
				uint8_t* PartitionEntry = (uint8_t*)malloc(ALIGN_UP_BY(PartitionEntrySize, 512));
				blk_dev->ReadDeviceSector(GptHeader->PartitionEntryLBA, (uint8_t*)PartitionEntry, (ALIGN_UP_BY(PartitionEntrySize, 512)) >> 9);

				Crc32 = RuntimeDriverCalculateCrc32(PartitionEntry, PartitionEntrySize);
				Crc32 = ~Crc32;
				free(PartitionEntry);

				if (GptHeader->PartitionEntryArrayCRC32 != Crc32) {
					return false;
				}

				return true;

			}

			Gpt(device::Device* dev)
				:Label("gpt", dev) {

				RuntimeDriverInitializeCrc32Table();

				if (!Probe(blk_dev))
					return;

				if (blk_dev->ReadDeviceSector(0, (uint8_t*)&mbr, 1) != 1)
					return;

				if (blk_dev->ReadDeviceSector(1, (uint8_t*)&gpt_header, 1) != 1)
					return;

				int part_entry_size = gpt_header.SizeOfPartitionEntry*gpt_header.NumberOfPartitionEntries;
				int part_entry_secs = part_entry_size >> 9;

				part_entries = (EFI_PARTITION_ENTRY*)
					malloc(part_entry_size);

				if (blk_dev->ReadDeviceSector(2, (uint8_t*)part_entries, part_entry_secs) != part_entry_secs) {
					free(part_entries);
					part_entries = 0;
					return;
				}

				for (int i = 0;i < gpt_header.NumberOfPartitionEntries;i++) {
					if (!IS_ZERO_GUID(part_entries[i].UniquePartitionGUID)) {
						auto new_part_dev = new device::PartitionDevice(
							blk_dev,
							part_entries[i].StartingLBA,
							part_entries[i].EndingLBA,
							i + 1,
							&part_entries[i]
						);
						part_array.push_back(new_part_dev);
					}
				}

				auto part_dev = new device::PartitionDevice(
					blk_dev,
					blk_dev->GetDeviceStartLBA(),
					blk_dev->GetDeviceStartLBA(),
					-1
				);
				part_array.push_back(part_dev);

			}

			const EFI_PARTITION_ENTRY* GetPartitionEntry(uint8_t Index);

			//获得一个分区对象
			virtual device::PartitionDevice* GetPartitionDevice(uint8_t PartNumber) override;

			//生成label
			virtual bool MakeLabel() override;

			//创建分区
			virtual device::PartitionDevice* MakePart(sec_off_t Size, sec_off_t Start) override;

			//设置分区类型
			virtual void SetPartitionType(uint8_t PartNumber, uint16_t PartType) override;

			//将label写入到blockio设备中
			virtual bool Sync() override;

		};

	}

}