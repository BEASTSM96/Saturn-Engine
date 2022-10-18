#include <sppch.h>

//------------------------------------------------------------------------------
// LICENSE
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.
//
// CREDITS
//   Written by Michal Cichon
//------------------------------------------------------------------------------
# include "builders.h"
# define IMGUI_DEFINE_MATH_OPERATORS
# include <imgui_internal.h>


//------------------------------------------------------------------------------
namespace ed   = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

util::BlueprintNodeBuilder::BlueprintNodeBuilder(ImTextureID texture, int textureWidth, int textureHeight):
    HeaderTextureId(texture),
    HeaderTextureWidth(textureWidth),
    HeaderTextureHeight(textureHeight),
    CurrentNodeId(0),
    CurrentStage(Stage::Invalid),
    HasHeader(false)
{
}

void util::BlueprintNodeBuilder::Begin(ed::NodeId id)
{
}

void util::BlueprintNodeBuilder::End()
{
}

void util::BlueprintNodeBuilder::Header(const ImVec4& color)
{
}

void util::BlueprintNodeBuilder::EndHeader()
{
}

void util::BlueprintNodeBuilder::Input(ed::PinId id)
{
}

void util::BlueprintNodeBuilder::EndInput()
{
}

void util::BlueprintNodeBuilder::Middle()
{
}

void util::BlueprintNodeBuilder::Output(ed::PinId id)
{
}

void util::BlueprintNodeBuilder::EndOutput()
{
}

bool util::BlueprintNodeBuilder::SetStage(Stage stage)
{
    return false;
}

void util::BlueprintNodeBuilder::Pin(ed::PinId id, ed::PinKind kind)
{
}

void util::BlueprintNodeBuilder::EndPin()
{
}
