#include "stdafx.h"

#include "filesystem.hpp"
#include "server.hpp"
#include "gui.hpp"

static const char* kCompanyName = "Syandus";

namespace fs = boost::filesystem;

std::string get_my_basename() {
  wchar_t buf[4096];
  GetModuleFileNameW(NULL, buf, 4096);
  std::string path = to_utf8(buf);
  path = fs::path(path).filename().generic_string();
  size_t i = path.find(".");
  if (i == path.npos)
    return path;
  else
    return std::string(path.begin(), path.begin() + i);
}

static std::string confirm_app_data_directory() {
  wchar_t* wpath;
  auto ret = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &wpath);
  if (ret != S_OK) {
    error("failed to find Local AppData folder");
  }
  std::string path = to_utf8(wpath);
  CoTaskMemFree(wpath);

  path += "/";
  path += kCompanyName;
  path += "/";
  path += get_my_basename();
  // Important if there is unicode / CJK characters!
  std::wstring final_path = to_utf16(path);
  fs::path p(final_path);
  fs::create_directories(p);
  return path;
}

static std::wstring locate_asset_pack() {
  wchar_t buf[4096];
  GetModuleFileNameW(NULL, buf, 4096);
  std::wstring module_path(buf);
  fs::path p(module_path);
  p = p.parent_path();

  boost::system::error_code ec;
  auto end = fs::directory_iterator();
  for (fs::directory_iterator iter(p); iter != end; iter.increment(ec)) {
    fs::path child = iter->path();
    if (!fs::is_regular_file(child)) continue;
    std::wstring filename = child.generic_wstring();
    auto pos = filename.find(L".enigma");
    if (pos == std::wstring::npos) continue;
    if (pos + strlen(".enigma") != filename.size()) continue;
    return filename;
  }
  error("could not find enigma file pack");
  return L"";
}

wxDECLARE_EVENT(StartServerEvent, wxCommandEvent);

class MyApp;
class PollTimer : public wxTimer {
 public:
  PollTimer(MyApp* app) : mApp(app) {}
  ~PollTimer() {}
  virtual void Notify();

 private:
  MyApp* mApp;
};

class MyApp : public wxApp {
 public:
  MyApp() { Bind(StartServerEvent, &MyApp::StartServer, this); }
  ~MyApp() {}

  virtual bool OnInit() {
    std::string data_directory = confirm_app_data_directory();
    std::wstring asset_pack = locate_asset_pack();
    mAssetPackPath = asset_pack;

    std::string filename_asset_pack = to_utf8(
        fs::path(asset_pack.c_str()).filename().generic_wstring().c_str());
    size_t index = filename_asset_pack.find(".");
    std::string basename(filename_asset_pack.begin(),
                         filename_asset_pack.begin() + index);

    std::string key_file = basename + ".key";
    std::string key_file_path = data_directory + "/" + key_file;
    mKeyFilePath = key_file_path;

    if (!fs::exists(fs::path(to_utf16(key_file_path)))) {
      KeyRequestInfo info;
      info.basename = basename;
      info.filename = key_file;
      info.path = key_file_path;
      std::string title = "Requesting License For Resource: " + basename;
      auto frame = new KeyRequesterFrame(this, info);
      frame->Show();
    } else {
      wxCommandEvent event(StartServerEvent);
      wxPostEvent(this, event);
    }

    return true;
  }
  void StartServer(wxCommandEvent& event) {
    mFilesystem = std::make_unique<Filesystem>(mAssetPackPath);
    mFilesystem->load_key(mKeyFilePath);

    mServer = std::make_unique<Server>(mFilesystem.get());
    mServer->find_free_port_and_bind();

    auto gui = new Gui(wxT("Simple"));
    gui->Show();

    mPollTimer = std::make_unique<PollTimer>(this);
    mPollTimer->SetOwner(this);
    mPollTimer->Start(100);
  }
  void poll() {
    if (mServer) mServer->poll();
  }

 private:
  std::unique_ptr<Filesystem> mFilesystem;
  std::unique_ptr<Server> mServer;
  std::wstring mAssetPackPath;
  std::string mKeyFilePath;
  std::unique_ptr<PollTimer> mPollTimer;
};

void PollTimer::Notify() { mApp->poll(); }

IMPLEMENT_APP(MyApp)
