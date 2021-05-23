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

#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#pragma warning(disable: 26495)
#pragma warning(disable: 26451)
#pragma warning(disable: 26812)
#pragma warning(disable: 6001)
#pragma warning(disable: 4244)


class MathType
{
public:
	static void Init() {};
};

class Random : MathType
{
public:
	static void Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	static float Float()
	{
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}

	static int Int()
	{
		return (int)s_Distribution(s_RandomEngine) / (int)std::numeric_limits<uint32_t>::max();
	}

	/*
	* Returns a Random bool value, either 'true' or 'false'
	*/
	static bool Bool()
	{
		//fifty fifty chance
		std::bernoulli_distribution dist(0.5);
		return dist(s_RandomEngine);
	}

private:
	static std::mt19937 s_RandomEngine;
	static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};


class Math
{
public:
	static void Init() 
	{
		MathType::Init();
	}

private:
	friend class MathType;

};

namespace Saturn {
	/*
	using Vector1				=							glm::vec1;
	using Vector2				=							glm::vec2;
	using Vector3				=							glm::vec3;
	using Vector4				=							glm::vec4;
	///
	using Mat2					=							glm::mat2;
	using Mat2x2				=							glm::mat2x2;
	using Mat2x3				=							glm::mat2x3;
	using Mat2x4				=							glm::mat2x4;
	//
	using Mat3					=							glm::mat3;
	using Mat3x2				=							glm::mat3x2;
	using Mat3x3				=							glm::mat2x3;
	using Mat3x4				=							glm::mat2x4;
	//
	using Mat4					=							glm::mat4;
	using Mat4x2				=							glm::mat4x2;
	using Mat4x3				=							glm::mat4x3;
	using Mat4x4				=							glm::mat4x4;
	*/
}
