
#pragma once

#include <config.h>
#include <base.h>

#include <device.h>
#include <label/label.h>
#include <fs/filesystem.h>

namespace ptfs {

	namespace filesystem {

		class NullFileSystem :public FileSystem {

		private:

		public:

			NullFileSystem()
				:FileSystem() {
				strcpy(FsName, "raw");
				Type = PARTITION_FAT16;
			}
			NullFileSystem(device::PartitionDevice* Device)
				:FileSystem(Device) {
				strcpy(FsName, "raw");
				Type = PARTITION_FAT16;
			}

			bool MakeFs() { return true; }

			bool Sync() { return true; }

		};

	}

}