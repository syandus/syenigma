#include "stdafx.h"
#include "filesystem.hpp"

using namespace boost::interprocess;

static void error(std::string msg) {
  std::cerr << msg << std::endl;
  throw std::exception();
}

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

Filesystem::Filesystem(std::string pack_path)
    : mPackPath(pack_path), mFileBytes(nullptr) {
  // TODO: actually route key info here
  mKey.resize(CryptoPP::AES::DEFAULT_KEYLENGTH, '\0');
  mIV.resize(CryptoPP::AES::BLOCKSIZE, '\0');

  this->load_pack(pack_path);
}

Filesystem::~Filesystem() {}

void Filesystem::decrypt(const std::string& ciphertext,
                         std::string& plaintext) {
  std::string compressed_plaintext;

  CryptoPP::AES::Decryption aes_decryption(&mKey[0], mKey.size());
  CryptoPP::CBC_Mode_ExternalCipher::Decryption cbc_decryption(aes_decryption,
                                                               &mIV[0]);
  CryptoPP::StreamTransformationFilter stf_decryptor(
      cbc_decryption, new CryptoPP::StringSink(compressed_plaintext));
  stf_decryptor.Put(reinterpret_cast<const unsigned char*>(ciphertext.c_str()),
                    ciphertext.size());
  stf_decryptor.MessageEnd();

  snappy::Uncompress(compressed_plaintext.c_str(), compressed_plaintext.size(),
                     &plaintext);
}

void Filesystem::load_pack(std::string pack_path) {
  mFileMapping = file_mapping(pack_path.c_str(), read_only);
  mMappedRegion = mapped_region(mFileMapping, read_only);
  mBytes = (const byte*)mMappedRegion.get_address();
  mSize = mMappedRegion.get_size();

  this->read_header();
}

void Filesystem::read_header() {
  static const char* magic = "SYENIGMA";

  mPathToBytes.clear();
  mPathToEncryptedSize.clear();

  const byte* p = mBytes;
  if (memcmp(magic, p, 8))
    error("file pack does not start with magic 'SYENIGMA'");
  p += 8;
  size_t encrypted_header_length = read_size_t(p);
  std::string encrypted_header((const char*)p, encrypted_header_length);
  std::string headerstr;
  this->decrypt(encrypted_header, headerstr);
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
}

bool Filesystem::exists(std::string path) {
  return mPathToBytes.count(path) > 0;
}

std::string Filesystem::get_file(std::string path) {
  if (!this->exists(path)) error("path in file pack does not exist: " + path);

  const byte* p = mPathToBytes[path];
  size_t n = mPathToEncryptedSize[path];
  std::string ciphertext((const char*)p, n);
  std::string plaintext;
  this->decrypt(ciphertext, plaintext);
  return plaintext;
}

std::vector<std::string> Filesystem::get_file_list() {
  std::vector<std::string> files;
  for (auto iter : mPathToBytes) {
    files.push_back(iter.first);
  }
  return files;
}
