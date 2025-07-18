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

#include <nakama-cpp/NTypes.h>
#include <nakama-cpp/URLParts.h>

#include <optional>
#include <string>

NAKAMA_NAMESPACE_BEGIN

typedef std::string Base64Buffer;

/**
 * Encode bytes buffer using `base64` algorithm
 *
 * @param buffer byte buffer to encode
 * @return std::string encoded `base64` string
 */
std::string base64Encode(const Base64Buffer& buffer);

/**
 * Encode bytes buffer using `base64Url` algorithm
 *
 * @param buffer byte buffer to encode
 * @return std::string encoded `base64Url` string
 */
std::string base64EncodeUrl(const Base64Buffer& buffer);

/**
 * Decode `base64Url` string
 *
 * @param base64str `base64Url` string
 * @return Base64Buffer byte buffer
 */
Base64Buffer base64DecodeUrl(const std::string& base64str);

/**
 * Encode string to allow use it in an URL
 *
 * @param str string to encode
 * @return std::string encoded string
 */
std::string encodeURIComponent(std::string decoded);

/**
 * returns true if a string starts with the specified prefix
 *
 * @param str The string
 * @param prefix The prefix to check
 * @return bool
 */
bool isStringStartsWith(const std::string& str, const std::string& prefix);

// Definitely not fully compliant, but good enough for us
std::optional<URLParts> ParseURL(const std::string& url);

NAKAMA_NAMESPACE_END
