# Architecture: layers and packages

The engine core is built as 13 layers (A..M). Each layer is one module (package):
one folder under `modules/`, one C++ namespace, one CMake target, one test executable.

## Layers

| Layer | Module (folder) | Namespace | Classes | Depends on | Status |
|---|---|---|---|---|---|
| A | `core` | `td::core` | 13 of 26 (see below) | std only | **done** |
| B | `defs` | `td::defs` | 55 | A | planned |
| C | `validation` | `td::validation` | 24 | B | planned |
| D | `persistence` | `td::persistence` | 29 | C | planned |
| E | `mechanics` | `td::mechanics` | 44 | C | planned |
| F | `spatial` | `td::spatial` | 23 | B | planned |
| G | `state` | `td::state` | 30 | F, E | planned |
| H | `commands` | `td::commands` | 22 | G | planned |
| I | `engine` | `td::engine` | 35 | H | planned |
| J | `campaign` | `td::campaign` | 22 | I, D | planned |
| K | `replay` | `td::replay` | 16 | J | planned |
| L | `editor` | `td::editor` | 21 | K | planned |
| M | `api` | `td::api` / `td::godot` | 16 | L | planned |

Class counts are taken from the layer diagram. For layer A, the block description
lists 26 classes; only the 13 that the next layers use are written now.

```
M  Public API and Godot Integration
 |
L  Dev Tools
 |
K  Replay
 |
J  Campaign and Session ------+
 |                            |
I  Simulation Engine          |
 |                            |
H  Commands                   |
 |                            |
G  Simulation State           |
 |        |                   |
 |   E  Mechanics and Plugins |
 |        |                   D  Persistence
F  Spatial |                   |
 |        +---- C  Validation -+
 |             |
 +---- B  Definitions
        |
        A  Foundation
```

Only direct dependencies are drawn; the rest are transitive (for example M -> A).

## The dependency rule

Every dependency points down. A module may use the modules it depends on and,
through them, everything below. Nothing ever depends upward, so the graph has
no cycles. This is enforced by the build: a module's `td_add_module(... DEPENDS ...)`
lists exactly the modules it may include, and a header from any other module is
not on its include path.

## Folder layout of one module

```
modules/<name>/
  CMakeLists.txt              td_add_module(<name> SOURCES ... DEPENDS ... TESTS ...)
  include/td/<name>/*.h       public headers, included as "td/<name>/Thing.h"
  src/*.cpp
  tests/*.cpp                 Catch2; one ctest entry per module
  README.md
```

Module folders for layers that are not written yet contain only a `README.md`
(namespace, dependencies, status). They are added to `modules/CMakeLists.txt`
in the commit that implements them.

## Working in commits

One layer is one commit, bottom-up, and every commit builds and passes all tests
on its own.

| # | Commit | Adds |
|---|---|---|
| 1 | `feat(core): layer A, foundation` | repository skeleton, CMake, `td::core` with tests |
| 2 | `feat(defs): layer B, definitions` | configuration data model |
| 3 | `feat(validation): layer C` | validators, ports `IMechanicCatalog` / `IRouteVerifier` |
| 4 | `feat(persistence): layer D` | JSON read/write of definitions |
| 5 | `feat(mechanics): layer E` | mechanics and plugins |
| 6 | `feat(spatial): layer F` | hex map, routing |
| 7 | `feat(state): layer G` | simulation state |
| 8 | `feat(commands): layer H` | player commands |
| 9 | `feat(engine): layer I` | tick loop and systems |
| 10 | `feat(campaign): layer J` | campaign and session |
| 11 | `feat(replay): layer K` | replay and checksums |
| 12 | `feat(editor): layer L` | developer tools |
| 13 | `feat(api): layer M` | public API, GDExtension wrapper |

## Block A: classes written when first needed

The block description (`docs/design/A_foundation_detailed.md`) lists 26 classes.
Writing classes nobody uses yet is wasted code, so layer A contains only what
layers B and C use. The rest is added to `td::core` in the commit of the layer
that first needs it (the layer shown is the expected one).

| Class | Added with layer |
|---|---|
| `InstanceId`, `MonotonicCounter` | G (state) |
| `Segment`, `HexLayout` | F (spatial) |
| `IRandomGenerator`, `DeterministicRandom` | E (mechanics) or I (engine) |
| `Sha256`, `Digest` | D (persistence): config hash; K (replay): checksums |
| `EventPayload`, `EventDraft`, `Event`, `IEventSink`, `NullEventSink` | I (engine) |

`EventTypeId` is added together with the events.

## Design documents

`docs/design/` holds the agreed block descriptions (`00_DECISIONS.md`, blocks A, B, C).
The code follows them class by class.
