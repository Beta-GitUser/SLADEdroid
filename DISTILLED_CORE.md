# SLADEdroid distilled-core

This branch is the portability/refactoring line for SLADEdroid.

## Goal

Extract SLADE's format/resource/editor logic that can run without desktop UI dependencies, then build an Android frontend around it.

## First core boundary

### Portable core targets

- `src/Archive/` — WAD/archive formats, archive entries and directories
- `src/Utility/` — memory/data, compression, parsing/tokenizing, properties, math, colours, and portable string/path functionality after wx types are removed
- `src/Game/` — Doom/game definitions that do not depend on desktop UI
- `src/SLADEMap/` — map data structures and map format readers/writers
- `src/Graphics/Palette/`
- Doom image/texture data representations after wxImage/OpenGL dependencies are split out

### Desktop/frontend-only for the first cut

- `src/Application/`
- UI/editor folders
- `src/OpenGL/` desktop renderer
- desktop file dialogs and process launching
- clipboard integration
- wxWebView/start page
- wxIPC/single-instance handling
- FTGL rendering
- desktop-specific SFML rendering
- optional audio/network/scripting integrations until the core builds independently

## Major dependency cuts

1. Stop using `Application/Main.h` as the universal include for portable files.
2. Introduce a small core common header containing standard-library includes and portable project types.
3. Remove wx types from core public interfaces.
4. Replace `wxFile`-based archive/memory I/O with portable stream/data abstractions.
5. Split filesystem operations from archive/resource logic.
6. Split `wxImage` decoding from the Doom/SLADE image representation.
7. Keep the desktop OpenGL renderer behind a frontend-specific boundary.
8. Leave optional scripting/audio/network facilities disabled until the portable core builds independently.

## Target dependency direction

    Android / Desktop frontend
              |
        platform adapters
              |
        distilled core
              |
      formats + data models
              |
       stdlib / small libs

The core must not depend on wxWidgets, desktop OpenGL, desktop windowing, or Android APIs.

## First functional milestone

A standalone core test target should be able to:

- open a WAD
- enumerate entries
- read entry bytes
- add/replace/remove/rename entries
- preserve directory/namespace structure
- save a WAD
- reopen the result and verify it

Android UI work starts after this target builds without wxWidgets.

## Upstream relationship

The default branch remains the upstream-compatible line. Changes on `distilled-core` should be small, focused, and easy to compare against upstream SLADE.
