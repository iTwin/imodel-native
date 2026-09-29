// Snappy config.h - generated for Bentley imodel-native build
// This replaces the CMake-generated config.h for the vendored snappy 1.2.2

#ifndef THIRD_PARTY_SNAPPY_OPENSOURCE_CMAKE_CONFIG_H_
#define THIRD_PARTY_SNAPPY_OPENSOURCE_CMAKE_CONFIG_H_

// Compiler intrinsics - available on all our target compilers (MSVC, GCC, Clang)
#if defined(__GNUC__) || defined(__clang__)
#define HAVE_ATTRIBUTE_ALWAYS_INLINE 1
#define HAVE_BUILTIN_CTZ 1
#define HAVE_BUILTIN_EXPECT 1
#define HAVE_BUILTIN_PREFETCH 1
#else
#define HAVE_ATTRIBUTE_ALWAYS_INLINE 0
#define HAVE_BUILTIN_CTZ 0
#define HAVE_BUILTIN_EXPECT 0
#define HAVE_BUILTIN_PREFETCH 0
#endif

// POSIX headers
#if defined(__linux__) || defined(__APPLE__)
#define HAVE_SYS_MMAN_H 1
#define HAVE_FUNC_MMAP 1
#define HAVE_FUNC_SYSCONF 1
#define HAVE_UNISTD_H 1
#define HAVE_SYS_UIO_H 1
#define HAVE_SYS_RESOURCE_H 1
#define HAVE_SYS_TIME_H 1
#else
#define HAVE_SYS_MMAN_H 0
#define HAVE_FUNC_MMAP 0
#define HAVE_FUNC_SYSCONF 0
#define HAVE_UNISTD_H 0
#define HAVE_SYS_UIO_H 0
#define HAVE_SYS_RESOURCE_H 0
#define HAVE_SYS_TIME_H 0
#endif

// Windows
#if defined(_WIN32)
#define HAVE_WINDOWS_H 1
#else
#define HAVE_WINDOWS_H 0
#endif

// Optional compression libraries - we don't link against these
#define HAVE_LIBLZO2 0
#define HAVE_LIBZ 0
#define HAVE_LIBLZ4 0

// SIMD support - disable SSSE3/BMI2 to avoid requiring -mssse3/-mbmi2 compiler flags
#define SNAPPY_HAVE_SSSE3 0
#define SNAPPY_HAVE_X86_CRC32 0
#define SNAPPY_HAVE_BMI2 0

// ARM NEON
#if defined(__aarch64__) || defined(_M_ARM64)
#define SNAPPY_HAVE_NEON 1
#else
#define SNAPPY_HAVE_NEON 0
#endif
#define SNAPPY_HAVE_NEON_CRC32 0

// Endianness - all our targets are little-endian
#define SNAPPY_IS_BIG_ENDIAN 0

#endif  // THIRD_PARTY_SNAPPY_OPENSOURCE_CMAKE_CONFIG_H_
