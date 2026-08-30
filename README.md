# algo

`algo` is a deterministic, non-owning functional algorithm library extracted
from the Wozzits Engine project.

This initial version is deliberately a faithful baseline. The four historical
algorithm headers and their legacy tests are preserved unchanged so later API
consolidation can proceed from a working, measurable extraction.

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

## Build and test

```sh
cmake -S . -B build -DALGO_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Tests use an installed GTest package when available and otherwise fetch the
pinned upstream GTest tag.
