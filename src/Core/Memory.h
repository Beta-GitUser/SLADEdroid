#pragma once

#include "Common.h"

namespace slade::memory
{
inline u16 readL16(const u8* data, unsigned ofs)
{
	return static_cast<u16>(data[ofs]) | static_cast<u16>(data[ofs + 1]) << 8;
}
inline u32 readL24(const u8* data, unsigned ofs)
{
	return static_cast<u32>(data[ofs]) | static_cast<u32>(data[ofs + 1]) << 8 | static_cast<u32>(data[ofs + 2]) << 16;
}
inline u32 readL32(const u8* data, unsigned ofs)
{
	return static_cast<u32>(data[ofs]) | static_cast<u32>(data[ofs + 1]) << 8 | static_cast<u32>(data[ofs + 2]) << 16 | static_cast<u32>(data[ofs + 3]) << 24;
}
inline u16 readB16(const u8* data, unsigned ofs)
{
	return static_cast<u16>(data[ofs + 1]) | static_cast<u16>(data[ofs]) << 8;
}
inline u32 readB24(const u8* data, unsigned ofs)
{
	return static_cast<u32>(data[ofs + 2]) | static_cast<u32>(data[ofs + 1]) << 8 | static_cast<u32>(data[ofs]) << 16;
}
inline u32 readB32(const u8* data, unsigned ofs)
{
	return static_cast<u32>(data[ofs + 3]) | static_cast<u32>(data[ofs + 2]) << 8 | static_cast<u32>(data[ofs + 1]) << 16 | static_cast<u32>(data[ofs]) << 24;
}
}
