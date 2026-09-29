#pragma once

#include "Common.h"
#include "SeekableData.h"
#include "File.h"

namespace slade
{
// Portable byte buffer used by the distilled core. This deliberately has no
// dependency on wxWidgets or any desktop file/UI type.
class MemChunk : public SeekableData
{
public:
	MemChunk() = default;
	explicit MemChunk(uint32_t size);
	MemChunk(const uint8_t* data, uint32_t size);
	~MemChunk() override;

	MemChunk(const MemChunk&) = delete;
	MemChunk& operator=(const MemChunk&) = delete;
	MemChunk(MemChunk&& other) noexcept;
	MemChunk& operator=(MemChunk&& other) noexcept;

	uint8_t& operator[](unsigned index) { return data_[index]; }
	const uint8_t& operator[](unsigned index) const { return data_[index]; }

	const uint8_t* data() const { return data_; }
	uint8_t* data() { return data_; }
	bool hasData() const { return data_ != nullptr && size_ != 0; }

	unsigned size() const override { return size_; }
	unsigned currentPos() const override { return cur_ptr_; }
	bool seek(unsigned offset) override { return seek(offset, SEEK_CUR); }
	bool seekFromStart(unsigned offset) override { return seek(offset, SEEK_SET); }
	bool seekFromEnd(unsigned offset) override { return seek(offset, SEEK_END); }
	bool read(void* buffer, unsigned count) override;
	bool write(const void* buffer, unsigned count) override;

	bool clear();
	bool reSize(uint32_t new_size, bool preserve_data = true);

	bool importFile(string_view filename, uint32_t offset = 0, uint32_t len = 0);
	bool importFile(File& file, uint32_t len = 0);
	bool importMem(const uint8_t* start, uint32_t len);
	bool importMem(const MemChunk& other) { return importMem(other.data_, other.size_); }

	bool exportFile(string_view filename, uint32_t start = 0, uint32_t size = 0) const;
	bool exportFile(File& file, uint32_t start = 0, uint32_t size = 0) const;
	bool exportMemChunk(MemChunk& mc, uint32_t start = 0, uint32_t size = 0) const;

	bool write(unsigned offset, const void* data, unsigned size, bool expand);
	bool read(unsigned offset, void* buf, unsigned size) const;
	bool write(const void* data, uint32_t size, uint32_t start);
	bool read(void* buf, uint32_t size, uint32_t start);
	bool seek(uint32_t offset, uint32_t origin);
	bool readMC(MemChunk& mc, uint32_t size);

	bool fillData(uint8_t value) const;
	string asString(uint32_t offset = 0, uint32_t length = 0) const;

protected:
	uint8_t* allocData(uint32_t size, bool set_data = true);

	uint8_t* data_ = nullptr;
	uint32_t cur_ptr_ = 0;
	uint32_t size_ = 0;
};
} // namespace slade
