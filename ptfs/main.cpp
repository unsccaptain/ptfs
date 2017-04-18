
#include <config.h>
#include <base.h>
#include <disk_device.h>

#include "label/label.h"
#include "label/msdos.h"
#include "fs/null.h"
#include "fs/fat.h"

using namespace ptfs;

void main()
{

	device::DiskDevice* DiskDev = new device::DiskDevice("\\\\.\\physicaldrive1");
	label::Msdos::Probe(DiskDev);

	label::Msdos* Label = new label::Msdos(DiskDev);
	Label->MakeLabel();

	filesystem::Fat* fat = new filesystem::Fat();
	fat->SetFsLabel("sdfdssdd");
	fat->SetClusterSizeInByte(4096);
	fat->SetLogicalSizeInByte(512);
	Label->MakePart(200 * 1024 * 1024 / 512, 0, fat);

	DiskDev->Lock();
	Label->Sync();
	DiskDev->Unlock();

	fat->ReleaseObject();
	DiskDev->ReleaseObject();

}