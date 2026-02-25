#!/bin/bash
#
# Build, deploy, and run nakama-cpp integration tests on an Android emulator/device.
#
# Usage:
#   ./deploy.sh [device-serial]
#
# Prerequisites:
#   - ANDROID_NDK_HOME set (or auto-detected from ANDROID_HOME/ndk/)
#   - ninja on PATH
#   - Test server running: docker compose -f integrationtests/server/docker-compose.yml up -d
#   - An Android emulator or device connected

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# --- Platform detection ---
_uname_s=$(uname -s)
case "$_uname_s" in
  MINGW*|MSYS*|CYGWIN*) HOST_OS="windows";;
  Darwin*)               HOST_OS="darwin";;
  Linux*)                HOST_OS="linux";;
  *)                     HOST_OS=$(echo "$_uname_s" | tr '[:upper:]' '[:lower:]');;
esac

# Auto-detect ANDROID_HOME on Windows from LOCALAPPDATA
if [ -z "${ANDROID_HOME:-}" ]; then
  case "$HOST_OS" in
    windows)
      _localappdata="${LOCALAPPDATA:-}"
      if [ -n "$_localappdata" ] && [ -d "$_localappdata/Android/Sdk" ]; then
        export ANDROID_HOME="$_localappdata/Android/Sdk"
      fi;;
    darwin)
      if [ -d "$HOME/Library/Android/sdk" ]; then
        export ANDROID_HOME="$HOME/Library/Android/sdk"
      fi;;
  esac
fi
if [ -z "${ANDROID_HOME:-}" ]; then
  echo "Error: ANDROID_HOME not set and could not be auto-detected."
  exit 1
fi
# Normalize Windows backslashes so CMake doesn't choke on \U escape sequences
ANDROID_HOME="${ANDROID_HOME//\\//}"

# Resolve adb — prefer PATH, fall back to ANDROID_HOME
if command -v adb &>/dev/null; then
  ADB="adb"
else
  ADB="$ANDROID_HOME/platform-tools/adb"
  if [ ! -x "$ADB" ] && [ -x "${ADB}.exe" ]; then
    ADB="${ADB}.exe"
  fi
fi

# Gradle 7.x requires JDK 17. Auto-detect if current JAVA_HOME is too new.
_need_jdk17=false
if [ -n "${JAVA_HOME:-}" ]; then
  _jv=$("$JAVA_HOME/bin/java" -version 2>&1 | head -1 | sed 's/.*"\([0-9]*\)\..*/\1/')
  [ "${_jv:-0}" -gt 17 ] 2>/dev/null && _need_jdk17=true
elif command -v java &>/dev/null; then
  _jv=$(java -version 2>&1 | head -1 | sed 's/.*"\([0-9]*\)\..*/\1/')
  [ "${_jv:-0}" -gt 17 ] 2>/dev/null && _need_jdk17=true
fi
if [ "$_need_jdk17" = true ]; then
  _found=""
  case "$HOST_OS" in
    windows)
      for _d in "/c/Program Files/Eclipse Adoptium"/jdk-17.*; do
        [ -d "$_d" ] && _found="$_d" && break
      done;;
    darwin)
      for _d in /Library/Java/JavaVirtualMachines/temurin-17.*/Contents/Home; do
        [ -d "$_d" ] && _found="$_d" && break
      done;;
    linux)
      for _d in /usr/lib/jvm/temurin-17-* /usr/lib/jvm/java-17-*; do
        [ -d "$_d" ] && _found="$_d" && break
      done;;
  esac
  if [ -n "$_found" ]; then
    echo "Auto-detected JDK 17: $_found"
    export JAVA_HOME="$_found"
  else
    echo "Warning: Java ${_jv} detected but Gradle 7.x needs JDK 17. Set JAVA_HOME to JDK 17."
  fi
fi

PACKAGE="com.heroiclabs.nakamatest"
ACTIVITY="${PACKAGE}/.MainActivity"
TIMEOUT=300  # seconds to wait for test completion
LOG_TAG="nakama"
APK_PATH="build/outputs/apk/customDebugType/nakamatest-customDebugType.apk"

# --- Parse args ---
abi="arm64-v8a"
serial=""
skip_build=false

while [[ $# -gt 0 ]]; do
  case "$1" in
    --abi) abi="$2"; shift 2;;
    --skip-build) skip_build=true; shift;;
    *) serial="$1"; shift;;
  esac
done

cmake_preset="android-${abi}"

