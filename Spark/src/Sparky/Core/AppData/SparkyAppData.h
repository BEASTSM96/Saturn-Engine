#pragma once


#include <filesystem>
#include <string>

class SparkyAppData
{
public:

	SparkyAppData();

	~SparkyAppData() = default;



	std::string  operator/(std::string_view relative_path)  const;
	std::wstring operator/(std::wstring_view relative_path) const;

public:

	std::basic_string< std::filesystem::path::value_type > Path(void) const { return path_; }

private:

	std::filesystem::path path_;

};