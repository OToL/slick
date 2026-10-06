# What is it?

*Slick* is a simple framework to experiment with various Game Development aspects and ideas ...
- Core languages' constructs (e.g. latest C/C++ standard, Zig, etc.)
- Game engine features
- Gameplay mechanics
- Rendering techniques
- Profiling & optimization tricks
- LLM assisted development
- etc.

# Pillars

- Fast iterations: Time from source modifications (code, data, etc.) to testing must be as short as possible.
- Simplicity: Minimize encapsulation and avoid unnecessary generalizations.
- Explicitness: Interfaces must be explicit in their name, parameters, intents, etc.

# Naming convention

- file extensions:
    - c++ header/source: hpp/cpp
    - c header/source: h/c
    - hidden inline implementaton: inl
- file names: snake case e.g. dispatch_tag.hpp
- types (class, struct, enums, etc.): pascal case e.g. MyClass
- struct/class methods: camel case e.g. setLookAt
- constants and enum values: screaming snake case e.g. MY_CONSTANT
- variables: \[scope specifier\]_ + snake case
    - global : `g_` e.g. g_my_var
    - struct/class with methods : `m_` e.g. m_my_member
    - plain data struct (i.e. w/o method), parameter or local: no scope modifider e.g. my_member

# Depot structure

```
<root>/
├── _build/                      Generated build system files (ninja, etc.)
│   └── <target>_<config>/
├── slk/                         Shared c/c++ engine modules used by other components
│   ├── src/
│   │   └── <module name>/
│   ├── data/
│   ├── ...
│   └── test/                    Unit tests
├── extern/                      External dependencies
│   ├── redist/                  Pre-built external dependencies
│   │   ├── include/
│   │   │   └── <dep name>/
│   │   ├── lib/
│   │   │   └── <target>/
│   │   └── bin/
│   │       └── <target>/
│   └── src/                     Dependencies sources (Git sub-module or snapshot) 
│       └── <dep name>/
├── scripts/                     Utility scripts e.g. make, build, etc.
├── tools/                       Tools sources
│   └── <tool name>/
│       ├── src/
│       ├── data/
│       ├── ...
│       └── extern/
├── samples/                     Applications demonstrating/testing specific features
│   └── <sample name>/
│       ├── src/
│       ├── data/
│       ├── ...
│       └── extern/
├── projects/                    Personal projects to learn & investigate new things
│   └── <project name>/
│       ├── src/
│       ├── data/
│       ├── ...
│       └── extern/
└── README.md
```

- Folders containing generated files (e.g. \_build) must have their name starting with '_'.
- Simple projects, samples and tools can have all their sources stored directly under their root.

# TODO

## Board

- [ ] Camera management in demos for trackpad and mouse/kb
- [ ] Camera limit angle to avoid gimbal lock
- [ ] Bgfx type conversion
- [ ] Use slk namespace for projects
- [ ] Hash support + litterals and use it for DemoId

## Backlog

- [ ] Load launch.json only when it has changed
- [ ] pass Demo grid `grid_scalar_params` as parameter
- [ ] Infinite Grid e.g. offset the grid when too far
- [ ] Use rag for extern knowledge
- [ ] Claude skill to use nvim
- [ ] Ignore cmake errors from 3rdparty
- [ ] Instruct claude to use servers (e.g. lsp and dap) when asking for something related
- [ ] Give some intruction to claude about cmake and build
- [ ] Try Metal GPU capture + maybe claude skill
- [ ] Use launch.json for F6
- [ ] Error managenent (look at c++ contract)
- [ ] Rework src folder e.g. utils, etc.
- [ ] Update `Depot Structure`
- [ ] Enable sanitizers (UB, fuzzer, Adress, etc.)
- [ ] Try clang-tidy
- [ ] Impolement own parser for launch.json instead of using the dap config + timestamp to not parse the file each time

## Archives

- [x] Launch (F6) and active project using the launch json --> use dap to populate everything e.g. including active projects
- [x] Rename color constants
- [x] Rework graphics sandbox e.g. `demos`, how demos are declared, etc.
- [x] Remove raylib in favor of bgfx
- [x] Clean naming convention in slk
- [x] Remove pre-built libs
- [x] Remove PS3 (code, tools, cmake, etc.)
- [x] Use singleton patten instead of global vars for InputApi and rename file form input.hpp to input_api.hpp
- [x] Replace `samples` by `projects`
- [x] Cleanup nvim environment management (SetActiveProject, SetActiveTargetPlatform, get rid of BuildDarwin, good defaults, etc.)
- [x] Compile single file
- [x] Single file compilation