# --- Resolve ANDROID_NDK_HOME ---
if [ -z "${ANDROID_NDK_HOME:-}" ]; then
  android_home="${ANDROID_HOME:-$HOME/Library/Android/sdk}"
  if [ -d "$android_home/ndk" ]; then
    ANDROID_NDK_HOME=$(ls -d "$android_home/ndk"/*/ 2>/dev/null | sort -V | tail -1 | sed 's:/$::')
  fi
  if [ -z "${ANDROID_NDK_HOME:-}" ] || [ ! -d "${ANDROID_NDK_HOME}" ]; then
    echo "Error: Could not find Android NDK. Set ANDROID_NDK_HOME."
    exit 1
  fi
  echo "Auto-detected NDK: $ANDROID_NDK_HOME"
  export ANDROID_NDK_HOME
fi
ANDROID_NDK_HOME="${ANDROID_NDK_HOME//\\//}"

# --- Build native libraries ---
if [ "$skip_build" = false ]; then
  echo "=== Building native libraries (preset: ${cmake_preset}) ==="

  # Configure and build from the repo root (where CMakePresets.json lives)
  pushd "$REPO_ROOT" > /dev/null

  cmake --preset "$cmake_preset" \
    -DBUILD_TESTING=ON \
    -DCMAKE_ANDROID_NDK="$ANDROID_NDK_HOME" \
    -DCMAKE_ANDROID_ARCH_ABI="$abi" \
    -DCMAKE_MAKE_PROGRAM="$(command -v ninja)"

  cmake --build "build/${cmake_preset}" --config Debug \
    --target nakama-sdk nakama-test

  popd > /dev/null

  echo "=== Staging native libraries ==="

  # Stage .so files for Gradle
  jni_dir="$SCRIPT_DIR/jniLibs/${abi}"
  mkdir -p "$jni_dir"
  cp "$REPO_ROOT/build/${cmake_preset}/Debug/libnakama-sdk.so" "$jni_dir/"
  cp "$REPO_ROOT/build/${cmake_preset}/integrationtests/Debug/libnakama-test.so" "$jni_dir/"

  # Include libc++_shared.so (required by ANDROID_STL=c++_shared)
  case "$abi" in
    arm64-v8a)   triple="aarch64-linux-android";;
    armeabi-v7a) triple="arm-linux-androideabi";;
  esac
  stl_lib="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/${HOST_OS}-x86_64/sysroot/usr/lib/${triple}/libc++_shared.so"
  if [ -f "$stl_lib" ]; then
    cp "$stl_lib" "$jni_dir/"
  else
    echo "Warning: libc++_shared.so not found at $stl_lib"
  fi

  echo "=== Building APK ==="
  cd "$SCRIPT_DIR"
  ./gradlew assembleCustomDebugType -Pabi="$abi"
fi

cd "$SCRIPT_DIR"

# --- Resolve device serial ---
if [ -z "$serial" ]; then
  device_count=$("$ADB" devices | grep -c -w 'device$' || true)
  if [ "$device_count" -eq 0 ]; then
    echo "Error: No connected devices/emulators found."
    echo "Start an emulator or connect a device, then retry."
    exit 1
  elif [ "$device_count" -gt 1 ]; then
    echo "Error: Multiple devices connected. Specify a serial:"
    "$ADB" devices
    exit 1
  fi
  serial=$("$ADB" devices | grep -w 'device$' | head -1 | awk '{print $1}')
  echo "Auto-detected device: $serial"
fi

adb_cmd() {
  "$ADB" -s "$serial" "$@"
}

# --- Set up adb reverse port forwarding ---
# The emulator's 127.0.0.1 is its own loopback, not the host.
# adb reverse makes the emulator's ports tunnel back to the host,
# where Docker is running the Nakama test server.
echo "Setting up port forwarding..."
for port in 7349 7350 7351; do
  adb_cmd reverse tcp:$port tcp:$port
done

# --- Install APK ---
if [ ! -f "$APK_PATH" ]; then
  echo "Error: APK not found at $APK_PATH"
  echo "Build it first with: $0 (without --skip-build)"
  exit 1
fi

echo "Installing APK..."
adb_cmd uninstall "$PACKAGE" 2>/dev/null || true
adb_cmd install -r "$APK_PATH"

# --- Clear logcat and launch ---
adb_cmd logcat -c
echo "Launching $ACTIVITY..."
adb_cmd shell am start -n "$ACTIVITY"

# --- Monitor logcat for test results ---
echo "Waiting for test results (timeout: ${TIMEOUT}s)..."
echo "---"

# Stream logcat in the background, filtering for our tag.
logfile=$(mktemp)
trap 'rm -f "$logfile"; kill $logcat_pid 2>/dev/null || true' EXIT

adb_cmd logcat -s "${LOG_TAG}:V" -v raw > "$logfile" 2>/dev/null &
logcat_pid=$!

# Follow the log file and watch for the summary line.
result=""
deadline=$((SECONDS + TIMEOUT))
last_pos=0

while [ $SECONDS -lt $deadline ]; do
  current_size=$(wc -c < "$logfile" 2>/dev/null || echo 0)
  if [ "$current_size" -gt "$last_pos" ]; then
    tail -c +"$((last_pos + 1))" "$logfile" | while IFS= read -r line; do
      echo "$line"
    done
    if grep -q "Tests failed:" "$logfile"; then
      fail_count=$(grep "Tests failed:" "$logfile" | grep -o '[0-9]\+' | head -1 || echo "")
      if [ "$fail_count" = "0" ]; then
        result="passed"
      else
        result="failed"
      fi
      break
    fi
    last_pos=$current_size
  fi
  sleep 1
done

kill $logcat_pid 2>/dev/null || true

echo "---"

# --- Report result ---
if [ -z "$result" ]; then
  echo "TIMEOUT: Tests did not complete within ${TIMEOUT}s."
  echo "Dumping last logcat output:"
  tail -50 "$logfile" 2>/dev/null || true
  exit 1
elif [ "$result" = "passed" ]; then
  echo "ALL TESTS PASSED"
  exit 0
else
  echo "TESTS FAILED"
  exit 1
fi
