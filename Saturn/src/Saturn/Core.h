#pragma once

#include <memory>

#if (__cplusplus >= 201402L)
#error C++ 14 is not supported
#else
#ifdef _WIN32
/* Windows x64/x86 */
#ifdef _WIN64
	/* Windows x64  */
#define SAT_PLATFORM_WINDOWS
#ifdef SAT_DLL && SAT_PLATFORM_WINDOWS
#define SATURN_API 
#else
#define SATURN_API
#endif
#else
	/* Windows x86 */
#error "x86 Builds are not supported!"
#endif
#elif defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
/* TARGET_OS_MAC exists on all the platforms
 * so we must check all of them (in this order)
 * to ensure that we're running on MAC
 * and not some other Apple platform */
#if TARGET_IPHONE_SIMULATOR == 1
#error "IOS simulator is not supported!"
#elif TARGET_OS_IPHONE == 1
#define SAT_PLATFORM_IOS
#error "IOS is not supported!"
#elif TARGET_OS_MAC == 1
#define SAT_PLATFORM_MACOS
#error "MacOS is not supported!"
#else
#error "Unknown Apple platform!"
#endif
 /* We also have to check __ANDROID__ before __linux__
  * since android is based on the linux kernel
  * it has __linux__ defined */
#elif defined(__ANDROID__)
#define SAT_PLATFORM_ANDROID
#error "Android is not supported!"
#elif defined(__linux__)
#define SAT_PLATFORM_LINUX
#error "Linux is not supported!"
#else
/* Unknown compiler/platform */
#error "Unknown platform!"
#endif // End of platform detection

#ifdef SAT_DLL

#define SAT_DLL_IMPORT __declspec(dllimport)
#define SAT_DLL_EXPORT __declspec(dllexport)

#else
#define SAT_DLL_IMPORT
#define SAT_DLL_EXPORT

#endif // SAT_DLL


#ifdef SAT_DEBUG
	#if defined(SAT_PLATFORM_WINDOWS)
		#define SAT_DEBUGBREAK() __debugbreak()
	#elif defined(SAT_PLATFORM_LINUX)
			#include <signal.h>
			#define SAT_DEBUGBREAK() raise(SIGTRAP)
	#else
			#error "Platform doesn't support debugbreak yet!"
	#endif
	#define SAT_ENABLE_ASSERTS
#else
	#define SAT_DEBUGBREAK()
#endif

// TODO: Make this macro able to take in no arguments except condition
#ifdef SAT_ENABLE_ASSERTS
	#define SAT_ASSERT(x, ...) { if(!(x)) {/*SAT_ERROR("Assertion Failed: {0}", __VA_ARGS__);*/ SAT_DEBUGBREAK(); } }
	#define SAT_CORE_ASSERT(x, ...) { if(!(x)) { /*SAT_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);*/ SAT_DEBUGBREAK(); } }
#else
	#define SAT_CL_ASSERT(x, ...)
	#define SAT_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define SAT_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#define SAT_class(name) class SATURN_API(name)

#define SAT_FORCEINLINE inline

#define SAT_FORCE_INLINE __forceinline

namespace Saturn {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using RefSR = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename ... Args>
	constexpr RefSR<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	class RefCounted
	{
	public:
		void IncRefCount() const
		{
			m_RefCount++;
		}
		void DecRefCount() const
		{
			m_RefCount--;
		}

		uint32_t GetRefCount() const { return m_RefCount; }
	private:
		mutable uint32_t m_RefCount = 0; // TODO: atomic
	};

	template<typename T>
	class Ref
	{
	public:
		Ref()
			: m_Instance(nullptr)
		{
		}

		Ref(std::nullptr_t n)
			: m_Instance(nullptr)
		{
		}

		Ref(T* instance)
			: m_Instance(instance)
		{
			static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!");

			IncRef();
		}

		template<typename T2>
		Ref(const Ref<T2>& other)
		{
			m_Instance = (T*)other.m_Instance;
			IncRef();
		}

		template<typename T2>
		Ref(Ref<T2>&& other)
		{
			m_Instance = (T*)other.m_Instance;
			other.m_Instance = nullptr;
		}

		~Ref()
		{
			DecRef();
		}

		Ref(const Ref<T>& other)
			: m_Instance(other.m_Instance)
		{
			IncRef();
		}

		Ref& operator=(std::nullptr_t)
		{
			DecRef();
			m_Instance = nullptr;
			return *this;
		}

