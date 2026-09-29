#pragma once

#include "Common.h"
#include "SeekableData.h"

namespace slade
{
// Platform-neutral seekable byte stream used by the portable core.
// Android, desktop, and other frontends can provide their own implementation.
class File : public SeekableData
{
public:
	~File() override = default;

	virtual bool isOpen() const = 0;
	virtual bool flush() = 0;
};
}
