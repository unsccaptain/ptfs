#pragma once

#include <config.h>
#include <base.h>
#include <partition_device.h>

namespace ptfs {

	namespace filesystem {

		class NullFileSystem;

		class FileSystem:public Base {

		protected:
			
			char		FsName[16];
			uint8_t		Type;

			device::PartitionDevice* Dev;

		public:

			FileSystem() {}
			FileSystem(device::PartitionDevice* Device) 
				:Dev(Device) {}

			void SetPartitionDevice(device::PartitionDevice* Device) {
				if (Dev == NULL)
					Dev = Device;
				else
					Status = ERR_INVALID_OP;
			}

			uint8_t	GetType() { return Type; }

			virtual bool MakeFs() = 0;

			virtual bool Sync() = 0;

		};

		FileSystem* FsObjGenerator(device::PartitionDevice* Device, uint8_t Likely);

	}

}