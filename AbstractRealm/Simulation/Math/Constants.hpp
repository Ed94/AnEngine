/*

*/



#pragma once



#include "Simulation/Units.hpp"



namespace Sci
{
	// Compile-Time

	constexpr
	DecN CompareEpsilon()
	{
		return 
			
			LAL::IsOfClass<LAL::f32, DecN>() ? 
			// f32
			Meta::DecN_CompareAccuracy == Meta::ECompareAccuracy::High ? 
				LAL::Float32_HighAccuracy : 
				LAL::Float32_LowAccuracy  :
			// f64
			Meta::DecN_CompareAccuracy == Meta::ECompareAccuracy::Nano ?
					LAL::Float64_NanoAccuracy :
			Meta::DecN_CompareAccuracy == Meta::ECompareAccuracy::High ? 
					LAL::Float64_HighAccuracy :
					LAL::Float64_LowAccuracy
			;
	}

	constexpr
	DecN CompareEpsilon_Squared()
	{
		return CompareEpsilon() * CompareEpsilon();
	}


	/*

	*/
	constexpr 
	DecN UnitEpsilon()
	{
		return 
			
			LAL::IsOfClass<LAL::f32, DecN>() ? 
			// f32
			Meta::DecN_UnitAccuracy == Meta::ECompareAccuracy::High ?
				LAL::Float32_HighAccuracy : 
				LAL::Float32_LowAccuracy  : 
			// f64
			Meta::DecN_UnitAccuracy == Meta::ECompareAccuracy::Nano ? 
				LAL::Float64_NanoAccuracy : 
			Meta::DecN_UnitAccuracy == Meta::ECompareAccuracy::High ? 
				LAL::Float64_HighAccuracy :
				LAL::Float32_LowAccuracy
			;
	}

	constexpr 
	DecN Sqrt_OneHalf()
	{
		return std::sqrt(DecN(1.0) / DecN(2.0));
	}
	
	constexpr
	DecN Sqrt_2()
	{
		return std::sqrt( DecN(2.0) );
	}

	constexpr
	DecN Log_2()
	{
		return std::log( DecN(2.0) );
	}

	constexpr
	DecN Pi()
	{
		return std::atan( DecN(1.0) ) * 4;
	}

	constexpr
	DecN Tau()
	{
		return std::atan( DecN(1.0) ) * 8;
	}

	constexpr
	DecN Euluer()
	{
		return std::exp( DecN(1.0) );
	}

	constexpr 
	DecN Infinity()
	{
		return LAL::IsOfClass<LAL::f32, DecN>() ? LAL::Float32_Infinity : LAL::Float64_Infinity;
	}

	constexpr 
	DecN NaN()
	{
		return LAL::IsOfClass<LAL::f32, DecN>() ? LAL::Float32_QNaN : LAL::Float64_QNaN;
	}
}
