#pragma once

#include <config.h>
#include <base.h>
#include <disk_device.h>
#include <partition.h>
#include <fs/filesystem.h>
#include <vector>

#include "label.h"

namespace ptfs {

	namespace label {

		class Gpt :public Label {

		private:

			DosRawTable Mbr;

			EFI_PARTITION_TABLE_HEADER GptHeader;

			std::vector<Partition>Partitions;

			std::vector<EFI_PARTITION_ENTRY*>PartitionEntries;

		public:

			Gpt(device::Device* dev)
				:Label("gpt", dev) {}

			static bool Probe(device::Device* Dev) {

				DosRawTable Mbr;
				if (Dev->ReadDeviceSector(0, (uint8_t*)&Mbr, 1) != 1)
					return false;

				if (Mbr.mbr_signature != MSDOS_MAGIC)
					return false;

				if (Mbr.partitions[0].Type != PARTITION_GPT)
					return false;

				EFI_PARTITION_TABLE_HEADER GptHeader;
				if (Dev->ReadDeviceSector(1, (uint8_t*)&Mbr, 1) != 1)
					return false;

				/* UEFI SPEC 5.3.2 */

				/* EFI PART */
				if (GptHeader.Header.Signature != 0x5452415020494645)
					return false;

				/* 必须比结构体大 */
				if (GptHeader.Header.HeaderSize < 0x92)
					return false;

				uint32_t oldCrc32, Crc32;
				oldCrc32 = GptHeader.Header.CRC32;
				GptHeader.Header.CRC32 = 0;

				/* 计算头部校验和 */
				Crc32 = make_crc(0xFFFFFFFF, (unsigned char*)&GptHeader, GptHeader.Header.HeaderSize);
				if (oldCrc32 != Crc32)
					return false;

				/* 计算分区表校验和 */
				uint32_t PartitionEntrySize = GptHeader.SizeOfPartitionEntry*GptHeader.NumberOfPartitionEntries;
				uint8_t* PartitionEntry = (uint8_t*)malloc(ALIGN_UP_BY(PartitionEntrySize, 512));
				Dev->ReadDeviceSector(GptHeader.PartitionEntryLBA, (uint8_t*)PartitionEntry, (ALIGN_UP_BY(PartitionEntrySize, 512)) >> 9);

				Crc32 = make_crc(0xFFFFFFFF, PartitionEntry, PartitionEntrySize);
				free(PartitionEntry);

				if (GptHeader.PartitionEntryArrayCRC32 != Crc32) {
					return false;
				}

				return true;

			}

			const EFI_PARTITION_ENTRY* GetPartitionEntry(uint8_t Index);

			bool MakeLabel();

		};

	}

}