#pragma once

#include <config.h>
#include <base.h>
#include <disk_device.h>
#include <partition.h>
#include <fs/filesystem.h>
#include <vector>

#include "label.h"

namespace ptfs{

	namespace label {

		/* this MBR boot code is loaded into 0000:7c00 by the BIOS.  See mbr.s for
		* the source, and how to build it
		*/

		static const char MBR_BOOT_CODE[] = {
			0xfa, 0xb8, 0x00, 0x10, 0x8e, 0xd0, 0xbc, 0x00,
			0xb0, 0xb8, 0x00, 0x00, 0x8e, 0xd8, 0x8e, 0xc0,
			0xfb, 0xbe, 0x00, 0x7c, 0xbf, 0x00, 0x06, 0xb9,
			0x00, 0x02, 0xf3, 0xa4, 0xea, 0x21, 0x06, 0x00,
			0x00, 0xbe, 0xbe, 0x07, 0x38, 0x04, 0x75, 0x0b,
			0x83, 0xc6, 0x10, 0x81, 0xfe, 0xfe, 0x07, 0x75,
			0xf3, 0xeb, 0x16, 0xb4, 0x02, 0xb0, 0x01, 0xbb,
			0x00, 0x7c, 0xb2, 0x80, 0x8a, 0x74, 0x01, 0x8b,
			0x4c, 0x02, 0xcd, 0x13, 0xea, 0x00, 0x7c, 0x00,
			0x00, 0xeb, 0xfe
		};

		class Msdos :public Label{

		private:

			DosRawTable Mbr;

			std::vector<Partition> Partitions;

		public:

			Msdos(device::Device* dev):Label("msdos",dev){
			
				DosRawTable Mbr;
				if (Dev->ReadDeviceSector(0, (uint8_t*)&Mbr, 1) != 1)
					return;
			
				memcpy(&this->Mbr, &Mbr, sizeof(Mbr));

				for (uint8_t i = 0;i < 4;i++) {

					if (Mbr.partitions[i].Type != 0) {

						device::PartitionDevice* Device = new device::PartitionDevice(dev, 
							Mbr.partitions[i].Start, 
							Mbr.partitions[i].Length
						);

						filesystem::FileSystem* Fs = filesystem::FsObjGenerator(Device, Mbr.partitions[i].Type);

						Partition CurPart(Mbr.partitions[i].Start, Mbr.partitions[i].Length, Fs, i + 1);
						Partitions.push_back(CurPart);

					}

				}

			}

			static bool Probe(device::Device* Dev) {

				DosRawTable Mbr;
				if (Dev->ReadDeviceSector(0, (uint8_t*)&Mbr, 1) != 1)
					return false;

				if (Mbr.partitions[0].Type == PARTITION_GPT)
					return false;

				if (Mbr.mbr_signature != MSDOS_MAGIC)
					return false;

				return true;

			}

			const DosRawPartition* GetPartitionEntry(uint8_t Index);
			
			Partition* GetPartitionObj(uint8_t PartNumber);

			bool MakeLabel();

			bool MakePart(sec_off_t Size, sec_off_t Start, filesystem::FileSystem*Fs);

			void Sync();

			void ReleaseObject() {

				Dev->DEC_REF();

			}

		};

	}

}