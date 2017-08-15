/*--------------------------------------------------------------------------------------+                                                                                                                             '
|
|     $Source: PublicAPI/DgnPlatform/TileWriter.h $
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
#include <DgnPlatform/TileIO.h>

#define BEGIN_TILEWRITER_NAMESPACE    BEGIN_TILETREE_NAMESPACE namespace TileWriter {
#define END_TILEWRITER_NAMESPACE      } END_TILETREE_NAMESPACE
#define USING_NAMESPACE_TILEWRITER    using namespace BentleyApi::Dgn::TileTree::TileWriter;


BEGIN_TILEWRITER_NAMESPACE


DEFINE_POINTER_SUFFIX_TYPEDEFS(ICesiumPublisher)
DEFINE_POINTER_SUFFIX_TYPEDEFS(PublishedTile)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MeshMaterial)
DEFINE_POINTER_SUFFIX_TYPEDEFS(TileTexture)

DEFINE_REF_COUNTED_PTR(TileTexture)
DEFINE_REF_COUNTED_PTR(PublishedTile)


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct TileTexture : Render::Texture
 {
    CreateParams        m_createParams;
    Render::Image               m_image;

    TileTexture() { }
    TileTexture(Render::ImageCR image, CreateParams const& createParams) : m_image(image), m_createParams(createParams) { }


    bool GetRepeat() const { return !m_createParams.m_isTileSection; }
 }; // TileTexture


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016                                                     a
//=======================================================================================
struct Writer
{
public:
    Writer(StreamBufferR buffer, GeometricModelR model) : m_buffer(buffer), m_model(model) { }
    BentleyStatus WriteGltf(DPoint3dCR centroid);

protected:
    Json::Value         m_json;
    ByteStream          m_binaryData;
    StreamBufferR       m_buffer;
    GeometricModelCR    m_model;

    size_t              BinaryDataSize() const { return m_binaryData.size(); }
    void const*         BinaryData() const;
    void                WriteLength(uint32_t startPosition, uint32_t lengthDataPosition);
    void                AddBinaryData (void const* data, size_t size); 
    void                PadBinaryDataToBoundary(size_t boundarySize = 4);
    void                PadToBoundary(size_t boundarySize = 4);
    void                AddExtensions(DPoint3dCR centroid);
    void                AddDefaultScene ();
    void                AddMeshUInt16Attributes(Json::Value& primitive, uint16_t const* attributes16, size_t nAttributes, Utf8StringCR idStr, Utf8CP name, Utf8CP semantic);
    void                AddBatchIds(Json::Value& primitive, Render::FeatureIndex const& featureIndex, size_t nVertices, Utf8StringCR idStr);
    void                AddColors(Json::Value& primitive, Render::ColorIndex const& colorIndex, size_t nVertices, Utf8StringCR idStr);
    void                AddAccessor(uint32_t componentType, Utf8StringCR accessorId, Utf8StringCR bufferViewId, size_t count, Utf8CP type);
    Utf8String          AddQuantizedPointsAttribute(Render::QPoint3dCP qPoints, size_t nPoints, Render::QPoint3d::Params params, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddQuantizedParamAttribute(FPoint2d const* params, size_t nParams, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddParamAttribute(FPoint2d const* params, size_t nParams, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddMeshIndices(Utf8StringCR name, uint32_t const* indices, size_t numIndices, Utf8StringCR idStr, size_t maxIndex);
    Utf8String          AddMeshTriangleIndices(Utf8StringCR name, Render::Primitives::TriangleList const& triangles, Utf8StringCR idStr, size_t maxIndex);
    void                AddMeshPointRange (Json::Value& positionValue, DRange3dCR pointRange);
    BentleyStatus       CreateMeshMaterialJson(Json::Value& matJson, Render::Primitives::ColorTableCR colorTable, MeshMaterialCR meshMaterial, Utf8StringCR suffix); 
    Json::Value         AddNormals (Render::OctEncodedNormalCP normals, size_t numNormals, Utf8String name, Utf8CP id);
    Json::Value         AddNormalPairs(Render::OctEncodedNormalPairCP pairs, size_t numPairs, Utf8String name, Utf8CP id);
    void                AppendPolylineToBufferView(Render::MeshPolylineCR polyline, bool useShortIndices);
    Json::Value         AddPolylines(Render::Primitives::PolylineList const& polylines, size_t maxIndex, Utf8StringCR name, Utf8StringCR idStr); 

    static Json::Value  CreateColorJson(RgbFactorCR color);
    static Json::Value  CreateDecodeQuantizeValues(double const* min, double const* max, size_t nComponents);


    template<typename T> void AddBufferView(Utf8CP name, T const* bufferData, size_t count)
        {
        Json::Value&    views = m_json["bufferViews"];

        auto bufferDataSize = count * sizeof(*bufferData);
        auto& view = (views[name] = Json::objectValue);
        view["buffer"] = "binary_glTF";
        view["byteOffset"] = m_binaryData.size();
        view["byteLength"] = bufferDataSize;

        size_t binaryDataSize = m_binaryData.size();
        m_binaryData.resize(binaryDataSize + bufferDataSize);
        memcpy(m_binaryData.data() + binaryDataSize, bufferData, bufferDataSize);
        }

    template<typename T> void AddBufferView(Utf8CP name, T const& bufferData)
        {
        AddBufferView (name, bufferData.data(), bufferData.size());
        }
};  // Writer


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



END_TILEWRITER_NAMESPACE
