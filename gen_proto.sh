#!/bin/bash

# Copyright 2020 The Nakama Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

shopt -s extglob

### TODO entire platform specific protoc version depending on host machine.
./protoc-3.14.0-win64/bin/protoc -Iproto --cpp_out=./src/api \
./proto/github.com/heroiclabs/nakama-common/api/api.proto \
./proto/github.com/heroiclabs/nakama-common/rtapi/realtime.proto \
./proto/apigrpc.proto \
./proto/google/protobuf/any.proto \
./proto/google/protobuf/descriptor.proto \
./proto/google/protobuf/empty.proto \
./proto/google/protobuf/struct.proto \
./proto/google/protobuf/timestamp.proto \
./proto/google/protobuf/wrappers.proto \
./proto/google/api/annotations.proto \
./proto/google/api/http.proto \
./proto/protoc-gen-openapiv2/options/annotations.proto \
./proto/protoc-gen-openapiv2/options/openapiv2.proto