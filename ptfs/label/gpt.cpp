
#include "config.h"
#include "base.h"
#include "disk_device.h"
#include "label/gpt.h"
#include "partition.h"
#include "partition_device.h"



namespace ptfs {

	namespace label {

		const EFI_PARTITION_ENTRY* Gpt::GetPartitionEntry(uint8_t Index) {

			return PartitionEntries[Index];

		}

		bool Gpt::MakeLabel() {

			sec_off_t DevSize = Dev->GetSize();

			memset(&Mbr, 0, sizeof(Mbr));
			Mbr.magic = 0xAA55;
			Mbr.partitions[0].BootIndicator = 0;
			Mbr.partitions[0].Type = PARTITION_GPT;
			Mbr.partitions[0].Start = 0;
			Mbr.partitions[0].Length = 0xFFFFFFFF;

			GptHeader.Header.Signature = 0x5452415020494645;
			GptHeader.Header.HeaderSize = 0x92;
			GptHeader.Header.Reserved = 0;
			GptHeader.Header.Revision = 0x10000;

			GptHeader.FirstUsableLBA = 1 + 128 * 128 / 512;
			GptHeader.AlternateLBA = Dev->GetSize() - 1;
			GptHeader.SizeOfPartitionEntry = 128;
			GptHeader.MyLBA = 1;
			GptHeader.NumberOfPartitionEntries = 0;
			GptHeader.LastUsableLBA = Dev->GetSize() - GptHeader.FirstUsableLBA;
			GenerateGuid(&GptHeader.DiskGUID);

			/* 不急着计算CRC，后面可能还要改NumberOfPartitionEntries字段 */

			return true;

		}

	}

}