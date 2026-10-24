# DnD5E Character Generator — Modular Foundation

The original 2018 prototype (`main.cpp`) is preserved at the repository root. The new
foundation is a **C++17, offline-first, data-driven 2014 5e character engine** with a
Qt 6 desktop preview for **macOS, Windows, and Linux**.

This is **not a finished character creator**: the current milestone loads modular
JSON definitions, validates dependencies/references, and calculates a demonstration
race/subrace/trait/feat subset. No completed class, skill, spell, equipment, saving,
PDF field mapping, or full 2014 SRD library is included yet.

## Build

Requirements: CMake 3.20+, C++17 compiler, Boost headers 1.75+.
Qt 6.4+ Widgets is additionally needed to build the desktop frontend.
Boost.JSON is compiled into the core (no separate Boost.JSON runtime library).

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
./build/dnd5e-cli Content
```

On Windows use the executable in the configuration directory (e.g., `build/Debug/`).
On macOS build using Xcode/Qt Creator; on Windows use MSVC/Qt Creator; on Linux
use Clang or GCC. Qt creates a platform-native desktop application from a **single
cross-platform codebase**. The CLI and tests build without Qt. To explicitly skip
GUI detection, add `-DDND_BUILD_GUI=OFF`.

The desktop program loads content next to its executable, from an installed
`Content/` directory, and from a writable user data `Content/` directory selected
by `QStandardPaths::AppDataLocation`. Homebrew can be installed without touching
the application bundle. The source directory is also consulted during development.

## Package grammar v1.0

A content package contains a `manifest.json` with `id`, `name`, `version`,
`schema_version`, `ruleset`, and `requires`. Definitions have `id`, `kind`, `name`,
`ruleset`, optional `traits`/`feats` references, and optional `effects`. Definitions use globally unique
qualified identifiers such as `examplepack:runic_dwarf`. `subrace.parent` references
its race. Referenced definitions must exist, match kinds/edition, and belong to the
same package or a **direct declared dependency**. Duplicate IDs and broken
references are rejected with diagnostics. No package contains executable code.

This first milestone recognizes four kinds: `race`, `subrace`, `trait`, `feat`.
Recognized effects: `ability.bonus`, `hit_points.bonus_per_level`,
`sense.grant`. Unknown effect operations are rejected (not silently ignored).

See `Content/Core2014/` and `Content/Homebrew/ExamplePack/` for examples.
Homebrew `ExamplePack` adds a subrace to the base Dwarf and automatically
grants a separately defined homebrew feat via a `feats` reference.

### Design boundaries

- Only **2014** calculations are implemented. Content bears a `ruleset` field so
  a 2024 implementation can be added without combining incompatible rules.
- Character choices are immutable inputs to `resolveCharacter`; each edit triggers
  a complete deterministic recalculation with no global mutation.
- The core distinguishes extra HP from traits from *total* maximum HP; classes,
  Constitution modifier, level-up policy, and hit dice have not been implemented.
- Ability scores are currently manually editable for the prototype preview.
- Input data is treated as untrusted. No JSON-defined scripts or native plugins.
- PDF export will consume resolved character data, not recalculate rules in UI.
- This demo race dataset is **partial**; it intentionally omits unimplemented
  proficiencies, save advantage, and other full-rule features.

## Distribution and licenses

Redistributed 2014 rules content should come from SRD 5.1 or other content with
suitable permissions. For SRD 5.1 content, include the following required notice:

> This work includes material taken from the System Reference Document 5.1
> ("SRD 5.1") by Wizards of the Coast LLC and available at
> https://dnd.wizards.com/resources/systems-reference-document. The SRD 5.1 is
> licensed under the Creative Commons Attribution 4.0 International License
> available at https://creativecommons.org/licenses/by/4.0/legalcode.

Qt distribution is subject to its applicable open-source or commercial license.
The app must package Qt runtime libraries and platform plugins for each OS.
The intended deliverables are `.app`/DMG for macOS, an installer for Windows,
and an AppImage/portable bundle for Linux, once packaging and CI are implemented.
