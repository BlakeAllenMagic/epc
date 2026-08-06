#minimum and policy set
cmake_minimum_required(VERSION 3.16)

#names the project and declares what language
project(epc C)

#generates build/compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#cmake standard C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

#build policy
add_compile_options(-Wall -Wextra -Werror)

#defines build target from source, no type keyword so defaults to static (libcrc.a)
#static linking means CRC code is copied into the test binary at link time, with no runtime shared libary path to manage
add_library(crc src/core/crc16.c)

#adds include/ to header search path.
#PRIVATE: use only for .c files that need
#INTERFACE: use only for users sources
#PUBLIC: both
target_include_directories(crc PUBLIC include)

#turns on test support and generates CTestTestfile.cmake in directory that ctest reads
#without it, nothing runs
enable_testing()

#same as add library but produces a runnable binary at build/test_crc16
add_executable(test_crc16 tests/test_crc16.c)

#TODO: review what these are for
target_compile_options(test_crc16 PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
target_link_options(test_crc16 PRIVATE -fsanitize=address,undefined)

#links test against the library
#beyond linking, makes the PUBLIC include path flow into test_crc16
#also creates a dependency edge, so editing src/crc16.c and rebuilding relinks test automatically.
#omitting PRIVATE is legacy syntax
target_link_libraries(test_crc16 crc)

#registers a test named crc16 that runs the test_crc16 binary.
#because test_crc16 is a known target name, CMake substitutes its actual path
#pass/fail determined by exit code, zero pass, nonzero fail, hence test_crc16 only returns 0 if success
add_test(NAME crc16 COMMAND test_crc16)