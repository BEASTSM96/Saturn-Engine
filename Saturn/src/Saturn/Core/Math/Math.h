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
	static void Init() {
		MathType::Init();
	}

private:
	friend class MathType;

};

namespace Saturn {

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

	struct Vec2
	{
		float x;
		float y;

		Vec2() { x = 0.0f; y = 0.0f; };
		Vec2( float _x, float _y ) { x = _x; y = _y; };
	};


	struct Vec3
	{
		float x;
		float y;
		float z;

		Vec3() { x = 0.0f; y = 0.0f; z = 0.0f; };
		Vec3( float _x, float _y, float _z ) { x = _x; y = _y; z = _z; };
	};

	struct Vec4
	{
		float x;
		float y;
		float z;
		float w;

		Vec4() { x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f; };
		Vec4( float _x, float _y, float _z, float _w ) { x = _x; y = _y; z = _z; w = _w; };
	};


	static int Abs( int wa )
	{
		return glm::abs( wa );
	}

	static int Sin( int wa )
	{
		return glm::sin( wa );
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static uint64_t AddInt64( uint64_t wa, uint64_t wa2 )
	{

		uint64_t newnum = wa + wa2;

		return newnum;

	}

	static uint32_t AddInt32( uint32_t wa, uint32_t wa2 )
	{

		uint32_t newnum = wa + wa2;

		return newnum;

	}

	static int AddInt( int wa, int wa2 )
	{

		int newnum = wa + wa2;

		return newnum;

	}

	static float AddFloat( float wa, float wa2 )
	{

		float newnum = wa + wa2;

		return newnum;

	}

	static double AddDouble( double wa, double wa2 )
	{

		float newnum = AddFloat( float( wa ), float( wa2 ) );

		return double( newnum );

	}

	static double AddDoubleInt( double wa, double wa2 )
	{

		int newnum = AddInt( int( wa ), int( wa2 ) );

		return double( newnum );

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	static uint64_t DivideInt64( uint64_t wa, uint64_t wa2 )
	{

		uint64_t newnum = wa / wa2;

		return newnum;

	}

	static uint32_t DivideInt32( uint32_t wa, uint32_t wa2 )
	{

		uint32_t newnum = wa / wa2;

		return newnum;

	}

	static int DivideInt( int wa, int wa2 )
	{

		int newnum = wa / wa2;

		return newnum;

	}

	static float DivideFloat( float wa, float wa2 )
	{

		float newnum = wa / wa2;

		return newnum;

	}

	static double DivideDouble( double wa, double wa2 )
	{

		float newnum = DivideFloat( float( wa ), float( wa2 ) );

		return double( newnum );

	}

	static double DivideDoubleInt( double wa, double wa2 )
	{

		int newnum = DivideFloat( int( wa ), int( wa2 ) );

		return double( newnum );

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	static uint64_t MultiplyInt64( uint64_t wa, uint64_t wa2 )
	{

		uint64_t newnum = wa * wa2;

		return newnum;

	}

	static uint32_t MultiplyInt32( uint32_t wa, uint32_t wa2 )
	{

		uint32_t newnum = wa * wa2;

		return newnum;

	}

	static int MultiplyInt( int wa, int wa2 )
	{

		int newnum = wa * wa2;

		return newnum;

	}

	static float MultiplyFloat( float wa, float wa2 )
	{

		float newnum = wa * wa2;

		return newnum;

	}

	static double MultiplyDouble( double wa, double wa2 )
	{

		float newnum = MultiplyFloat( float( wa ), float( wa2 ) );

		return double( newnum );

	}

	static double MultiplyDoubleInt( double wa, double wa2 )
	{


		int newnum = MultiplyDouble( int( wa ), int( wa2 ) );

		return double( newnum );

	}

}
