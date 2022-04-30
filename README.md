# Saturn
[![License](https://img.shields.io/badge/license-MIT-green.svg)](https://github.com/BEASTSM96/Saturn-Engine/blob/master/LICENSE) 
![repo-size](https://img.shields.io/github/repo-size/BEASTSM96/Saturn-Engine) 
[![CL(Windows)](https://github.com/BEASTSM96/Saturn-Engine/actions/workflows/Windows.yml/badge.svg)](https://github.com/BEASTSM96/Saturn-Engine/actions/workflows/Windows.yml)
[![CL(Linux)](https://github.com/BEASTSM96/Saturn-Engine/actions/workflows/Linux.yml/badge.svg)](https://github.com/BEASTSM96/Saturn-Engine/actions/workflows/Linux.yml)

![SaturnLogo](/Titan/assets/Icons/SaturnLogov1.png?raw=true "SaturnLogov1")

Saturn is primarily an early-stage game engine for Windows.

## Getting Started
Visual Studio 2019 is recommended, Saturn is officially untested on other development environments whilst we focus on a Windows build.

Start by cloning the repository with `git clone --recursive https://github.com/BEASTSM96/Saturn-Engine`.

If the repository was cloned non-recursively previously, use `git submodule update --init` to clone the necessary submodules.

## Seting up the project (Windows)

### Premake

If you want to use Premake, start by going into the `scrtpts` folder, then run `WindowsGenProjects.bat` that will create the project files.
Then open `Saturn.sln` right click on the solution and click build *or `Ctrl Shift B`.*