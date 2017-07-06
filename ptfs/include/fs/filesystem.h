#pragma once

#include <config.h>
#include <base.h>
#include <volume/volume.h>
#include <label/label.h>

namespace ptfs {

	namespace filesystem {

		class FileSystem:public Base {

		protected:
			
			char			FsName[16];
			uint32_t		Type;
			char			LabelName[256];

			uint16_t		ClusterSizeInByte = 4096;
			uint16_t		LogicalSizeInByte = 512;

			volume::Volume*	Volume;

		public:

			FileSystem() {}

			FileSystem(volume::Volume* Device)
				:Volume(Device) {}

			FileSystem(uint16_t ClusterSize, uint16_t LogicalSize)
				:ClusterSizeInByte(ClusterSize), LogicalSizeInByte(LogicalSize) {}

			FileSystem(volume::Volume* Device, uint16_t ClusterSize, uint16_t LogicalSize)
				:Volume(Device), ClusterSizeInByte(ClusterSize), LogicalSizeInByte(LogicalSize) {}

			void SetDevice(volume::Volume* Device) {
				if (Volume == NULL)
					Volume = Device;
				else
					Status = ERR_INVALID_OP;
			}

			uint8_t	GetType() { return Type; }

			uint16_t GetClusterSizeInByte() { return ClusterSizeInByte; }
			void SetClusterSizeInByte(uint16_t n) { ClusterSizeInByte = n; }
			uint16_t GetLogicalSizeInByte() { return LogicalSizeInByte; }
			void SetLogicalSizeInByte(uint16_t n) { LogicalSizeInByte = n; }

			void SetFsLabel(char* Label) { if (Label)memcpy(LabelName, Label, 11); }

			virtual bool MakeFs(uint32_t SpecificType, uint32_t ClusterSize) = 0;

			virtual bool SyncPartitionTable(label::Label* Label) = 0;

			virtual bool Sync() = 0;

		//public:

			//virtual file::File* OpenFile(wchar* PathName) = 0;

		};

		FileSystem* FsObjGenerator(device::Device* Device, uint8_t Likely);

	}

}