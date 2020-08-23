#include "sppch.h"
#include "SparkyAppData.h"


#include <array>
#include <cstdlib>
#include <cstring>

#if defined( _WIN32 )
#include <ShlObj.h>
#elif defined( __linux__ ) // _WIN32
#include <sys/stat.h>
#endif // __linux__


SparkyAppData::SparkyAppData(void)
{
#if defined( _WIN32 )

	wchar_t buf[FILENAME_MAX + 1] = { };

	PWSTR local_app_data_folder_path;
	if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local_app_data_folder_path) != S_OK)
		return;

	if (wcscat_s(buf, std::size(buf), local_app_data_folder_path) != 0)
		return;

	if (wcscat_s(buf, std::size(buf), L"\\Sparky-Engine") != 0)
		return;

	if (!CreateDirectoryW(buf, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS)
		return;

	path_.assign(std::begin(buf), std::begin(buf) + wcsnlen(buf, std::size(buf)));
	path_ = path_.lexically_normal();
#endif // else
}

std::string SparkyAppData::operator/(std::string_view relative_path) const
{
	std::filesystem::path abs_path = (path_ / relative_path);
	abs_path = abs_path.lexically_normal();

	return abs_path.string();
}

std::wstring SparkyAppData::operator/(std::wstring_view relative_path) const
{
	std::filesystem::path abs_path = (path_ / relative_path);
	abs_path = abs_path.lexically_normal();

	return abs_path.wstring();
}
