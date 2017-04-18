#pragma once

#include <config.h>
#include <base.h>
#include <partition_device.h>

namespace ptfs {

	namespace filesystem {

		class NullFileSystem;

		class FileSystem:public Base {

		protected:
			
			char			FsName[16];
			uint32_t		Type;
			char			LabelName[8];

			uint16_t		ClusterSizeInByte;
			uint16_t		LogicalSizeInByte;

			device::PartitionDevice* Dev;

		public:

			FileSystem() {}

			FileSystem(device::PartitionDevice* Device) 
				:Dev(Device) {}

			FileSystem(uint16_t ClusterSize, uint16_t LogicalSize)
				:ClusterSizeInByte(ClusterSize), LogicalSizeInByte(LogicalSize) {}

			FileSystem(device::PartitionDevice* Device, uint16_t ClusterSize, uint16_t LogicalSize)
				:Dev(Device), ClusterSizeInByte(ClusterSize), LogicalSizeInByte(LogicalSize) {}

			void SetPartitionDevice(device::PartitionDevice* Device) {
				if (Dev == NULL)
					Dev = Device;
				else
					Status = ERR_INVALID_OP;
			}

			uint8_t	GetType() { return Type; }

			uint16_t GetClusterSizeInByte() { return ClusterSizeInByte; }

			void SetClusterSizeInByte(uint16_t n) { ClusterSizeInByte = n; }

			uint16_t GetLogicalSizeInByte() { return LogicalSizeInByte; }

			void SetLogicalSizeInByte(uint16_t n) { LogicalSizeInByte = n; }

			void SetFsLabel(char* Label) { if (Label)memcpy(LabelName, Label, 8); }

			virtual bool MakeFs(uint32_t Type, uint32_t ClusterSize) = 0;

			virtual bool Sync() = 0;

		};

		FileSystem* FsObjGenerator(device::PartitionDevice* Device, uint8_t Likely);

	}

}