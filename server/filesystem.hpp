class Filesystem {
 public:
  Filesystem(std::string pack_path);
  ~Filesystem();
  
  bool exists(std::string path);
  std::string get_file(std::string path);
  void decrypt(const std::string& ciphertext, std::string& plaintext);
  std::vector<std::string> get_file_list();

 private:
  void load_pack(std::string pack_path);
  void read_header();
  
  std::vector<byte> mKey;
  std::vector<byte> mIV;
  
  const std::string mPackPath;
  boost::interprocess::file_mapping mFileMapping;
  boost::interprocess::mapped_region mMappedRegion;
  
  const byte* mBytes;
  size_t mSize;
  
  const byte* mFileBytes;
  std::unordered_map<std::string, const byte*> mPathToBytes;
  std::unordered_map<std::string, size_t> mPathToEncryptedSize;
};
