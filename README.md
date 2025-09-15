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

&lt;root&gt;/
├─ _build/                      Generated build system files
│   └─ &lt;target&gt;_&lt;config&gt;/
├─ slk/                         Shared c/c++ engine modules
│   ├─ src/
│   │   └─ &lt;module name&gt;/
│   ├─ data/
│   ├─ ...
│   └─ test/                    Unit tests
├─ extern/                      External dependencies
│   ├─ redist/                  Pre-built external dependencies
│   │   ├─ include/
│   │   │   └─ &lt;dep name&gt;/
│   │   ├─ lib/
│   │   │   └─ &lt;target&gt;/
│   │   └─ bin/
│   │       └─ &lt;target&gt;/
│   └─ src/                     Dependencies Git sub-module or snapshot to build them from sources
│       └─ &lt;dep name&gt;/
├─ scripts/                     Utility scripts e.g. make, build, etc.
├─ tools/                       Tools sources
│   ├─ shared/                  Optional common resources
│   │   ├─ data/
│   │   ├─ ...
│   │   └─ extern/
│   └─ &lt;tool name&gt;/             Simple tools can have all sources directly in the folder
│       ├─ src/
│       ├─ data/
│       ├─ ...
│       └─ extern/
├─ samples/                     Sample applications demonstrating Slick features
│   ├─ shared/                  Optional common resources
│   │   ├─ data/
│   │   ├─ ...
│   │   └─ extern/
│   └─ &lt;sample name&gt;/           Simple sample can have all sources directly in the folder
│       ├─ src/
│       ├─ data/
│       ├─ ...
│       └─ extern/
├─ projects/                    Personal projects to learn & investigate new things
│   ├─ shared/                  Optional common resources
│   │   ├─ data/
│   │   ├─ ...
│   │   └─ extern/
│   └─ &lt;project name&gt;/          Simple projects can have all sources directly in the folder
│       ├─ src/
│       ├─ data/
│       ├─ ...
│       └─ extern/
└─ README.md

