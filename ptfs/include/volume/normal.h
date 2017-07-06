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

			//��������
			virtual ssize_t	ReadVolumeSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//����д��
			virtual ssize_t	WriteVolumeSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//Ϊ����Ӻ���豸
			virtual bool AddBlockDevice(device::Device* Device) override;

			//ͬ��������
			virtual void SyncPartitionTable(label::Label* Label, uint16_t PartType) override;

		};
		
	}

}