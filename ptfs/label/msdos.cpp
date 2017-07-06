
#include <config.h>
#include <base.h>

#include <device.h>
#include <sys/timeb.h>

#include <label/msdos.h>

namespace ptfs {

	namespace label {

		const DosRawPartition* Msdos::GetPartitionEntry(uint8_t Index) {
			return (const DosRawPartition*)&mbr.partitions[Index];
		}

		device::PartitionDevice* Msdos::GetPartitionDevice(uint8_t PartNumber) {
			for (size_t i = 0;i < part_array.size();i++) {
				if (part_array[i]->GetPartitionNumber() == PartNumber)
					return part_array[i];
			}
			return NULL;
		}

		bool Msdos::MakeLabel() {

			struct _timeb timebuffer;
			
			DosRawTable Mbr;
			memset(&Mbr, 0, sizeof(Mbr));

			memcpy(Mbr.boot_code, MBR_BOOT_CODE, sizeof(MBR_BOOT_CODE));
			Mbr.magic = MSDOS_MAGIC;

			/* ����ʱ������һ���������Ϊǩ�� */
			_ftime_s(&timebuffer);
			srand(timebuffer.millitm);
			
			Mbr.mbr_signature = rand();

			memcpy(&this->mbr, &Mbr, sizeof(Mbr));

			for (size_t i = 0;i < part_array.size();i++) {
				delete part_array[i];
			}
			part_array.clear();

			return true;

		}

		device::PartitionDevice* Msdos::MakePart(sec_off_t Size, sec_off_t Start) {

			/* �������� */
			sec_off_t sec_align = blk_dev->GetOptimumAlign();

			/* �����������С */
			sec_off_t real_size = ALIGN_DOWN_BY(Size, sec_align);

			/* ���Խ��ܵ�������ʧ */
			sec_off_t size_loss = real_size * 0.1;

			/* ��ʵ��ʼ��ַ */
			sec_off_t real_start = 0;

			/* ���ô�С,���ô�С�����������ٿ������Ĵ�С�ܷ�����ɽ���������ʧ */
			sec_off_t usable_size = 0;

			/* ������ */
			uint8_t part_number;

			if (part_array.size() == 4) {
				Status = ERR_NOMORE_ENRTY;
				return false;
			}

			if (Start >= blk_dev->GetDeviceSize() || Size >= blk_dev->GetDeviceSize()) {
				Status = ERR_INVALID_POS;
				return false;
			}

			/* ���δָ����ʼ��ַ����û�з��� */

			if ((part_array.size() == 0) || (Start == 0)) {

				/* ָ������ʼ��ַ��һ��ʼû�з�����������ʼ��ַΪ׼ */
				if (Start) {
					real_start = ALIGN_UP_BY(Start, sec_align);

					if (real_start < sec_align * 256)
						real_start = sec_align * 256;

					part_number = 1;
				}
				/* ��ʼû�з�������δָ����ʼ��ַ,�Ǿ�ֱ��������������С��256��Ϊ��ʼ��ַ */
				else if (part_array.size() == 0) {

					real_start = sec_align;
					part_number = 1;

				}
				/* �з�����û��ָ����ʼ��ַ��˵�����ڷ�������������һ������������ַ�����Ϊ��ʼ��ַ */
				else {

					device::PartitionDevice* last_part = part_array[part_array.size() - 1];
					real_start = ALIGN_UP_BY(last_part->GetDeviceStartLBA() + last_part->GetDeviceSize(), sec_align);
					part_number = part_array.size() + 1;

				}

				/* ���ô�Сȡ�豸��С����ʼ��ַ */
				usable_size = blk_dev->GetDeviceSize() - real_start;

			}
			else {

				sec_off_t boundary_down;
				sec_off_t boundary_up = 0;

				for (size_t i = 0;i < part_array.size();i++) {

					boundary_down = ALIGN_DOWN_BY(part_array[i]->GetDeviceSize(), sec_align);

					if (Start > boundary_up&&Start < boundary_down) {

						real_start = ALIGN_UP_BY(Start, sec_align);
						usable_size = boundary_down - boundary_up;
						part_number = i + 2;
						break;

					}

					boundary_up = ALIGN_UP_BY(boundary_down + part_array[i]->GetDeviceSize(), sec_align);

					/* �����ʼ��ַ������������ͻ��ֱ�ӷ���ʧ�� */
					if ((Start >= boundary_down) &&
						(Start <= boundary_up)) {
						Status = ERR_INVALID_POS;
						return false;
					}

				}

				/* ���һ���������� */
				if (real_start == 0) {

					real_start = ALIGN_UP_BY(Start, sec_align);
					usable_size = blk_dev->GetDeviceSize() - real_start;
					part_number = part_array.size() + 1;

				}

			}

			/* �ռ�����ʼ��ַ�Ϳ��ô�С�󣬿�ʼ�������߽� */
			if (real_start + real_size > usable_size) {

				sec_off_t overflow = real_start + real_size - usable_size;

				if (overflow > size_loss) {

					Status = ERR_NO_SUITABLE;
					return false;

				}
				else {

					real_size -= ALIGN_UP_BY(overflow, sec_align);

				}

			}

			device::PartitionDevice* Part =
				new device::PartitionDevice(blk_dev, real_start, real_start + real_size, part_number);
			part_array.push_back(Part);

			DosRawPartition* PtEntry = &mbr.partitions[part_number - 1];

			PtEntry->BootIndicator = 0;

			/* ������ʼ�ͽ���CHS,��ʵû���ٱ�Ҫ */
			uint16_t c = real_start / (255 * 63) ;
			uint8_t rc = c & 0xFF;
			uint8_t h = (real_start / 63) % 255 +0;
			uint8_t s = ((real_start % 63) + 0) | ((c >> 8) << 6) + 1;
			PtEntry->CHS_Start = { h ,s ,rc };

			c = (real_start + real_size) / (255 * 63);
			rc = c & 0xFF;
			h = ((real_start + real_size) / 63) % 255;
			s = ((real_start % 63) + 0) | ((c >> 8) << 6) + 1;
			PtEntry->CHS_END = { h ,s,rc };

			PtEntry->Length = real_size;
			PtEntry->Start = real_start;

			return Part;

		}

		//���÷������ͣ����Է�����
		void Msdos::SetPartitionType(uint8_t PartNumber, uint16_t PartType) {
			mbr.partitions[PartNumber - 1].Type = PartType;
		}

		//�����̸�ʽͬ����Ӳ��
		bool Msdos::Sync() {
			if (blk_dev->WriteDeviceSector(0, (uint8_t*)&mbr, 1) != 1) {
				Status = ERR_INVALID_DEV;
				return false;
			}
			return true;
		}

	}

}