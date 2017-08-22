/*-------------------------------------------------------------------------------------+                                                                                           
|

|     $Source: DgnCore/TileWriter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>
#include <DgnPlatform/TileWriter.h>
#include <folly/BeFolly.h>

#include "../TilePublisher/lib/Constants.h" // ###TODO: Move this stuff.


USING_NAMESPACE_TILETREE
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES
       

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileUtil::WriteJsonToFile (WCharCP fileName, Json::Value const& value)
    {
    BeFile          outputFile;

    if (BeFileStatus::Success != outputFile.Create (fileName))
        return ERROR;
   
    Utf8String  string = Json::FastWriter().write(value);

    return BeFileStatus::Success == outputFile.Write (nullptr, string.data(), string.size()) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileUtil::ReadJsonFromFile (Json::Value& value, WCharCP fileName)
    {
    Json::Reader    reader;
    ByteStream      inputData;
    BeFile          inputFile;

    return BeFileStatus::Success == inputFile.Open (fileName, BeFileAccess::Read) &&
           BeFileStatus::Success == inputFile.ReadEntireFile (inputData) &&
           reader.parse ((char*) inputData.GetData(), (char*) (inputData.GetData() + inputData.GetSize()), value) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
WString TileUtil::GetRootNameForModel(DgnModelId modelId, bool asClassifier)
    {
    WString name = asClassifier ? L"Classifier" : L"Model";
    name.append(1, '_');
    WChar idBuf[17];
    BeStringUtilities::FormatUInt64(idBuf, _countof(idBuf), modelId.GetValue(), HexFormatOptions::None);
    name.append(idBuf);
    return name;
    }

BEGIN_TILEWRITER_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Writer::WriteGltf(DPoint3dCR centroid)
    {
    AddExtensions(centroid);
    AddDefaultScene();

    Utf8String  sceneStr = Json::FastWriter().write(m_json);
    uint32_t    sceneStrLength = static_cast<uint32_t>(sceneStr.size());

    long    startPosition =  m_buffer.GetSize();
    m_buffer.Append((const uint8_t*) s_gltfMagic, 4);
    m_buffer.Append(s_gltfVersion2);
    long    lengthDataPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);
    m_buffer.Append((const uint8_t*) &sceneStrLength, sizeof(sceneStrLength));
    m_buffer.Append((const uint8_t*) &s_gltfSceneFormat, sizeof(s_gltfSceneFormat));
    m_buffer.Append((const uint8_t*) sceneStr.data(), sceneStrLength);
    if (!m_binaryData.empty())
        m_buffer.Append((const uint8_t*) m_binaryData.data(), BinaryDataSize());

    WriteLength(startPosition, lengthDataPosition);
    m_buffer.Append(m_binaryData.data(), m_binaryData.size());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Writer::WriteLength(uint32_t startPosition, uint32_t lengthDataPosition)
    {
    uint32_t    dataSize = static_cast<uint32_t> (m_buffer.GetSize() - startPosition);
    memcpy(m_buffer.GetDataP() + lengthDataPosition, &dataSize, sizeof(uint32_t));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    Writer::AddBinaryData (void const* data, size_t size) 
    {
    m_binaryData.Append (static_cast<uint8_t const*> (data), size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    Writer::PadBinaryDataToBoundary(size_t boundarySize)
    {
    while (0 != (m_binaryData.GetSize() % boundarySize))
        m_binaryData.Append((uint8_t) 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    Writer::PadToBoundary(size_t boundarySize)
    {
    while (0 != (m_buffer.GetSize() % boundarySize))
        m_buffer.Append((uint8_t) 0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Writer::AddExtensions(DPoint3dCR centroid)
    {
    m_json["extensionsUsed"] = Json::arrayValue;
    m_json["extensionsUsed"].append("KHR_binary_glTF");
    m_json["extensionsUsed"].append("CESIUM_RTC");
    m_json["extensionsUsed"].append("WEB3D_quantized_attributes");

    m_json["glExtensionsUsed"] = Json::arrayValue;
    m_json["glExtensionsUsed"].append("OES_element_index_uint");

    m_json["extensions"]["CESIUM_RTC"]["center"] = Json::arrayValue;
    m_json["extensions"]["CESIUM_RTC"]["center"].append(centroid.x);
    m_json["extensions"]["CESIUM_RTC"]["center"].append(centroid.y);
    m_json["extensions"]["CESIUM_RTC"]["center"].append(centroid.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Writer::AddDefaultScene ()
    {
    m_json["scene"] = "defaultScene";
    m_json["scenes"]["defaultScene"]["nodes"] = Json::arrayValue;
    m_json["scenes"]["defaultScene"]["nodes"].append("rootNode");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Writer::AddMeshUInt16Attributes(Json::Value& primitive, uint16_t const* attributes16, size_t nAttributes, Utf8StringCR idStr, Utf8CP name, Utf8CP semantic)
    {
    Utf8String suffix(name);
    suffix.append(idStr);

    Utf8String bvId  = "bv" +  suffix;
    Utf8String accId = "acc" + suffix;

    primitive["attributes"][semantic] = accId;

    // Use uint8 if possible to save space in tiles and memory in browser
    bvector<uint8_t> attributes8;
    auto             componentType = GLTF_UNSIGNED_BYTE;

    for (size_t i=0; i<nAttributes; i++)
        {
        if (attributes16[i] > 0xff)
            {
            componentType = GLTF_UNSIGNED_SHORT;
            break;
            }
        }


    if (GLTF_UNSIGNED_BYTE == componentType)
        {
        attributes8.reserve(nAttributes);
        for (size_t i=0; i<nAttributes; i++)
            attributes8.push_back(static_cast<uint8_t>(attributes16[i]));

        AddBufferView(bvId.c_str(), attributes8.data(), nAttributes);
        }
    else
        {
        AddBufferView(bvId.c_str(), attributes16, nAttributes * sizeof(uint16_t));
        }

    AddAccessor(componentType, accId, bvId, nAttributes, "SCALAR");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Writer::AddBatchIds(Json::Value& primitive, FeatureIndex const& featureIndex, size_t nVertices, Utf8StringCR idStr)
    {
    if (featureIndex.IsEmpty())
        {
        BeAssert(false);
        return;
        }

    bvector<uint16_t>   batchIds;

    for (size_t i=0; i<nVertices; i++)
        batchIds.push_back(featureIndex.IsUniform() ? featureIndex.m_featureID : featureIndex.m_featureIDs[i]);
   
    AddMeshUInt16Attributes(primitive, batchIds.data(), batchIds.size(), idStr, "Batch_", "BATCHID");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Writer::AddColors(Json::Value& primitive, ColorIndex const& colorIndex, size_t nVertices, Utf8StringCR idStr)
    {
    BeAssert (colorIndex.m_numColors > 1);
    AddMeshUInt16Attributes(primitive, colorIndex.m_nonUniform.m_indices, nVertices, idStr, "ColorIndex_", "_COLORINDEX");
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Writer::CreateDecodeQuantizeValues(double const* min, double const* max, size_t nComponents)
    {
    Json::Value         decodeMatrix = Json::arrayValue;
    Json::Value         quantizeValue = Json::objectValue;

   for (size_t i=0; i<nComponents; i++)
        {
        for (size_t j=0; j<nComponents; j++)
            decodeMatrix.append ((i==j) ? ((max[i] - min[i]) / Quantization::RangeScale()) : 0.0);

        decodeMatrix.append (0.0);
        }

    for (size_t i=0; i<nComponents; i++)
        decodeMatrix.append (min[i]);

    decodeMatrix.append (1.0);
    quantizeValue["decodeMatrix"] = decodeMatrix;

    for (size_t i=0; i<3; i++)
        {
        quantizeValue["decodedMin"].append (min[i]);
        quantizeValue["decodedMax"].append (max[i]);
        }

    return quantizeValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Writer::AddAccessor(uint32_t componentType, Utf8StringCR accessorId, Utf8StringCR bufferViewId, size_t count, Utf8CP type)
    {
    Json::Value         accessor   = Json::objectValue;

    accessor["componentType"] = componentType;
    accessor["bufferView"] = bufferViewId;
    accessor["byteOffset"] = 0;
    accessor["count"] = count;
    accessor["type"] = type;

    BeAssert(!m_json["accessors"].isMember(accessorId));
    m_json["accessors"][accessorId] = accessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Writer::AddQuantizedPointsAttribute(QPoint3dCP qPoints, size_t nPoints, QPoint3d::Params params, Utf8StringCR name, Utf8StringCR id) 
    {
    Utf8String          nameId =  name + id,
                        accessorId = Utf8String("acc") + nameId,
                        bufferViewId = Utf8String("bv") + nameId;
    DRange3d            range = params.GetRange();

    AddBufferView(bufferViewId.c_str(), qPoints, nPoints);
    AddAccessor(GLTF_UNSIGNED_SHORT, accessorId, bufferViewId, nPoints, "VEC3");

    m_json["bufferViews"][bufferViewId]["target"] = GLTF_ARRAY_BUFFER;
    m_json["accessors"][accessorId]["extensions"]["WEB3D_quantized_attributes"] = CreateDecodeQuantizeValues(&range.low.x, &range.high.x, 3);

    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Writer::AddQuantizedParamAttribute(FPoint2d const* params, size_t nParams, Utf8StringCR name, Utf8StringCR id) 
    {
    QPoint2dList        qParams;
    
    qParams.InitFrom(params, nParams);

    Utf8String          nameId =  Utf8String(name) + Utf8String(id),
                        accessorId = Utf8String("acc") + nameId,
                        bufferViewId = Utf8String("bv") + nameId;
    DRange2d            range = qParams.GetParams().GetRange();

    AddBufferView(bufferViewId.c_str(), qParams);
    AddAccessor(GLTF_UNSIGNED_SHORT, accessorId, bufferViewId, nParams, "VEC2");
    m_json["bufferViews"][bufferViewId]["target"] = GLTF_ARRAY_BUFFER;
    m_json["accessors"][accessorId]["extensions"]["WEB3D_quantized_attributes"] = CreateDecodeQuantizeValues(&range.low.x, &range.high.x, 2);

    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String    Writer::AddParamAttribute(FPoint2d const* params, size_t nParams, Utf8StringCR name, Utf8StringCR id) 
    {
    Utf8String          nameId =  Utf8String(name) + Utf8String(id),
                        accessorId = Utf8String("acc") + nameId,
                        bufferViewId = Utf8String("bv") + nameId;

    AddBufferView(bufferViewId.c_str(), params, nParams);
    AddAccessor(GLTF_FLOAT, accessorId, bufferViewId, nParams, "VEC2");
    m_json["bufferViews"][bufferViewId]["target"] = GLTF_ARRAY_BUFFER;

    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Writer::AddMeshIndices(Utf8StringCR name, uint32_t const* indices, size_t numIndices, Utf8StringCR idStr, size_t maxIndex)
    {
    Utf8String          nameId           = name + idStr,
                        accIndexId       = "acc" + nameId,
                        bvIndexId        = "bv"  + nameId;
    bool                useShortIndices;

    if (0 == maxIndex)
        {
        size_t      i;

        for (i=0; i<numIndices; i++)
            if (indices[i] > 0xffff)
                break;

        useShortIndices = (i == numIndices);
        }
    else
        {
        useShortIndices  = maxIndex < 0xffff;
        }

 
    if (useShortIndices)
        {
        bvector<uint16_t>   shortIndices;

        shortIndices.reserve(numIndices);

        for (size_t i=0; i<numIndices; i++)
            shortIndices.push_back ((uint16_t) indices[i]);

        AddBufferView(bvIndexId.c_str(), shortIndices.data(), shortIndices.size());
        }
    else
        {
        AddBufferView(bvIndexId.c_str(), indices, numIndices);
        }
    m_json["bufferViews"][bvIndexId]["target"] =  GLTF_ELEMENT_ARRAY_BUFFER;
    AddAccessor(useShortIndices ? GLTF_UNSIGNED_SHORT : GLTF_UINT32, accIndexId, bvIndexId, numIndices, "SCALAR");

    return accIndexId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Writer::AddMeshTriangleIndices(Utf8StringCR name, TriangleList const& triangles, Utf8StringCR idStr, size_t maxIndex)
    {
    return AddMeshIndices(name, triangles.Indices().data(), triangles.Indices().size(), idStr, maxIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Writer::AddMeshPointRange (Json::Value& positionValue, DRange3dCR pointRange)
    {
    positionValue["min"] = Json::arrayValue;
    positionValue["min"].append(pointRange.low.x);
    positionValue["min"].append(pointRange.low.y);
    positionValue["min"].append(pointRange.low.z);
    positionValue["max"] = Json::arrayValue;
    positionValue["max"].append(pointRange.high.x);
    positionValue["max"].append(pointRange.high.y);
    positionValue["max"].append(pointRange.high.z);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Writer::CreateColorJson(RgbFactorCR color)
    {
    Json::Value     colorJson = Json::objectValue;

    colorJson.append(color.red);
    colorJson.append(color.green);
    colorJson.append(color.blue);

    return colorJson;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Writer::AddNormals (OctEncodedNormalCP normals, size_t numNormals, Utf8String name, Utf8CP id)
    {
    Utf8String nameId = name + id,
               accessorId = Utf8String("acc") + nameId,
               bufferViewId = Utf8String("bv") + nameId;

    AddBufferView(bufferViewId.c_str(), normals, numNormals);
    AddAccessor(GLTF_UNSIGNED_BYTE, accessorId, bufferViewId, numNormals, "VEC2");

    m_json["bufferViews"][bufferViewId]["target"] = GLTF_ARRAY_BUFFER;

    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Writer::AddNormalPairs(OctEncodedNormalPairCP pairs, size_t numPairs, Utf8String name, Utf8CP id)
    {
    Utf8String nameId = name + id,
               accessorId = Utf8String("acc") + nameId,
               bufferViewId = Utf8String("bv") + nameId;

    AddBufferView(bufferViewId.c_str(), pairs, numPairs);
    AddAccessor(GLTF_UNSIGNED_BYTE, accessorId, bufferViewId, numPairs, "VEC4");

    m_json["bufferViews"][bufferViewId]["target"] = GLTF_ARRAY_BUFFER;

    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Writer::AppendPolylineToBufferView(MeshPolylineCR polyline, bool useShortIndices)
    {
    m_binaryData.Append(polyline.GetStartDistance());
    m_binaryData.Append(polyline.GetRangeCenter().x);
    m_binaryData.Append(polyline.GetRangeCenter().y);
    m_binaryData.Append(polyline.GetRangeCenter().z);
    m_binaryData.Append((uint32_t) polyline.GetIndices().size());
    for (auto& index : polyline.GetIndices())
        {
        if (useShortIndices)
            m_binaryData.Append((uint16_t) index);
        else
            m_binaryData.Append(index);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Writer::AddPolylines(PolylineList const& polylines, size_t maxIndex, Utf8StringCR name, Utf8StringCR idStr) 
    {
    Utf8String          nameId = name + idStr;
    Utf8String          bufferViewId= "bv_" + nameId;
    Utf8String          accessorId   = "acc_" + nameId;
    Json::Value         bufferViewJson;
    size_t              bufferViewOffset = m_binaryData.size();
    bool                useShortIndices = maxIndex < 0xffff;

    bufferViewJson["buffer"] = "binary_glTF";
    bufferViewJson["byteOffset"] = (uint32_t) bufferViewOffset;

    for (auto& polyline : polylines)
        AppendPolylineToBufferView(polyline, useShortIndices);
    
    bufferViewJson["byteLength"] = m_binaryData.size() -  bufferViewOffset;
    m_json["bufferViews"][bufferViewId] = bufferViewJson;

    AddAccessor(useShortIndices ? GLTF_UNSIGNED_SHORT : GLTF_UINT32, accessorId, bufferViewId, polylines.size(), "PLINE");
    return accessorId;
    }


END_TILEWRITER_NAMESPACE
   



