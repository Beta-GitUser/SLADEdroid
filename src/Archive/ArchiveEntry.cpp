#include "ArchiveEntry.h"
#include "Archive.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace slade
{
namespace
{
constexpr uint64_t MaxEntrySize = 512ull * 1024ull * 1024ull;

string upperCopy(string_view value)
{
	string result{ value };
	for (auto& c : result)
		c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
	return result;
}

string fileNameToLumpName(string_view value)
{
	string result{ value };
	for (auto& c : result)
	{
		if (c == '\\' || c == '/')
			c = '_';
	}
	return result;
}

string lumpNameToFileName(string_view value)
{
	string result{ value };
	for (auto& c : result)
	{
		if (c == '\\')
			c = '/';
	}
	return result;
}

string humanSize(uint32_t size)
{
	if (size < 1024)
		return std::to_string(size) + " B";
	if (size < 1024 * 1024)
		return std::to_string(size / 1024) + " KB";
	if (size < 1024 * 1024 * 1024)
		return std::to_string(size / (1024 * 1024)) + " MB";
	return std::to_string(size / (1024 * 1024 * 1024)) + " GB";
}
} // namespace

ArchiveEntry::ArchiveEntry(string_view name, uint32_t size) :
	name_{ name }, upper_name_{ upperCopy(name) }, size_{ size }, type_{ EntryType::unknownType() }
{
}

ArchiveEntry::ArchiveEntry(ArchiveEntry& copy) :
	name_{ copy.name_ },
	upper_name_{ copy.upper_name_ },
	size_{ copy.size_ },
	type_{ copy.type_ },
	ex_props_{ copy.ex_props_ },
	encrypted_{ copy.encrypted_ },
	reliability_{ copy.reliability_ }
{
	data_.importMem(copy.rawData(true), copy.size());
	ex_props_.remove("ZipIndex");
	ex_props_.remove("Offset");
	ex_props_.remove("filePath");
}

string_view ArchiveEntry::nameNoExt() const
{
	auto pos = name_.find('.');
	return pos == string::npos ? string_view{ name_ } : string_view{ name_.data(), pos };
}

string_view ArchiveEntry::upperNameNoExt() const
{
	auto pos = upper_name_.find('.');
	return pos == string::npos ? string_view{ upper_name_ } : string_view{ upper_name_.data(), pos };
}

string_view ArchiveEntry::ext() const
{
	auto pos = name_.find('.');
	return pos == string::npos ? string_view{} : string_view{ name_.data() + pos };
}

string_view ArchiveEntry::upperExt() const
{
	auto pos = upper_name_.find('.');
	return pos == string::npos ? string_view{} : string_view{ upper_name_.data() + pos };
}

Archive* ArchiveEntry::parent() const
{
	return parent_.expired() ? nullptr : parent_.lock()->archive();
}

Archive* ArchiveEntry::topParent() const
{
	if (parent_.expired())
		return nullptr;
	auto dir = parent_.lock();
	if (!dir->archive()->parentEntry())
		return dir->archive();
	return dir->archive()->parentEntry()->topParent();
}

string ArchiveEntry::path(bool include_name) const
{
	auto result = parent_.expired() ? string{} : parent_.lock()->path();
	if (include_name)
		result += name_;
	return result;
}

const uint8_t* ArchiveEntry::rawData(bool allow_load)
{
	return data(allow_load).data();
}

MemChunk& ArchiveEntry::data(bool allow_load)
{
	auto parent_archive = parent();
	if (allow_load && !isLoaded() && parent_archive && size_ > 0 && size_ <= MaxEntrySize)
	{
		data_loaded_ = parent_archive->loadEntryData(this);
		setState(State::Unmodified);
	}
	return data_;
}

ArchiveEntry* ArchiveEntry::nextEntry()
{
	return parent_.expired() ? nullptr : parent_.lock()->entryAt(parent_.lock()->entryIndex(this) + 1);
}

ArchiveEntry* ArchiveEntry::prevEntry()
{
	return parent_.expired() ? nullptr : parent_.lock()->entryAt(parent_.lock()->entryIndex(this) - 1);
}

shared_ptr<ArchiveEntry> ArchiveEntry::getShared()
{
	return parent_.expired() ? nullptr : parent_.lock()->sharedEntry(this);
}

int ArchiveEntry::index()
{
	return parent_.expired() ? -1 : parent_.lock()->entryIndex(this);
}

void ArchiveEntry::setName(string_view name)
{
	name_ = name;
	upper_name_ = upperCopy(name);
}

void ArchiveEntry::setState(State state, bool silent)
{
	if (state_locked_ || (state == State::Unmodified && state_ == State::Unmodified))
		return;
	if (state == State::Unmodified)
		state_ = State::Unmodified;
	else if (state > state_)
		state_ = state;
	if (!silent)
		stateChanged();
}

void ArchiveEntry::unloadData(bool force)
{
	if (!data_.hasData() || !data_loaded_)
		return;
	if (!force && state_ != State::Unmodified)
		return;
	data_.clear();
	setLoaded(false);
}

void ArchiveEntry::lock()
{
	locked_ = true;
	stateChanged();
}

void ArchiveEntry::unlock()
{
	locked_ = false;
	stateChanged();
}

void ArchiveEntry::formatName(const ArchiveFormat& format)
{
	name_ = fileNameToLumpName(name_);
	if (format.max_name_length > 0 && static_cast<int>(name_.size()) > format.max_name_length)
		name_.resize(format.max_name_length);
	if (format.prefer_uppercase)
		name_ = upperCopy(name_);
	if (format.supports_dirs && (name_.find('/') != string::npos || name_.find('\\') != string::npos))
		name_ = lumpNameToFileName(name_);
	if (!format.names_extensions)
		if (auto pos = name_.find('.'); pos != string::npos)
			name_.resize(pos);
	upper_name_ = upperCopy(name_);
}

bool ArchiveEntry::rename(string_view new_name)
{
	if (locked_)
		return false;
	setName(new_name);
	setState(State::Modified);
	return true;
}

bool ArchiveEntry::resize(uint32_t new_size, bool preserve_data)
{
	if (locked_ || new_size > MaxEntrySize)
		return false;
	if (!data_.reSize(new_size, preserve_data))
		return false;
	size_ = new_size;
	setState(State::Modified);
	return true;
}

void ArchiveEntry::clearData()
{
	if (locked_)
		return;
	data_.clear();
	size_ = 0;
	data_loaded_ = false;
}

bool ArchiveEntry::importMem(const void* data, uint32_t size)
{
	if ((!data && size) || locked_ || size > MaxEntrySize)
		return false;
	clearData();
	if (!data_.importMem(static_cast<const uint8_t*>(data), size))
		return false;
	size_ = size;
	setLoaded();
	setType(EntryType::unknownType());
	setState(State::Modified);
	return true;
}

bool ArchiveEntry::importMemChunk(MemChunk& mc)
{
	return mc.hasData() && importMem(mc.data(), mc.size());
}

bool ArchiveEntry::importFile(string_view filename, uint32_t offset, uint32_t size)
{
	if (locked_)
		return false;

	StdFile file(filename, StdFile::Mode::Read);
	if (!file.isOpen())
		return false;
	if (offset > file.size())
		return false;

	if (size == 0)
		size = file.size() - offset;
	if (size > file.size() - offset || size > MaxEntrySize)
		return false;

	if (!file.seekFromStart(offset))
		return false;
	if (!data_.importFile(file, size))
		return false;

	size_ = size;
	setLoaded();
	setType(EntryType::unknownType());
	setState(State::Modified);
	return true;
}

bool ArchiveEntry::importFileStream(File& file, uint32_t len)
{
	if (locked_ || !file.isOpen())
		return false;
	if (len == 0)
		len = file.size() - std::min(file.currentPos(), file.size());
	if (len > MaxEntrySize || len > file.size() - std::min(file.currentPos(), file.size()))
		return false;
	if (!data_.importFile(file, len))
		return false;
	size_ = data_.size();
	setLoaded();
	setType(EntryType::unknownType());
	setState(State::Modified);
	return true;
}

bool ArchiveEntry::importEntry(ArchiveEntry* entry)
{
	return !locked_ && entry && importMem(entry->rawData(), entry->size());
}

bool ArchiveEntry::exportFile(string_view filename)
{
	StdFile file(filename, StdFile::Mode::Write);
	if (!file.isOpen())
		return false;
	auto data = rawData();
	return !data || file.write(data, size());
}

bool ArchiveEntry::write(const void* data, uint32_t size)
{
	if (locked_)
		return false;
	if (!isLoaded())
		rawData(true);
	if (!data_.write(data, size))
		return false;
	size_ = data_.size();
	setState(State::Modified);
	return true;
}

bool ArchiveEntry::read(void* buf, uint32_t size)
{
	if (!isLoaded())
		rawData(true);
	return data_.read(buf, size);
}

string ArchiveEntry::sizeString() const
{
	return humanSize(size());
}

void ArchiveEntry::stateChanged()
{
	auto parent_archive = parent();
	if (parent_archive)
		parent_archive->entryStateChanged(this);
}

void ArchiveEntry::setExtensionByType()
{
	if (!type_ || (parent() && !parent()->formatDesc().names_extensions))
		return;

	string filename = name_;
	const auto slash = filename.find_last_of("/\\");
	const auto dot = filename.find_last_of('.');
	if (dot == string::npos || (slash != string::npos && dot < slash))
		filename += '.';
	else
		filename.resize(dot + 1);
	filename += type_->extension();

	if (auto parent_archive = parent())
		parent_archive->renameEntry(this, filename);
	else
		rename(filename);
}

bool ArchiveEntry::isInNamespace(string_view ns)
{
	if (!parent())
		return false;
	if (ns == "graphics" && parent()->formatId() == "wad")
		ns = "global";
	return parent()->detectNamespace(this) == ns;
}

ArchiveEntry* ArchiveEntry::relativeEntry(string_view at_path, bool allow_absolute_path) const
{
	if (parent_.expired())
		return nullptr;
	auto archive = parent_.lock()->archive();
	auto include = archive->entryAtPath(path().append(at_path));
	if (!include && allow_absolute_path)
		include = archive->entryAtPath(at_path);
	return include;
}
} // namespace slade
