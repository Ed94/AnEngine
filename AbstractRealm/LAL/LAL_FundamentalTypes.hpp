/*
C++ STL Fundamental Types

Last Modified: 5/18/2020
*/



#pragma once



#include "LAL_Cpp_STL.hpp"
#include "LAL_Reflection.hpp"



namespace LAL
{
	// Types

	// Integers

	// Flexible

	using ui16 = unsigned int;
	using si16 = signed   int;
	using ui32 = unsigned int;
	using si32 = signed   int;

	// Strict

	// Signed
	using s8  = signed           char;
	using s16 = signed short     int ;
	using s32 = signed long      int ;
	using s64 = signed long long int ;

	// Unsigned

	using u8  = unsigned            char;
	using u16 = unsigned short      int ;
	using u32 = unsigned long       int ;
	using u64 = unsigned long long  int ;

	// Floats

	using f32 = float ;
	using f64 = double;



	// Functions

	using std::signbit;

	template<typename Type>
	constexpr bool IsInt()
	{
		return IsSameType<Type, s8 >() || 
			   IsSameType<Type, s16>() || 
			   IsSameType<Type, s32>() || 
			   IsSameType<Type, s64>() ||
			   IsSameType<Type, u8 >() ||
			   IsSameType<Type, u16>() ||
			   IsSameType<Type, u32>() ||
			   IsSameType<Type, u64>()   ;
	}

	template<typename Type>
	constexpr bool IsFloat()
	{
		return IsSameType<Type, f32>() || IsSameType<Type, f64>();
	}


	// Literals

	constexpr u16 operator"" _uI16 (unsigned long long _uI16)
	{
		return u16(_uI16);
	}

	constexpr short operator"" _s (unsigned long long _s)
	{
		return short(_s);
	}
}