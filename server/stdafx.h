#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "targetver.h"
#include <tchar.h>
#include <windows.h>
#include <ShlObj.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <unordered_map>

#include <boost/lexical_cast.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <boost/algorithm/string.hpp>

#include <cryptlib.h>
#include <aes.h>
#include <sha.h>
#include <modes.h>
#include <filters.h>

#include <snappy.h>

// mongoose forces _UNICODE to be turned off, which conflicts with the default
// usage by wxWidgets below. Unfortunately there doesn't seem to be a way to
// disable this, so mongoose header code will always have to be in a separate
// file, and can't intermix with wxWidgets code
/* #include <mongoose.h> */

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/statline.h>

// place global util functions here
void error(const std::string& msg);
std::string get_extension(std::string path);
std::string to_utf8(const wchar_t* input_str);
std::wstring to_utf16(const std::string& input);
std::string GetMachineId();
std::string base64_decode(const std::string& s);
