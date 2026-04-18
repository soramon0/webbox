#include "mime_type.h"

std::string get_content_type(const std::string &file_path) {
  static std::map<std::string, std::string> mime_map;

  if (mime_map.empty()) {
    mime_map[".css"] = "text/css";
    mime_map[".csv"] = "text/csv";
    mime_map[".gif"] = "image/gif";
    mime_map[".htm"] = "text/html";
    mime_map[".html"] = "text/html";
    mime_map[".ico"] = "image/x-icon";
    mime_map[".jpeg"] = "image/jpeg";
    mime_map[".jpg"] = "image/jpeg";
    mime_map[".js"] = "application/javascript";
    mime_map[".json"] = "application/json";
    mime_map[".png"] = "image/png";
    mime_map[".pdf"] = "application/pdf";
    mime_map[".svg"] = "image/svg+xml";
    mime_map[".txt"] = "text/plain";
  }

  size_t pos = file_path.rfind(".");
  if (pos != std::string::npos) {
    std::string ext = file_path.substr(pos);

    for (size_t i = 0; i < ext.length(); ++i) {
      ext[i] = static_cast<char>(tolower(ext[i]));
    }

    std::map<std::string, std::string>::const_iterator it = mime_map.find(ext);
    if (it != mime_map.end()) {
      return it->second;
    }
  }

  return "application/octet-stream";
}
