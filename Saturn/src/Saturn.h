/********************************************************************************************
*                                                                                           *
*                                                                                           *
*                                                                                           *
* MIT License                                                                               *
*                                                                                           *
* Copyright (c) 2020 - 2023 BEAST                                                           *
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

// --- Core
#include "Saturn/Core/App.h"
#include "Saturn/Core/Base.h"
#include "Saturn/Core/EnvironmentVariables.h"
#include "Saturn/Core/Events.h"
#include "Saturn/Core/Input.h"
#include "Saturn/Core/Layer.h"
#include "Saturn/Core/MouseButton.h"
#include "Saturn/Core/Timer.h"
#include "Saturn/Core/Timestep.h"
#include "Saturn/Core/UUID.h"
#include "Saturn/Core/Memory/Buffer.h"
#include "Saturn/Core/AABB/AABB.h"

// --- ImGui
#include "Saturn/ImGui/ContentBrowserPanel.h"
#include "Saturn/ImGui/SceneHierarchyPanel.h"
#include "Saturn/ImGui/Styles.h"
#include "Saturn/ImGui/ImGuiAuxiliary.h"
#include "Saturn/ImGui/Panel/Panel.h"

// --- Don't include any rendering files as the user shouldn't need them.
// --- Rendering

// --- Scene
#include "Saturn/Scene/Components.h"
#include "Saturn/Scene/Entity.h"
#include "Saturn/Scene/Scene.h"