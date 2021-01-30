#!/bin/bash
echo "Setting up Android: $1"
echo "export ARCH=arm64"
echo "export SUBARCH=arm64"
echo "export DTC_EXT=dtc"
echo "export CROSS_COMPILE=~/android/kernel/toolchain/"$1"/bin/aarch64-linux-android-"
echo ""
