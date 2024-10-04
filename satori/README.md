To generate the interface:

   ```shell
  protoc -I. -I../../satori/build/grpc-gateway-v2.3.0/third_party/googleapis -I../../satori/vendor/github.com/grpc-ecosystem/grpc-gateway/v2 --grpc_out=./src --plugin=protoc-gen-grpc=%YOUR_PATH_TO_GRPC_CPP_PLUGIN%/grpc_cpp_plugin --proto_path=../../satori/api/ satori.proto
   ```