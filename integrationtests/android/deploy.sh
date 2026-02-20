#!/bin/bash

set -euo pipefail

serial="${1:-}"
if [ -z "$serial" ]; then
  echo "Usage: $0 <device-serial>"
  exit 1
fi

adb -s "$serial" uninstall com.heroiclabs.nakamatest || true
adb -s "$serial" install -r ./build/outputs/apk/debug/nakamatest-debug.apk
adb -s "$serial" logcat -c
adb -s "$serial" shell am start -n com.heroiclabs.nakamatest/.MainActivity
adb -s "$serial" logcat
