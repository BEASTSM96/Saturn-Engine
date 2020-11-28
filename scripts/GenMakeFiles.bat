@echo off
pushd %~dp0\..\
call vendor\bin\premake\premake5.exe gmake
popd
PAUSE