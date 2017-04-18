
#include "base.h"
#include <sys/timeb.h>

namespace ptfs {

	uint32_t crc32_table[256];

	void GenerateGuid(guid_t Guid) {

		struct _timeb timebuffer;
		uint32_t s;

		_ftime_s(&timebuffer);
		srand(timebuffer.millitm);

		Guid->Data1 = rand();
		s = rand();
		memcpy(&Guid->Data2, &s, 4);

	}

	int make_crc32_table()
	{

		uint32_t c;
		int i = 0;
		int bit = 0;

		for (i = 0; i < 256; i++)
		{
			c = (uint32_t)i;

			for (bit = 0; bit < 8; bit++)
			{
				if (c & 1)
				{
					c = (c >> 1) ^ (0xEDB88320);
				}
				else
				{
					c = c >> 1;
				}

			}
			crc32_table[i] = c;
		}

		return 1;
	}

	uint32_t make_crc(uint32_t crc, unsigned char *string, uint32_t size)
	{

		while (size--)
			crc = (crc >> 8) ^ (crc32_table[(crc ^ *string++) & 0xff]);

		return crc;

	}

}