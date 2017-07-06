
#include "base.h"
#include <sys/timeb.h>

namespace ptfs {

	uint32_t crc32_table[256];

	void GenerateGuid(guid_t Guid) {
#ifdef _WIN32
		CoCreateGuid((GUID*)Guid);
#endif // _WIN32
	}

}