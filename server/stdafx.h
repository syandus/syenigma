// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "targetver.h"
#include <tchar.h>
#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

#include <boost/lexical_cast.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include <cryptlib.h>
#include <aes.h>
#include <modes.h>
#include <filters.h>

#include "mongoose.h"
