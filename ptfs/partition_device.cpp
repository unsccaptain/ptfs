
#include <config.h>
#include <base.h>
#include <device.h>
#include <disk_device.h>
#include <partition_device.h>
#include <label/label.h>
#include <volume/volume.h>
#include <os-dep.h>
#include <err.h>

namespace ptfs {

	namespace device {

		//�����������豸
		ssize_t PartitionDevice::ReadDeviceSector(sec_off_t Offset,uint8_t* Buffer,sec_off_t Size) {
			if (Offset + Size >= dev_size)
				return 0;
			return blk_dev->ReadDeviceSector(dev_start_sec + Offset, Buffer, Size);
		}

		//����д�����豸
		ssize_t PartitionDevice::WriteDeviceSector(sec_off_t Offset,uint8_t* Buffer,sec_off_t Size) {
			if (Offset + Size >= dev_size)
				return 0;
			return blk_dev->WriteDeviceSector(dev_start_sec + Offset, Buffer, Size);
		}

		//��ȡ�豸��С
		sec_off_t PartitionDevice::GetDeviceSize() {
			return dev_size;
		}

		//��ȡ�豸��ʼLBA
		sec_off_t PartitionDevice::GetDeviceStartLBA() {
			return dev_start_sec;
		}

		//��ȡ��Ѷ����С
		sec_off_t PartitionDevice::GetOptimumAlign() {
			return 8;
		}

		//��ȡĬ�϶����С
		sec_off_t PartitionDevice::GetDefaultAlign() {
			return 8;
		}

		void PartitionDevice::SyncPartitionTable(label::Label* Label, uint16_t PartType) {
			assert(Label != NULL);
			Label->SetPartitionType(part_number, PartType);
		}

		int __cdecl PartitionComp(void const* a, void const* b) {
		
			PartitionDevice* first = (PartitionDevice*)a;
			PartitionDevice* second = (PartitionDevice*)b;

			return (first->GetDeviceStartLBA() < second->GetDeviceStartLBA());

		}

	}

}