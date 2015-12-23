#include "stdafx.h"

#include "gui.hpp"
#include "keyrequester.hpp"
#include "server.hpp"

wxDECLARE_EVENT(StartServerEvent, wxCommandEvent);
wxDEFINE_EVENT(StartServerEvent, wxCommandEvent);

////////////////////////////////////////////////////////////////////////////////

KeyRequesterFrame::KeyRequesterFrame(wxApp* parent, KeyRequestInfo info)
    : mParent(parent),
      mKeyRequestInfo(info),
      wxFrame(NULL, wxID_ANY, "MSD Scientific Education", wxDefaultPosition,
              wxDefaultSize,
              wxDEFAULT_FRAME_STYLE & (~wxRESIZE_BORDER) & (~wxMAXIMIZE_BOX)) {
  this->SetIcon(wxICON(IDI_ICON1));
  auto panel = new wxPanel(this, wxID_ANY);

  auto sizer = new wxBoxSizer(wxVERTICAL);

  auto& font0 =
      wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  auto& font1 =
      wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

  auto line0 =
      new wxStaticText(panel, wxID_ANY, "Requesting Access For Resource:",
                       wxDefaultPosition, wxDefaultSize);
  line0->SetFont(font0);
  sizer->Add(line0, 0, wxALIGN_CENTER | wxALL, 8);

  auto line1 = new wxStaticText(panel, wxID_ANY, mKeyRequestInfo.basename,
                                wxDefaultPosition, wxDefaultSize,
                                wxALIGN_CENTRE_HORIZONTAL);
  line1->SetFont(font1);
  sizer->Add(line1, 0, wxEXPAND | wxALL, 8);

  auto static_line = new wxStaticLine(panel, wxID_ANY);
  sizer->Add(static_line, 0, wxEXPAND | wxALL, 8);

  auto username_label =
      new wxStaticText(panel, wxID_ANY, "Username:", wxDefaultPosition,
                       wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
  sizer->Add(username_label, 0, wxALIGN_CENTER);
  auto username =
      new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
  username->SetFont(font0);
  sizer->Add(username, 0, wxEXPAND | wxALL, 4);
  mUsernameTextCtrl = username;

  auto password_label =
      new wxStaticText(panel, wxID_ANY, "Password:", wxDefaultPosition,
                       wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
  sizer->Add(password_label, 0, wxALIGN_CENTER);
  auto password =
      new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
  password->SetFont(font0);
  sizer->Add(password, 0, wxEXPAND | wxALL, 4);
  mPasswordTextCtrl = password;

  auto submit = new wxButton(panel, kButtonRequestLicense, "Request License",
                             wxDefaultPosition, wxSize(100, 44));
  sizer->Add(submit, 0, wxALIGN_CENTER | wxALL, 4);

  auto help = new wxStaticText(
      panel, wxID_ANY, "For technical support, contact admin@msdscience.com",
      wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
  sizer->Add(help, 0, wxEXPAND | wxALL, 8);

  panel->SetSizerAndFit(sizer);
  sizer->SetSizeHints(this);
  this->Centre();
}

KeyRequesterFrame::~KeyRequesterFrame() {}

BEGIN_EVENT_TABLE(KeyRequesterFrame, wxFrame)
EVT_BUTTON(kButtonRequestLicense, KeyRequesterFrame::RequestLicense)
END_EVENT_TABLE()

void KeyRequesterFrame::RequestLicense(wxCommandEvent& event) {
  std::string username(mUsernameTextCtrl->GetValue().utf8_str());
  std::string password(mPasswordTextCtrl->GetValue().utf8_str());
  KeyRequester requester(mKeyRequestInfo.basename, username, password);
  requester.poll();
  if (requester.success()) {
    std::ofstream ofs(mKeyRequestInfo.path, std::ios::binary);
    ofs << requester.get_key();
    ofs.close();

    this->Close();

    wxCommandEvent event(StartServerEvent);
    wxPostEvent(mParent, event);
  } else {
    auto msgbox = std::make_unique<wxMessageDialog>(
        this, requester.get_failure_reason(), "License Request Failed",
        wxOK | wxCENTRE | wxICON_ERROR);
    msgbox->ShowModal();
  }

  return;
}

////////////////////////////////////////////////////////////////////////////////

Gui::Gui(wxApp* parent, KeyRequestInfo info, Server* server)
    : mParent(parent),
      mKeyRequestInfo(info),
      mServer(server),
      wxFrame(NULL, wxID_ANY, "MSD Scientific Education", wxDefaultPosition,
              wxSize(250, 150),
              wxDEFAULT_FRAME_STYLE & (~wxRESIZE_BORDER) & (~wxMAXIMIZE_BOX)) {
  this->SetIcon(wxICON(IDI_ICON1));
  auto panel = new wxPanel(this, wxID_ANY);
  auto sizer = new wxBoxSizer(wxVERTICAL);

  auto& font0 =
      wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  auto& font1 =
      wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

  auto line0 = new wxStaticText(panel, wxID_ANY, "Resource:", wxDefaultPosition,
                                wxDefaultSize);
  line0->SetFont(font0);
  sizer->Add(line0, 0, wxALIGN_CENTER | wxALL, 8);

  auto line1 = new wxStaticText(panel, wxID_ANY, mKeyRequestInfo.basename,
                                wxDefaultPosition, wxDefaultSize,
                                wxALIGN_CENTRE_HORIZONTAL);
  line1->SetFont(font1);
  sizer->Add(line1, 0, wxEXPAND | wxALL, 8);

  auto button_sizer = new wxBoxSizer(wxHORIZONTAL);
  sizer->Add(button_sizer, 0, wxALIGN_CENTER | wxALL, 4);

  auto about =
      new wxButton(panel, kAbout, "About", wxDefaultPosition, wxSize(100, 44));
  button_sizer->Add(about, 0, wxALIGN_CENTER | wxALL, 4);

  auto open_page = new wxButton(panel, kOpenWebPage, "Open Resource",
                                wxDefaultPosition, wxSize(100, 44));
  button_sizer->Add(open_page, 0, wxALIGN_CENTER | wxALL, 4);
  open_page->SetFocus();

  auto warning = new wxStaticText(
      panel, wxID_ANY,
      "Note: Do not close this program \nuntil you are finished using this "
      "resource in your browser.",
      wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
  sizer->Add(warning, 0, wxEXPAND | wxALL, 8);

  panel->SetSizerAndFit(sizer);
  sizer->SetSizeHints(this);
  this->Centre();
}

Gui::~Gui() {}

BEGIN_EVENT_TABLE(Gui, wxFrame)
EVT_BUTTON(kOpenWebPage, Gui::OpenWebPage)
EVT_BUTTON(kAbout, Gui::OpenAbout)
END_EVENT_TABLE()

static LONG GetStringRegKey(HKEY hKey, const std::wstring& strValueName,
                            std::wstring& strValue,
                            const std::wstring& strDefaultValue) {
  strValue = strDefaultValue;
  WCHAR szBuffer[1024];
  DWORD dwBufferSize = sizeof(szBuffer);
  ULONG nError;
  nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL,
                            (LPBYTE)szBuffer, &dwBufferSize);
  if (ERROR_SUCCESS == nError) {
    strValue = szBuffer;
  }
  return nError;
}

void Gui::OpenWebPage(wxCommandEvent& event) {
  std::wstring browser;
  HKEY hkey;
  RegOpenKeyEx(HKEY_CLASSES_ROOT, L"http\\shell\\open\\command", 0, KEY_READ,
               &hkey);
  GetStringRegKey(hkey, L"", browser, L"");
  std::wstring url(to_utf16(mServer->get_url()).c_str());
  if (browser.empty()) {
    // this buggy and sometimes doesn't work. it actually mysteriously blocks
    // the program. maybe one of those things that was caused by the debugger
    // hanging up Chrome interop and requires a reboot?
    ShellExecute(0, L"open", url.c_str(), 0, 0, SW_SHOW);
  } else {
    std::wstring cmd;
    if (browser.find(L"%1") != std::wstring::npos) {
      boost::replace_all(browser, L"%1", url);
      cmd = browser;
    } else {
      cmd = browser + L" " + url;
    }
    auto dummy = cmd.c_str();
    STARTUPINFO sui;
    PROCESS_INFORMATION pi;
    ZeroMemory(&sui, sizeof(sui));
    sui.cb = sizeof(sui);
    ::CreateProcess(NULL, &cmd[0], NULL, NULL, NULL, FALSE, 0, NULL, &sui, &pi);
  }
}

void Gui::OpenAbout(wxCommandEvent& event) {
  namespace fs = boost::filesystem;
  wchar_t buf[4096];
  GetModuleFileNameW(NULL, buf, 4096);
  std::wstring module_path(buf);
  fs::path p(module_path);
  p = p.parent_path();
  p /= L"LICENSES.txt";

  ShellExecute(0, L"open", p.generic_wstring().c_str(), 0, 0, SW_SHOW);
}
