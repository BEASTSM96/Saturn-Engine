#pragma once

#include <random>

namespace Saturn {

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
			return  0.0f; //(float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
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


	static uint64_t AddInt64(uint64_t wa, uint64_t wa2) {
	
		uint64_t newnum = wa + wa2;
	
		return newnum;

	}

	static uint32_t AddInt32(uint32_t wa, uint32_t wa2) {

		uint32_t newnum = wa + wa2;

		return newnum;

	}

	static int AddInt(int wa, int wa2) {

		int newnum = wa + wa2;

		return newnum;

	}

	static float AddFloat(float wa, float wa2) {

		float newnum = wa + wa2;

		return newnum;

	}

	static double AddDouble(double wa, double wa2) {
		
		float newnum = AddFloat(float(wa), float(wa2));

		return double(newnum);

	}

	static double AddDoubleInt(double wa, double wa2) {

		int newnum = AddInt(int(wa), int(wa2));

		return double(newnum);

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	static uint64_t DivideInt64(uint64_t wa, uint64_t wa2) {

		uint64_t newnum = wa / wa2;

		return newnum;

	}

	static uint32_t DivideInt32(uint32_t wa, uint32_t wa2) {

		uint32_t newnum = wa / wa2;

		return newnum;

	}

	static int DivideInt(int wa, int wa2) {

		int newnum = wa / wa2;

		return newnum;

	}

	static float DivideFloat(float wa, float wa2) {

		float newnum = wa / wa2;

		return newnum;

	}

	static double DivideDouble(double wa, double wa2) {

		float newnum = DivideFloat(float(wa), float(wa2));

		return double(newnum);

	}

	static double DivideDoubleInt(double wa, double wa2) {

		int newnum = DivideFloat(int(wa), int(wa2));

		return double(newnum);

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	static uint64_t MultiplyInt64(uint64_t wa, uint64_t wa2) {

		uint64_t newnum = wa * wa2;

		return newnum;

	}

	static uint32_t MultiplyInt32(uint32_t wa, uint32_t wa2) {

		uint32_t newnum = wa * wa2;

		return newnum;

	}

	static int MultiplyInt(int wa, int wa2) {

		int newnum = wa * wa2;

		return newnum;

	}

	static float MultiplyFloat(float wa, float wa2) {

		float newnum = wa * wa2;

		return newnum;

	}

	static double MultiplyDouble(double wa, double wa2) {

		float newnum = MultiplyFloat(float(wa), float(wa2));

		return double(newnum);

	}

	static double MultiplyDoubleInt(double wa, double wa2) {

		
		int newnum = MultiplyDouble(int(wa), int(wa2));

		return double(newnum);

	}


}