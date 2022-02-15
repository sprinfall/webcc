#ifndef WEBCC_FS_H_
#define WEBCC_FS_H_

// Use std or boost filesystem according to config.

#include "webcc/config.h"  // for WEBCC_USE_STD_FILESYSTEM

#if WEBCC_USE_STD_FILESYSTEM
#include <filesystem>
#include <fstream>
#else
#include "boost/filesystem/fstream.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#endif  // WEBCC_USE_STD_FILESYSTEM

namespace webcc {
namespace fs {

#if WEBCC_USE_STD_FILESYSTEM

// types
using std::error_code;
using std::ifstream;
using std::ofstream;
using std::filesystem::path;
using std::filesystem::filesystem_error;

// functions
using std::filesystem::absolute;
using std::filesystem::canonical;
using std::filesystem::rename;
using std::filesystem::remove;
using std::filesystem::exists;
using std::filesystem::is_directory;
using std::filesystem::is_regular_file;
using std::filesystem::create_directory;
using std::filesystem::create_directories;
using std::filesystem::current_path;
using std::filesystem::temp_directory_path;

#else

// types
using boost::system::error_code;
using boost::filesystem::ifstream;
using boost::filesystem::ofstream;
using boost::filesystem::path;
using boost::filesystem::filesystem_error;

// functions
using boost::filesystem::absolute;
using boost::filesystem::canonical;
using boost::filesystem::rename;
using boost::filesystem::remove;
using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::filesystem::is_regular_file;
using boost::filesystem::create_directory;
using boost::filesystem::create_directories;
using boost::filesystem::current_path;
using boost::filesystem::temp_directory_path;

#endif  // WEBCC_USE_STD_FILESYSTEM

}  // namespace fs
}  // namespace webcc

#endif  // WEBCC_FS_H_
