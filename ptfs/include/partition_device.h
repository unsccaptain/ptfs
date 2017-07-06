#pragma once

#include <config.h>
#include <base.h>
#include <device.h>
#include <disk_device.h>
#include <os-dep.h>
#include <err.h>

/*
�����PartitionDevice������ʾ���豸�е�һ������
���ȿ������������̣�Ҳ������һ�οհ׿ռ䣬Ҳ�����Ǿ�����
������Label�е�Partition���ʾ�ڷ�������������ķ���
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

			//�������ڵ�block device
			Device*			blk_dev = NULL;

			//������ʼ��ַ
			sec_off_t		dev_start_sec = 0;
			
			//����������ַ
			sec_off_t		dev_end_sec = 0;
			
			//������
			uint16_t		part_number;
			
			//���������ַ
			void*			part_entry;

			//���ڷ���������ԣ�����������ļ�ϵͳ��ص����޷�����Ľ���
			//����п��ܣ�����Ӧ�ñ�����������Ĵ���

			//��������
			char*			part_name;

			//��������
			uint16_t		part_type;

			//����������ϵľ�
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

			//�����������豸
			virtual ssize_t ReadDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//����д�����豸
			virtual ssize_t WriteDeviceSector(sec_off_t Offset, uint8_t* Buffer, sec_off_t Size) override;

			//��ȡ�豸��С
			virtual sec_off_t GetDeviceSize() override;

			//��ȡ�豸��ʼLBA
			virtual sec_off_t GetDeviceStartLBA() override;

			//��ȡ��Ѷ����С
			virtual sec_off_t GetOptimumAlign() override;

			//��ȡĬ�϶����С
			virtual sec_off_t GetDefaultAlign() override;

			//GET/SET����

			uint16_t GetPartitionNumber() {return part_number;}
			void SetPartitionNumber(uint16_t pn) {part_number = pn;}

			volume::Volume* GetPartitionUpperVolume() { return upper_vol; }
			void SetPartitionUpperVolume(volume::Volume* vol) { upper_vol = vol; }

			Device* GetParentDevice() {
				return blk_dev;
			}
			
			//�������ύ��������
			void SyncPartitionTable(label::Label* Label, uint16_t PartType);

		};

		int __cdecl PartitionComp(void const* a, void const* b);

	}

}