#include "memoryTools.h"

void SetBytes(void* dst, BYTE* bytes, unsigned int size)
{
	DWORD old;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &old);
	memcpy(dst, bytes, size);
	VirtualProtect(dst, size, old, &old);
}

void SetByte(void* dst, BYTE byte, unsigned int count)
{
	DWORD old;
	VirtualProtect(dst, count, PAGE_EXECUTE_READWRITE, &old);
	memset(dst, byte, count);
	VirtualProtect(dst, count, old, &old);
}