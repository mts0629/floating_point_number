# floating_point_number

Know floating-point number mechanisms

- Emulation of IEEE 754 binary32 expression (as `Binary32`)
- Emulation of binary32 calculation process

## Environment

- GCC 7.5.0
- GNU Make 4.1
- clang-format 14.0.0

## Build

`$ make`

Test programs are built in `build/bin/` (build and run with `make test`).

- `test/parse_fp32.c`: parse between `float` and `Binary32` type
- `test/calc_fp32.c`: calculation with `Binary32`

## License

Unlicense
