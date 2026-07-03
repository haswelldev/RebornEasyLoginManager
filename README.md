# Reborn Easy Login Manager

[![Website](https://img.shields.io/badge/Website-Visit-e8b24c?style=for-the-badge)](https://haswelldev.github.io/RebornEasyLoginManager/)
[![Windows Download](https://img.shields.io/badge/Download-Windows-blue?style=for-the-badge&logo=windows)](https://github.com/haswelldev/RebornEasyLoginManager/releases/latest/download/EasyLoginManager.exe)
[![macOS Download](https://img.shields.io/badge/Download-macOS-white?style=for-the-badge&logo=apple&logoColor=black)](https://github.com/haswelldev/RebornEasyLoginManager/releases/latest/download/EasyLoginManager.dmg)


A lightweight, cross-platform account manager designed for Lineage II players ([L2 Reborn](https://l2reborn.org/)). Securely manage your accounts, passwords, and descriptions with ease — everything stays **local to your machine**.

**Website:** https://haswelldev.github.io/RebornEasyLoginManager/


### Features

- **Multi-Account Management**: Add, edit, duplicate, reorder, and delete accounts (up to 199 per file).
- **Drag-to-reorder**: Drag a row with the mouse — a ghost of the row follows the cursor, an insertion line shows exactly where it will land, and the list auto-scrolls at the edges. Move to Top / Up / Down / Bottom buttons and shortcuts are also available.
- **Menu bar & keyboard shortcuts**: Full menu bar with accelerators (Ctrl+N/O/S, Ctrl+Shift+S to Save As, F2 to rename, Del to delete, and more).
- **Search / filter**: Instantly filter accounts by ID or description.
- **Copy password**: Right-click any account to copy its password to the clipboard without revealing it.
- **Safe editing**: Unsaved-changes tracking with a title-bar asterisk and prompts before closing, opening, or starting a new file.
- **Drag & drop**: Drop a `.ini` file onto the window to open it.
- **Game-client settings**: The `showOnStart` / `hideLogin` flags read by the L2 Reborn client can be toggled from Manager Settings (they are stored in the `.ini`, not settings of the manager itself).
- **Status bar**: Shows the current file path and account count, with inline save/copy feedback.
- **Inline Editing**: Quickly rename a description with double-click or F2.
- **Internationalization**: Full support for English, French, Greek, Portuguese, Polish, Ossetian, and Chinese.
- **Portable & Compact**: 
  - **Windows**: Single-file distribution! All translations are embedded directly into the executable.
  - **macOS**: Native `.app` bundle with DMG installer.
- **Modern UI**: Built with wxWidgets for a native look and feel on all platforms.
- **Tested**: Core INI parsing/serialization and translation loading are covered by a Catch2 unit-test suite (`-DELM_BUILD_TESTS=ON`), run in CI.

### Development

#### Prerequisites
- **Windows**: `vcpkg` (for dependency management).
- **macOS**: `Homebrew` (for wxWidgets).

#### Build Instructions

##### Windows (Visual Studio)
```powershell
# Install dependencies via vcpkg
vcpkg install

# Configure and build
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build build --config Release
```

##### macOS
```bash
# Install wxWidgets
brew install wxwidgets dylibbundler

# Configure and build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Technical Details

- **Language Support**: Uses a custom `LanguageManager` to handle JSON-based translations.
- **Windows Resource Integration**: Translation files are compiled into the Windows executable as `RCDATA` resources, allowing for a single-file distribution without external dependency folders.
- **Static Linking**: On Windows, it uses static linking to ensure the executable runs on systems without requiring additional DLLs.

### Translations

We currently support:
- English (`en.json`)
- French (`fr.json`)
- Greek (`el.json`)
- Portuguese (`pt.json`)
- Polish (`pl.json`)
- Ossetian (`os.json`)
- Chinese (`zh.json`)

To contribute a new translation, simply add a new `.json` file to the `i18n/` directory and update the `LanguageManager` and UI.

---

*Developed for the Reborn Lineage community.*
