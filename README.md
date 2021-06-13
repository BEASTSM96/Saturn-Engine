# Saturn [![License](https://img.shields.io/badge/license-MIT-green.svg)](https://github.com/BEASTSM96/Saturn-Engine/blob/master/LICENSE) ![repo-size](https://img.shields.io/github/repo-size/BEASTSM96/Saturn-Engine) ![num-code](https://img.shields.io/tokei/lines/github/BEASTSM96/Saturn-Engine) ![build-status](https://img.shields.io/github/workflow/status/BEASTSM96/Saturn-Engine/build)


Saturn is primarily an early-stage game engine for Windows.

![SaturnLogo](/Titan/assets/.github/i/sat/SaturnLogov1.png?raw=true "SaturnLogov1")

## Getting Started
Visual Studio 2019 is recommended, Saturn is officially untested on other development environments whilst we focus on a Windows build.

Start by cloning the repository with `git clone --recursive https://github.com/BEASTSM96/Saturn-Engine`.

If the repository was cloned non-recursively previously, use `git submodule update --init` to clone the necessary submodules.

## Seting up the project (Windows)

### Premake

If you want to use Premake, start by going into the `scrtpts` folder, then run `WindowsGenProjects.bat` that will create the project files.
Then open `Saturn.sln` right click on the solution and click build *or `Ctrl Shift B`.*

## 

### Main features to come:
- Support for Linux, Android and iOS ❗
    - Native rendering API support (DirectX, Vulkan) ❗
- Fully featured viewer and editor applications
- Full Runtime interaction and behavior
- Procedural terrain and world generation
- Artificial Intelligence
- Audio system
- C# Scripting ✔️
- Asset Manager ✔️
