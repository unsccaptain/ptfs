
#include <config.h>
#include <base.h>
#include <disk_device.h>

#include "label/label.h"
#include "label/gpt.h"
#include "label/msdos.h"
#include "fs/null.h"
#include "fs/fat.h"
#include "volume/normal.h"

using namespace ptfs;

void main()
{

	device::DiskDevice* DiskDev = new device::DiskDevice("\\\\.\\physicaldrive2");
	//label::Msdos::Probe(DiskDev);

	DiskDev->Lock();

	//在磁盘上创建LABEL
	label::Gpt* Label = new label::Gpt(DiskDev);
	Label->MakeLabel();

	//在Label中创建Partition
	device::PartitionDevice* Part1 = Label->MakePart(200 * 1024 * 1024 / 512, 0);

	//将Partition分配给Volume
	volume::NormalVolume* Volume = new volume::NormalVolume(Part1);
	Volume->SetVolumeName("asdf1234");

	//将Volume分配为Fat分区
	filesystem::Fat* FatSystem = new filesystem::Fat(Volume);
	FatSystem->MakeFs(PARTITION_FAT32, 4096);

	//同步文件系统和分区表
	FatSystem->Sync();
	FatSystem->SyncPartitionTable(Label);

	Label->Sync();

	DiskDev->Unlock();

	//fat->ReleaseObject();
	//DiskDev->ReleaseObject();

}