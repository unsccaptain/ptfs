#pragma once

#include <config.h>
#include <base.h>
#include <disk_device.h>
#include <partition_device.h>
#include <volume/volume.h>
#include <fs/filesystem.h>
#include <vector>

#include "label.h"

namespace ptfs{

	namespace label {

		/* this MBR boot code is loaded into 0000:7c00 by the BIOS.  See mbr.s for
		* the source, and how to build it
		*/

		static const char MBR_BOOT_CODE[] = {
			0xfa, 0xb8, 0x00, 0x10, 0x8e, 0xd0, 0xbc, 0x00,
			0xb0, 0xb8, 0x00, 0x00, 0x8e, 0xd8, 0x8e, 0xc0,
			0xfb, 0xbe, 0x00, 0x7c, 0xbf, 0x00, 0x06, 0xb9,
			0x00, 0x02, 0xf3, 0xa4, 0xea, 0x21, 0x06, 0x00,
			0x00, 0xbe, 0xbe, 0x07, 0x38, 0x04, 0x75, 0x0b,
			0x83, 0xc6, 0x10, 0x81, 0xfe, 0xfe, 0x07, 0x75,
			0xf3, 0xeb, 0x16, 0xb4, 0x02, 0xb0, 0x01, 0xbb,
			0x00, 0x7c, 0xb2, 0x80, 0x8a, 0x74, 0x01, 0x8b,
			0x4c, 0x02, 0xcd, 0x13, 0xea, 0x00, 0x7c, 0x00,
			0x00, 0xeb, 0xfe
		};

		class Msdos :public Label{

		private:

			DosRawTable mbr;

			std::vector<device::PartitionDevice*> part_array;

		public:

			//判断一个dev是不是mbr格式的
			static bool Probe(device::Device* blk_dev) {

				DosRawTable Mbr;
				if (blk_dev->ReadDeviceSector(0, (uint8_t*)&Mbr, 1) != 1)
					return false;

				if (Mbr.partitions[0].Type == PARTITION_GPT)
					return false;

				if (Mbr.magic != MSDOS_MAGIC)
					return false;

				if (Mbr.mbr_signature == 0)
					return false;

				return true;

			}

			//初始化一个msdos的label
			Msdos(device::Device* dev):Label("msdos",dev){
			
				if (!Probe(blk_dev)) {
					part_array.clear();
					return;
				}

				DosRawTable Mbr;
				if (blk_dev->ReadDeviceSector(0, (uint8_t*)&Mbr, 1) != 1)
					return;
			
				memcpy(&this->mbr, &Mbr, sizeof(Mbr));

				for (uint8_t i = 0;i < 4;i++) {
					if (Mbr.partitions[i].Type != 0) {
						device::PartitionDevice* Device = new device::PartitionDevice(dev,
							Mbr.partitions[i].Start,
							Mbr.partitions[i].Length,
							i + 1
						);
						part_array.push_back(Device);
					}
				}

			}

			//获得一个分区表项
			const DosRawPartition* GetPartitionEntry(uint8_t Index);
			
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