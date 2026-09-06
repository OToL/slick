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

- [x] Clean naming convention in slk
- [ ] Remove PS3 (code, tools, cmake, etc.)
- [ ] Remove raylib in favor of bgfx
- [ ] Use singleton patten instead of global vars for InputApi and rename file form input.hpp to input_api.hpp
- [ ] Replace `samples` by `projects`
- [ ] Rework src folder e.g. utils, etc.
- [ ] Update `Depot Structure`
- [ ] Rework grphics sandbox e.g. `demos`, how demos are declared, etc.
- [ ] Remove pre-built libs
- [ ] Error managenebt (look at c++ contract)
- [ ] Single file compilation
