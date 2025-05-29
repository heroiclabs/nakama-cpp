/*
 * Copyright 2019 The Nakama Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "StrUtil.h"
#include <google/protobuf/stubs/strutil.h>
#include <sstream>

#include <regex>

namespace Nakama {

using namespace std;

std::string base64Encode(const Base64Buffer& buffer) {
  std::string base64str;

  google::protobuf::Base64Escape(
      (const unsigned char*)buffer.data(), static_cast<int>(buffer.size()), &base64str, true);

  return base64str;
}

std::string base64EncodeUrl(const Base64Buffer& buffer) {
  std::string base64str;

  google::protobuf::WebSafeBase64Escape(
      (const unsigned char*)buffer.data(), static_cast<int>(buffer.size()), &base64str, true);

  return base64str;
}

Base64Buffer base64DecodeUrl(const std::string& base64str) {
  Base64Buffer buffer;

  google::protobuf::WebSafeBase64Unescape(base64str, &buffer);

  return buffer;
}

std::string encodeURIComponent(std::string decoded) {
  std::ostringstream oss;
  std::regex r("[-.0-9A-Za-z_~]");

  for (char c : decoded) {
    if (std::regex_match(std::string(1, c), r)) {
      oss << c;
    } else {
      oss << '%' << std::uppercase << std::hex << static_cast<uint16_t>(0xff & c);
    }
  }
  return oss.str();
}

bool isStringStartsWith(const string& str, const string& prefix) {
  bool res = false;

  if (str.size() >= prefix.size()) {
    res = (str.compare(0, prefix.size(), prefix) == 0);
  }

  return res;
}

opt::optional<URLParts> ParseURL(const string& url) {
  const std::regex re("([^:]+)://([^:/]+)(:([0-9]+))?/(.+)", std::regex::extended);
  std::smatch m;
  if (!std::regex_match(url, m, re)) {
    return opt::nullopt;
  }

  opt::optional<uint16_t> port(opt::nullopt);
  if (m[4].length() > 0) {
    auto portNum = std::strtoul(m[4].str().c_str(), nullptr, 10);
    if (portNum > 0 && portNum <= std::numeric_limits<uint16_t>::max()) {
      port = static_cast<uint16_t>(portNum);
    } else {
      return opt::nullopt;
    }
  }

  URLParts parts{
      m[1].str(), // scheme
      m[2].str(), // host
      port,       // port
      m[5].str(), // pathAndArgs
      url         // url
  };
  return opt::make_optional(parts);
}

} // namespace Nakama
