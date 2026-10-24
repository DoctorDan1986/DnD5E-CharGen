# DnD5E Character Generator — Modular Foundation v0.2.0

**2014 5e first; 2024 definitions reserved for a future implementation.**

This is a portable, offline-first C++17 rules engine with a Qt 6 Widgets GUI for
**macOS, Windows, and Linux**. The original 2018 `main.cpp` is preserved on the
repository's `master` branch. The new foundation is on a development branch.

**This is still a prototype, not a full 5e character sheet generator.** It does
not implement complete SRD content, backgrounds, proper class progression, all
feats, equipment management, spellcasting, full HP/AC, character-sheet PDFs, or
advancement legality. All custom examples are clearly marked *Example* and are
not official 5e content.

## Build

Requirements: CMake 3.20+, C++17 compiler, Boost 1.75+ (Boost.JSON headers),
Qt 6.4+ Widgets for GUI. Build-time Boost dependency is header-only with
`src/json.cpp`; the finished application does not need a separate Boost.JSON DLL.

### macOS (Qt's official installer + Homebrew Boost)

If Qt is installed from the official Qt Online Installer at, for example,
`$HOME/Qt/6.12.0/macos`, run from the project directory:

```sh
cmake -S . -B build -DDND_BUILD_GUI=ON \
  -DBoost_ROOT="$(brew --prefix boost)" \
  -DQt6_ROOT="$HOME/Qt/6.12.0/macos"
cmake --build build --parallel
ctest --test-dir build --output-on-failure
open build/dnd5e-gui.app
```

The Qt path above is **only an example**; use the directory containing
`lib/cmake/Qt6/Qt6Config.cmake` on your machine.

### CLI + unit tests (no Qt needed)

```sh
cmake -S . -B build-cli -DDND_BUILD_GUI=OFF -DBUILD_TESTING=ON
cmake --build build-cli --parallel
ctest --test-dir build-cli --output-on-failure
./build-cli/dnd5e-cli Content
```

Windows: use MSVC with a matching Qt Desktop kit (Qt kit and compiler architecture
must agree). Linux: use Qt 6 Widgets development packages and GCC or Clang.
The GUI is one cross-platform codebase, though platform packaging is unfinished.
With `DND_BUILD_GUI=ON`, configuration fails explicitly if Qt 6 Widgets is absent.

## JSON package grammar (schema 1.0; expanded in v0.2)

Every package has a `manifest.json` containing `id`, `name`, `version`,
`schema_version`, `ruleset`, and `requires`. IDs are lowercase namespaced strings,
such as `examplepack:rune_memory`. Packages are scanned recursively. A reference
to a definition in another package requires a declared, direct dependency;
missing dependencies or malformed definitions generate diagnostics and are
rejected rather than silently executed. Modules contain no executable code.

Recognized definition kinds: `race`, `subrace`, `class`, `trait`, `feat`.

Recognized effects:

| Operation | Required arguments |
|---|---|
| `ability.bonus` | `ability`, `value` |
| `hit_points.bonus_per_level` | `value` |
| `sense.grant` | `name`, `range` |
| `proficiency.grant` | `category`, `id` |
| `proficiency.expertise` | `category` (`skill` or `tool`), `id` |

Proficiency categories are `skill`, `saving_throw`, `tool`, `weapon`, `armor`,
`language`. IDs for skills and saves are checked against the 2014 5e rules.
Duplicate proficiency grants do not double the proficiency bonus. Expertise is
allowed only on an already-proficient skill or tool and doubles proficiency for
skill checks. All 18 skill modifiers and six saving throw bonuses are derived.

### Automatically generated choices

A feature can request a player choice using `choices`:

```json
{
  "id": "examplepack:rune_memory",
  "kind": "trait",
  "name": "Rune Memory (Example)",
  "ruleset": "5e-2014",
  "choices": [{
    "id": "rune_skills",
    "label": "Choose two skills",
    "min": 2,
    "max": 2,
    "options": [
      {"id": "arcana", "label": "Arcana", "effects": [
        {"op": "proficiency.grant", "category": "skill", "id": "arcana"}
      ]},
      {"id": "history", "label": "History", "effects": [
        {"op": "proficiency.grant", "category": "skill", "id": "history"}
      ]}
    ]
  }]
}
```

`max` cannot exceed available options; `min <= max`. An absent required choice
produces a pending request and incomplete status, not invented default values.
Submitted choices are checked for valid IDs, duplicates, and size constraints.
The GUI renders one dropdown for single selections and checkboxes for multiple.
Every choice is saved by **source ID and local choice ID**:
`examplepack:rune_memory#rune_skills`. A level-gated source is keyed as
`examplepack:practice_guardian@3#bonus_skill`.

### Feature progression and prerequisites

`progression` entries are keyed by level; each can supply `traits`, `feats`,
`effects` and `choices`. Level gates apply from the designated level onward.
Selected feats may declare an AND-list of `prerequisites`, whose types are
`level`, `ability`, `race`, `subrace`, `class`, `feat`. A feat's own bonuses do not
make it eligible for itself. **Note:** retrospective feat-acquisition legality,
multiclassing, and prerequisites for non-selected granted features are not
implemented in this milestone.

Non-ability effects can have a `when` condition, combining `level_at_least`,
`score_at_least` + `ability`, and/or `equipment_tag`. Conditional ability increases
are deliberately rejected to avoid dependency cycles. Equipment tags are still
prototype inputs, not a completed equipment/inventory rules engine.

### Save format

The GUI has **Open character JSON** and **Save character JSON** actions.
Character files have `format: "dnd5e.character"` and `version: 1` and store
edition, character IDs, level, base ability scores, feat IDs, equipment tags,
and option selections. Computed bonuses are *not* saved, so reopened characters
recalculate from definitions. Save uses Qt `QSaveFile` for atomic replacement.
A saved character may be an incomplete draft; invalid or unavailable definitions
are rejected upon loading. Future versions should track exact module versions
and support intentional migrations when homebrew data changes.

### Try the example package

The included **ExamplePack** adds Runic Dwarf (two-of-three skill choice),
Practice Guardian (illustrative level-gated class features and conditional
shield training), Iron Constitution, Rune Mastery (expertise), and Studious
Focus (requires level 4 and INT 13). These exist to exercise the grammar, **not**
to represent complete official race/class mechanics.

## Design boundaries

- Content, calculation, and Qt UI remain separate components.
- All computations are deterministic and rebuilt from selections on each update.
- Only 2014 5e calculations are implemented; edition tags prevent mixing rules.
- Character sheet PDFs will consume resolved character data rather than compute rules themselves.
- No JSON-executed code, arbitrary scripts, or unchecked C++ plugin loading.
- Required choice and prerequisite failures are visible rather than auto-resolved.
- Qt Widgets GUI source has not yet been independently compiled/tested in the
  development environment for v0.2.0; C++ core/CLI and automated tests pass on Linux.

## Licensing

Only redistribute content licensed or permitted for reuse. SRD 5.1 is available
under Creative Commons BY 4.0; this project is not a copy of every official
2014 sourcebook. The SRD attribution notice when redistributing its material is:

> This work includes material taken from the System Reference Document 5.1
> ("SRD 5.1") by Wizards of the Coast LLC and available at
> https://dnd.wizards.com/resources/systems-reference-document. The SRD 5.1 is
> licensed under the Creative Commons Attribution 4.0 International License
> available at https://creativecommons.org/licenses/by/4.0/legalcode.

Distribution of Qt libraries and plugins must comply with applicable Qt licenses.
Standalone macOS/Windows/Linux release packages are planned, not yet built.
