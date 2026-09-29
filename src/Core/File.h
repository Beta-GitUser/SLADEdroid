#pragma once

#include "Common.h"
#include "SeekableData.h"

namespace slade
{
// Portable seekable byte stream used by the core. Platform frontends can
// provide alternate implementations (for example an Android content URI).
class File : public SeekableData
{
public:
	~File() override = default;

	virtual bool isOpen() const = 0;
	virtual bool flush() = 0;
};

class StdFile final : public File
{
public:
	enum class Mode
	{
		Read,
		Write,
		ReadWrite,
		Append,
	};

	StdFile() = default;
	StdFile(string_view path, Mode mode = Mode::Read) { open(path, mode); }
	~StdFile() override { close(); }

	bool open(string_view path, Mode mode = Mode::Read);
	void close();

	bool isOpen() const override { return stream_.is_open(); }
	bool flush() override;

	unsigned currentPos() const override;
	unsigned size() const override { return size_; }

	bool seek(unsigned offset) override;
	bool seekFromStart(unsigned offset) override;
	bool seekFromEnd(unsigned offset) override;

	bool read(void* buffer, unsigned count) override;
	bool write(const void* buffer, unsigned count) override;

private:
	void updateSize();

	std::fstream stream_;
	unsigned    size_ = 0;
};
} // namespace slade
