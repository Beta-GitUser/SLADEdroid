#include "File.h"

#include <fstream>

namespace slade
{
bool StdFile::open(string_view path, Mode mode)
{
	if (isOpen())
		return false;

	std::ios::openmode flags = std::ios::binary;
	switch (mode)
	{
	case Mode::Read: flags |= std::ios::in; break;
	case Mode::Write: flags |= std::ios::out | std::ios::trunc; break;
	case Mode::ReadWrite: flags |= std::ios::in | std::ios::out; break;
	case Mode::Append: flags |= std::ios::out | std::ios::app; break;
	}

	stream_.open(string{ path }, flags);
	if (!stream_.is_open())
		return false;

	updateSize();
	return true;
}

void StdFile::close()
{
	if (stream_.is_open())
		stream_.close();
	size_ = 0;
}

bool StdFile::flush()
{
	if (!stream_.is_open())
		return false;
	stream_.flush();
	return !stream_.fail();
}

unsigned StdFile::currentPos() const
{
	if (!stream_.is_open())
		return 0;

	auto pos = stream_.tellg();
	if (pos < 0)
		return 0;
	return static_cast<unsigned>(pos);
}

bool StdFile::seek(unsigned offset)
{
	return seekFromStart(currentPos() + offset);
}

bool StdFile::seekFromStart(unsigned offset)
{
	if (!stream_.is_open())
		return false;

	stream_.clear();
	stream_.seekg(offset, std::ios::beg);
	stream_.seekp(offset, std::ios::beg);
	return !stream_.fail();
}

bool StdFile::seekFromEnd(unsigned offset)
{
	if (!stream_.is_open())
		return false;

	stream_.clear();
	stream_.seekg(-static_cast<std::streamoff>(offset), std::ios::end);
	stream_.seekp(-static_cast<std::streamoff>(offset), std::ios::end);
	return !stream_.fail();
}

bool StdFile::read(void* buffer, unsigned count)
{
	if (!stream_.is_open() || !buffer)
		return false;
	stream_.read(static_cast<char*>(buffer), static_cast<std::streamsize>(count));
	return stream_.gcount() == static_cast<std::streamsize>(count);
}

bool StdFile::write(const void* buffer, unsigned count)
{
	if (!stream_.is_open() || !buffer)
		return false;
	stream_.write(static_cast<const char*>(buffer), static_cast<std::streamsize>(count));
	if (stream_.fail())
		return false;
	updateSize();
	return true;
}

void StdFile::updateSize()
{
	if (!stream_.is_open())
	{
		size_ = 0;
		return;
	}

	auto current = stream_.tellg();
	stream_.clear();
	stream_.seekg(0, std::ios::end);
	auto end = stream_.tellg();
	if (end >= 0)
		size_ = static_cast<unsigned>(end);
	stream_.clear();
	if (current >= 0)
	{
		stream_.seekg(current);
		stream_.seekp(current);
	}
}
} // namespace slade
