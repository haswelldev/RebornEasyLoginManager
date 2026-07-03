# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

EasyLoginManager is a cross-platform C++11/wxWidgets GUI app for managing Lineage II (L2 Reborn) account credentials stored in a proprietary INI file format. It supports up to 199 accounts per file.

## Build Commands

### macOS
```bash
brew install wxwidgets dylibbundler   # one-time setup
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Windows (Visual Studio with vcpkg)
```powershell
vcpkg install   # installs wxwidgets from vcpkg.json manifest
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build build --config Release
```

### Tests (Catch2, opt-in)
```bash
brew install catch2   # one-time setup
cmake -S . -B build-tests -DCMAKE_BUILD_TYPE=Debug -DELM_BUILD_TESTS=ON
cmake --build build-tests --target ELMTests --parallel
ctest --test-dir build-tests --output-on-failure
```
The suite covers `IniParser` and `LanguageManager::LoadFromContent`. CI runs it (job `test` in `build.yml`) before the platform builds.

## Architecture

The app is a single-window wxWidgets application with these components:

- **`main.cpp`** — `MyApp::OnInit()` sets the app/vendor name (single `wxConfig` store), resolves the language (saved setting, then system language if a matching translation ships), loads it via `LanguageManager`, then constructs `MainFrame`.
- **`MainFrame`** — The sole application window. Menu bar + toolbar buttons route to shared command handlers. Holds `std::vector<Account> m_accounts` as the in-memory model, synced to a `wxListView` where each row stores its account index via `SetItemData` (rows can be filtered by the search box, so row index ≠ account index). Tracks unsaved changes via `MarkDirty()`; close/open/new prompt through `ConfirmDiscardChanges()`. Inline description editing uses a floating `wxTextCtrl` overlay. Drag-to-reorder shows a semi-transparent ghost row (`wxPopupWindow`), a thin insertion line between rows, and edge auto-scroll — the drop uses the same insertion index as the line (`DragTargetIndex`, range `[0, count]`).
- **`IniParser`** — Free functions `ParseIni`/`SerializeIni` (pure, unit-tested). `MainFrame` does file I/O around them.
- **`Account`** — Plain struct: `id`, `password`, `description`.
- **`AccountDialog`** — Modal dialog for add/edit. Reveal/hide password toggle; validates non-empty ID and warns on duplicates (gets the account list + index being edited).
- **`SettingsDialog`** — Modal dialog for `showOnStart` and `hideLogin`. These are pass-through flags written to the `.ini` for the **game client** to read; the manager itself does not act on them (the dialog says so).
- **`LanguageManager`** — Singleton. `LoadLanguage(code)` resolves the JSON (Windows resource → filesystem), `LoadFromContent(code, json)` parses it (testable, no I/O). A global helper `L("KEY")` is used throughout for all UI strings.
- **`Languages.h`** — Single source of truth for available languages (`{code, endonym}` table) driving the picker, code↔index mapping, and startup detection.

## INI File Format

The managed file uses the `[L2REBORN_EASYLOGIN]` section with 1-indexed slots:
```
[L2REBORN_EASYLOGIN]
1_id=username
1_password=secret
1_description=My Account
showOnStart=0
hideLogin=0
```

`ParseIni` parses this manually (no ini library); unknown lines are preserved verbatim and re-emitted on save. `SerializeIni` always rewrites all 199 slots, padding empty ones.

## Internationalization

- Translation files live in `i18n/<lang>.json` (e.g. `en`, `fr`, `el`, `pt`, `pl`, `os`, `zh`).
- `LanguageManager::LoadLanguage` uses a minimal hand-written JSON parser (no external dependency).
- **On Windows**, translation JSON files are compiled into the executable as `RCDATA` resources (named `ID_I18N_EN`, etc.) via `resources.rc`, enabling single-file distribution. The loader tries Windows resources first, then falls back to the filesystem.
- **On macOS**, JSON files are bundled inside `EasyLoginManager.app/Contents/Resources/i18n/`.
- To add a language: add `i18n/<code>.json` (same key set as `en.json`), add the `{code, endonym}` entry to the table in `Languages.h`, add the file to `CMakeLists.txt`'s `I18N_FILES`, and add `ID_I18N_<CODE>` to `resources.rc` for Windows. The picker, selection mapping, and startup detection all derive from the `Languages.h` table.

## Platform Notes

- Windows builds use `/utf-8` compile flag and `WIN32_EXECUTABLE` (no console window). Static CRT linkage (`MultiThreaded`) ensures a single `.exe` with no DLL dependencies.
- macOS builds produce a `.app` bundle with bundle ID `com.l2reborn.easyloginmanager`.
- The CI workflow (`.github/workflows/build.yml`) builds both platforms and publishes `.exe` and `.dmg` artifacts; releases are triggered by `v*` tags.
