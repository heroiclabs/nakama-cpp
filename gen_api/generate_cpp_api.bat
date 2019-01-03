@echo off

set NAKAMA=C:\dev\sandbox\my\nakama\nakama-2.3.0
set GRPC_GATEWAY=C:\dev\sandbox\my\libs\grpc-gateway-1.6.2
set NAKAMA_CPP=%CD%\..

set GRPC=%NAKAMA_CPP%\third_party\grpc
set GOOGLEAPIS=%GRPC_GATEWAY%\third_party\googleapis
set GRPC_CPP_PLUGIN=%NAKAMA_CPP%\build\third_party\grpc\Debug\grpc_cpp_plugin.exe
set PROTOC=%NAKAMA_CPP%\build\third_party\grpc\third_party\protobuf\Debug\protoc.exe
set PROTOBUF_SRC=%GRPC%\third_party\protobuf\src
set OUT=%CD%\cppout

if not exist %NAKAMA% (
    echo ERROR: not exist %NAKAMA%
    goto error
)

if not exist %GRPC% (
    echo ERROR: not exist %GRPC%
    goto error
)

if not exist %GRPC_GATEWAY% (
    echo ERROR: not exist %GRPC_GATEWAY%
    goto error
)

if not exist %GOOGLEAPIS% (
    echo ERROR: not exist %GOOGLEAPIS%
    goto error
)

if not exist %GRPC_CPP_PLUGIN% (
    echo ERROR: not exist %GRPC_CPP_PLUGIN%
    goto error
)

if not exist %PROTOC% (
    echo ERROR: not exist %PROTOC%
    goto error
)

if not exist %PROTOBUF_SRC% (
    echo ERROR: not exist %PROTOBUF_SRC%
    goto error
)

set CUR_DIR=%CD%

if not exist %OUT% mkdir %OUT%
if not exist %OUT%\google\api mkdir %OUT%\google\api
if not exist %OUT%\google\rpc mkdir %OUT%\google\rpc

if not exist github.com (
    mkdir github.com\heroiclabs\nakama\api
    mkdir github.com\heroiclabs\nakama\apigrpc
    mkdir github.com\heroiclabs\nakama\rtapi
    mklink github.com\heroiclabs\nakama\api\api.proto %NAKAMA%\api\api.proto
    mklink github.com\heroiclabs\nakama\apigrpc\apigrpc.proto %NAKAMA%\apigrpc\apigrpc.proto
    mklink github.com\heroiclabs\nakama\rtapi\realtime.proto %NAKAMA%\rtapi\realtime.proto
)

echo generating apigrpc

%PROTOC% -I. -I%GRPC_GATEWAY% -I%GOOGLEAPIS% -I%PROTOBUF_SRC% --grpc_out=%OUT% --plugin=protoc-gen-grpc=%GRPC_CPP_PLUGIN% github.com\heroiclabs\nakama\apigrpc\apigrpc.proto
if %errorlevel% neq 0 goto error

%PROTOC% -I. -I%GRPC_GATEWAY% -I%GOOGLEAPIS% -I%PROTOBUF_SRC% --cpp_out=%OUT% github.com\heroiclabs\nakama\apigrpc\apigrpc.proto
if %errorlevel% neq 0 goto error

%PROTOC% -I. -I%GRPC_GATEWAY% -I%GOOGLEAPIS% -I%PROTOBUF_SRC% --cpp_out=%OUT% github.com\heroiclabs\nakama\api\api.proto
if %errorlevel% neq 0 goto error

cd %GOOGLEAPIS%\google\rpc

%PROTOC% -I. -I%GRPC_GATEWAY% -I%GOOGLEAPIS% -I%PROTOBUF_SRC% --cpp_out=%OUT%\google\rpc status.proto
if %errorlevel% neq 0 goto error

cd %GOOGLEAPIS%\google\api

%PROTOC% -I. -I%GRPC_GATEWAY% -I%GOOGLEAPIS% -I%PROTOBUF_SRC% --cpp_out=%OUT%\google\api annotations.proto
if %errorlevel% neq 0 goto error

%PROTOC% -I. -I%GRPC_GATEWAY% -I%GOOGLEAPIS% -I%PROTOBUF_SRC% --cpp_out=%OUT%\google\api http.proto
if %errorlevel% neq 0 goto error

cd %CUR_DIR%

%PROTOC% -I. -I%GRPC_GATEWAY% -I%GOOGLEAPIS% -I%PROTOBUF_SRC% --cpp_out=%OUT% %GRPC_GATEWAY%\protoc-gen-swagger\options\annotations.proto
if %errorlevel% neq 0 goto error

%PROTOC% -I. -I%GRPC_GATEWAY% -I%GOOGLEAPIS% -I%PROTOBUF_SRC% --cpp_out=%OUT% %GRPC_GATEWAY%\protoc-gen-swagger\options\openapiv2.proto
if %errorlevel% neq 0 goto error


echo generating rtapi

%PROTOC% -I. -I%PROTOBUF_SRC% --cpp_out=%OUT% github.com\heroiclabs\nakama\rtapi\realtime.proto
if %errorlevel% neq 0 goto error

goto quit

:error
echo Stopped because of error

:quit
cd %CUR_DIR%
