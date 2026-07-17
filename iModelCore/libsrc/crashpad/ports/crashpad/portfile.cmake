# Local vcpkg overlay port for crashpad. Used on WINDOWS ONLY: vcpkg_run_install.bat passes
# --overlay-ports for this directory, while vcpkg_run_install.sh (Linux/macOS/Android) does
# not, so those platforms build crashpad from the upstream vcpkg registry port instead.
#
# Forked from the upstream vcpkg registry `crashpad` port at version 2024-04-11#13, then
# modified locally (extra patches plus the clang-cl / MSBuild-header handling below). See
# ../../readme.md ("Updating crashpad") for the full version-update procedure. On Windows the
# version built is pinned by the three REF commit hashes below (crashpad, mini_chromium,
# lss), not by the top-level iModelCore/libsrc/crashpad/vcpkg.json constraint; keep them in
# sync with the upstream version used by the other platforms.

vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL https://chromium.googlesource.com/crashpad/crashpad
    REF 7e0af1d4d45b526f01677e74a56f4a951b70517d
    PATCHES
        fix-linux.patch
        fix-lib-name-conflict.patch
        crashpad-memset-errors-5758170.diff # https://chromium-review.googlesource.com/c/crashpad/crashpad/+/7270947
)

vcpkg_find_acquire_program(PYTHON3)
x_vcpkg_get_python_packages(OUT_PYTHON_VAR PYTHON3
    PYTHON_EXECUTABLE "${PYTHON3}"
    PYTHON_VERSION "3"
    PACKAGES setuptools
)
vcpkg_replace_string("${SOURCE_PATH}/.gn" "script_executable = \"python3\"" "script_executable = \"${PYTHON3}\"")

# mini_chromium contains the toolchains and build configuration
if(NOT EXISTS "${SOURCE_PATH}/third_party/mini_chromium/mini_chromium/BUILD.gn")
    vcpkg_from_git(OUT_SOURCE_PATH mini_chromium
        URL "https://chromium.googlesource.com/chromium/mini_chromium"
        REF dce72d97d1c2e9beb5e206c6a05a702269794ca3
        PATCHES
            fix-std-20.patch
            ndk-toolchain.diff
            support-build-tools-sku.diff
            fix-lib-name-conflict-1.patch
    )

    file(REMOVE_RECURSE "${SOURCE_PATH}/third_party/mini_chromium/mini_chromium")
    file(RENAME "${mini_chromium}" "${SOURCE_PATH}/third_party/mini_chromium/mini_chromium")
endif()

if(NOT EXISTS "${SOURCE_PATH}/third_party/lss/lss/BUILD.gn" AND (VCPKG_TARGET_IS_ANDROID OR VCPKG_TARGET_IS_LINUX))
    vcpkg_from_git(OUT_SOURCE_PATH lss
        URL https://chromium.googlesource.com/linux-syscall-support
        REF 9719c1e1e676814c456b55f5f070eabad6709d31
    )
    file(REMOVE_RECURSE "${SOURCE_PATH}/third_party/lss/lss")
    file(RENAME "${lss}" "${SOURCE_PATH}/third_party/lss/lss")
endif()

function(replace_gn_dependency INPUT_FILE OUTPUT_FILE LIBRARY_NAMES)
    if(NOT DEFINED VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "debug")
        unset(_LIBRARY_DEB CACHE)
        find_library(_LIBRARY_DEB NAMES ${LIBRARY_NAMES}
          PATHS "${CURRENT_INSTALLED_DIR}/debug/lib"
          NO_DEFAULT_PATH)

        if(_LIBRARY_DEB MATCHES "-NOTFOUND")
            message(FATAL_ERROR "Could not find debug library with names: ${LIBRARY_NAMES}")
        endif()
    endif()

    unset(_LIBRARY_REL CACHE)
    find_library(_LIBRARY_REL NAMES ${LIBRARY_NAMES}
        PATHS "${CURRENT_INSTALLED_DIR}/lib"
        NO_DEFAULT_PATH)

    if(_LIBRARY_REL MATCHES "-NOTFOUND")
        message(FATAL_ERROR "Could not find release library with names: ${LIBRARY_NAMES}")
    endif()

    if(VCPKG_BUILD_TYPE STREQUAL "release")
        set(_LIBRARY_DEB ${_LIBRARY_REL})
    endif()

    set(_INCLUDE_DIR "${CURRENT_INSTALLED_DIR}/include")

    file(REMOVE "${OUTPUT_FILE}")
    configure_file("${INPUT_FILE}" "${OUTPUT_FILE}" @ONLY)
