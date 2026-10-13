[![CI](https://github.com/BlakeAllenMagic/epc/actions/workflows/ci.yaml/badge.svg)](https://github.com/BlakeAllenMagic/epc/actions/workflows/ci.yaml)

# EPC

Embedded Platform Controller.
A general-use embedded platform controller to manage an embedded system, including validating data, logging events, and ensuring fault-tolerant operation.
Target hardware is an STM32 Nucleo-F439ZI. Board bring-up is confirmed. Current work is portable logic that builds and runs on a Linux host, tested automatically on every push.

## Status

| Module | Description | State |
|---|---|---|
| `crc16` | CRC-16/CCITT-FALSE | Done, tested |
| `frame` | COBS framing with CRC-16 integrity check | Done, tested |
| UART on target | Send and receive frames on the board | Next |

## Build

Requires CMake 3.16 or newer and a C11 compiler.

```
cmake -S . -B build
cmake --build build
```

Builds use `-Wall -Wextra -Werror`. Test targets also build with AddressSanitizer and UndefinedBehaviorSanitizer.

## Test

```
cd build && ctest --output-on-failure
```

CMake 3.16 does not support `ctest --test-dir`, so the command above changes into the build folder first.

## CI

GitHub Actions configures, builds, and runs all tests on a clean Ubuntu machine for every push and pull request. See `.github/workflows/ci.yaml`.

## Layout

```
include/epc/        Public headers
src/core/           Portable logic (no HW dependencies)
src/port/           HW-specific. Not implemented yet.
tests/              One executable per core module, e.g. test_crc16.c, test_frame.c
.github/workflows/  CI configuration
```

## Frame format

Each message on the wire is built as:

```
[ payload ][ CRC-16 ]  ->  COBS encode  ->  [ encoded bytes ][ 0x00 ]
```

- Payload: 0 to 256 bytes.
- CRC-16/CCITT-FALSE over the payload, appended big-endian (high byte first). Big-endian is used so that running the CRC over payload plus CRC gives 0, which lets the decoder verify a frame in one pass.
- COBS removes every `0x00` from payload and CRC, so `0x00` appears only as the frame delimiter. A receiver can always find the next frame boundary, even after errors.
- Maximum encoded frame: 261 bytes (`EPC_FRAME_MAX_ENCODED`).

The decoder returns the payload length on success, `EPC_FRAME_NEED_MORE` when no complete frame is present yet, or a negative error code for bad data. Leading delimiters are skipped.

## Conventions

- The `crc16` test asserts the published CRC-16/CCITT-FALSE check value `0x29B1` for ASCII input `123456789`.
- Frame test vectors were checked against an independent CRC implementation (`crccalc.com`), not against this project's own code.
