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

- Fast iterations: Time from source modifications (code, data, etc.) to testing must be as short as possible
- Simplicity: Minimize encapsulation and avoid unnecessary generalizations
- Explicitness: Interfaces must be explicit

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
│   ├── shared/                  Optional common resources
│   │   ├── data/
│   │   ├── ...
│   │   └── extern/
│   └── <tool name>/             Simple tools can have all sources directly in the folder
│       ├── src/
│       ├── data/
│       ├── ...
│       └── extern/
├── samples/                     Applications demonstrating/testing specific features
│   ├── shared/                  Optional common resources
│   │   ├── data/
│   │   ├── ...
│   │   └── extern/
│   └── <sample name>/           Simple sample can have all sources directly in the folder
│       ├── src/
│       ├── data/
│       ├── ...
│       └── extern/
├── projects/                    Personal projects to learn & investigate new things
│   ├── shared/                  Optional common resources
│   │   ├── data/
│   │   ├── ...
│   │   └── extern/
│   └── <project name>/          Simple projects can have all sources directly in the folder
│       ├── src/
│       ├── data/
│       ├── ...
│       └── extern/
└── README.md
```

_NOTE_: folders containing generated files have a name starting with '_' e.g. _build