endfunction()

replace_gn_dependency(
    "${CMAKE_CURRENT_LIST_DIR}/zlib.gn"
    "${SOURCE_PATH}/third_party/zlib/BUILD.gn"
    "z;zs;zlib;zd;zsd;zlibd"
)

set(OPTIONS "target_cpu=\"${VCPKG_TARGET_ARCHITECTURE}\"")
set(OPTIONS_DBG "is_debug=true")
set(OPTIONS_REL "")

if(VCPKG_TARGET_IS_ANDROID)
    vcpkg_cmake_get_vars(cmake_vars_file)
    include("${cmake_vars_file}")
    string(APPEND OPTIONS " target_os=\"android\" android_ndk_root=\"${VCPKG_DETECTED_CMAKE_ANDROID_NDK}\"")

elseif(VCPKG_TARGET_IS_LINUX)
    string(APPEND OPTIONS " target_os=\"linux\"")

elseif(VCPKG_TARGET_IS_OSX)
    string(APPEND OPTIONS " target_os=\"mac\"")

elseif(VCPKG_TARGET_IS_WINDOWS AND NOT VCPKG_TARGET_IS_MINGW)
    string(APPEND OPTIONS " target_os=\"win\"")

    # Load toolchains
    vcpkg_cmake_get_vars(cmake_vars_file)
    include("${cmake_vars_file}")

    cmake_path(CONVERT "${VCPKG_DETECTED_CMAKE_CXX_COMPILER}" TO_CMAKE_PATH_LIST CRASHPAD_CXX_COMPILER_PATH NORMALIZE)
    string(REGEX REPLACE "/VC/Tools/.*" "" CRASHPAD_VISUAL_STUDIO_PATH "${CRASHPAD_CXX_COMPILER_PATH}")
    if(NOT CRASHPAD_VISUAL_STUDIO_PATH STREQUAL CRASHPAD_CXX_COMPILER_PATH
       AND EXISTS "${CRASHPAD_VISUAL_STUDIO_PATH}/VC/Auxiliary/Build/vcvarsall.bat")
        # mini_chromium checks VSINSTALLDIR before it falls back to vswhere.
        set(ENV{VSINSTALLDIR} "${CRASHPAD_VISUAL_STUDIO_PATH}")
    endif()

    # Detect clang-cl. The clang triplets chainload the clang-cl toolchain, so
    # VCPKG_DETECTED_CMAKE_CXX_COMPILER points at clang-cl.exe. mini_chromium's Windows
    # toolchain hardcodes the MSVC tools (cl.exe/lib.exe/link.exe); swap them for the LLVM
    # equivalents (all resolvable by bare name on PATH) so crashpad is built with clang.
    set(CRASHPAD_USE_CLANG OFF)
    if(VCPKG_DETECTED_CMAKE_CXX_COMPILER MATCHES "clang")
        set(CRASHPAD_USE_CLANG ON)
    endif()

    if(CRASHPAD_USE_CLANG)
        message(STATUS "Building crashpad with clang-cl toolchain")

        # mini_chromium invokes each tool via `ninja -t msvc -e environment.<arch> -- <tool>`,
        # which runs <tool> under the vcvars-generated environment. That environment's PATH
        # contains the MSVC bin directory (so bare cl.exe resolves) but NOT the LLVM bin
        # directory, so bare clang-cl.exe fails with
        # "CreateProcess: The system cannot find the file specified."
        # Reference the LLVM tools by absolute path instead. Use the 8.3 short path because
        # `ninja -t msvc` tokenizes the command on spaces and "Program Files" would split it.
        if(NOT DEFINED ENV{LLVM_DIR})
            message(FATAL_ERROR "LLVM_DIR is not set; cannot locate the clang-cl toolchain for crashpad.")
        endif()
        set(CRASHPAD_LLVM_BIN_NATIVE "$ENV{LLVM_DIR}\\bin")
        execute_process(
            COMMAND cmd /c for %I in ("${CRASHPAD_LLVM_BIN_NATIVE}") do @echo %~sI
            OUTPUT_VARIABLE CRASHPAD_LLVM_BIN
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE CRASHPAD_SHORTPATH_RESULT)
        if(NOT CRASHPAD_SHORTPATH_RESULT EQUAL 0 OR CRASHPAD_LLVM_BIN STREQUAL "")
            set(CRASHPAD_LLVM_BIN "${CRASHPAD_LLVM_BIN_NATIVE}")
        endif()
        file(TO_CMAKE_PATH "${CRASHPAD_LLVM_BIN}" CRASHPAD_LLVM_BIN)
        message(STATUS "Using LLVM tools from: ${CRASHPAD_LLVM_BIN}")

        vcpkg_replace_string(
            "${SOURCE_PATH}/third_party/mini_chromium/mini_chromium/build/config/BUILD.gn"
            "      cc = \"cl.exe\"\n      cxx = \"cl.exe\"\n      ar = \"lib.exe\"\n      ld = \"link.exe\""
            "      cc = \"${CRASHPAD_LLVM_BIN}/clang-cl.exe\"\n      cxx = \"${CRASHPAD_LLVM_BIN}/clang-cl.exe\"\n      ar = \"${CRASHPAD_LLVM_BIN}/llvm-lib.exe\"\n      ld = \"${CRASHPAD_LLVM_BIN}/lld-link.exe\"")
    endif()

    set(OPTIONS_DBG "${OPTIONS_DBG} \
        extra_cflags_c=\"${VCPKG_COMBINED_C_FLAGS_DEBUG}\" \
        extra_cflags_cc=\"${VCPKG_COMBINED_CXX_FLAGS_DEBUG}\" \
        extra_ldflags=\"${VCPKG_COMBINED_SHARED_LINKER_FLAGS_DEBUG}\" \
        extra_arflags=\"${VCPKG_COMBINED_STATIC_LINKER_FLAGS_DEBUG}\"")

    set(OPTIONS_REL "${OPTIONS_REL} \
        extra_cflags_c=\"${VCPKG_COMBINED_C_FLAGS_RELEASE}\" \
        extra_cflags_cc=\"${VCPKG_COMBINED_CXX_FLAGS_RELEASE}\" \
        extra_ldflags=\"${VCPKG_COMBINED_SHARED_LINKER_FLAGS_RELEASE}\" \
        extra_arflags=\"${VCPKG_COMBINED_STATIC_LINKER_FLAGS_RELEASE}\"")

    if(CRASHPAD_USE_CLANG)
        # clang-cl does not use MSVC whole-program optimization, and llvm-lib/lld-link do not
        # accept /LTCG. mini_chromium compiles with /W4 /WX; clang 22 emits additional warnings
        # that would break the build under -Werror, so relax it for third-party crashpad code.
        # The MSVC STL (14.44 / VS2022 17.14+) requires Clang 19+ by default and hard-errors
        # (STL1000) on older toolsets. CI ships Clang 18, so define
        # _ALLOW_COMPILER_AND_STL_VERSION_MISMATCH to permit it, matching winntclangmdl.mki.
        set(DISABLE_WHOLE_PROGRAM_OPTIMIZATION "extra_cflags=\"-Wno-error /D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH\"")
    else()
        set(DISABLE_WHOLE_PROGRAM_OPTIMIZATION "\
            extra_cflags=\"/GL-\" \
            extra_ldflags=\"/LTCG:OFF\" \
            extra_arflags=\"/LTCG:OFF\"")
    endif()

    set(OPTIONS_DBG "${OPTIONS_DBG} ${DISABLE_WHOLE_PROGRAM_OPTIMIZATION}")
    set(OPTIONS_REL "${OPTIONS_REL} ${DISABLE_WHOLE_PROGRAM_OPTIMIZATION}")
