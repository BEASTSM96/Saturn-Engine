@echo off

rem %1 is the buildtool location 

pushd "%~dp0"

IF EXIST ..\SaturnBuildTool.exe (
	..\SaturnBuildTool.exe %*
		popd

		exit /B 0
) ELSE (
	ECHO BUIDLTOOLS NOT FOUND!
	popd
	exit /B 1
)