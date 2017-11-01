/*--------------------------------------------------------------------------------------+                                                                                                                             '
|
|     $Source: PublicAPI/DgnPlatform/TileIO.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/DgnTexture.h>
#include <DgnPlatform/DgnMaterial.h>

#define BEGIN_TILETREE_IO_NAMESPACE BEGIN_TILETREE_NAMESPACE namespace IO {
#define END_TILETREE_IO_NAMESPACE } END_TILETREE_NAMESPACE
#define USING_NAMESPACE_TILETREE_IO using namespace BentleyApi::Dgn::TileTree::IO;

//=======================================================================================
//! Classes and constants for reading and writing 3D tiles in various formats.
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
BEGIN_TILETREE_IO_NAMESPACE

//! A web standard for defining 3D content. 3D tile formats contain gltf data along with additional data.
struct Gltf
{
    static constexpr Utf8CP Magic() { return "glTF"; }
    static constexpr uint32_t Version1() { return 1; }
    static constexpr uint32_t Version2() { return 2; }
    static constexpr uint32_t Version() { return Version1(); }
    static constexpr uint32_t SceneFormat() { return 0 ;}

    enum Constants : int32_t
    {
        // Primitive types
        Lines = 1,
        LineStrip = 3,
        Triangles = 4,

        // Data types
        SignedByte = 0x1400,
        UnsignedByte = 0x1401,
        SignedShort = 5122,
        UnsignedShort = 5123,
        UInt32 = 5125,
        Float = 5126,
        Rgb = 6407,
        Rgba = 6408,
        IntVec2 = 0x8b53,
        IntVec3 = 0x8b54,
        FloatVec2 = 35664,
        FloatVec3 = 35665,
        FloatVec4 = 35666,
        FloatMat3 = 35675,
        FloatMat4 = 35676,
        Sampler2d = 35678,

        // Miscellaneous
        CullFace = 2884,
        DepthTest = 2929,
        Nearest = 0x2600,
        Linear = 9729,
        LinearMipmapLinear = 9987,
        ClampToEdge = 33071,
        ArrayBuffer = 34962,
        ElementArrayBuffer = 34963,
        FragmentShader = 35632,
        VertexShader = 35633,
    };
};

//! A web standard for defining 'batched 3D models'
struct B3dm
{
    static constexpr Utf8CP Magic() { return "b3dm"; }
    static constexpr uint32_t Version() { return 1; }
};

//! A web standard for defining 'instanced 3D models'
struct I3dm
{
    static constexpr Utf8CP Magic() { return "i3dm"; }
    static constexpr uint32_t Version() { return 1; }
};

//! A web standard for defining point clouds.
struct PointCloud
{
    static constexpr Utf8CP Magic() { return "pnts"; }
    static constexpr uint32_t Version() { return 1; }
};

//! A web standard for defining vector tiles.
struct Vector
{
    static constexpr Utf8CP Magic() { return "vctr"; }
    static constexpr uint32_t Version() { return 1; }
};

//! A web standard for combining multiple tiles of other formats (b3dm, i3dm, etc) into a single file.
struct Composite
{
    static constexpr Utf8CP Magic() { return "cmpt"; }
    static constexpr uint32_t Version() { return 1; }
};

//=======================================================================================
// DgnTiles are tiles with GLTF like structure (JSon with binary data) - but optimized
// for caching tiles (storing feature table directly etc.)
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct DgnTile
{
    static constexpr Utf8CP Magic() { return "dgnT"; }
    static constexpr uint32_t Version() { return 0; }
};

enum class ReadStatus
    {
    Success = 0,
    InvalidHeader,    
    ReadError,
    BatchTableParseError,
    SceneParseError,
    SceneDataError,
    FeatureTableError,
    };

enum class WriteStatus
    {
    Success = 0,
    NoGeometry,
    UnableToLoadTile,
    UnableToOpenFile,
    Aborted,
    };

enum class Flags : uint32_t
    {
    None            = 0,
    ContainsCurves  = 0x0001,
    Incomplete      = 0x0001 << 1,
    IsLeaf          = 0x0001 << 2,
    };

DGNPLATFORM_EXPORT BentleyStatus WriteDgnTile(StreamBufferR streamBuffer, ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, GeometricModelR model, DPoint3dCR centroid, bool isLeaf);
DGNPLATFORM_EXPORT ReadStatus ReadDgnTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, GeometricModelR model, Render::System& renderSystem, bool& isLeaf);

// Read meshes from cache data into a MeshBuilderMap, optionally excluding specific elements.
DGNPLATFORM_EXPORT ReadStatus ReadDgnTile(Render::Primitives::MeshBuilderMapR builderMap, StreamBufferR streamBuffer, GeometricModelR model, Render::System& renderSystem, Flags& flags, DgnElementIdSet const& skipElems=DgnElementIdSet());

ENUM_IS_FLAGS(Flags);

END_TILETREE_IO_NAMESPACE
