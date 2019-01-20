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

#pragma once

#include <string>

namespace Nakama {

    typedef std::string Base64Buffer;

    std::string base64_encode(const Base64Buffer& buffer);

    Base64Buffer base64_decode(const std::string& base64str);

    std::string getJsonFieldValue(const std::string& json, const std::string& field_name);

    std::string url_encode(const std::string& str);

} // namespace Nakama
