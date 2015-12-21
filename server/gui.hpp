#pragma once

class Server;

struct KeyRequestInfo {
  std::string basename;
  std::string filename;
  std::string path;
};

enum {
  kButtonRequestLicense = wxID_HIGHEST + 1,
  kOpenWebPage,
  kAbout,
};

class KeyRequesterFrame : public wxFrame {
 public:
  KeyRequesterFrame(wxApp* parent, KeyRequestInfo info);
  ~KeyRequesterFrame();
  
  void RequestLicense(wxCommandEvent& event);
  
  DECLARE_EVENT_TABLE();
      
 private:
  wxApp* mParent;
  const KeyRequestInfo mKeyRequestInfo;
  wxTextCtrl* mUsernameTextCtrl;
  wxTextCtrl* mPasswordTextCtrl;
};

class Gui : public wxFrame {
 public:
  Gui(wxApp* parent, KeyRequestInfo info, Server* server);
  ~Gui();
  
  void OpenWebPage(wxCommandEvent& event);
  void OpenAbout(wxCommandEvent& event);
  
  DECLARE_EVENT_TABLE();

 private:
  wxApp* mParent;
  const KeyRequestInfo mKeyRequestInfo;
  Server* mServer;
};
