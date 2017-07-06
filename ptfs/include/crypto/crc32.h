
/*
计算CRC32是从UEFI源码中抄来的，应该是标准算法
*/

#pragma once

#include <base.h>

uint32_t
RuntimeDriverCalculateCrc32(
	IN  unsigned char	*Data,
	IN  uintptr_t		DataSize
);

void
RuntimeDriverInitializeCrc32Table(
	void
);

