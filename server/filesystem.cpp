#include "stdafx.h"
#include "filesystem.hpp"

using namespace boost::interprocess;

static size_t read_size_t(const byte*& p) {
  size_t n = *reinterpret_cast<const size_t*>(p);
  p += sizeof(size_t);
  return n;
}

static std::string read_string(const byte*& p) {
  std::string s;
  while (*p) {
    s += *p;
    ++p;
  }
  // skip over null byte
  ++p;
  return s;
}

static std::string read_file(const std::string& path) {
  std::ifstream ifs(to_utf16(path).c_str(), std::ifstream::binary);
  std::string data((std::istreambuf_iterator<char>(ifs)),
                   (std::istreambuf_iterator<char>()));
  return data;
}

Filesystem::Filesystem(std::wstring pack_path) : mFileBytes(nullptr) {
  // TODO: actually route key info here
  mKey.resize(CryptoPP::AES::DEFAULT_KEYLENGTH, '\0');
  mIV.resize(CryptoPP::AES::BLOCKSIZE, '\0');

  this->load_pack(pack_path);
}

Filesystem::~Filesystem() {
  UnmapViewOfFile(mBytes);
  CloseHandle(mMapHandle);
  CloseHandle(mFileHandle);
}

void Filesystem::decrypt(const std::string& ciphertext, const std::string& key,
                         std::string& plaintext, bool uncompress) {
  std::string compressed_plaintext;

  CryptoPP::AES::Decryption aes_decryption((byte*)&key[0], key.size());
  CryptoPP::CBC_Mode_ExternalCipher::Decryption cbc_decryption(aes_decryption,
                                                               &mIV[0]);
  auto string_sink = new CryptoPP::StringSink(compressed_plaintext);
  CryptoPP::StreamTransformationFilter stf_decryptor(
      cbc_decryption, string_sink);
  stf_decryptor.Put(reinterpret_cast<const unsigned char*>(ciphertext.c_str()),
                    ciphertext.size());
  stf_decryptor.MessageEnd();
  /* delete string_sink; */

  if (uncompress) {
    snappy::Uncompress(compressed_plaintext.c_str(),
                       compressed_plaintext.size(), &plaintext);
  } else {
    plaintext = compressed_plaintext;
  }
}

void Filesystem::load_pack(std::wstring pack_path) {
  mFileHandle = CreateFile(pack_path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                           NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);
  if (mFileHandle == INVALID_HANDLE_VALUE) {
    error("could not open enigma file pack");
  }

  LARGE_INTEGER size;
  if (!GetFileSizeEx(mFileHandle, &size)) {
    error("could not determine enigma file pack size");
  }
  mSize = size.QuadPart;

  mMapHandle = CreateFileMapping(mFileHandle, NULL, PAGE_READONLY, 0, 0, 0);
  if (mMapHandle == NULL) {
    error("could not create file mapping for enigma file pack");
  }
  mBytes = (const byte*)MapViewOfFile(mMapHandle, FILE_MAP_READ, 0, 0, 0);
  if (!mBytes) {
    auto err = GetLastError();
    error("could not MapViewOfFile of enigma file pack");
  }
}

bool Filesystem::exists(std::string path) {
  if (mPathToBytes.size() == 0) return false;
  return mPathToBytes.count(path) > 0;
}

std::string Filesystem::get_file(std::string path) {
  if (!this->exists(path)) error("path in file pack does not exist: " + path);

  const byte* p = mPathToBytes[path];
  size_t n = mPathToEncryptedSize[path];
  std::string ciphertext((const char*)p, n);
  std::string plaintext;
  this->decrypt(ciphertext, mKey, plaintext);
  return plaintext;
}

std::vector<std::string> Filesystem::get_file_list() {
  std::vector<std::string> files;
  for (auto iter : mPathToBytes) {
    files.push_back(iter.first);
  }
  return files;
}

void Filesystem::load_key(std::string path) {
  std::string encrypted_key = read_file(path);
  std::string keystr = decrypt_key(encrypted_key);
  mKey = keystr;

  mPathToBytes.clear();
  mPathToEncryptedSize.clear();

  const byte* p = mBytes;
  static const char* magic = "SYENIGMA";
  if (memcmp(magic, p, 8))
    error(
        "File pack has been corrupted! It does not start with magic "
        "'SYENIGMA'");
  p += 8;
  size_t encrypted_header_length = read_size_t(p);
  std::string encrypted_header((const char*)p, encrypted_header_length);
  std::string headerstr;
  this->decrypt(encrypted_header, mKey, headerstr);
  const byte* header = (const byte*)&headerstr[0];

  p += encrypted_header_length;
  mFileBytes = p;

  p = header;
  const byte* pend = header + headerstr.length();
  int64_t offset = 0;
  while (p < pend) {
    std::string path = read_string(p);
    size_t encrypted_file_size = read_size_t(p);
    const byte* encrypted_file_data = &mFileBytes[offset];
    offset += encrypted_file_size;
    mPathToBytes[path] = encrypted_file_data;
    mPathToEncryptedSize[path] = encrypted_file_size;
    /* std::cout << path << " " << encrypted_file_size << "\n"; */
  }
  
  std::string datestr = this->get_file("/EXPIRATION_DATE");
  boost::algorithm::trim_all(datestr);
  std::stringstream ss;
  ss << datestr;
  std::time_t timestamp;
  ss >> timestamp;
  std::time_t current_time = std::time(nullptr);
  if (current_time > timestamp) {
    error("Access to this resource has expired.");
  }
}

std::string Filesystem::decrypt_key(std::string encrypted_key) {
  std::string machine_id = GetMachineId();

  CryptoPP::SHA256 hash;
  byte digest[CryptoPP::SHA256::DIGESTSIZE];
  hash.CalculateDigest(digest, (byte*)machine_id.c_str(), machine_id.length());

  std::string secret((char*)digest, CryptoPP::AES::DEFAULT_KEYLENGTH);
  /* std::vector<byte> iv(CryptoPP::AES::BLOCKSIZE, '\0'); */
  std::string key;
  this->decrypt(encrypted_key, secret, key, false);
  return key;
}
