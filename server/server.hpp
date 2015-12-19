#pragma once

class Filesystem;
struct mg_connection;
struct mg_mgr;

class Server {
 public:
  static void s_ev_handler(struct mg_connection* nc, int ev, void* ev_data);
  
  Server(Filesystem* filesystem);
  ~Server();
  
  void find_free_port_and_bind();
  std::string get_url();
  
  void poll();
  
 private:
  void ev_handler(struct mg_connection* nc, int ev, void* ev_data);
  void get_file(std::string uri, struct mg_connection* nc);
  void fail(struct mg_connection* nc);
  void push(struct mg_connection* nc, const char* str);
  void push(struct mg_connection* nc, const std::string& s);
  
  struct mg_mgr* mManager;
  Filesystem* mFilesystem;
  int mBoundPort;
  std::string mRequestHeader;
  std::unordered_map<std::string, std::string> mMimeTypes; 
};
