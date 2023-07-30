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

*You may have to add the premake executable to you PATH environment varible.*

## Generating the project files (Windows)

To generate the project files, you can just run the premake executable that you downloaded, if you already have premake installed make sure it can support generating Visual Studio 2022 project files *so v5.0.0-beta1 onwards*.

So for generating the project files on Visual Studio 2022 you'd do `premake5.exe vs2022`

## Compiling the engine

To compile the engine simply open the newly generated project files and build the solution.

## Features
- [x] Core
  - [x] Custom Title bar
  - [x] Event System
  - [x] Input
  - [x] Window
  - [x] Asset Manager
  - [x] Projects
  - [x] ECS (entt)
- [ ] Renderer
  - [x] Core Renderer
  - [x] PBR
  - [x] Shadows
  - [x] Material
  - [x] Bloom
  - [ ] Soft shadows
  - [ ] SSAO, HBAO+, GTAO, VXAO
  - [x] Instanced Rendering
  - [x] Skybox
  - [ ] Text rendering
- [ ] Scripting (C++)
  - [x] Core scripting
  - [x] Core build tool
  - [ ] Scriptable Components
  - [ ] Public property viewer in the editor
  - [ ] General Game API and framework
- [ ] Physics
  - [x] Core PhysX API
  - [x] Mesh Colliders
  - [x] Cooking of mesh colldiers
  - [ ] Joints
  - [ ] Ragdolls
- [ ] Audio
  - [ ] Initial Audio API
  - [x] Initial Play/Stop
  - [x] Sound Assets
