/**
*  MegaSkirmish — standalone Yuri's Revenge companion DLL
*
*  Adds extra AI-controlled houses to an offline game so a single human
*  can play with or against up to 24 AI (or watch 25 AI play among
*  themselves). Loaded automatically by Syringe alongside the stock
*  CnCNet spawner / Ares / Phobos — drop the DLL in the game folder,
*  exactly like Phobos.
*
*  Reads its own config file ("megaskirmish.ini") which the CnCNet client
*  never touches, so the 8-slot client UI is not a limit.
*
*  Uses YRpp (Phobos-developers) + Syringe (Ares developers). GPLv3.
*  SCOPE: offline only; no extra HUMAN players (engine net layer caps at 8).
*/

#include "MegaSkirmish.h"

#include <Syringe.h>
#include <YRPPCore.h>
#include <Helpers/Macro.h>
#include <Unsorted.h>

#include <cstdio>
#include <cstdarg>

bool MegaSkirmish::Enabled = false;
bool MegaSkirmish::HousesCreated = false;

// ---------------------------------------------------------------------------
// No host (declhost) declaration is needed: Syringe identifies the target
// from the hook addresses themselves, exactly like the CnCNet spawner and
// Phobos, which also omit it. This keeps the DLL loadable across the
// Ares / CnCNet / Steam gamemd variants.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Minimal logger -> megaskirmish.log next to the game exe. Independent of the
// spawner's logger so this DLL has zero link-time dependency on it.
// ---------------------------------------------------------------------------
void MegaSkirmish::Log(const char* format, ...)
{
	static FILE* pLog = nullptr;
	if (!pLog)
		fopen_s(&pLog, "megaskirmish.log", "w");

	if (!pLog)
		return;

	va_list args;
	va_start(args, format);
	vfprintf(pLog, format, args);
	va_end(args);
	fflush(pLog);
}

bool __stdcall DllMain(HANDLE hInstance, DWORD dwReason, LPVOID /*reserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls(reinterpret_cast<HMODULE>(hInstance));

	return TRUE;
}

// ---------------------------------------------------------------------------
// Command-line gate. We only activate for an offline spawn (-SPAWN), so we
// never interfere with the main menu, campaign, or map editor.
// Hook mirrors the spawner's own command-line parse point (0x52F639).
// ---------------------------------------------------------------------------
DEFINE_HOOK(0x52F639, MegaSkirmish_ParseCommandLine, 0x5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	for (int i = 1; i < nNumArgs; ++i)
	{
		if (_stricmp(ppArgs[i], "-SPAWN") == 0)
			MegaSkirmish::Enabled = true;
	}

	if (MegaSkirmish::Enabled)
		MegaSkirmish::Log("[MegaSkirmish] Armed (offline spawn detected).\n");

	return 0; // continue to original code
}
