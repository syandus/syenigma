class Filesystem {
 public:
  Filesystem(std::wstring pack_path);
  ~Filesystem();
  
  bool exists(std::string path);
  void load_key(std::string path);
  std::string get_file(std::string path);
  void decrypt(const std::string& ciphertext, std::string& plaintext);
  std::vector<std::string> get_file_list();

 private:
  void load_pack(std::wstring pack_path);
  
  HANDLE mFileHandle;
  HANDLE mMapHandle;
  const byte* mBytes;
  size_t mSize;
  
  std::vector<byte> mKey;
  std::vector<byte> mIV;
  
  const byte* mFileBytes;
  std::unordered_map<std::string, const byte*> mPathToBytes;
  std::unordered_map<std::string, size_t> mPathToEncryptedSize;
};
