#include "FileSystem.h"

#include <filesystem>
#include <fstream>

namespace slade::filesystem
{
bool exists(string_view path)
{
	return !path.empty() && std::filesystem::exists(string{ path });
}

bool isDirectory(string_view path)
{
	return !path.empty() && std::filesystem::is_directory(string{ path });
}

bool remove(string_view path)
{
	if (path.empty())
		return false;
	std::error_code ec;
	std::filesystem::remove_all(string{ path }, ec);
	return !ec;
}

bool copy(string_view from, string_view to, bool overwrite)
{
	if (from.empty() || to.empty())
		return false;
	std::error_code ec;
	auto options = overwrite ? std::filesystem::copy_options::overwrite_existing
	                         : std::filesystem::copy_options::none;
	std::filesystem::copy_file(string{ from }, string{ to }, options, ec);
	return !ec;
}

bool createDirectories(string_view path)
{
	if (path.empty())
		return false;
	std::error_code ec;
	return std::filesystem::create_directories(string{ path }, ec) ||
	       (!ec && std::filesystem::is_directory(string{ path }));
}

string readText(string_view path)
{
	std::ifstream file(string{ path }, std::ios::binary);
	if (!file)
		return {};
	return { std::istreambuf_iterator<char>{ file }, std::istreambuf_iterator<char>{} };
}

bool writeText(string_view path, string_view text)
{
	std::ofstream file(string{ path }, std::ios::binary);
	if (!file)
		return false;
	file.write(text.data(), static_cast<std::streamsize>(text.size()));
	return !file.fail();
}

time_t modifiedTime(string_view path)
{
	std::error_code ec;
	auto file_time = std::filesystem::last_write_time(string{ path }, ec);
	if (ec)
		return 0;

	// C++17 has no clock_cast. Convert through the difference between the
	// filesystem clock and system_clock at approximately the same instant.
	auto system_time = std::chrono::system_clock::now() +
	                   (file_time - std::filesystem::file_time_type::clock::now());
	return std::chrono::system_clock::to_time_t(system_time);
}

vector<string> filesInDirectory(string_view path, bool recursive)
{
	vector<string> result;
	if (path.empty())
		return result;

	std::error_code ec;
	if (recursive)
	{
		for (const auto& entry : std::filesystem::recursive_directory_iterator(string{ path }, ec))
			if (entry.is_regular_file())
				result.push_back(entry.path().string());
	}
	else
	{
		for (const auto& entry : std::filesystem::directory_iterator(string{ path }, ec))
			if (entry.is_regular_file())
				result.push_back(entry.path().string());
	}
	return result;
}
} // namespace slade::filesystem
