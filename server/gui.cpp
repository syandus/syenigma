#include "stdafx.h"

#include "gui.hpp"
#include "keyrequester.hpp"

wxDECLARE_EVENT(StartServerEvent, wxCommandEvent);
wxDEFINE_EVENT(StartServerEvent, wxCommandEvent);

////////////////////////////////////////////////////////////////////////////////

KeyRequesterFrame::KeyRequesterFrame(wxApp* parent, KeyRequestInfo info)
    : mParent(parent),
      mKeyRequestInfo(info),
      wxFrame(NULL, wxID_ANY, "", wxDefaultPosition, wxDefaultSize) {
  auto panel = new wxPanel(this, wxID_ANY);

  auto sizer = new wxBoxSizer(wxVERTICAL);

  auto& font0 =
      wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  auto& font1 =
      wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

  auto line0 =
      new wxStaticText(panel, wxID_ANY, "Requesting License For Resource:",
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

Gui::Gui(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(250, 150)) {
  this->Centre();
}

Gui::~Gui() {}
