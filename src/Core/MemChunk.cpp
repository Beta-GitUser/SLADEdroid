#include "MemChunk.h"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace slade
{
MemChunk::MemChunk(uint32_t size) : size_{ size }
{
	if (size_)
		allocData(size_);
}

MemChunk::MemChunk(const uint8_t* data, uint32_t size) : size_{ size }
{
	if (size_)
	{
		allocData(size_);
		if (data)
			std::memcpy(data_, data, size_);
	}
}

MemChunk::~MemChunk() { delete[] data_; }

MemChunk::MemChunk(MemChunk&& other) noexcept
	: data_{ other.data_ }, cur_ptr_{ other.cur_ptr_ }, size_{ other.size_ }
{
	other.data_ = nullptr;
	other.cur_ptr_ = 0;
	other.size_ = 0;
}

MemChunk& MemChunk::operator=(MemChunk&& other) noexcept
{
	if (this == &other)
		return *this;

	delete[] data_;
	data_ = other.data_;
	cur_ptr_ = other.cur_ptr_;
	size_ = other.size_;
	other.data_ = nullptr;
	other.cur_ptr_ = 0;
	other.size_ = 0;
	return *this;
}

uint8_t* MemChunk::allocData(uint32_t size, bool set_data)
{
	if (!size)
		return nullptr;
	delete[] data_;
	data_ = new (std::nothrow) uint8_t[size];
	if (data_ && set_data)
		std::memset(data_, 0, size);
	return data_;
}

bool MemChunk::clear()
{
	delete[] data_;
	data_ = nullptr;
	size_ = 0;
	cur_ptr_ = 0;
	return true;
}

bool MemChunk::reSize(uint32_t new_size, bool preserve_data)
{
	if (!new_size)
		return clear();

	uint8_t* replacement = new (std::nothrow) uint8_t[new_size];
	if (!replacement)
		return false;

	std::memset(replacement, 0, new_size);
	if (preserve_data && data_)
		std::memcpy(replacement, data_, std::min(size_, new_size));

	delete[] data_;
	data_ = replacement;
	size_ = new_size;
	cur_ptr_ = std::min(cur_ptr_, size_);
	return true;
}

bool MemChunk::importFile(string_view filename, uint32_t offset, uint32_t len)
{
	std::ifstream file(string{ filename }, std::ios::binary);
	if (!file)
		return false;

	file.seekg(0, std::ios::end);
	const auto end = file.tellg();
	if (end < 0 || offset > static_cast<uint64_t>(end))
		return false;

	const uint64_t available = static_cast<uint64_t>(end) - offset;
	if (!len || len > available)
		len = static_cast<uint32_t>(std::min<uint64_t>(available, UINT32_MAX));

	if (!reSize(len, false))
		return false;
	file.seekg(offset, std::ios::beg);
	return len == 0 || static_cast<bool>(file.read(reinterpret_cast<char*>(data_), len));
}

bool MemChunk::importFile(File& file, uint32_t len)
{
	if (!file.isOpen())
		return false;

	const auto offset = file.currentPos();
	if (!len || offset + len > file.size())
		len = file.size() - offset;

	if (!reSize(len, false))
		return false;
	return len == 0 || file.read(data_, len);
}

bool MemChunk::importMem(const uint8_t* start, uint32_t len)
{
	if (!start && len)
		return false;
	if (!reSize(len, false))
		return len == 0;
	if (len)
		std::memcpy(data_, start, len);
	cur_ptr_ = 0;
	return true;
}

bool MemChunk::exportFile(string_view filename, uint32_t start, uint32_t size) const
{
	std::ofstream file(string{ filename }, std::ios::binary | std::ios::trunc);
	if (!file)
		return false;
	return exportFile(*reinterpret_cast<File*>(nullptr), start, size) ||
		       (start <= size_ && (size == 0 || start + size <= size_) &&
				file.write(reinterpret_cast<const char*>(data_ + start), size ? size : size_ - start).good());
}

bool MemChunk::exportFile(File& file, uint32_t start, uint32_t size) const
{
	if (!file.isOpen() || start > size_ || start + size > size_)
		return false;
	if (!size)
		size = size_ - start;
	return !size || file.write(data_ + start, size);
}

bool MemChunk::exportMemChunk(MemChunk& mc, uint32_t start, uint32_t size) const
{
	if (start > size_ || start + size > size_)
		return false;
	if (!size)
		size = size_ - start;
	return mc.importMem(data_ + start, size);
}

bool MemChunk::write(unsigned offset, const void* data, unsigned size, bool expand)
{
	if (!data || offset > UINT32_MAX - size)
		return false;
	if (offset + size > size_ && (!expand || !reSize(offset + size, true)))
		return false;
	std::memcpy(data_ + offset, data, size);
	return true;
}

bool MemChunk::read(unsigned offset, void* buf, unsigned size) const
{
	if (!buf || offset > size_ || size > size_ - offset)
		return false;
	std::memcpy(buf, data_ + offset, size);
	return true;
}

bool MemChunk::write(const void* data, uint32_t size, uint32_t start)
{
	if (!seek(start, SEEK_SET))
		return false;
	return write(data, size);
}

bool MemChunk::read(void* buf, uint32_t size, uint32_t start)
{
	if (!seek(start, SEEK_SET))
		return false;
	return read(buf, size);
}

bool MemChunk::seek(uint32_t offset, uint32_t origin)
{
	uint64_t position = 0;
	if (origin == SEEK_CUR)
		position = static_cast<uint64_t>(cur_ptr_) + offset;
	else if (origin == SEEK_SET)
		position = offset;
	else if (origin == SEEK_END)
		position = offset > size_ ? 0 : size_ - offset;
	else
		return false;
	cur_ptr_ = static_cast<uint32_t>(std::min<uint64_t>(position, size_));
	return true;
}

bool MemChunk::readMC(MemChunk& mc, uint32_t size)
{
	if (size > size_ - std::min(cur_ptr_, size_))
		return false;
	if (!mc.importMem(data_ + cur_ptr_, size))
		return false;
	cur_ptr_ += size;
	return true;
}

bool MemChunk::fillData(uint8_t value) const
{
	if (!data_)
		return false;
	std::memset(data_, value, size_);
	return true;
}

string MemChunk::asString(uint32_t offset, uint32_t length) const
{
	if (offset > size_)
		return {};
	if (!length || offset + length > size_)
		length = size_ - offset;
	return string{ reinterpret_cast<const char*>(data_ + offset), length };
}
} // namespace slade
