To generate the interface, you need the repositories nakama and satori in the same parent folder as this repository.

On nakama:
    Successfully run the README.md section called `### Full Source Builds` at least until subsection `3. Re-generate the protocol buffers, gateway code and console UI.`.

On satori:
    Successfully run the README.md section called `#### (Re-)generate Protobuf Stubs`.

Here:
   ```shell
  protoc -I. -I../../nakama/build/grpc-gateway-v2.3.0/third_party/googleapis -I../../nakama/vendor/github.com/grpc-ecosystem/grpc-gateway/v2 --cpp_out=./src --plugin=protoc-gen-grpc=%YOUR_PATH_TO_GRPC_CPP_PLUGIN%/grpc_cpp_plugin --proto_path=../../satori/api/ satori.proto
  protoc -I. -I../../nakama/build/grpc-gateway-v2.3.0/third_party/googleapis -I../../nakama/vendor/github.com/grpc-ecosystem/grpc-gateway/v2 --grpc_out=./src --plugin=protoc-gen-grpc=%YOUR_PATH_TO_GRPC_CPP_PLUGIN%/grpc_cpp_plugin --proto_path=../../satori/api/ satori.proto
   ```