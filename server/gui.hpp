#pragma once

struct KeyRequestInfo {
  std::string basename;
  std::string filename;
  std::string path;
};

class KeyRequesterFrame : public wxFrame {
 public:
  KeyRequesterFrame(wxApp* parent, KeyRequestInfo info);
  ~KeyRequesterFrame();
  
  void RequestLicense(wxCommandEvent& event);
  
  DECLARE_EVENT_TABLE();
      
  enum {
    kButtonRequestLicense = wxID_HIGHEST + 1,
  };

 private:
  wxApp* mParent;
  const KeyRequestInfo mKeyRequestInfo;
  wxTextCtrl* mUsernameTextCtrl;
  wxTextCtrl* mPasswordTextCtrl;
};

class Gui : public wxFrame {
 public:
  Gui(const wxString& title);
  ~Gui();

 private:
};
