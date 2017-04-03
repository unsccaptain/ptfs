#pragma once

#include "config.h"
#include "base.h"

namespace ptfs {

	namespace device {
	
		device_t	OpenDevice(char* PathName);

		ssize_t		ReadDevice(device_t Device, sec_off_t Offset, uint8_t* Buffer, sec_off_t Size);

		ssize_t		WriteDevice(device_t Device, sec_off_t Offset, uint8_t* Buffer, sec_off_t Size);

		sec_off_t	GetDeviceSize(device_t Device);

		void		GetDeviceGeometry(device_t Device, uint16_t* c, uint8_t* h, uint8_t* s);

	}

}