#include "stdafx.h"
#include "server.hpp"
#include "filesystem.hpp"

#include "mongoose.h"

Server::Server(Filesystem* filesystem)
    : mManager(nullptr), mFilesystem(filesystem), mBoundPort(-1) {
  mManager = (struct mg_mgr*)malloc(sizeof(struct mg_mgr));
  mg_mgr_init(mManager, this);

  mMimeTypes[""] = "application/octet-stream";
  mMimeTypes["css"] = "text/css";
  mMimeTypes["eot"] = "application/vnd.ms-fontobject";
  mMimeTypes["gif"] = "image/gif";
  mMimeTypes["html"] = "text/html";
  mMimeTypes["jpg"] = "image/jpeg";
  mMimeTypes["js"] = "application/javascript";
  mMimeTypes["less"] = "text/css";
  mMimeTypes["map"] = "application/json";
  mMimeTypes["md"] = "text/markdown";
  mMimeTypes["mp3"] = "audio/mpeg";
  mMimeTypes["mp4"] = "video/mp4";
  mMimeTypes["otf"] = "application/font-otf";
  mMimeTypes["png"] = "image/png";
  mMimeTypes["sh"] = "text/x-shellscript";
  mMimeTypes["svg"] = "image/svg+xml";
  mMimeTypes["swf"] = "application/x-shockwave-flash";
  mMimeTypes["ttf"] = "application/font-ttf";
  mMimeTypes["txt"] = "text/plain";
  mMimeTypes["vtt"] = "text/vtt";
  mMimeTypes["webm"] = "video/webm";
  mMimeTypes["woff"] = "application/font-woff";
  mMimeTypes["woff2"] = "application/font-woff2";
  mMimeTypes["xml"] = "application/xml";
  mMimeTypes["zip"] = "application/octet-stream";
  // custom
  mMimeTypes["sql"] = "text/plain";

  for (auto file : mFilesystem->get_file_list()) {
    std::string ext = get_extension(file);
    if (mMimeTypes.count(ext) == 0) {
      std::stringstream ss;
      ss << "unrecognized extension: " << ext << std::endl;
      OutputDebugStringA(ss.str().c_str());
    }
  }
}

Server::~Server() {
  mg_mgr_free(mManager);
  free(mManager);
}

void Server::find_free_port_and_bind() {
  for (int port = 9000; port < 65536; ++port) {
    std::stringstream ss;
    ss << "127.0.0.1:" << port;
    auto* conn = mg_bind(mManager, ss.str().c_str(), Server::s_ev_handler);
    if (conn) {
      mBoundPort = port;
      break;
    }
  }
}

std::string Server::get_url() {
  std::stringstream ss;
  ss << "http://localhost:" << mBoundPort;
  return ss.str();
}

void Server::s_ev_handler(struct mg_connection* nc, int ev, void* ev_data) {
  Server* server = reinterpret_cast<Server*>(nc->mgr->user_data);
  server->ev_handler(nc, ev, ev_data);
}

// TODO: eliminate usage of member variables, or at least convert them TLS
// variables, so we can use multithreading
void Server::ev_handler(struct mg_connection* nc, int ev, void* ev_data) {
  struct mbuf* io = &nc->recv_mbuf;

  switch (ev) {
    case MG_EV_RECV: {
      mRequestHeader += std::string(io->buf, io->len);
      mbuf_remove(io, io->len);

      http_message request_message;
      static const int is_req = 1;
      int complete =
          mg_parse_http(mRequestHeader.c_str(), mRequestHeader.size(),
                        &request_message, is_req);
      if (!complete) return;
      /* OutputDebugStringA(mRequestHeader.c_str()); */

      std::string method =
          std::string(request_message.method.p, request_message.method.len);
      std::string uri =
          std::string(request_message.uri.p, request_message.uri.len);

      std::vector<uint64_t> range;
      for (int i = 0; i < MG_MAX_HTTP_HEADERS; ++i) {
        struct mg_str namestr = request_message.header_names[i];
        struct mg_str valuestr = request_message.header_values[i];
        if (!namestr.p) continue;
        std::string name(namestr.p, namestr.len);
        std::string value(valuestr.p, valuestr.len);
        if (name == "Range") {
          std::vector<std::string> tokens;
          boost::split(tokens, value, boost::is_any_of("="));
          if (tokens.size() == 2 && tokens[0] == "bytes") {
            std::string bytes = tokens[1];
            tokens.clear();
            boost::split(tokens, bytes, boost::is_any_of("-"));
            for (auto token : tokens) {
              if (!token.empty())
                range.push_back(boost::lexical_cast<uint64_t>(token));
            }
          }
        }
      }

      if (method == "GET") {
        this->get_file(uri, range, nc);
      } else {
        // in case we browser tries to POST??
        this->fail(nc);
      }

      mRequestHeader.clear();
      memset(&mRequestHeader, 0, sizeof(mRequestHeader));
      break;
    }
    default:
      break;
  }
}

void Server::push(struct mg_connection* nc, const char* str) {
  mg_send(nc, str, ::strlen(str));
}

void Server::push(struct mg_connection* nc, const std::string& s) {
  mg_send(nc, s.c_str(), s.size());
}

void Server::get_file(std::string uri, std::vector<uint64_t> range,
                      struct mg_connection* nc) {
  // requesting the root redirects to index.html
  if (uri == "/") uri = "/index.html";

  if (!mFilesystem->exists(uri)) {
    this->fail(nc);
    return;
  }

  std::string file = mFilesystem->get_file(uri);
  std::string ext = get_extension(uri);

  if (range.size() > 0) {
    push(nc, "HTTP/1.1 206 Partial Content\r\n");
  } else {
    push(nc, "HTTP/1.1 200 OK\r\n");
  }

  push(nc, "Content-Type: ");
  auto iter = mMimeTypes.find(ext);
  if (iter != mMimeTypes.end()) {
    push(nc, iter->second);
  } else {
    std::cerr << "unrecognized file extension for MIME type: " << ext << "\n";
    push(nc, "application/octet-stream");
  }
  push(nc, "\r\n");

  push(nc, "Cache-Control: max-age=3600");
  push(nc, "\r\n");

  if (range.size() > 0) {
    push(nc, "Accept-Ranges: bytes");
    push(nc, "\r\n");

    uint64_t n = file.size();
    uint64_t begin = range[0];
    uint64_t end = n - 1;
    if (range.size() >= 2) {
      end = range[1];
    }
    
    uint64_t len = end - begin + 1;
    push(nc, "Content-Length: ");
    push(nc, boost::lexical_cast<std::string>(len));
    push(nc, "\r\n");

    std::stringstream ss;
    ss << "Content-Range: bytes " << begin << "-" << end << "/" << n << "\r\n";
    push(nc, ss.str());
    
    file = file.substr(begin, len);
  } else {
    push(nc, "Content-Length: ");
    push(nc, boost::lexical_cast<std::string>(file.size()));
    push(nc, "\r\n");
  }

  // terminating new line
  push(nc, "\r\n");
  push(nc, file);
}

void Server::fail(struct mg_connection* nc) {
  mg_printf(nc,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "\r\n");
  /* OutputDebugStringA("404\n"); */
}

void Server::poll() { mg_mgr_poll(mManager, 0); }
