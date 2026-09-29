#pragma once

#include "Common.h"

namespace slade::filesystem
{
bool exists(string_view path);
bool isDirectory(string_view path);
bool remove(string_view path);
bool copy(string_view from, string_view to, bool overwrite = true);
bool createDirectories(string_view path);
string readText(string_view path);
bool writeText(string_view path, string_view text);
time_t modifiedTime(string_view path);
vector<string> filesInDirectory(string_view path, bool recursive = false);
}
