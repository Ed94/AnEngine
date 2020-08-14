// Parent Header
#include "CyclerPool.hpp"




namespace Core::Concurrency
{
	struct Unit
	{
		using CyclerT = Cycler;

		Unit() : Cycler(), Thread(NULL)
		{}

		CyclerT  Cycler;
		DataSize Thread;
	};

	DynamicArray<Unit> Pool;

	bool   Initiated   = false;
	uInt16 ActiveUnits = 0    ;

	void CyclerPool::ActivateUnit()
	{
		OSAL::RequestThread(&Cycler::Initiate, &Pool[ActiveUnits++].Cycler);
	}

	const Cycler& CyclerPool::GetCycler(uInt16 _unit)
	{
		return Pool[_unit].Cycler;
	}

	uInt16 CyclerPool::GetNumUnits()
	{
		return ActiveUnits;
	}

	void CyclerPool::Initialize()
	{
		Pool.resize(OSAL::GetNumberOfLogicalCores());

		Initiated = true;
	}

	bool CyclerPool::IsShutdown()
	{
		return
		Initiated = true ?
			ActiveUnits > 0 ? false : true : 
		true;
	}

	bool CyclerPool::RequestShutdown()
	{		
		for (uInt16 unitIndex = 0; unitIndex < ActiveUnits; unitIndex++)
		{
			Pool[unitIndex].Cycler.Lapse();
		}

		ActiveUnits = 0;

		return true;
	}
}