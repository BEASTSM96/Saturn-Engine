<h3 align="center">Saturn</h3>

<p align=center>
    <a href="https://github.com/BEASTSM96/Saturn-Engine/blob/master/LICENSE"><img alt="License" src="https://img.shields.io/badge/license-MIT-green.svg"></a>
    <a href="https://img.shields.io/github/repo-size/BEASTSM96/Saturn-Engine"><img alt="Repo Size" src="https://img.shields.io/github/repo-size/BEASTSM96/Saturn-Engine"></a>
    <a href="https://github.com/BEASTSM96/Saturn-Engine/actions/workflows/Windows.yml"><img alt="Repo Size" src="https://github.com/BEASTSM96/Saturn-Engine/actions/workflows/Windows.yml/badge.svg"></a>
    <a href="https://trello.com/b/baqP3fvB/saturn-engine"><img alt="Trello" src="https://img.shields.io/badge/Trello-saturn--engine-blue"></a>
</p>

<p align=left>
    Saturn is primarily an early-stage game engine for Windows.
    <br>
    Currently Saturn is built in Vulkan, in the furture we want to support other graphics APIs.
</p>

## Getting Started

Visual Studio 2022 is recommended as Saturn is officially untested on other development environments whilst we focus on a Windows build.

First, start by cloning the repository with `git clone --recursive https://github.com/BEASTSM96/Saturn-Engine`.

If the repository was previously cloned non-recursively then use `git submodule update --init` to clone the necessary submodules.

Make sure to check that you are on the branch `vulkan`. If not you can run `git checkout vulkan`

## Generating project files (Windows)

In order to start you will need to download <a href="https://premake.github.io/">Premake</a>

<a href="https://premake.github.io/download">Download</a> ·
<a href="https://premake.github.io/docs/What-Is-Premake">Learn More</a>

*You may have to add the premake executable to you PATH environment variable.*

To generate the project files, you can run the premake executable that you downloaded, if you already have premake installed make sure it can support generating Visual Studio 2022 project files (premake version v5.0.0-beta1 onwards).

So for generating the project files on Visual Studio 2022 you'd do `premake5.exe vs2022`

## Compiling the engine (Windows)

To compile the engine simply open the newly generated project files and build the entire solution.

## Running the engine (Windows)

Before launching the editor you must create a new project as this repo does not contain a default project.
So, set the project browser as the startup project and run (F5), create a new project and launch it from the browser.

*You will need to edit SaturnEditor.cpp in order for your project to launch by default!*

## Todo list

- [x] Core
  - [x] Custom Title bar
  - [x] Event System
  - [x] Input
  - [x] Window
  - [x] Asset Manager
  - [x] Projects
  - [x] ECS (entt)
  - [x] Virtual Filesystem
- [ ] Renderer
  - [x] Core Renderer
  - [x] PBR
  - [x] Shadows
  - [x] Material
  - [x] Bloom
  - [x] Soft shadows
  - [ ] SSAO, HBAO+, GTAO, ~~VXAO~~
  - [x] Instanced Rendering
  - [x] Skybox
  - [x] Shader Hot Reloading
- [ ] 2D Renderer
  - [x] Line Rendering (world space)
  - [x] Quad Rendering (world space)
  - [ ] Text rendering (world space)
  - [x] Billboard Rendering (world space)
  - [ ] Line Rendering (screen space)
  - [ ] Quad Rendering (screen space)
  - [ ] Text Rendering (screen space)
  - [ ] Game UI System
- [ ] Scripting (C++)
  - [x] Core scripting
  - [x] Core build tool
  - [ ] Scriptable Components
  - [ ] Public property viewer in the editor
  - [ ] General Game API and framework
  - [ ] Hot Code Reloading
- [ ] Physics
  - [x] Core PhysX API
  - [x] Mesh Colliders (Cube, Sphere, etc)
  - [x] Cooking of mesh colliders
  - [ ] Joints
  - [ ] Ragdolls
- [x] Audio
  - [x] Initial Audio API
  - [x] Sound Assets
  - [x] Sound Node Editor
  - [x] Audio Spatialization
  - [x] Audio Groups
  - [x] Audio Listeners
- [ ] Asset Bundle
  - [x] Package all assets (in binary format)
  - [x] Compression
  - [ ] Security (encrypt asset bundle header and encrypt asset file header)
- [ ] Editor
  - [ ] Undo/Redo
  - [x] Content Browser Search
  - [ ] Content Browser Thumbnails
  - [ ] Drag and drop meshes relative to mouse position in world
  - [ ] Mouse Picking (select meshes from world)
  - [ ] Selection Outline
- [ ] Releases
  - [x] Alpha 0.0.1
  - [x] Alpha 0.1.1
  - [ ] Alpha 0.1.2

*This list is not updated often, for more accurate updates and timelines please check the Trello!*
