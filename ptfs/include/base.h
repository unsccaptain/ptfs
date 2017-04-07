
#pragma once

#include "config.h"

#ifdef CFG_WIN

#include "windows.h"

typedef UINT8		uint8_t;
typedef UINT16		uint16_t;
typedef UINT32		uint32_t;
typedef UINT64		uint64_t;

typedef INT8		int8_t;
typedef INT16		int16_t;
typedef INT32		int32_t;
typedef INT64		int64_t;

typedef UINT32		err_t;

typedef UINT64		sec_off_t;
typedef UINT32		ssize_t;

typedef HANDLE		device_t;

#endif


/* µÿ÷∑∂‘∆Î∫Í */

#define ALIGN_DOWN_BY(length, alignment) \
    ((uintptr_t)(length) & ~(alignment - 1))

#define ALIGN_UP_BY(length, alignment) \
    (ALIGN_DOWN_BY(((uintptr_t)(length) + alignment - 1), alignment))

#define ALIGN_DOWN_POINTER_BY(address, alignment) \
    ((void*)((uintptr_t)(address) & ~((uintptr_t)alignment - 1)))

#define ALIGN_UP_POINTER_BY(address, alignment) \
    (ALIGN_DOWN_POINTER_BY(((uintptr_t)(address) + alignment - 1), alignment))

#define ALIGN_DOWN(length, type) \
    ALIGN_DOWN_BY(length, sizeof(type))

#define ALIGN_UP(length, type) \
    ALIGN_UP_BY(length, sizeof(type))

#define ALIGN_DOWN_POINTER(address, type) \
    ALIGN_DOWN_POINTER_BY(address, sizeof(type))

#define ALIGN_UP_POINTER(address, type) \
    ALIGN_UP_POINTER_BY(address, sizeof(type))

namespace ptfs {

	class Base {

	protected:

		err_t Status;

		uint32_t Refs;

	public:

		err_t GetStatus() { return Status; }

		void INC_REF() { Refs++; }

		void DEC_REF() { Refs--; }

		bool NoMoreRef() { return Refs == 0; }

		void ReleaseObject() {};

	};

}