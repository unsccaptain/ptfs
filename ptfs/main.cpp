
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

	//�ڴ����ϴ���LABEL
	label::Gpt* Label = new label::Gpt(DiskDev);
	Label->MakeLabel();

	//��Label�д���Partition
	device::PartitionDevice* Part1 = Label->MakePart(200 * 1024 * 1024 / 512, 0);

	//��Partition�����Volume
	volume::NormalVolume* Volume = new volume::NormalVolume(Part1);
	Volume->SetVolumeName("asdf1234");

	//��Volume����ΪFat����
	filesystem::Fat* FatSystem = new filesystem::Fat(Volume);
	FatSystem->MakeFs(PARTITION_FAT32, 4096);

	//ͬ���ļ�ϵͳ�ͷ�����
	FatSystem->Sync();
	FatSystem->SyncPartitionTable(Label);

	Label->Sync();

	DiskDev->Unlock();

	//fat->ReleaseObject();
	//DiskDev->ReleaseObject();

}