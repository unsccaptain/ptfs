#pragma once

#include <config.h>
#include <base.h>
#include <disk_device.h>
#include <partition.h>
#include <fs/filesystem.h>
#include <vector>

#include "label.h"

namespace ptfs {

	namespace label {

		class Gpt :public Label {

		private:

			DosRawTable Mbr;

			std::vector<Partition>PartitionTable;

		public:

			Gpt(device::Device* dev) :Label("msdos", dev) {}

		}

	}

}