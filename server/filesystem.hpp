class Filesystem {
 public:
  Filesystem(std::wstring pack_path);
  ~Filesystem();

  void load_key(std::string path);

  bool exists(std::string path);
  std::string get_file(std::string path);

  void decrypt(const std::string& ciphertext, const std::string& key,
               std::string& plaintext, bool uncompress = true);
  std::vector<std::string> get_file_list();

 private:
  void load_pack(std::wstring pack_path);
  std::string decrypt_key(std::string encrypted_key);

  HANDLE mFileHandle;
  HANDLE mMapHandle;
  const byte* mBytes;
  size_t mSize;

  std::string mKey;
  std::vector<byte> mIV;

  const byte* mFileBytes;
  std::unordered_map<std::string, const byte*> mPathToBytes;
  std::unordered_map<std::string, size_t> mPathToEncryptedSize;
};
