
/*
����CRC32�Ǵ�UEFIԴ���г����ģ�Ӧ���Ǳ�׼�㷨
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

