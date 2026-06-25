/**
*  MegaSkirmish — standalone YR companion DLL
*  See Main.cpp for project overview. GPLv3.
*/

#pragma once

#include <Windows.h>

class HouseClass;

namespace MegaSkirmish
{
	// Total slots the engine bitfield ceiling allows: 25 players + Neutral
	// + Special = 27 of 32 bits in HouseClass::Allies (and friends).
	static constexpr int MaxExtraHouses = 25;

	extern bool Enabled;        // set when -SPAWN seen on the command line
	extern bool HousesCreated;  // one-shot guard for the creation hook

	void Log(const char* format, ...);

	// One configured extra AI house, read from megaskirmish.ini.
	struct ExtraHouseConfig
	{
		bool   Defined       = false;
		int    Country       = -1;   // index into HouseTypeClass::Array
		int    Color         = -1;   // spawn-style color index
		int    Difficulty    = 1;    // reversed: 0=Hard 1=Normal 2=Easy
		int    SpawnWaypoint = -1;   // -1 => auto-spread over 0..7
		double CreditsFactor = 1.0;
		bool   IsObserver    = false;
	};

	// Reads [House0]..[House24] from megaskirmish.ini.
	struct Config
	{
		bool             AnyDefined   = false;
		bool             ObserverMode = false; // 25 AI, no human control
		int              DefaultCredits = 10000;
		ExtraHouseConfig Houses[MaxExtraHouses];

		void Load();
	};

	Config* GetConfig();

	// The actual work: create the configured AI houses. Called once, late,
	// after the engine + stock spawner have built the native <=8 houses.
	void CreateExtraHouses();
}
