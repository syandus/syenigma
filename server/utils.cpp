#include "stdafx.h"

namespace bai = boost::archive::iterators;

std::string get_extension(std::string path) {
  size_t i = path.rfind(".");
  if (i == path.npos) return "";
  return std::string(path.begin() + i + 1, path.end());
}

std::string to_utf8(const wchar_t* input_str) {
  size_t n = wcslen(input_str);
  std::string output(n, '\0');
  WideCharToMultiByte(CP_UTF8, 0, input_str, n, &output[0], n, NULL, NULL);
  return output;
}

std::wstring to_utf16(const std::string& input) {
  size_t n = input.size();
  std::wstring output(n, '\0');
  MultiByteToWideChar(CP_UTF8, 0, input.c_str(), n, &output[0], n);
  return output;
}

void error(const std::string& msg) {
  MessageBoxA(NULL, msg.c_str(), "Fatal Error", MB_OK | MB_ICONERROR);
  ExitProcess(1);
}

std::string GetMachineId() {
  DWORD serial_number;
  GetVolumeInformation(NULL, NULL, 0, &serial_number, NULL, NULL, NULL, 0);
  std::stringstream stream;
  stream << std::hex << serial_number;
  std::string hex_serial_number(stream.str());
  return hex_serial_number;
}

std::string base64_decode(const std::string& s) {
  typedef bai::transform_width<bai::binary_from_base64<const char*>, 8, 6>
      base64_dec;

  std::stringstream os;
  unsigned int size = s.size();
  // Remove the padding characters, cf.
  // https://svn.boost.org/trac/boost/ticket/5629
  if (size && s[size - 1] == '=') {
    --size;
    if (size && s[size - 1] == '=') --size;
  }
  if (size == 0) return std::string();

  std::copy(base64_dec(s.data()), base64_dec(s.data() + size),
            std::ostream_iterator<char>(os));

  return os.str();
}
