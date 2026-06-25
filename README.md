# MegaSkirmish — standalone companion DLL for Yuri's Revenge

Adds extra **AI** houses to an offline YR game so one human can play with
or against more than the usual 7 AI — up to 24 — or sit back and watch up
to 25 AI fight each other. Built to be dropped into the game folder like
Phobos and loaded automatically by Syringe, including through the normal
**CnCNet client**.

Offline only. It deliberately does **not** add extra human players — the
engine's networking layer is hard-capped at 8 and that's a different,
much larger project.

---

## Why this is a separate DLL (and how it dodges the 8-slot client)

The CnCNet client UI only has 8 player slots, so it can never put a 9th
player into the `spawn.ini` it generates. This DLL sidesteps that
completely: it reads its **own** file, `megaskirmish.ini`, which the
client doesn't know about and never overwrites. You set up a normal
1-human skirmish in the client; the DLL quietly appends the AI houses
from `megaskirmish.ini` once the game launches.

Loading is automatic. Syringe scans the game folder and injects every
hook-bearing DLL it finds (that's how Ares.dll and Phobos.dll "just
work"). Because the CnCNet client launches the game through Syringe,
your DLL rides along the same way — no client modification, no replacing
the stock spawner.

```
You set up a normal skirmish in the CnCNet client   (its job, <=8 slots)
        |
        v
Client launches via Syringe -> auto-loads stock spawner + Ares/Phobos + MegaSkirmish.dll
        |
        v
Engine + stock spawner build the native <=8 houses    (untouched)
        |
        v
MegaSkirmish hook fires once, reads megaskirmish.ini, appends AI houses
```

---

## The 25 ceiling (why not 100)

YR stores per-house relationships — alliances, spy-reveal, base-spacing —
as single 32-bit bitfields indexed by house. With the always-present
Neutral and Special houses, that leaves room for ~25 real players before
the 32nd bit overflows and memory corrupts. The DLL logs a warning if any
house crosses that line. (This is almost certainly the same ceiling the
Tiberian Sun "25 players" modder hit — it's structural, not a choice.)

---

## Build (Windows, Visual Studio 2022)

1. Install **VS2022** with the *Desktop development with C++* workload.
2. Folder layout expected:
   ```
   MegaSkirmish/
     MegaSkirmish.vcxproj
     megaskirmish.ini      (sample config)
     src/                  (Main.cpp, Config.cpp, ExtraHouses.cpp, MegaSkirmish.h)
     YRpp/                 (the Phobos-developers/YRpp headers — bundled)
   ```
   YRpp is header-only and already included here. To refresh it later:
   `git clone https://github.com/Phobos-developers/YRpp.git`
3. Open the `.vcxproj`, select **Release | Win32** (x86 — gamemd.exe is
   32-bit, the DLL must match), Build.
4. Output: `Release\MegaSkirmish.dll`.

If the build complains about a missing macro or header, it's almost
always a YRpp version drift — pull the latest YRpp and rebuild.

---

## Install & run (through CnCNet)

1. In a **scratch copy** of your CnCNet YR install (never your live one),
   drop in:
   - `MegaSkirmish.dll`
   - `megaskirmish.ini`  (start with the sample — one `[House0]`)
2. Launch the CnCNet client, start a **Skirmish**: just you + 0 AI is
   fine for the first test (the extra house comes from the INI, not the
   lobby). Pick any standard map.
3. Start the game. Afterward, read `megaskirmish.log` in the game folder —
   every step is logged with a `[MegaSkirmish]` prefix.

### What success looks like (milestone 1)

In `megaskirmish.log`, in order:
1. `Armed (offline spawn detected).`
2. `Config loaded: 1 extra house(s)`
3. `House0 -> ArrayIndex N, country 8 (Yuri...)`  — house was created
4. `House0: <MCV> placed at X,Y`  — it has a unit on the map

Then in-game: a 9th colored house exists and its MCV is somewhere on the
map (set `FogOfWar=No` / use an observer view to confirm). If the AI
actively plays, wonderful. If the MCV just sits there, that's the known
next problem (see below) — not a failure.

---

## If it doesn't work — the likely failure points

This DLL has been written carefully against the YRpp headers but has not
yet met a running `gamemd.exe`. The spots most likely to need adjustment,
all tagged `VERIFY:` in the source:

1. **The trigger hook address** (`0x6878E0` in `ExtraHouses.cpp`). If the
   log shows "Armed" + "Config loaded" but never "ArrayIndex", the hook
   fired too early (houses not built yet) or not at all. The source
   comment lists the fallback addresses to try, in order.
2. **House exists, MCV exists, AI inert.** The engine primes AI brains
   during its own init, before our late-created house exists. Fix is to
   find that AI-activation step and either call it for our houses or move
   our creation earlier. This is the main "phase 2" research task.
3. **Wrong faction/color.** Index-mapping between spawn-style indices and
   the engine arrays — see the two `VERIFY` notes. Cosmetic, easy.
4. **Crash at game end / score screen.** Score structures are sized for
   8. Test with short games and avoid the score screen until handled.

For any crash, grab the address from the exception and `megaskirmish.log`;
cross-referencing that address against the YRpp headers points straight
at the engine function to hook next. That's the whole method from 9 to 25.

---

## Roadmap

- **9 works -> fill more `[HouseN]` sections** up toward 24. Pure config.
- **Real start positions** for >8 houses (so they don't share waypoints):
  reimplement `ScenarioClass::ReadStartPoints`. Biggest remaining job.
- **Distinct AI personalities** (not clones): no engine work needed —
  AITriggers are house/side-filterable, so this is an INI-generation
  problem, solvable with the kind of Python tooling you already build.
- **Watch-25-AI mode:** set `ObserverMode=yes` in megaskirmish.ini.

## License & credits

GPLv3. Built on **YRpp** and **Syringe** (Ares developers) and informed
by the **CnCNet/yrpp-spawner** and **Phobos** projects. Keep this DLL
offline / LAN — running modified injectors on CnCNet's live servers
violates their ToS.
