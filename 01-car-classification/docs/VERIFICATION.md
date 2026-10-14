# Verification record

Verified on October 14, 2026 (UTC), using Linux x86-64, GCC 13.3.0,
CMake/CTest 4.4.4, and Python 3.12.14.

| Check | Result |
|---|---|
| Fresh C++17 Release build | Passed; no compiler warnings emitted |
| Complete CTest suite | 14/14 passed |
| Application-only build (`BUILD_TESTING=OFF`) | Passed |
| ASan/UBSan Debug build | Passed |
| ASan/UBSan CTest suite, with leak detection disabled | 14/14 passed |
| Four generated CSVs and three generated SVGs | Byte-for-byte identical to the uploaded originals |
| Three source datasets | Byte-for-byte identical to the uploaded originals |

The initial sanitizer run could not complete because LeakSanitizer reported
that process inspection was unavailable under this execution environment.
The suite was rerun with:

```bash
ASAN_OPTIONS="halt_on_error=1:detect_leaks=0" \
UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1" \
ctest --test-dir build-sanitize --output-on-failure
```

Address and undefined-behavior instrumentation remained enabled. This result
is not a successful leak check. The README retains the normal local sanitizer
command, without disabling leak detection. These checks cover the existing
suite and do not establish correctness for every possible input.

## Memory profiling

Memory profiling was reported complete during the earlier local development
workflow. No verified profiling measurements or raw profiler report are
included in this cleaned source package, so this revision makes no claim about
peak memory, allocation counts, or a measured performance improvement.
Sanitizer checks and resource profiling are separate forms of evidence.

For a reproducible local profiling run, build with debug symbols and profile
each executable against the unchanged datasets, from the project root:

```bash
cmake -S . -B build-profile -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-profile --parallel
```

On macOS, these commands report execution time and maximum resident set size:

```bash
/usr/bin/time -l ./build-profile/data_analysis
/usr/bin/time -l ./build-profile/linear_classifier
/usr/bin/time -l ./build-profile/mcculloch_pitts
```

For allocation-level analysis, use an allocation profiler such as Instruments
and retain its report alongside the compiler version, build configuration,
input dataset, command, and date. No new profiling measurements were collected
for this cleanup.