		Ref& operator=(const Ref<T>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(const Ref<T2>& other)
		{
			other.IncRef();
			DecRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(Ref<T2>&& other)
		{
			DecRef();

			m_Instance = other.m_Instance;
			other.m_Instance = nullptr;
			return *this;
		}

		operator bool() { return m_Instance != nullptr; }
		operator bool() const { return m_Instance != nullptr; }

		T* operator->() { return m_Instance; }
		const T* operator->() const { return m_Instance; }

		T& operator*() { return *m_Instance; }
		const T& operator*() const { return *m_Instance; }

		T* Raw() { return  m_Instance; }
		const T* Raw() const { return  m_Instance; }

		void Reset(T* instance = nullptr)
		{
			DecRef();
			m_Instance = instance;
		}

		template<typename... Args>
		static Ref<T> Create(Args&&... args)
		{
			return Ref<T>(new T(std::forward<Args>(args)...));
		}
	private:
		void IncRef() const
		{
			if (m_Instance)
				m_Instance->IncRefCount();
		}

		void DecRef() const
		{
			if (m_Instance)
			{
				//m_Instance->DecRefCount();
				//if (m_Instance->GetRefCount() == 0)
				{
					delete m_Instance;
				}
			}
		}

		template<class T2>
		friend class Ref;
		T* m_Instance;
	};


	typedef struct fIO //base engine file system
	{
		float			SavingRate;                     // = 5.0f
		const char*		IniFilename;                    // = "{engver}Saturn.ini"
		const char*		LogFilename;                    // = "Saturn.log"

		//////////////////////////////////////////////////////////////////////////////////////////////////////

		float			ScSavingRate;							// = 5.0f
		const char*		ScFilename;						       // = "{gamename ? enginegame ? edname}.sats"
		const char*		ScLogFilename;                        // = "{scname}.log"

		//////////////////////////////////////////////////////////////////////////////////////////////////////

		float			MapSavingRate;							// = 5.0f
		const char*		MapFilename;						       // = "{mapname}.smap"
		const char*		MapLogFilename;                        // = "{mapname}.log"


	};
	typedef fIO FileIO;

#ifndef SAT_VERSON 1.0
#define SAT_VERSON 1.0
#endif //!SAT_VERSON 1.0


}


/* def's core stuff*/
#if defined (SAT_PLATFORM_WINDOWS)
#define SAT_CORE_DELAY(...) ::std::this_thread::sleep_for(__VA_ARGS__)
#endif

#define SAT_MoveMemory MoveMemory 
#define SAT_CopyMemory CopyMemory 
#define SAT_FillMemory FillMemory 
#define SAT_ZeroMemory ZeroMemory

#define SAT_FILEOPENNAMEA OPENFILENAMEA
#define SAT_FILEOPENNAME OPENFILENAME

#define SAT_SYM
#ifdef SAT_SYM
	#define TYPE_INT int
	#define TYPE_FLOAT float
	#define TYPE_INT_PTR TYPE_INT PTR
	#define TYPE_FLOAT_PTR TYPE_FLOAT PTR
	#define __VOID void
	#define __VOID_PTR __VOID PTR
	#define TYPE_VOID __VOID
	#define TYPE_VOID_PTR __VOID_PTR
	#define __UNSIGNED unsigned
	#define TYPE__UNSIGNED_INT __UNSIGNED TYPE_INT
	#define TYPE__UNSIGNED_FLOAT __UNSIGNED TYPE_FLOAT
	#define TYPE__UNSIGNED_INT __UNSIGNED TYPE_INT PTR
	#define TYPE__UNSIGNED_FLOAT __UNSIGNED TYPE_FLOAT PTR
	#define NULLPTR nullptr
	#define TYPE_NULL NULL
	#define TYPE_NULL_PTR NULLPTR
	#define TYPE_ZERO 0
	#define TYPE_ONE 1
	#define SAT_ZeroMemory RtlZeroMemory
	#define PTR *
	#define NEW new
	#define PTR_POINT ->
	#define TYPENAME typename
	#define TEMPLATE template
	#define CLASS class
	#define OP_BACKSL /
	#define OP_FWDSL \
	
#if TEST_CODE

CLASS MyClass{
public:
	MyClass();
	~MyClass();

	__VOID Test() {
	
	
	}


}

CLASS MyOtherClass{

public:
	MyOtherClass();
	~MyOtherClass();


	__VOID GetPRT()
	{
		MyClass* test = new MyClass();
		test PTR_POINT Test();
	}

}

#endif // TEST_CODE



	#ifdef MR


		#define USE_OPENGL 0
		#if defined (SAT_PLATFORM_WINDOWS)
				#define USE_DX 0
		#else
				#define USE_DX 0
		#endif
		#define USE_VULKAN 0

	#endif // MR

	#define TYPE_OSSTREAM std::ofstream
#endif // SAT_SYM

#endif
