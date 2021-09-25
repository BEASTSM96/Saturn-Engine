#!/bin/bash

PREMAKE="5.0.0-Alpha15"
os=$(uname -s)
mechine=$(uname -m)

if [ "$os" == "Linux" ]; then
	os="linux"
else
	os="windows"
fi

if [ "$os" == "windows" ]; then
	# Download premake executable
	$(curl -L -o "tmp/premake5.zip" "https://github.com/premake/premake-core/releases/download/v$PREMAKE_VERSION/premake-$PREMAKE_VERSION-windows.zip")
	$(unzip -u -q "tmp/premake5.zip" -d "../vendor/bin/premake")
fi

if [ "$os" == "Linux" ]; then
    $(curl -L -o "tmp/premake5.tar.gz" "https://github.com/premake/premake-core/releases/download/v$PREMAKE_VERSION/premake-$PREMAKE_VERSION-linux.tar.gz")
	$(tar -xvzf "tmp/premake5.tar.gz" -C "../vendor/bin/premake")
fi