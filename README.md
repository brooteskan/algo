# algo

`algo` is a deterministic, non-owning functional algorithm library extracted
from the Wozzits Engine project.

The `v0.0.1-wozzits-baseline` tag preserves the four historical algorithm
headers and their legacy tests unchanged. Development after that tag treats
`algo/next.h` as the canonical API while retaining the older headers as
behavioral references.

## CMake target

```cmake
find_package(algo CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE algo::algo)
```

Existing include paths remain valid:

```cpp
#include <algo/algo.h>
#include <algo/ops.h>
#include <algo/next.h>
#include <algo/pipeline.h>
```

## Baseline status

The original Wozzits tests are included, including
`AlgoApplySpec.MultipleOpsExecuteSequentially`. That test expects pipeline-like
composition from the older `algo::apply`, while the current implementation
invokes each operation against the original input and shared bounded output.

CTest runs it separately as an expected failure. A new passing characterization
test records the implementation's current behavior. If the legacy expectation
starts passing, the expected-failure CTest fails and requires an intentional
baseline update.

See [BASELINE.md](BASELINE.md) and [KNOWN_ISSUES.md](KNOWN_ISSUES.md).

## Canonical `next` API

`algo/next.h` supplies immediate transform, filter, and reduce operations plus
composable map/filter pipelines. Range execution reports whether every input
was processed or an output sink rejected a value:

```cpp
const auto operation =
    wz::core::algo::next::filter(is_active)
    | wz::core::algo::next::map(to_transform);

const auto status = operation(nodes, output);
if (wz::core::algo::next::was_truncated(status))
{
    // The output sink rejected a value before the input was exhausted.
}
```

Per-element `apply_all` retains the boolean continuation contract required by
polytree traversal sinks. The range-level operation converts that signal into
`execution_status::completed` or `execution_status::truncated`.

## Build and test

```sh
git submodule update --init --recursive
cmake -S . -B build -DALGO_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests use the pinned GoogleTest submodule described below.

## Pinned source dependencies

Run `git submodule update --init --recursive` after cloning or changing revisions.
Standalone tests build GoogleTest from the `external/googletest` gitlink. CMake
requires that exact initialized revision; it does not fetch dependencies or use
an installed GoogleTest package. No sibling-checkout paths are needed.
