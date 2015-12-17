#pragma once

class Packer {
 public:
  Packer();
  ~Packer();

  bool pack(std::string input_directory, std::string output_file);
  void encrypt(const std::string& plaintext, byte* key, size_t key_len,
               byte* iv, size_t iv_len, std::string& output);

 private:
};
