#include "stdafx.h"
#include "server.hpp"
#include "filesystem.hpp"

static std::string get_extension(std::string path) {
  size_t i = path.rfind(".");
  if (i == path.npos) return "";
  return std::string(path.begin() + i + 1, path.end());
}

Server::Server(Filesystem* filesystem)
    : mBoundPort(-1), mFilesystem(filesystem) {
  memset(&mRequestHeader, 0, sizeof(mRequestHeader));
  mMimeTypes[""] = "application/octet-stream";
  mMimeTypes["txt"] = "text/plain";
  mMimeTypes["css"] = "text/css";
  mMimeTypes["js"] = "application/javascript";
  mMimeTypes["html"] = "text/html";
  mMimeTypes["mp3"] = "audio/mpeg";
  mMimeTypes["mp4"] = "video/mp4";
  mMimeTypes["webm"] = "video/webm";
  mMimeTypes["png"] = "image/png";
  mMimeTypes["jpg"] = "image/jpeg";
  mMimeTypes["gif"] = "image/gif";
  
  for (auto file : mFilesystem->get_file_list()) {
    std::string ext = get_extension(file);
    if (mMimeTypes.count(ext) == 0) {
      std::cerr << "unrecognized extension: " << ext << std::endl;
      throw std::exception();
    }
  }
}

Server::~Server() {}

void Server::find_free_port_and_bind(struct mg_mgr* mgr) {
  for (int port = 9000; port < 65536; ++port) {
    std::stringstream ss;
    ss << "127.0.0.1:" << port;
    auto* conn = mg_bind(mgr, ss.str().c_str(), Server::ev_handler);
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

void Server::ev_handler(struct mg_connection* nc, int ev, void* ev_data) {
  Server* server = reinterpret_cast<Server*>(nc->mgr->user_data);
  server->ev_handler(nc, ev);
}

void Server::ev_handler(struct mg_connection* nc, int ev) {
  struct mbuf* io = &nc->recv_mbuf;

  switch (ev) {
    case MG_EV_RECV: {
      mRequestHeader += std::string(io->buf, io->len);
      mbuf_remove(io, io->len);

      static const int is_req = 1;
      int complete =
          mg_parse_http(mRequestHeader.c_str(), mRequestHeader.size(),
                        &mRequestMessage, is_req);
      if (!complete) return;

      std::string method =
          std::string(mRequestMessage.method.p, mRequestMessage.method.len);
      std::string uri =
          std::string(mRequestMessage.uri.p, mRequestMessage.uri.len);

      if (method == "GET") {
        this->get_file(uri, nc);
      } else {
        // in case we try to POST??
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

void Server::get_file(std::string uri, struct mg_connection* nc) {
  // requesting the root redirects to index.html
  if (uri == "/") uri = "/index.html";

  if (!mFilesystem->exists(uri)) {
    this->fail(nc);
    return;
  }

  std::string file = mFilesystem->get_file(uri);
  std::string ext = get_extension(uri);

  push(nc, "HTTP/1.1 200 OK\r\n");

  push(nc, "Content-Type: ");
  auto iter = mMimeTypes.find(ext);
  if (iter != mMimeTypes.end()) {
    push(nc, iter->second);
  } else {
    std::cerr << "unrecognized file extension for MIME type: " << ext << "\n";
    push(nc, "application/octet-stream");
  }
  push(nc, "\r\n");

  push(nc, "Content-Length: ");
  push(nc, boost::lexical_cast<std::string>(file.size()));
  push(nc, "\r\n");

  // terminating new line
  push(nc, "\r\n");

  push(nc, file);
}

void Server::fail(struct mg_connection* nc) {
  mg_printf(nc,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "\r\n");
}

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  const char* asset_pack = "files.enigma";
  if (args.size() == 2) asset_pack = args[1].c_str();

  Filesystem filesystem(asset_pack);

  struct mg_mgr mgr;
  Server server(&filesystem);
  mg_mgr_init(&mgr, &server);

  server.find_free_port_and_bind(&mgr);

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
