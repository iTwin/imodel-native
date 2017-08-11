/*-------------------------------------------------------------------------------------+                                                                                           
|

|     $Source: DgnCore/CacheTileWriter.cpp $
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
USING_NAMESPACE_TILEWRITER
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES
       

BEGIN_TILEWRITER_NAMESPACE


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct DgnCacheTileWriter : TileWriter::Writer
{

    DEFINE_T_SUPER(TileWriter::Writer);

public:
    DgnCacheTileWriter(StreamBufferR streamBuffer, GeometricModelR model) : TileWriter::Writer(streamBuffer, model) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CreateColorTable(ColorTableCR colorTable)
    {
    Json::Value     jsonTable = Json::arrayValue;

    for (auto& entry : colorTable)
        jsonTable[entry.second] = entry.first;

    return jsonTable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CreateMeshEdges(MeshEdgesCR meshEdges, size_t maxIndex, Utf8StringCR idStr)
    {
    Json::Value     edgesValue= Json::objectValue;

    if (!meshEdges.m_visible.empty())
        edgesValue["visibles"] = AddMeshIndices("visibles", meshEdges.m_visible.front().m_indices, 2 * meshEdges.m_visible.size(), idStr, maxIndex);

    if (!meshEdges.m_silhouette.empty())
        {
        edgesValue["silhouettes"]["indices"]  = AddMeshIndices("silhouettes", meshEdges.m_silhouette.front().m_indices, 2 * meshEdges.m_silhouette.size(), idStr, maxIndex);
        edgesValue["silhouettes"]["normalPairs"] = AddNormalPairs(meshEdges.m_silhouetteNormals.data(), meshEdges.m_silhouetteNormals.size(), "normalPairs", idStr.c_str());
        }

    if (!meshEdges.m_polylines.empty())
        edgesValue["polylines"] = AddPolylines(meshEdges.m_polylines, maxIndex, "polyEdge", idStr);

    return edgesValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CreateTriMesh(Json::Value& primitiveJson, MeshCR mesh, Utf8StringCR idStr)
    {
    DisplayParamsCR     displayParams = mesh.GetDisplayParams();

    if (!mesh.Params().empty() && displayParams.IsTextured())
        primitiveJson["attributes"]["TEXCOORD_0"] = AddParamAttribute (mesh.Params().data(), mesh.Params().size(), "Param", idStr.c_str());

    BeAssert(displayParams.IgnoresLighting() || !mesh.Normals().empty());

    if (!mesh.Normals().empty() && !displayParams.IgnoresLighting())        // No normals if ignoring lighting (reality meshes).
        primitiveJson["attributes"]["NORMAL"]  = AddNormals(mesh.Normals().data(), mesh.Normals().size(), "Normal", idStr.c_str());

    primitiveJson["indices"] = AddMeshTriangleIndices ("Indices", mesh.Triangles(), idStr, mesh.Points().size());

    if (mesh.GetEdges().IsValid())
        primitiveJson["edges"] = CreateMeshEdges(*mesh.GetEdges(), mesh.Points().size(), idStr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CreatePolylines(Json::Value& primitiveJson, MeshCR mesh, Utf8StringCR idStr)
    {
    primitiveJson["indices"] = AddPolylines(mesh.Polylines(), mesh.Points().size(), "polyline", idStr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  CreateMaterialJson(Json::Value& matJson, MeshCR mesh,  DisplayParamsCR displayParams, Utf8StringCR suffix) 
    {
    matJson["type"] = (uint8_t) displayParams.GetType();

    // GeomParams...
    if (displayParams.GetCategoryId().IsValid())
        matJson["categoryId"] = displayParams.GetCategoryId().GetValue();
    
    if (displayParams.GetSubCategoryId().IsValid())
        matJson["subCategoryId"] = displayParams.GetSubCategoryId().GetValue();


    if (displayParams.GetMaterialId().IsValid())
        matJson["materialId"] = displayParams.GetMaterialId().GetValue();

    matJson["class"] = (uint16_t) displayParams.GetClass();
    matJson["ignoreLighting"] = displayParams.IgnoresLighting();

    // GraphicsParams...
    matJson["fillColor"] = displayParams.GetFillColor();
    matJson["fillFlags"] = static_cast<uint32_t> (displayParams.GetFillFlags());

    // Are these needed for meshes (Edges??)
    matJson["lineColor"]  = displayParams.GetLineColor();     // Edges?
    matJson["lineWidth"]  = displayParams.GetLineWidth();
    matJson["linePixels"] = (uint32_t) displayParams.GetLinePixels();     // Edges?
    
    if (nullptr != displayParams.GetGradient())
        matJson["gradient"] = displayParams.GetGradient()->ToJson();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddFeatures (MeshCR mesh, Json::Value& primitiveJson, Utf8StringCR idStr)
    {
    FeatureIndex    featureIndex;

    mesh.ToFeatureIndex(featureIndex);
    if(featureIndex.IsEmpty())
        BeAssert(false && "Empty feature index");
    else if (featureIndex.IsUniform())
        primitiveJson["featureID"]  = featureIndex.m_featureID;
    else
        primitiveJson["featureIDs"] = AddMeshIndices("featureIDs", featureIndex.m_featureIDs, mesh.Points().size(), idStr, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteFeatureTable(FeatureTableCR featureTable)
    {
    uint32_t      startPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);              // Filled in below.

    m_buffer.Append(featureTable.GetMaxFeatures());
    m_buffer.Append((uint32_t) featureTable.size());
    for (auto& feature : featureTable)
        {
        m_buffer.Append(feature.first.GetElementId().GetValue());
        m_buffer.Append(feature.first.GetSubCategoryId().GetValue());
        m_buffer.Append(static_cast<uint32_t> (feature.first.GetClass()));
        m_buffer.Append(feature.second);
        }
    WriteLength(startPosition, startPosition);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus AddMesh(Json::Value& primitivesNode, MeshCR mesh, size_t& index)
    { 
    Utf8String          idStr(std::to_string(index++).c_str());
    Json::Value         materialJson = Json::objectValue, primitiveJson = Json::objectValue;

    if (SUCCESS != CreateMaterialJson(materialJson, mesh, mesh.GetDisplayParams(), idStr))
        return ERROR;

    Utf8String      accPositionId =  AddQuantizedPointsAttribute(mesh.Points().data(), mesh.Points().size(), mesh.Verts().GetParams(), "Position", idStr.c_str());
    primitiveJson["attributes"]["POSITION"] = accPositionId;

    if (!mesh.GetColorTable().IsUniform())
        AddMeshUInt16Attributes(primitiveJson, mesh.Colors().data(), mesh.Colors().size(), idStr, "ColorIndex_", "_COLORINDEX");

    AddFeatures (mesh, primitiveJson, idStr);
    AddMeshPointRange(m_json["accessors"][accPositionId], mesh.Verts().GetParams().GetRange());
 
    primitiveJson["colorTable"] = CreateColorTable(mesh.GetColorTable());
    primitiveJson["type"] = (uint32_t) mesh.GetType();
    primitiveJson["isPlanar"] = mesh.IsPlanar();

    if ((!mesh.Triangles().Empty() && SUCCESS == CreateTriMesh(primitiveJson, mesh, idStr)) ||
        (!mesh.Polylines().empty() && SUCCESS == CreatePolylines(primitiveJson, mesh, idStr)))
        {
        Utf8String  materialName = "Material" + idStr;
        m_json["materials"][materialName] = materialJson;
        primitiveJson["material"] = materialName;
        primitivesNode.append(primitiveJson);
        
        return SUCCESS;
        }

    return ERROR;
    }
 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AddMeshes(Render::Primitives::GeometryCollectionCR geometry)
    {
    Json::Value     meshes     = Json::objectValue;
    Json::Value     mesh       = Json::objectValue;
    Json::Value     nodes      = Json::objectValue;
    Json::Value     rootNode   = Json::objectValue;
    Json::Value     primitives = Json::arrayValue;
    Utf8String      meshName   = "Mesh";
    size_t          primitiveIndex = 0;

    for (auto& mesh : geometry.Meshes())
        AddMesh(primitives, *mesh, primitiveIndex);

    mesh["primitives"] = primitives;
    meshes[meshName] = mesh;
    rootNode["meshes"].append (meshName);
    nodes["rootNode"] = rootNode;
    m_json["meshes"] = meshes;
    m_json["nodes"]  = nodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WriteTile(ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, DPoint3dCR centroid, bool isLeaf)
    {
    uint32_t    startPosition = m_buffer.GetSize();
    uint32_t    flags = isLeaf ? TileIO::IsLeaf : TileIO::None;

    if (geometry.ContainsCurves())
        flags |=  TileIO::Flags::ContainsCurves;

    if (!geometry.IsComplete())
        flags |= TileIO::Flags::Incomplete;

    m_buffer.Append((const uint8_t *) s_dgnTileMagic, 4);
    m_buffer.Append(s_dgnTileVersion);
    m_buffer.Append(flags);
    m_buffer.Append(contentRange);

    uint32_t    lengthDataPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);              // Filled in below.
    WriteFeatureTable(geometry.Meshes().FeatureTable());
    PadToBoundary ();

    AddMeshes(geometry);
    if (SUCCESS != WriteGltf (centroid))
        return ERROR;

    PadToBoundary ();
    WriteLength(startPosition, lengthDataPosition);

    return SUCCESS;
    }
};  // DgnCacheTileWriter



END_TILEWRITER_NAMESPACE
   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileIO::WriteDgnTile(StreamBufferR streamBuffer, ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, GeometricModelR model, DPoint3dCR centroid, bool isLeaf)
    {
    return TileWriter::DgnCacheTileWriter(streamBuffer, model).WriteTile(contentRange, geometry, centroid, isLeaf);
    }





