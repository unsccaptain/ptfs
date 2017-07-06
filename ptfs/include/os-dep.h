#pragma once

#include "config.h"
#include "base.h"

namespace ptfs {

	namespace device {
	
		device_t	open_disk_device(char* PathName);

		ssize_t		read_device_sector(device_t Device, sec_off_t Offset, uint8_t* Buffer, sec_off_t Size);

		ssize_t		write_device_sector(device_t Device, sec_off_t Offset, uint8_t* Buffer, sec_off_t Size);

		sec_off_t	get_device_size(device_t Device);

		bool		query_adapter(device_t Device, adapter_parameter* p);

		bool		query_align(device_t Device, alignment_parameter* p);

		bool		query_geometry(device_t Device, uint16_t* c, uint8_t* h, uint8_t* s);

		bool		lock_disk_device(device_t Device);

		bool		unlock_disk_device(device_t Device);



	}

}