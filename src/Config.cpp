/**
*  MegaSkirmish — config reader (megaskirmish.ini)
*
*  Format (lives next to gamemd.exe; CnCNet client never touches it):
*
*    [MegaSkirmish]
*    ObserverMode=no        ; yes = 25 AI play each other, you spectate
*    DefaultCredits=10000
*
*    [House0]               ; first EXTRA house (engine houses 0..7 are
*    Country=8              ;   the stock spawner's; these stack on top)
*    Color=5
*    Difficulty=1           ; 0=Hard 1=Normal 2=Easy (engine convention)
*    SpawnWaypoint=-1       ; -1 = auto-pick; else a map waypoint index
*    CreditsFactor=1.0
*
*    [House1]
*    Country=3
*    ...up to [House24]
*
*  A [HouseN] section counts as "defined" the moment it has a Country.
*/

#include "MegaSkirmish.h"

#include <CCINIClass.h>
#include <cstdio>

static MegaSkirmish::Config TheConfig;

MegaSkirmish::Config* MegaSkirmish::GetConfig()
{
	return &TheConfig;
}

void MegaSkirmish::Config::Load()
{
	// Read through the engine's mix/file system so it also works when the
	// game is launched from odd working directories.
	CCINIClass* pINI = CCINIClass::LoadINIFile("megaskirmish.ini");

	if (!pINI)
	{
		MegaSkirmish::Log("[MegaSkirmish] No megaskirmish.ini found — nothing to add.\n");
		return;
	}

	this->ObserverMode   = pINI->ReadBool("MegaSkirmish", "ObserverMode", false);
	this->DefaultCredits = pINI->ReadInteger("MegaSkirmish", "DefaultCredits", 10000);

	int defined = 0;
	char section[16];

	for (int i = 0; i < MaxExtraHouses; ++i)
	{
		sprintf_s(section, sizeof(section), "House%d", i);

		if (!pINI->GetSection(section))
			continue;

		auto& h = this->Houses[i];
		h.Country = pINI->ReadInteger(section, "Country", -1);

		if (h.Country < 0)
			continue; // a section with no country isn't a real house

		h.Defined       = true;
		h.Color         = pINI->ReadInteger(section, "Color", -1);
		h.Difficulty    = pINI->ReadInteger(section, "Difficulty", 1);
		h.SpawnWaypoint = pINI->ReadInteger(section, "SpawnWaypoint", -1);
		h.CreditsFactor = pINI->ReadDouble(section, "CreditsFactor", 1.0);
		h.IsObserver    = pINI->ReadBool(section, "IsObserver", false);
		++defined;
	}

	this->AnyDefined = defined > 0;

	MegaSkirmish::Log("[MegaSkirmish] Config loaded: %d extra house(s), ObserverMode=%s\n",
		defined, this->ObserverMode ? "yes" : "no");

	CCINIClass::UnloadINIFile(pINI);
}
