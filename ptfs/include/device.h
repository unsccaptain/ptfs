#pragma once

#include "config.h"
#include "base.h"
#include "io_if.h"
#include "err.h"

namespace ptfs {

	namespace device {

		enum device_type {

			Disk,
			Partition,
			File

		};

		class Device :public Base {
			
		protected:

			device_type		Type;
			sec_off_t		Size = 0;

		public:

			Device() {}

			virtual ssize_t		ReadDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) = 0;

			virtual ssize_t		WriteDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) = 0;

			virtual sec_off_t	GetSize() = 0;

			virtual	sec_off_t	GetAlignInSector() = 0;

		};

	}

}