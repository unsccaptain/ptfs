#pragma once

#include <config.h>
#include <base.h>
#include <device.h>
#include <disk_device.h>
#include <os-dep.h>
#include <err.h>

/*
这里的PartitionDevice仅仅表示是设备中的一个区间
它既可以是整个磁盘，也可以是一段空白空间，也可以是卷或分区
而后面Label中的Partition则表示在分区表中有意义的分区
*/

namespace ptfs {

	namespace label {

		class Label;

	}

	namespace volume {

		class Volume;

	}

	namespace filesystem {

		class FileSystem;

	}

	namespace device {	

		class PartitionDevice :public Device {

		private:

			//分区所在的block device
			Device*			blk_dev = NULL;

			//分区起始地址
			sec_off_t		dev_start_sec = 0;
			
			//分区结束地址
			sec_off_t		dev_end_sec = 0;
			
			//分区号
			uint16_t		part_number;
			
			//分区表项地址
			void*			part_entry;

			//由于分区表的特性，下面两项和文件系统相关的项无法合理的解耦
			//如果有可能，还是应该避免下面两项的存在

			//分区名称
			char*			part_name;

			//分区类型
			uint16_t		part_type;

			//在这个分区上的卷
			volume::Volume* upper_vol;

		public:

			PartitionDevice() {
			}

			PartitionDevice(Device* Device, sec_off_t Start, sec_off_t End)
				:blk_dev(Device), dev_start_sec(Start), dev_end_sec(End), Device(Partition)
			{
				dev_size = dev_end_sec - dev_start_sec;
				part_number = -1;
			}

			PartitionDevice(Device* Device, sec_off_t Start, sec_off_t End, uint16_t PartNumber)
				:blk_dev(Device), dev_start_sec(Start), dev_end_sec(End), Device(Partition),
				part_number(PartNumber) 
			{
				dev_size = dev_end_sec - dev_start_sec;
			}

			PartitionDevice(Device* Device, sec_off_t Start, sec_off_t End, uint16_t PartNumber, void* Entry)
				:blk_dev(Device), dev_start_sec(Start), dev_end_sec(End), Device(Partition),
				part_number(PartNumber), part_entry(Entry) 
			{
				dev_size = dev_end_sec - dev_start_sec;
			}

			//扇区读分区设备
			virtual ssize_t ReadDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//扇区写分区设备
			virtual ssize_t WriteDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//获取设备大小
			virtual sec_off_t GetDeviceSize() override;

			//获取设备起始LBA
			virtual sec_off_t GetDeviceStartLBA() override;

			//获取最佳对齐大小
			virtual sec_off_t GetOptimumAlign() override;

			//获取默认对齐大小
			virtual sec_off_t GetDefaultAlign() override;

			//GET/SET属性

			uint16_t GetPartitionNumber() {return part_number;}
			void SetPartitionNumber(uint16_t pn) {part_number = pn;}

			volume::Volume* GetPartitionUpperVolume() { return upper_vol; }
			void SetPartitionUpperVolume(volume::Volume* vol) { upper_vol = vol; }

			Device* GetParentDevice() {
				return blk_dev;
			}
			
			//将分区提交至分区表
			void SyncPartitionTable(label::Label* Label, uint16_t PartType);

		};

		int __cdecl PartitionComp(void const* a, void const* b);

	}

}