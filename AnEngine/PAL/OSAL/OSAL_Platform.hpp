/*
Operating System Abstraction Layer: Platform Definitions

*/

#pragma once



// Platform

#ifdef _WIN32
	// Windows
	
	//#include "targetver.h"
	// Prevents the numeric limits error in LAL.
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
	// Windows Header Files
	#include <windows.h>
	#include <corecrt_io.h>
	#include <fcntl.h>

#endif

// Engine
#include "LAL/LAL.hpp"



namespace OSAL
{
	enum class EOS
	{
		Windows,
		Mac    ,
		Linux
	};


	#ifdef _WIN32
		constexpr EOS OS = EOS::Windows;

	#endif

	#ifdef __MACH__

	#endif

	#ifdef __linux__

	#endif 

	constexpr bool IsWindows = OS == EOS::Windows;
	constexpr bool IsMac     = OS == EOS::Mac    ;
	constexpr bool IsLinux   = OS == EOS::Linux  ;


	namespace PlatformBackend
	{
		template<OSAL::EOS>
		struct PlatformTypes_Maker;

		template<>
		struct PlatformTypes_Maker<EOS::Windows>
		{
			using OS_AppHandle    = HINSTANCE;
			using OS_Handle       = HANDLE   ;
			using OS_WindowHandle = HWND     ;

			using OS_CStr   = LPTSTR ;
			using OS_RoCStr = LPCTSTR;

			static OS_Handle InvalidHandle() { return INVALID_HANDLE_VALUE; };

			using ExitValT = int;
		};

		using PlatformTypes = PlatformTypes_Maker<OSAL::OS>;
	}

	using PlatformTypes = PlatformBackend::PlatformTypes;

	using OS_AppHandle    = PlatformTypes::OS_AppHandle   ;
	using OS_Handle       = PlatformTypes::OS_Handle      ;
	using OS_WindowHandle = PlatformTypes::OS_WindowHandle;
	using OS_CStr         = PlatformTypes::OS_CStr        ;
	using OS_RoCStr       = PlatformTypes::OS_RoCStr      ;
	using ExitValT        = PlatformTypes::ExitValT       ;

	constexpr auto OS_InvalidHandle = PlatformTypes::InvalidHandle;
}