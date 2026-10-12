# Image-filter C-model project

C++17 host/device simulation with a histogram-based median filter, C99 reference filters, and monochrome Y4M input/output. See [EXERCISES.md](EXERCISES.md) for the tasks and [REGISTER_MAP.md](REGISTER_MAP.md) for the device interface.

## Build on Linux

Requires CMake 3.16+, a C99/C++17 compiler, and Make. From the project directory:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Run samples and tests

From the project directory:

```sh
./build/median_demo data/lena_mono.y4m 1 result
./build/sobel_reference_demo data/lena_mono.y4m 0.7 edges_reference.y4m
./build/median_tests
```

- `median_demo [input.y4m] [radius] [output-prefix]`: compares device and reference pixels; the optional prefix saves `result_device.y4m` and `result_reference.y4m` in this example. Defaults: supplied image, radius 1, no files. The starter intentionally reports mismatches and exits with code 1 (exercise 1).
- `sobel_reference_demo [input.y4m] [gain] [output.y4m]`: runs the floating-point reference, model algorithm and saves a rounded 8-bit preview. Defaults: supplied image, gain 1, `sobel_reference.y4m`.
- `median_tests`: runs all registered tests without arguments; exits with 0 on success or 1 on failure. Add tests in `tests/median_tests.cpp`.

View Y4M files with [ffplay](https://ffmpeg.org/ffplay.html), FFmpeg's lightweight media player. Press `q` to quit:

```sh
ffplay result_device.y4m
```

## Y4M API

The `y4m_io` library exposes these functions in [include/y4m.h](include/y4m.h) for 8-bit monochrome (`Cmono`) images:

- `read_y4m(path, image, error)`: loads the first frame into `MonoImage` (`width`, `height`, packed `pixels`).
- `write_y4m(path, pixels, width, height, stride, error)`: writes one frame, omitting row padding. Stride is in bytes; the buffer must contain at least `(height - 1) * stride + width` bytes.

Both return `bool` and report failures through `error`. The supplied input is 512 × 512; see [data/README.md](data/README.md) for provenance.

## Code layout

| File | Responsibility |
| --- | --- |
| `CMakeLists.txt` | Libraries, demo executables, compiler settings, and input-data copying. |
| `include/register_map.h` | Common register offsets, command/status values, and register-bank size. |
| `include/base_filter.h` | Shared image descriptor, image limits. |
| `include/median_filter.h` | Median algorithm interfaces. |
| `include/sobel_filter.h` | Sobel filter algorithm interface. |
| `include/median_device.h`, `src/median_device.cpp` | Model device, worker thread, register access, validation, and status. |
| `include/host.h`, `src/host.cpp` | Host-side device interface. |
| `include/event.h` | Synchronization primitive used by the host and model. |
| `src/median_histogram.cpp` | Histogram-based median model algorithm. |
| `src/sobel_filter.cpp` | Sobel filter model algorithm. |
| `src/median_reference.c` | Independent C median reference using a padded image and sorting. |
| `include/sobel_reference.h`, `src/sobel_reference.c` | Floating-point edge-magnitude reference with packed double output. |
| `src/reference_image.h` | Shared image expansion with reflect-101 padding for the reference filters. |
| `src/sobel_reference_main.cpp` | Reference demo and 8-bit Y4M preview conversion. |
| `include/y4m.h`, `src/y4m.cpp` | Monochrome Y4M reader and buffer-to-file writer. |
| `src/main.cpp` | Median demo, memory comparison, and optional output saving. |
| `tests/median_tests.cpp` | Standalone test runner and device lifecycle test; extend with regression tests. |
| `data/lena_mono.y4m` | Default disk-based input image. |
