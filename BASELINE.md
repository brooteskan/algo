# Wozzits baseline

The imported files come from Wozzits `master` at:

```text
0ca3377dd5d9472a5a73426646026f2b085994e1
```

They are intentionally unchanged. The test-only `containers/buffer.h` is not a
public dependency of `algo::algo`.

## Imported files

| Repository path | Wozzits source path | SHA-256 |
|---|---|---|
| `include/algo/algo.h` | `window_engine/algo/algo.h` | `0F82DC3E8C9ACA5195D65BB0B1E1A6317C1F6C0375ACDE974F497FB516BD9BD4` |
| `include/algo/ops.h` | `window_engine/algo/ops.h` | `F9D3523210B7CF6907660F696FF956010DB4B755CA18BC1617E46D1A660A750D` |
| `include/algo/next.h` | `window_engine/algo/next.h` | `1B5A5644831EC0B3ACB91335851A4456E32317683AC0E432F95E021908197987` |
| `include/algo/pipeline.h` | `window_engine/algo/pipeline.h` | `7D4D61A963CD408ECFD30A3A79FBA1272A7D46BB1C8601B270DB3DAA3E3CC22C` |
| `tests/algo/algo_test_1.cpp` | `tests/algo/algo_test_1.cpp` | `5DABC62C65E4706CD92E76562904C01A9B2FC68BDA7A4962AF715F90F7FEB3DF` |
| `tests/algo/algo_test_next.cpp` | `tests/algo/algo_test_next.cpp` | `9152E5078F5F5DF7D1D8B9CB0C1F0FF7732B7C2816C5B241778E6767F1EED036` |
| `tests/algo/algo_test_pipeline.cpp` | `tests/algo/algo_test_pipeline.cpp` | `63C0837C9D933219E5D1F17406209750286C7D4925A78050BEB65D5E95630609` |
| `tests/support/containers/buffer.h` | `window_engine/containers/buffer.h` | `CD50F50E4F4E9E976D50315E15BD2D2E8AEE842DF71B14E58BD993367D57EA19` |

## Wozzits pre-extraction result

The six focused Wozzits executables built successfully with the `clang-debug`
preset. Five executables passed. `algo_algo_test_1` passed 28 of 29 tests and
failed only `AlgoApplySpec.MultipleOpsExecuteSequentially`.

The extracted package preserves that test and runs it as an expected failure.
