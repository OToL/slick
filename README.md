# What is it?

*Slick* is a simple framework to test, prototype and learn various software engineering topics/techinques such as ...
- Latest c/c++ constructs
- Game engine features
- Rendering techniques
- Profiling & optimization tricks
- LLM
- etc.

# Pillars

N/A

# Depot structure

```
<root>/
├── _build/                      Generated build system files
│   └── <target>_<config>/
├── slk/                         Shared c/c++ engine modules
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
│   └── src/                     Dependencies Git sub-module or snapshot to build them from sources
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
├── samples/                     Sample applications demonstrating Slick features
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

