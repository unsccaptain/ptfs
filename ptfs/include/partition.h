#pragma once

#include "config.h"
#include "base.h"

#include <fs/filesystem.h>
#include <label/label.h>

namespace ptfs {

	namespace label {

		class Partition {

		public:

			Partition(sec_off_t start_sector, sec_off_t total_sector, filesystem::FileSystem* fs, uint8_t part_number)
				:StartSector(start_sector), TotalSectors(total_sector), Fs(fs), PartNumber(part_number) {}

			uint8_t					PartNumber;

			sec_off_t				StartSector;
			sec_off_t				TotalSectors;

			filesystem::FileSystem*	Fs;

		};

	}

}