endif()

vcpkg_gn_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS "${OPTIONS}"
    OPTIONS_DEBUG "${OPTIONS_DBG}"
    OPTIONS_RELEASE "${OPTIONS_REL}"
)

vcpkg_gn_install(
    SOURCE_PATH "${SOURCE_PATH}"
    TARGETS client client:common util third_party/mini_chromium/mini_chromium/base handler:crashpad_handler
)

message(STATUS "Installing headers...")
set(PACKAGES_INCLUDE_DIR "${CURRENT_PACKAGES_DIR}/include/${PORT}")
function(install_headers DIR)
    file(COPY "${DIR}" DESTINATION "${PACKAGES_INCLUDE_DIR}" FILES_MATCHING PATTERN "*.h")
endfunction()
install_headers("${SOURCE_PATH}/client")
install_headers("${SOURCE_PATH}/util")
install_headers("${SOURCE_PATH}/third_party/mini_chromium/mini_chromium/base")
install_headers("${SOURCE_PATH}/third_party/mini_chromium/mini_chromium/build")

file(COPY "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/gen/build/chromeos_buildflags.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/${PORT}/build")
file(COPY "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/gen/build/chromeos_buildflags.h.flags" DESTINATION "${CURRENT_PACKAGES_DIR}/include/${PORT}/build")

