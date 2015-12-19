#pragma once

struct mg_connection;
struct mg_mgr;

class KeyRequester {
 public:
  static void s_ev_handler(struct mg_connection* nc, int ev, void* ev_data);

  KeyRequester(const std::string& resource_name, const std::string& username,
               const std::string& password);
  ~KeyRequester();

  void poll();

  bool success();
  std::string get_failure_reason();
  std::string get_key();

 private:
  void perform_request();
  void ev_handler(struct mg_connection* nc, int ev, void* ev_data);

  const std::string mResourceName;
  const std::string mUsername;
  const std::string mPassword;
  
  struct mg_mgr* mManager;
  bool mDone;
  bool mSuccess;
  std::string mFailureReason;
  std::string mKey;
};
