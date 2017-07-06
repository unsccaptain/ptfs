
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

			/* 利用时间生成一个随机数作为签名 */
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

			/* 扇区对齐 */
			sec_off_t sec_align = blk_dev->GetOptimumAlign();

			/* 对齐后的理想大小 */
			sec_off_t real_size = ALIGN_DOWN_BY(Size, sec_align);

			/* 可以接受的容量损失 */
			sec_off_t size_loss = real_size * 0.1;

			/* 真实起始地址 */
			sec_off_t real_start = 0;

			/* 可用大小,可用大小满足的情况下再看对齐后的大小能否满足可接受容量损失 */
			sec_off_t usable_size = 0;

			/* 分区号 */
			uint8_t part_number;

			if (part_array.size() == 4) {
				Status = ERR_NOMORE_ENRTY;
				return false;
			}

			if (Start >= blk_dev->GetDeviceSize() || Size >= blk_dev->GetDeviceSize()) {
				Status = ERR_INVALID_POS;
				return false;
			}

			/* 如果未指定起始地址或者没有分区 */

			if ((part_array.size() == 0) || (Start == 0)) {

				/* 指定了起始地址但一开始没有分区，就以起始地址为准 */
				if (Start) {
					real_start = ALIGN_UP_BY(Start, sec_align);

					if (real_start < sec_align * 256)
						real_start = sec_align * 256;

					part_number = 1;
				}
				/* 开始没有分区并且未指定起始地址,那就直接以物理扇区大小的256倍为起始地址 */
				else if (part_array.size() == 0) {

					real_start = sec_align;
					part_number = 1;

				}
				/* 有分区但没有指定起始地址，说明加在分区表最后，以最后一个扇区结束地址对齐后为起始地址 */
				else {

					device::PartitionDevice* last_part = part_array[part_array.size() - 1];
					real_start = ALIGN_UP_BY(last_part->GetDeviceStartLBA() + last_part->GetDeviceSize(), sec_align);
					part_number = part_array.size() + 1;

				}

				/* 可用大小取设备大小减起始地址 */
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

					/* 如果起始地址和其他分区冲突，直接返回失败 */
					if ((Start >= boundary_down) &&
						(Start <= boundary_up)) {
						Status = ERR_INVALID_POS;
						return false;
					}

				}

				/* 最后一个分区后面 */
				if (real_start == 0) {

					real_start = ALIGN_UP_BY(Start, sec_align);
					usable_size = blk_dev->GetDeviceSize() - real_start;
					part_number = part_array.size() + 1;

				}

			}

			/* 收集了起始地址和可用大小后，开始计算具体边界 */
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

			/* 计算起始和结束CHS,其实没多少必要 */
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

		//设置分区类型，忽略分区名
		void Msdos::SetPartitionType(uint8_t PartNumber, uint16_t PartType) {
			mbr.partitions[PartNumber - 1].Type = PartType;
		}

		//将磁盘格式同步到硬盘
		bool Msdos::Sync() {
			if (blk_dev->WriteDeviceSector(0, (uint8_t*)&mbr, 1) != 1) {
				Status = ERR_INVALID_DEV;
				return false;
			}
			return true;
		}

	}

}