#oldest version of CMake
cmake_minimum_required(VERSION 3.16)

#names the project and declares what language
project(epc C)

#requests C11, not a requirement if compiler can't
set(CMAKE_C_STANDARD 11)

#adds flags to every target defined in directory and subdirs
#catches C bugs that are technically legal but end up causing errors, like unused variables
#older style
#add_compile_options(-Wall -Wextra)

#modern style, applied per target
target_compile_options(crc PRIVATE -Wall -Wextra)

#defines build target from source, no type keyword so defaults to static (libcrc.a)
#static linking means CRC code is copied into the test binary at link time, with no runtime shared libary path to manage
add_library(crc src/crc16.c)

#adds include/ to header search path.
#PRIVATE: applies when compiling specific files only
#INTERFACE: applies to consumers only
#PUBLIC: both
#crc16.c includes crc16.h so crc build target needs it
#test_crc16.c compiles without any include path of its own
#TODO: explain some more
target_include_directories(crc PUBLIC include)

#turns on test support and generates CTestTestfile.cmake in directory that ctest reads
#without it, nothing runs
enable_testing()

#same as add library but produces a runnable binary at build/test_crc16
add_executable(test_crc16 tests/test_crc16.c)

#links test against the library
#beyond linking, makes the PUBLIC include path flow into test_crc16
#also creates a dependency edge, so editing src/crc16.c and rebuilding relinks test automatically.
#omitting PRIVATE is legacy syntax
target_link_libraries(test_crc16 crc)

#registers a test named crc16 that runs the test_crc16 binary.
#because test_crc16 is a known target name, CMake substitutes its actual path
#pass/fail determined by exit code, zero pass, nonzero fail, hence test_crc16 only returns 0 if success
add_test(NAME crc16 COMMAND test_crc16)