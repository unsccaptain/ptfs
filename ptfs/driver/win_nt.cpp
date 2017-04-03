#pragma once

#include <config.h>
#include <base.h>

#include <windows.h>
#include <io_if.h>

namespace ptfs {

	namespace device {

		device_t OpenDevice(char* PathName) {

			return CreateFile(PathName,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				NULL,
				NULL
			);

		}

		ssize_t ReadDevice(device_t Device, sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {

			ssize_t size;

			LONG High = 0;
			SetFilePointer(Device, Offset << 9, &High, FILE_BEGIN);

			if (ReadFile(Device,
				Buffer,
				Size << 9,
				(LPDWORD)&size,
				NULL
			)) {

				return size >> 9;

			}

			return 0;

		}

		ssize_t WriteDevice(device_t Device, sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {

			ssize_t size;

			LONG High = 0;
			SetFilePointer(Device, Offset << 9, &High, FILE_BEGIN);

			if (WriteFile(Device,
				Buffer,
				Size << 9,
				(LPDWORD)&size,
				NULL
			)) {

				return size >> 9;

			}

			return 0;

		}

		sec_off_t GetDeviceSize(device_t Device) {

			sec_off_t size;

			PDISK_GEOMETRY_EX gex = (PDISK_GEOMETRY_EX)malloc(4096);
			DWORD ret;
			BOOL bResult = DeviceIoControl(Device,     // device to be queried
				IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,     // operation to perform
				NULL, 0,
				gex, 4096,     // output buffer
				&ret,                 // # bytes returned
				(LPOVERLAPPED)NULL);  // synchronous I/O

			if (!bResult) {
				size = 0;
			}
			else {
				size = gex->DiskSize.QuadPart >> 9;
			}

			free(gex);
			return size;

		}

		void GetDeviceGeometry(device_t Device,uint16_t* c, uint8_t* h, uint8_t* s) {

			sec_off_t size;

			PDISK_GEOMETRY_EX gex = (PDISK_GEOMETRY_EX)malloc(4096);
			DWORD ret;
			BOOL bResult = DeviceIoControl(Device,     // device to be queried
				IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,     // operation to perform
				NULL, 0,
				gex, 4096,     // output buffer
				&ret,                 // # bytes returned
				(LPOVERLAPPED)NULL);  // synchronous I/O

			if (!bResult) {
				*c = 0;
				*h = 0;
				*s = 0;
			}
			else {
				*c = gex->Geometry.Cylinders.QuadPart;
				*h = gex->Geometry.TracksPerCylinder;
				*s = gex->Geometry.SectorsPerTrack;
			}

			free(gex);
			return;

		}

	}

}