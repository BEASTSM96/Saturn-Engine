@echo off
pushd %~dp0\..\
call tpremake\premake5.exe vs2019
popd
PAUSE