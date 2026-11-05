vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO zeux/pugixml
    REF "v${VERSION}"
    SHA512 b8a70f1f230b0902b719346ce0a551eafe534f81262280dceeb92d5ad90ea4e635173e08e225bf66eb5f4724ac4568bd40dc923f184571f02502dac49bc0b7f5
    HEAD_REF master
)

# Bentley overlay: reproduce the export configuration the file-by-file build used so the
# pugi:: symbols leave iTwinPugixml (see ../../VCPKG_MIGRATION.md, Step 2).
#
# Upstream pugixml 1.15 only annotates the *shared* CMake target with dllexport; the *static*
# target (the linkage we build) relies entirely on pugiconfig.hpp for PUGIXML_API. We build a
# static archive and combine it into our own iTwinPugixml shared/static library, so we inject
# the dllexport/dllimport block into pugiconfig.hpp. The same patched header is delivered to
# consumers, keeping a single source of truth for the annotation. No .def is involved.
vcpkg_replace_string(
    "${SOURCE_PATH}/src/pugiconfig.hpp"
    "#define HEADER_PUGICONFIG_HPP"
[[#define HEADER_PUGICONFIG_HPP

// Bentley: export the pugi API from iTwinPugixml. dllexport while building the library
// (__PUGIXML_BUILD__ is defined via PUGIXML_BUILD_DEFINES), dllimport for consumers.
#if defined (_WIN32)
    #ifdef __PUGIXML_BUILD__
        #define PUGIXML_API __declspec(dllexport)
    #else
        #define PUGIXML_API __declspec(dllimport)
    #endif
#endif]]
)

vcpkg_check_features(
    OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        compact PUGIXML_COMPACT
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DPUGIXML_BUILD_TESTS=OFF
        # Bentley: define __PUGIXML_BUILD__ for the static target so pugiconfig.hpp emits the
        # dllexport side of the annotation injected above (PUGIXML_BUILD_DEFINES feeds the
        # static target's compile definitions).
        "-DPUGIXML_BUILD_DEFINES=__PUGIXML_BUILD__"
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/${PORT})
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.md")
