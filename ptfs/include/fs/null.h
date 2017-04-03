
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
				:FileSystem() {}
			NullFileSystem(device::PartitionDevice* Device)
				:FileSystem(Device) {
				strcpy(FsName, "null");
				Type = PARTITION_EMPTY;
			}

			bool MakeFs() { return true; }

			bool Sync() { return true; }

		};

	}

}