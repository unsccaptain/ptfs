
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

		//扇区读分区设备
		ssize_t PartitionDevice::ReadDeviceSector(sec_off_t Offset,uint8_t* Buffer,sec_off_t Size) {
			if (Offset + Size >= dev_size)
				return 0;
			return blk_dev->ReadDeviceSector(dev_start_sec + Offset, Buffer, Size);
		}

		//扇区写分区设备
		ssize_t PartitionDevice::WriteDeviceSector(sec_off_t Offset,uint8_t* Buffer,sec_off_t Size) {
			if (Offset + Size >= dev_size)
				return 0;
			return blk_dev->WriteDeviceSector(dev_start_sec + Offset, Buffer, Size);
		}

		//获取设备大小
		sec_off_t PartitionDevice::GetDeviceSize() {
			return dev_size;
		}

		//获取设备起始LBA
		sec_off_t PartitionDevice::GetDeviceStartLBA() {
			return dev_start_sec;
		}

		//获取最佳对齐大小
		sec_off_t PartitionDevice::GetOptimumAlign() {
			return 8;
		}

		//获取默认对齐大小
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