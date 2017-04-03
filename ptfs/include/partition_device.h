#pragma once

#include "config.h"
#include "base.h"
#include "device.h"
#include "disk_device.h"
#include "io_if.h"
#include "err.h"

/*
�����PartitionDevice������ʾ���豸�е�һ������
���ȿ������������̣�Ҳ������һ�οհ׿ռ䣬Ҳ�����Ǿ�����
������Label�е�Partition���ʾ�ڷ�������������ķ���
*/

namespace ptfs {

	namespace device {

		class PartitionDevice :public Device {

		private:

			char*			VolumePathName;
			device_t		Handle = 0;

			Device*			Dev;
			sec_off_t		DevStartSec = 0;
			sec_off_t		DevSizeSec = 0;

		public:

			PartitionDevice(char* Vol) 
				: Dev(NULL) {

				Handle = OpenDevice(Vol);
				if (Handle == reinterpret_cast<device_t>(-1)) {
					Status = ERR_INVALID_DEV;
					return;
				}
				else {
					this->VolumePathName = _strdup(Vol);
					Status = ERR_SUCCESS;
				}

			}

			PartitionDevice(Device* Device, sec_off_t Start, sec_off_t Size) 
				:Dev(Device), DevStartSec(Start), DevSizeSec(Size) {}

			ssize_t ReadDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {

				if (Handle) {
					return ReadDevice(Handle, Offset, Buffer, Size);
				}
				else
				{

					if (Offset + Size >= DevSizeSec)
						return 0;

					return Dev->ReadDeviceSector(DevStartSec + Offset, Buffer, Size);

				}

			}

			ssize_t WriteDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {

				if (Handle) {
					return WriteDevice(Handle, Offset, Buffer, Size);
				}
				else
				{

					if (Offset + Size >= DevSizeSec)
						return 0;

					return Dev->WriteDeviceSector(DevStartSec + Offset, Buffer, Size);

				}

			}

			sec_off_t GetSize() {

				if (Size == 0) {

					if (Handle)
						Size = GetDeviceSize(Handle);
					else
						Size = DevSizeSec;

				}
				return Size;

			}

			sec_off_t GetAlignInSector() {

				return 8;

			}

		};

	}

}