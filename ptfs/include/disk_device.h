#pragma once

#include <config.h>
#include <base.h>
#include <device.h>
#include <os-dep.h>
//#include <volume/volume.h>

namespace ptfs {

	namespace volume {

		class Volume;

	}

	namespace device {

		class DiskDevice :public Device {

		private:

			adapter_parameter	disk_adapter_info
				= { 0,0 };
			alignment_parameter disk_align_info
				= { 0,0,0 };

			//物理磁盘使用
			char*				disk_name = NULL;
			device_t			disk_handle;
			
			//逻辑磁盘使用
			volume::Volume*		disk_vol_backend;

		public:

			DiskDevice() {}
			
			//创建物理磁盘设备
			DiskDevice(char* DevieName) 
			{
				disk_handle = open_disk_device(DevieName);
				if (disk_handle <= 0) {
					Status = ERR_INVALID_DEV;
					return;
				}
				else {
					disk_name = _strdup(DevieName);
					query_adapter(disk_handle, &disk_adapter_info);
					query_align(disk_handle, &disk_align_info);
					dev_size = get_device_size(disk_handle);
					dev_type = PhysicalDisk;
					Status = ERR_SUCCESS;
				}
			}

			//创建基于卷的逻辑磁盘设备
			DiskDevice(volume::Volume* VolBackend) 
			{
				if (VolBackend == NULL) {
					disk_vol_backend = VolBackend;
				}
				else
					Status = ERR_FIX_MEDIA;
			}

			//获取adapter信息
			const adapter_parameter* GetAdapterParameter()
			{
				return &disk_adapter_info;
			}

			//获取对齐信息
			const alignment_parameter* GetAlignmentParameter()
			{
				return &disk_align_info;
			}

			//扇区读设备
			virtual ssize_t ReadDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//扇区写设备
			virtual ssize_t WriteDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//获得设备起始LBA
			virtual sec_off_t GetDeviceStartLBA() override
			{
				return 0;
			}

			//获得设备大小LBA
			virtual sec_off_t GetDeviceSize() override 
			{
				return dev_size;
			}

			//锁定设备
			bool Lock() 
			{
				return lock_disk_device(disk_handle);
			}

			//解锁设备
			bool Unlock() 
			{
				return unlock_disk_device(disk_handle);
			}

			//获取最佳对齐大小
			virtual sec_off_t GetOptimumAlign() override;

			//获取默认对齐大小
			virtual sec_off_t GetDefaultAlign() override;

		};

	}

}