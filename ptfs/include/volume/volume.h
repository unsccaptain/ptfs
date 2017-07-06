#pragma once

#include <config.h>
#include <base.h>

//#include <fs/filesystem.h>
#include <device.h>
#include <label/label.h>
#include <vector>
#include <algorithm>

namespace ptfs {

	namespace volume {

		class Volume :public Base {

		protected:

			std::vector<device::Device*>	dev_seq;

			uint16_t						volume_type;

			char*							volume_name;

		public:

			//Volume(sec_off_t start_sector, sec_off_t total_sector, filesystem::FileSystem* fs)
			//	:StartSector(start_sector), TotalSectors(total_sector), Fs(fs) {}

			Volume(){}

			virtual ssize_t	ReadVolumeSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) = 0;

			virtual ssize_t	WriteVolumeSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) = 0;

			virtual bool AddBlockDevice(device::Device* Device) = 0;

			//设置属性

			uint16_t GetVolumeType() { return volume_type; }

			void SetVolumeType(uint16_t vol_type) { volume_type = vol_type; }

			char* GetVolumeName() { return volume_name; }

			void SetVolumeName(char* vol_name) { volume_name = strdup(vol_name); }

			//同步磁盘分区表
			virtual void SyncPartitionTable(label::Label* Label, uint16_t PartType) = 0;

			//获取卷总大小
			sec_off_t	GetVolumeSize() {

				sec_off_t to_size = 0;
				std::for_each(begin(dev_seq), end(dev_seq), [&to_size](device::Device* dev) {
					to_size += dev->GetDeviceSize();
				});
				return to_size;
			
			}

		};

	}

}