# On Windows/MSVC, mirror headers into the root include directory so MSBuild integration
# (which adds only <installed>/include) can resolve un-namespaced includes like
# "client/..." and "base/...".
if(VCPKG_TARGET_IS_WINDOWS AND NOT VCPKG_TARGET_IS_MINGW)
    message(STATUS "Mirroring headers into include root for MSBuild consumption...")
    file(COPY "${SOURCE_PATH}/client" DESTINATION "${CURRENT_PACKAGES_DIR}/include" FILES_MATCHING PATTERN "*.h")
    file(COPY "${SOURCE_PATH}/util" DESTINATION "${CURRENT_PACKAGES_DIR}/include" FILES_MATCHING PATTERN "*.h")
    file(COPY "${SOURCE_PATH}/third_party/mini_chromium/mini_chromium/base" DESTINATION "${CURRENT_PACKAGES_DIR}/include" FILES_MATCHING PATTERN "*.h")
    file(COPY "${SOURCE_PATH}/third_party/mini_chromium/mini_chromium/build" DESTINATION "${CURRENT_PACKAGES_DIR}/include" FILES_MATCHING PATTERN "*.h")
    file(COPY "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/gen/build/chromeos_buildflags.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include/build")
    file(COPY "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/gen/build/chromeos_buildflags.h.flags" DESTINATION "${CURRENT_PACKAGES_DIR}/include/build")
endif()

if(VCPKG_TARGET_IS_OSX)
    if(NOT DEFINED VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "debug")
        file(COPY "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg/obj/util/libmig_output.a" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")
    endif()
    file(COPY "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/obj/util/libmig_output.a" DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
endif()

vcpkg_copy_tools(
    TOOL_NAMES crashpad_handler
    SEARCH_DIR "${CURRENT_PACKAGES_DIR}/tools")

if(NOT VCPKG_TARGET_IS_WINDOWS OR VCPKG_TARGET_IS_MINGW)
    file(CHMOD "${CURRENT_PACKAGES_DIR}/tools/crashpad_handler" FILE_PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE
    )
endif()

# remove empty directories
file(REMOVE_RECURSE
    "${PACKAGES_INCLUDE_DIR}/util/net/testdata"
    "${PACKAGES_INCLUDE_DIR}/build/ios")

if(VCPKG_TARGET_IS_WINDOWS AND NOT VCPKG_TARGET_IS_MINGW)
    file(REMOVE_RECURSE
        "${CURRENT_PACKAGES_DIR}/include/util/net/testdata"
        "${CURRENT_PACKAGES_DIR}/include/build/ios")
endif()

configure_file("${CMAKE_CURRENT_LIST_DIR}/crashpadConfig.cmake.in"
        "${CURRENT_PACKAGES_DIR}/share/${PORT}/crashpadConfig.cmake" @ONLY)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/include/${PORT}/build/config")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/include/${PORT}/util/mach/__pycache__")

if(VCPKG_TARGET_IS_WINDOWS AND NOT VCPKG_TARGET_IS_MINGW)
    # Remove empty directory created under the mirrored root include
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/include/build/config")
endif()

vcpkg_copy_pdbs()
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
