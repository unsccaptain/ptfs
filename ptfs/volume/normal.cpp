
#include <base.h>

#include <volume/volume.h>
#include <volume/normal.h>

#include <device.h>

namespace ptfs {

	namespace volume {

		ssize_t	NormalVolume::ReadVolumeSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {

			return dev_seq[0]->ReadDeviceSector(Offset, Buffer, Size);

		}

		ssize_t	NormalVolume::WriteVolumeSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {

			return dev_seq[0]->WriteDeviceSector(Offset, Buffer, Size);
		}

		bool NormalVolume::AddBlockDevice(device::Device* Device) {
			if (dev_seq.size() == 1)
				return false;
			else {
				dev_seq.push_back(Device);
				return true;
			}
		}

		//同步分区表
		void NormalVolume::SyncPartitionTable(label::Label* Label, uint16_t PartType) {

			char* vol_name = volume_name;
			std::for_each(dev_seq.begin(), dev_seq.end(), [Label, PartType,vol_name](device::Device* dev) {
				if (dev->GetDeviceType() != device::Partition)
					return;
				device::PartitionDevice* part_dev = (device::PartitionDevice*)dev;
				part_dev->SyncPartitionTable(Label, PartType);
			});

		}

	}

}