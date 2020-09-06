@echo off
pushd %~dp0\..\
call vendor\bin\premake\premake5.exe vs2019
popd
PAUSE

pushd %~dp0\..\

start Sparky.sln

popd