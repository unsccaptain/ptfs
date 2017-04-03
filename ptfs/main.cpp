
#include <config.h>
#include <base.h>
#include <disk_device.h>

#include "label/label.h"
#include "label/msdos.h"
#include "fs/null.h"

using namespace ptfs;

void main()
{

	device::DiskDevice* DiskDev = new device::DiskDevice("\\\\.\\physicaldrive1");
	label::Msdos::Probe(DiskDev);

	label::Msdos* Label = new label::Msdos(DiskDev);
	Label->MakeLabel();

	filesystem::NullFileSystem* nfs = new filesystem::NullFileSystem();
	Label->MakePart(200 * 1024 * 1024 / 512, 0, nfs);

	Label->Sync();

}