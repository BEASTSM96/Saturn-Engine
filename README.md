<h3 align="center">Saturn</h3>

<p align=center>
    <a href="https://github.com/BEASTSM96/Saturn-Engine/blob/master/LICENSE"><img alt="License" src="https://img.shields.io/badge/license-MIT-green.svg"></a>
    <a href="https://img.shields.io/github/repo-size/BEASTSM96/Saturn-Engine"><img alt="Repo Size" src="https://img.shields.io/github/repo-size/BEASTSM96/Saturn-Engine"></a>
    <a href="https://github.com/BEASTSM96/Saturn-Engine/actions/workflows/Windows.yml"><img alt="Repo Size" src="https://github.com/BEASTSM96/Saturn-Engine/actions/workflows/Windows.yml/badge.svg"></a>
    <a href="https://trello.com/b/baqP3fvB/saturn-engine"><img alt="Trello" src="https://img.shields.io/badge/Trello-saturn--engine-blue"></a>
</p>

<p align=center>
    <!--- I wish the logo was smaller -->
    <img src="Titan/assets/Icons/SaturnLogov1.png" alt="Saturn" width="" height="">
</p>

<p align=left>
    Saturn is primarily an early-stage game engine for Windows.
    <br>
    Currently Saturn is built in Vulkan, in the furture we want to support other APIs.    
</p>

## Getting Started
Visual Studio 2022 is recommended, Saturn is officially untested on other development environments whilst we focus on a Windows build.

Start by cloning the repository with `git clone --recursive https://github.com/BEASTSM96/Saturn-Engine`.

If the repository was cloned non-recursively previously, use `git submodule update --init` to clone the necessary submodules.

## Buiding

In order to start building you will need <a href="https://premake.github.io/">Premake</a>

<a href="https://premake.github.io/download">Download</a>

<a href="https://premake.github.io/docs/What-Is-Premake">Learn More</a>

## Generating the project files (Windows)

To generate the project files, you can just run the premake executable that you downloaded, if you already have premake installed make sure it can support generating Visual Studio 2022 project files *so v5.0.0-beta1 onwards*.

So for generating the project files you should do : `premake5.exe vs2022`

## Compiling the engine

To compile the engine simply open the newly generated project files and build the solution.

## Features soon to come
PBR Renderer, equirectangular cubemaps, shadows and a material workflow.
<br>
PhysX.
<br>
C++/C# native scripting.
<br>
Linux support.
<br>
Project system.
<br>
Asset manager.
<br>
Game/Engine launcher.
<!--- UI REWORK -->
<!--- Vulkan 1.3 support -->
<!--- Controller support -->
<!--- Xbx support and/or PS4/5 support -->