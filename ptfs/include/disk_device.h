#pragma once

#include "config.h"
#include "base.h"
#include "device.h"
#include "io_if.h"

namespace ptfs {

	namespace device {

		class DiskDevice :public Device {

		private:

			char			*DeviceName = NULL;
			device_t		Handle;

		public:

			DiskDevice() {}
			DiskDevice(char* Name) {

				Handle = OpenDevice(Name);
				if (Handle == reinterpret_cast<device_t>(-1)) {
					Status = ERR_INVALID_DEV;
					return;
				}
				else {
					this->DeviceName = _strdup(Name);
					Status = ERR_SUCCESS;
				}

			}

			ssize_t ReadDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {

				return ReadDevice(Handle, Offset, Buffer, Size);

			}

			ssize_t WriteDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {

				return WriteDevice(Handle, Offset, Buffer, Size);

			}

			sec_off_t GetSize() {

				if (Size == 0)
					Size = GetDeviceSize(Handle);

				return Size;

			}

			sec_off_t GetAlignInSector() {

				return 8;

			}

			void GetGeometry(uint16_t* c, uint8_t* h, uint8_t* s) {

				GetDeviceGeometry(Handle, c, h, s);

			}

		};

	}

}