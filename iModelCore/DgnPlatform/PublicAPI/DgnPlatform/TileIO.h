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

BEGIN_TILETREE_NAMESPACE

static const char s_dgnTileMagic[]              = "dgnT";
static const uint32_t s_dgnTileVersion         = 0;


DEFINE_POINTER_SUFFIX_TYPEDEFS(ICesiumPublisher)
DEFINE_POINTER_SUFFIX_TYPEDEFS(PublishedTile)

DEFINE_REF_COUNTED_PTR(PublishedTile)

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct TileIO
{
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

    enum Flags
        {
        None            = 0,
        ContainsCurves  = 0x0001,
        Incomplete      = 0x0001 << 1,
        IsLeaf          = 0x0001 << 2,
        };

    // DgnTiles are tiles with GLTF like structure (JSon with binary data) - but optimized for caching tiles (storing feature table directly etc.)
    static BentleyStatus WriteDgnTile(StreamBufferR streamBuffer, ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, GeometricModelR model, DPoint3dCR centroid, bool isLeaf);
    static ReadStatus ReadDgnTile(ElementAlignedBox3dR contentRange, Render::Primitives::GeometryCollectionR geometry, StreamBufferR streamBuffer, GeometricModelR model, Render::System& renderSystem, bool& isLeaf);

};  

//=======================================================================================
// @bsistruct                                                       Ray.Bentley     08/2017
//! A node in the published tileset tree.  
//! This tree is used to construct the tileset metadata.
//=======================================================================================
struct PublishedTile : RefCountedBase
    {
private:
    BeFileName                  m_fileName;
    bvector<PublishedTilePtr>   m_children;
    DRange3d                    m_publishedRange;
    DRange3d                    m_tileRange;
    double                      m_tolerance;

public:
    PublishedTile(TileTree::TileCR inputTile, BeFileNameCR outputDirectory);
    BeFileNameCR GetFileName() const { return m_fileName; }
    bvector<PublishedTilePtr>& GetChildren() { return m_children; }
    bvector<PublishedTilePtr>const& GetChildren() const { return m_children; }
    void SetPublishedRange(DRange3dCR range) { m_publishedRange = range; }
    bool GetIsEmpty() const { return m_publishedRange.IsNull(); }
    DRange3dCR GetPublishedRange() const { return m_publishedRange; }
    DRange3dCR GetTileRange() const { return m_tileRange; }
    double GetTolerance() const { return m_tolerance; }
    };

//=======================================================================================
// @bsistruct                                                       Ray.Bentley     08/2017
//! Interface adopted by an object that publishes a cesium tileset to represent the
//! a GeometricModel.
//=======================================================================================
struct ICesiumPublisher
{
    //! Returns directory for tileset.
    virtual BeFileName  _GetOutputDirectory(GeometricModelCR model) const = 0;
    //! Invoked before a model is processed.
    virtual TileIO::WriteStatus _BeginProcessModel(GeometricModelCR model) { return TileIO::WriteStatus::Success; }
    //! Invoked after a model is processed, with the result of processing.
    virtual TileIO::WriteStatus _EndProcessModel(GeometricModelCR model, PublishedTileCR rootTile, TileIO::WriteStatus status) = 0;
    //! Write cesium tileset for a GeometricModel
    DGNPLATFORM_EXPORT static TileIO::WriteStatus WriteCesiumTileset(ICesiumPublisher& publisher, GeometricModelCR model, TransformCR transformFromDgn, double leafTolerance);


};  // ICesiumPublisher



END_TILETREE_NAMESPACE
