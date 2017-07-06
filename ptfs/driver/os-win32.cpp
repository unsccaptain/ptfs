#pragma once

#include <config.h>
#include <base.h>

#include <windows.h>
#include <os-dep.h>

#include <ntdddisk.h>
#include <ntddscsi.h>
#define _NTSCSI_USER_MODE_
#include "scsi.h"

namespace ptfs {

	namespace device {

		device_t open_disk_device(char* PathName) {

			return CreateFile(PathName,
				GENERIC_ALL,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				NULL,
				NULL
			);

		}

		ssize_t read_device_sector(device_t Device, sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {

			ssize_t size;
			LONG High = 0;
			SetFilePointer(Device, Offset << 9, &High, FILE_BEGIN);
			printf("read device error,error code:%d\n", GetLastError());
			if (ReadFile(Device, Buffer, Size << 9, (LPDWORD)&size, NULL)) {
				return size >> 9;
			}
			else {
				printf("read device error,error code:%d\n", GetLastError());
			}
			
			return	0;
		}

		ssize_t write_device_sector(device_t Device, sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) {

			ssize_t size;
			LONG High = 0;
			SetFilePointer(Device, Offset << 9, &High, FILE_BEGIN);

			if (WriteFile(Device, Buffer, Size << 9, (LPDWORD)&size, NULL)) {
				return size >> 9;
			}
			else {
				printf("write device error,error code:%d\n", GetLastError());
			}

			return 0;
		}

		sec_off_t get_file_size(device_t Device) {
			LARGE_INTEGER s;
			GetFileSizeEx(Device, &s);
			return s.QuadPart;
		}

		sec_off_t get_device_size(device_t Device) {

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

		bool query_geometry(device_t Device, uint16_t* c, uint8_t* h, uint8_t* s) {

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
			return bResult;

		}

		bool query_adapter(device_t Device, adapter_parameter* p) {

			STORAGE_PROPERTY_QUERY query;
			DWORD ret_size;
			DWORD length = 4096;
			PUCHAR buf = (PUCHAR)malloc(length);

			query.PropertyId = StorageAdapterProperty;
			query.QueryType = PropertyStandardQuery;

			//查询adapter信息
			BOOL ret = DeviceIoControl(Device,
				IOCTL_STORAGE_QUERY_PROPERTY,
				&query,
				sizeof(STORAGE_PROPERTY_QUERY),
				buf,
				length,
				&ret_size,
				NULL);

			if (!ret) {
				free(buf);
				return false;
			}

			PSTORAGE_ADAPTER_DESCRIPTOR adapter_desc = (PSTORAGE_ADAPTER_DESCRIPTOR)buf;
			p->PhysicalPageSize = adapter_desc->MaximumPhysicalPages;
			p->TransferLength = adapter_desc->MaximumTransferLength;
			free(buf);

			return true;

		}

		bool query_align(device_t Device, alignment_parameter* p) {

			STORAGE_PROPERTY_QUERY query;
			DWORD ret_size;
			DWORD length = 4096;
			PUCHAR buf = (PUCHAR)malloc(length);

			query.PropertyId = StorageAccessAlignmentProperty;
			query.QueryType = PropertyStandardQuery;

			//查询adapter信息
			BOOL ret = DeviceIoControl(Device,
				IOCTL_STORAGE_QUERY_PROPERTY,
				&query,
				sizeof(STORAGE_PROPERTY_QUERY),
				buf,
				length,
				&ret_size,
				NULL);

			if (!ret) {
				free(buf);
				return false;
			}

			PSTORAGE_ACCESS_ALIGNMENT_DESCRIPTOR align_desc
				= (PSTORAGE_ACCESS_ALIGNMENT_DESCRIPTOR)buf;
			p->PhysicalSectorInByte = align_desc->BytesPerPhysicalSector;
			p->LogicalSectorInByte = align_desc->BytesPerLogicalSector;
			p->CacheLineInByte = align_desc->BytesPerCacheLine;
			free(buf);

			return true;

		}

		//锁定磁盘
		bool lock_disk_device(device_t Device) {

			CHAR VolName[256];
			DWORD dwBytesReturned;
			DWORD DiskNumber;
			BOOLEAN bSucc, bInDisk = false, bReturn = true;
			HANDLE VolHandle, FindHandle;
			STORAGE_DEVICE_NUMBER Sdn;
			PVOLUME_DISK_EXTENTS Vde = (PVOLUME_DISK_EXTENTS)malloc(4096);

			/* 先获取磁盘号 */
			bSucc = DeviceIoControl(Device,
				IOCTL_STORAGE_GET_DEVICE_NUMBER,
				NULL, 0,
				&Sdn, sizeof(STORAGE_DEVICE_NUMBER),
				&dwBytesReturned,
				NULL
			);
			if (!bSucc) {
				bReturn = false;
				goto EXIT3;
			}

			DiskNumber = Sdn.DeviceNumber;

			/* 开始遍历卷 */
			FindHandle = FindFirstVolume(VolName, 256 * 2);
			if (FindHandle == NULL) {
				bReturn = false;
				goto EXIT3;
			}

			while (GetLastError() != ERROR_NO_MORE_FILES) {

				/* 打开遍历的卷 */
				VolName[strlen(VolName) - 1] = 0;
				HANDLE VolHandle = CreateFile(VolName,
					GENERIC_ALL,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					NULL,
					NULL
				);
				if (VolHandle == NULL)
					goto EXIT1;

				/* 检测卷是否在该磁盘上 */
				bSucc = DeviceIoControl(VolHandle, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, Vde, 4096, &dwBytesReturned, NULL);
				if (!bSucc)
					goto EXIT1;
				for (DWORD i = 0;i < Vde->NumberOfDiskExtents;i++) {
					if (Vde->Extents[i].DiskNumber == DiskNumber)
						bInDisk = TRUE;
				}
				if (!bInDisk) {
					CloseHandle(VolHandle);
					continue;
				}

				/* 如果卷在磁盘上，锁定并解除挂载 */
				bSucc = DeviceIoControl(VolHandle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwBytesReturned, NULL);
				if (!bSucc)
					goto EXIT1;
				DeviceIoControl(VolHandle, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &dwBytesReturned, NULL);

			EXIT1:
				bInDisk = FALSE;
				FindNextVolume(FindHandle, VolName, 256 * 2);
			}

		EXIT3:
			free(Vde);
			return bReturn;

		}

		bool unlock_disk_device(device_t Device) {

			return true;

		}

	}

}