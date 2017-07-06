
#pragma once

#include <config.h>
#include <base.h>

//#include <device.h>
//#include <label/label.h>
#include <fs/filesystem.h>
//#include <volume/volume.h>

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
			//NullFileSystem(volume::Volume* Volume)
			//	:FileSystem(Volume) {
			//	strcpy(FsName, "raw");
			//	Type = PARTITION_FAT16;
			//}

			virtual bool MakeFs(uint32_t SpecificType, uint32_t ClusterSize) override {
				return true;
			}

			virtual bool SyncPartitionTable(label::Label* Label) override {
				return true;
			}

			virtual bool Sync() override {
				return true;
			}

		};

	}

}