/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/TileWriter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/Render.h>
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/DgnTexture.h>
#include <DgnPlatform/DgnMaterial.h>
#include <DgnPlatform/TileIO.h>

BEGIN_TILE_IO_NAMESPACE

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016                                                     
//=======================================================================================
struct Writer
{
public:
    Writer(StreamBufferR buffer, GeometricModelR model) : m_buffer(buffer), m_model(model) { }
    BentleyStatus WriteGltf();
    void AddPrimitivesJson(Json::Value const& primitives);
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
    void                AddExtensions();
    void                AddDefaultScene ();
    void                AddMeshUInt16Attributes(Json::Value& primitive, uint16_t const* attributes16, size_t nAttributes, Utf8StringCR idStr, Utf8CP name, Utf8CP semantic);
    void                AddBatchIds(Json::Value& primitive, Render::FeatureIndex const& featureIndex, size_t nVertices, Utf8StringCR idStr);
    void                AddColors(Json::Value& primitive, Render::ColorIndex const& colorIndex, size_t nVertices, Utf8StringCR idStr);
    void                AddAccessor(Gltf::DataType componentType, Utf8StringCR accessorId, Utf8StringCR bufferViewId, size_t count, Utf8CP type);
    Utf8String          AddQuantizedPointsAttribute(Render::QPoint3dCP qPoints, size_t nPoints, Render::QPoint3d::Params params, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddQuantizedParamAttribute(FPoint2d const* params, size_t nParams, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddParamAttribute(FPoint2d const* params, size_t nParams, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddPointAttribute(FPoint3d const* points, size_t nPoints, Utf8StringCR name, Utf8StringCR id); 
    Utf8String          AddMeshIndices(Utf8StringCR name, uint32_t const* indices, size_t numIndices, Utf8StringCR idStr, size_t maxIndex);
    Utf8String          AddMeshTriangleIndices(Utf8StringCR name, Render::Primitives::TriangleList const& triangles, Utf8StringCR idStr, size_t maxIndex);
    void                AddMeshPointRange (Json::Value& positionValue, DRange3dCR pointRange);
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
        view["byteOffset"] = static_cast<uint32_t>(m_binaryData.size());
        view["byteLength"] = static_cast<uint32_t>(bufferDataSize);

        size_t binaryDataSize = m_binaryData.size();
        m_binaryData.resize(binaryDataSize + bufferDataSize);
        memcpy(m_binaryData.data() + binaryDataSize, bufferData, bufferDataSize);
        PadBinaryDataToBoundary();
        }

    template<typename T> void AddBufferView(Utf8CP name, T const& bufferData)
        {
        AddBufferView (name, bufferData.data(), bufferData.size());
        }
};  // Writer

END_TILE_IO_NAMESPACE

