#pragma once

#include "config.h"
#include "base.h"
#include "os-dep.h"
#include "err.h"

namespace ptfs {

	namespace device {

		enum device_type {
			PhysicalDisk,
			LogicalDisk,
			Partition,
			File
		};

		class Device :public Base {
			
		protected:

			device_type			dev_type;
			sec_off_t			dev_size = 0;

		public:

			Device() {}

			Device(device_type dt):dev_type(dt){}

			//��ȡ�豸����
			device_type			GetDeviceType() { return dev_type; }

			//�������豸
			virtual ssize_t		ReadDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) = 0;

			//����д�豸
			virtual ssize_t		WriteDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) = 0;

			//��ȡ�豸��С
			virtual sec_off_t	GetDeviceSize() = 0;

			//��ȡ�豸��ʼLBA
			virtual sec_off_t	GetDeviceStartLBA() = 0;

			//��ȡ���Ŷ����С
			virtual	sec_off_t	GetOptimumAlign() = 0;

			//��ȡĬ�϶����С
			virtual sec_off_t	GetDefaultAlign() = 0;

			//virtual void Commit

		};

	}

}