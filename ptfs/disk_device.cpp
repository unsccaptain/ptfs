
#include <config.h>
#include <base.h>
#include <device.h>
#include <os-dep.h>
#include <volume/volume.h>
#include <disk_device.h>

namespace ptfs {

	namespace device {

		//�������豸
		ssize_t DiskDevice::ReadDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {
			if (dev_type == PhysicalDisk)
				return read_device_sector(disk_handle, Offset, Buffer, Size);
			else
				return disk_vol_backend->ReadVolumeSector(Offset, Buffer, Size);
		}

		//����д�豸
		ssize_t DiskDevice::WriteDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {
			if (dev_type == PhysicalDisk)
				return write_device_sector(disk_handle, Offset, Buffer, Size);
			else
				return disk_vol_backend->WriteVolumeSector(Offset, Buffer, Size);
		}

		//��ȡ��Ѷ����С
		sec_off_t DiskDevice::GetOptimumAlign() {

			return 8;
			uint16_t physical_sec_align = disk_adapter_info.PhysicalPageSize >> 9;
			uint16_t trans_sec_align = (uint16_t)(disk_adapter_info.TransferLength >> 9);
			uint16_t default_sec_align = 8;
			uint16_t result = max(physical_sec_align, trans_sec_align);
			result = max(result, default_sec_align);
			return result;
		}

		//��ȡĬ�϶����С
		sec_off_t DiskDevice::GetDefaultAlign() {
			return 8;
		}

	}

}