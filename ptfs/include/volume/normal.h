#pragma once

#include <device.h>
#include "volume.h"
#include <algorithm>

namespace ptfs {

	namespace volume {

		class NormalVolume :public Volume {

		private:

		public:

			NormalVolume() {};

			NormalVolume(device::Device* Device) :Volume(){
				AddBlockDevice(Device);
			}

			//扇区读卷
			virtual ssize_t	ReadVolumeSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//扇区写卷
			virtual ssize_t	WriteVolumeSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//为卷添加后端设备
			virtual bool AddBlockDevice(device::Device* Device) override;

			//同步分区表
			virtual void SyncPartitionTable(label::Label* Label, uint16_t PartType) override;

		};
		
	}

}