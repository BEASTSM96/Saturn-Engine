/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2021 BEAST                                                           *
*                                                                                           *
* Permission is hereby granted, free of charge, to any person obtaining a copy              *
* of this software and associated documentation files (the "Software"), to deal             *
* in the Software without restriction, including without limitation the rights              *
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                 *
* copies of the Software, and to permit persons to whom the Software is                     *
* furnished to do so, subject to the following conditions:                                  *
*                                                                                           *
* The above copyright notice and this permission notice shall be included in all            *
* copies or substantial portions of the Software.                                           *
*                                                                                           *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE               *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,             *
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE             *
* SOFTWARE.                                                                                 *
*********************************************************************************************
*/

#pragma once

// For use by Saturn applications

#include "Saturn/Application.h"

#include "Saturn/Log.h"

#include "Saturn/KeyCodes.h"

#include "Saturn/MouseButtons.h"

#include "Saturn/Core/Timestep.h"

#include "Saturn/Layer.h"

#include "Saturn/ImGui/ImGuiLayer.h"

#include "Saturn/Input.h"

#include "Saturn/ImGui/ImGuiLayer.h"

#include "Saturn/Core/Serialisation/Serialiser.h"

#include "Saturn/Scene/Components.h"

#include "Saturn/Scene/ScriptableEntity.h"

#include "Saturn/Scene/Scene.h"

// ---Renderer---------------------
#include "Saturn/Renderer/Camera.h"
#include "Saturn/Renderer/Framebuffer.h"
#include "Saturn/Renderer/GraphicsContext.h"
#include "Saturn/Renderer/IndexBuffer.h"
#include "Saturn/Renderer/Material.h"
#include "Saturn/Renderer/Mesh.h"
#include "Saturn/Renderer/Pipeline.h"
#include "Saturn/Renderer/RenderCommandQueue.h"
#include "Saturn/Renderer/Renderer.h"
#include "Saturn/Renderer/RendererAPI.h"
#include "Saturn/Renderer/SceneRenderer.h"
// -----------------------------

// ---Entry Point---------------------
#include "Saturn/EntryPoint.h"
// ----------------------------------- 