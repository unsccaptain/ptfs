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

			//�������ʹ��
			char*				disk_name = NULL;
			device_t			disk_handle;
			
			//�߼�����ʹ��
			volume::Volume*		disk_vol_backend;

		public:

			DiskDevice() {}
			
			//������������豸
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

			//�������ھ���߼������豸
			DiskDevice(volume::Volume* VolBackend) 
			{
				if (VolBackend == NULL) {
					disk_vol_backend = VolBackend;
				}
				else
					Status = ERR_FIX_MEDIA;
			}

			//��ȡadapter��Ϣ
			const adapter_parameter* GetAdapterParameter()
			{
				return &disk_adapter_info;
			}

			//��ȡ������Ϣ
			const alignment_parameter* GetAlignmentParameter()
			{
				return &disk_align_info;
			}

			//�������豸
			virtual ssize_t ReadDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//����д�豸
			virtual ssize_t WriteDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//����豸��ʼLBA
			virtual sec_off_t GetDeviceStartLBA() override
			{
				return 0;
			}

			//����豸��СLBA
			virtual sec_off_t GetDeviceSize() override 
			{
				return dev_size;
			}

			//�����豸
			bool Lock() 
			{
				return lock_disk_device(disk_handle);
			}

			//�����豸
			bool Unlock() 
			{
				return unlock_disk_device(disk_handle);
			}

			//��ȡ��Ѷ����С
			virtual sec_off_t GetOptimumAlign() override;

			//��ȡĬ�϶����С
			virtual sec_off_t GetDefaultAlign() override;

		};

	}

}