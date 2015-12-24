#include "stdafx.h"

#include "keyrequester.hpp"

#include "mongoose.h"

// change this for your org
static const char* kQueryUrl = "http://msdscience.com/query.php";

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

KeyRequester::KeyRequester(const std::string& resource_name,
                           const std::string& username,
                           const std::string& password)
    : mResourceName(resource_name),
      mUsername(username),
      mPassword(password),
      mDone(true),
      mSuccess(false) {
  mManager = (struct mg_mgr*)malloc(sizeof(struct mg_mgr));
  mg_mgr_init(mManager, this);
  this->perform_request();
}

KeyRequester::~KeyRequester() {
  mg_mgr_free(mManager);
  free(mManager);
}

void KeyRequester::perform_request() {
  ptree pt;
  pt.put("action", "request_syenigma_key");
  pt.put("resource", mResourceName);
  pt.put("username", mUsername);
  pt.put("password", mPassword);
  pt.put("machine_id", GetMachineId());
  std::ostringstream buf;
  write_json(buf, pt, false);
  std::string json = buf.str();

  mDone = false;
  mg_connect_http(mManager, KeyRequester::s_ev_handler, kQueryUrl, NULL,
                  json.c_str());
}

void KeyRequester::s_ev_handler(struct mg_connection* nc, int ev,
                                void* ev_data) {
  KeyRequester* server = reinterpret_cast<KeyRequester*>(nc->mgr->user_data);
  server->ev_handler(nc, ev, ev_data);
}

void KeyRequester::poll() {
  while (!mDone) {
    mg_mgr_poll(mManager, 1000);
  }
}

void KeyRequester::ev_handler(struct mg_connection* nc, int ev, void* ev_data) {
  struct http_message* hm = (struct http_message*)ev_data;

  switch (ev) {
    case MG_EV_CONNECT: {
      if (*(int*)ev_data != 0) {
        mDone = 1;
        error(
            "Failed to contact license authorization server. Maybe you are "
            "offline?");
      }
      break;
    }
    case MG_EV_HTTP_REPLY: {
      nc->flags |= MG_F_CLOSE_IMMEDIATELY;
      std::string json(hm->body.p, hm->body.len);
      ptree pt;
      std::istringstream is(json);
      std::string base64_key;
      try {
        read_json(is, pt);
        int success = pt.get<int>("success");
        if (!success) {
          mSuccess = false;
          mFailureReason = pt.get<std::string>("reason");
          mDone = true;
          return;
        } else {
          mSuccess = true;
          base64_key = pt.get<std::string>("key");
        }
      } catch (...) {
        mSuccess = false;
        mFailureReason =
            "KeyRequester crashed due to malformed data. Maybe you have to use "
            "a browser to login to the network?";
      }
      
      mKey = base64_decode(base64_key);

      mDone = true;
      break;
    }
    default:
      break;
  }
}

bool KeyRequester::success() { return mSuccess; }

std::string KeyRequester::get_failure_reason() { return mFailureReason; }

std::string KeyRequester::get_key() { return mKey; }
