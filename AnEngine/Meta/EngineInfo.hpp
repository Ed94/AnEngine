/*
Engine Information

Last Modified: 5/19/2020
*/



#pragma once



// Engine
#include "LAL/LAL.hpp"
#include "Meta/Specification.hpp"

// Thirdparty
#include "magic_enum.hpp"



namespace Meta
{
	// Usings

	using namespace LAL;


	
	// Enums

	// 0.1: MVP Version
	class EEngineVersion
	{
	public:

		enum
		{
			Major = 0 ,
			Minor = 99,
			Patch = 0
		};
	};

	/*
	The different operational modes the engine can behave in. Must be determined before running.

	It affects how the PAL and Core loads its respective modules.
	*/
	enum class EOperationMode
	{
		/*
		Provide the dev suite of tools on top of the application runtime.
		*/
		Development,
		
		/*
		Provide both debugging and profiling tools on top of the application runtime.		
		*/
		Debugging  ,

		/*
		Provide only profiling tools on top of the application runtime.
		*/
		Profiling,
		
		/*
		Provide only the application runtime, no tool overhead.	
		*/
		User,
	};



	// Compile Time

	constexpr char EngineName[] = "Abstract Realm";

	constexpr auto BuildType_Name = magic_enum::enum_name(BuildType);


	/*
	Defines the default operational mode of the engine.

	Determined by the compilation intention.
	*/
	constexpr EOperationMode Default_OperationalMode()
	{
		switch (BuildType)
		{
			case ECompilationIntention::Development:
			{
				return EOperationMode::Development;
			}
			case ECompilationIntention::User_WithDebug:
			{
				return EOperationMode::Debugging;
			}
			case ECompilationIntention::User:
			{
				return EOperationMode::User;
			}
		}
	}



	StaticData
	(
		/*
		Determines the operational mode of the engine.

		Depending on build type, different operational modes of the engine may be available.

		Note: Changing the operational mode will require many modules to reinitialize themselves.
		*/
		eGlobal data< EOperationMode> OperationalMode;

		eGlobal data< bool> UseEditor   ;
		eGlobal data< bool> UseDebug    ;
		eGlobal data< bool> UseProfiling;

		/**
		 * Determines whether the engine should use the core's concurrency module (multi-threading at the CPU).
		 * 
		 * Concurrency still occurs on other hardware such as the GPU. (Just not from the Host CPU)
		 */
		eGlobal data< bool> UseConcurrency;
	)
}
