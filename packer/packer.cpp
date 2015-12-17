#include "stdafx.h"

#include "packer.hpp"

namespace fs = boost::filesystem;

Packer::Packer() {}

Packer::~Packer() {}

static void unixify(std::string& path) { boost::replace_all(path, "\\", "/"); }

bool Packer::pack(std::string input_directory, std::string output_file) {
  fs::path root(input_directory);
  if (!fs::is_directory(root)) {
    std::cerr << "given [input_directory] is not a directory: "
              << input_directory << "\n";
    return false;
  }

  byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], iv[CryptoPP::AES::BLOCKSIZE];
  memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
  memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

  std::queue<fs::path> q;
  q.push(root);
  std::vector<std::string> files;

  std::ofstream enigma_file(output_file, std::ios::binary);
  enigma_file << "SYENIGMA";
  std::stringstream header;
  std::string ciphertexts;

  while (!q.empty()) {
    fs::path path = q.front();
    q.pop();

    boost::system::error_code ec;
    auto end = fs::directory_iterator();
    for (fs::directory_iterator iter(path); iter != end; iter.increment(ec)) {
      fs::path child_path = iter->path();
      if (fs::is_directory(child_path)) {
        q.push(child_path);
      } else {
        std::string file_path = child_path.generic_string();
        std::ifstream ifs(file_path, std::ifstream::binary);
        std::string data((std::istreambuf_iterator<char>(ifs)),
                         (std::istreambuf_iterator<char>()));

        std::string packed_path = file_path.substr(input_directory.size());
        unixify(packed_path);
        files.push_back(file_path);

        std::string ciphertext;
        this->encrypt(data, key, CryptoPP::AES::DEFAULT_KEYLENGTH, iv,
                      CryptoPP::AES::BLOCKSIZE, ciphertext);

        std::cout << packed_path << "\n";
        header.write(packed_path.c_str(), packed_path.size());
        header.write("\0", 1);

        // this is not necessary, as the pointer is simply the size of the
        // previous data block
        /*
        // pointer offset from data block
        size_t pos = ciphertexts.size();
        header.write(reinterpret_cast<const char*>(&pos),
                          sizeof(pos));
                          */

        // size of cipher text in data block
        size_t ciphertext_len = ciphertext.size();
        header.write(reinterpret_cast<const char*>(&ciphertext_len),
                     sizeof(ciphertext_len));

        // ciphertext and plaintext size are different
        /* std::cout << data.size() << " " << ciphertext.size() << "\n"; */
        ciphertexts += ciphertext;
      }
    }
  }

  const auto& headerstr = header.str();
  std::string encrypted_header;
  this->encrypt(headerstr, key, CryptoPP::AES::DEFAULT_KEYLENGTH, iv,
                CryptoPP::AES::BLOCKSIZE, encrypted_header);

  size_t encrypted_header_len = encrypted_header.size();
  enigma_file.write(reinterpret_cast<const char*>(&encrypted_header_len),
                    sizeof(encrypted_header_len));
  enigma_file.write(encrypted_header.c_str(), encrypted_header.size());

  enigma_file.write(ciphertexts.c_str(), ciphertexts.size());

  return true;
}

void Packer::encrypt(const std::string& plaintext, byte* key, size_t key_len,
                     byte* iv, size_t iv_len, std::string& ciphertext) {
  std::string compressed_plaintext;
  snappy::Compress(plaintext.c_str(), plaintext.size(), &compressed_plaintext);

  CryptoPP::AES::Encryption aes_encryption(key, key_len);
  CryptoPP::CBC_Mode_ExternalCipher::Encryption cbc_encryption(aes_encryption,
                                                               iv);

  CryptoPP::StreamTransformationFilter stf_encryptor(
      cbc_encryption, new CryptoPP::StringSink(ciphertext));
  stf_encryptor.Put(
      reinterpret_cast<const unsigned char*>(compressed_plaintext.c_str()),
      compressed_plaintext.length());
  stf_encryptor.MessageEnd();
}
int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  if (args.size() != 3) {
    std::cout << args[0] << " [input directory] [output file]\n";
    return 0;
  }

  std::string input_directory = args[1];
  std::string output_file = args[2];
  Packer packer;
  packer.pack(input_directory, output_file);
  return 0;
}
