#pragma once

class Filesystem;

class Server {
 public:
  static void ev_handler(struct mg_connection* nc, int ev, void* p);
  
  Server(Filesystem* filesystem);
  ~Server();
  
  void find_free_port_and_bind(struct mg_mgr* mgr);
  std::string get_url();
  
 private:
  void ev_handler(struct mg_connection* nc, int ev);
  void get_file(std::string uri, struct mg_connection* nc);
  void fail(struct mg_connection* nc);
  void push(struct mg_connection* nc, const char* str);
  void push(struct mg_connection* nc, const std::string& s);
  
  Filesystem* mFilesystem;
  int mBoundPort;
  std::string mRequestHeader;
  http_message mRequestMessage;
  std::unordered_map<std::string, std::string> mMimeTypes; 
};
