# EPC
Embedded Platform Controller.
This is a general-use embedded platform controller to manage an embedded system, including validating data, logging events, and ensuring fault-tolerant operation.

Hardware is not yet in hand. Current work is portable logic that builds and runs on a Linux host.
## Build

Requires CMake 3.16 or newer and a C11 compiler.

```
cmake -S . -B build
cmake --build build
```

## Test

```
cd build && ctest --output-on-failure
```

CMake 3.16 does not support 'ctest --test-dir'.

## Layout

```
include/epc/    Public headers
src/core/       Portable logic (no HW dependencies)
src/port/       HW-specific. Not implemented yet.
tests/          One executable per core module, e.g. test_crc16.c
```

## Conventions

The 'crc16' test asserts published CRC-16/CCITT-FALSE check value '0x29B1' for ASCII input '123456789'.