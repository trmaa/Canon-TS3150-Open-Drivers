#! /bin/sh

OS_BITS=$(getconf LONG_BIT)
INSTALLER_I386="installer_i386"
INSTALLER_X86_64="installer_x86-64"

cd "$(dirname "$0")"
cd Resources

if [ $OS_BITS -eq 32 ];then
    echo "32bit system"
    ./$INSTALLER_I386
else
    echo "64bit system"
    ./$INSTALLER_X86_64
fi
