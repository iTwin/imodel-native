/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/DGNdbImporter.cpp $
|    $RCSfile: DGNdbImporter.cpp,v $
|   $Revision: 1.0 $
|       $Date: 2016/06/29 12:15:00 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"

#include <ScalableMesh/Import/Plugin/InputExtractorV0.h>
#include <ScalableMesh/Import/Plugin/SourceV0.h>

#include <ScalableMesh/Plugin/IScalableMeshPolicy.h>
#include <ScalableMesh/Plugin/IScalableMeshFileUtilities.h>

#include <ScalableMesh/Type/IScalableMeshLinear.h>
#include <ScalableMesh/Type/IScalableMeshMesh.h>
#include "..\ScalableMesh\ScalableMeshGraph.h"
#include "..\CGALEdgeCollapse.h"
#undef static_assert
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <json/json.h>
#undef MAX


USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VERSION(0)
USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_DGNPLATFORM

using namespace BENTLEY_NAMESPACE_NAME::ScalableMesh::Plugin;

namespace
    { //BEGIN UNNAMED NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Elenie.Godzaridis   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
    class DgnDbSource : public SourceMixinBase<DgnDbSource>
    {
public:
    int                             GetFeatureCount() { return m_featureCount; }
    int                             GetPointCount() { return m_pointCount; }
    DgnDb*                          GetDatabase() { return m_database.get(); }

private:
    friend class                    DgnDbSourceCreator;

    int                             m_featureCount = 500;
    int                             m_pointCount = 50000;
    DgnDbPtr                           m_database;
    
    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    explicit                        DgnDbSource(const WString&   pi_rFilePath)
        {
        BeSQLite::DbResult status;
        DgnDb::OpenParams openParams(BeSQLite::Db::OpenMode::Readonly);
        BeFileName fileName(pi_rFilePath);
        m_database = DgnDb::OpenDgnDb(&status, fileName, openParams);
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void                    _Close                 () override
        {
        m_database = nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ContentDescriptor          _CreateDescriptor() const override
        {

        //GCSFactory::Status gcsCreateStatus;
       //     GCS gcs = GetGCSFactory().Create(wktStr.c_str(), gcsCreateStatus);
            ScalableMeshData data = ScalableMeshData::GetNull();
        return ContentDescriptor
            (
            L"",
            ILayerDescriptor::CreateLayerDescriptor(L"",
            MeshType3d64fCreator().Create(),
            GCS::GetNull(),
            0,
            data)
            );
        }

        /*---------------------------------------------------------------------------------**//**
        * @description
        * @bsimethod                                                  Elenie.Godzaridis   04/2015
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual const WChar*         _GetType() const override
            {
            return L"DgnDb file";
            }


    };

    struct NullContext : ViewContext
        {
        void _AllocateScanCriteria() override { ; }
        QvElem* _DrawCached(IStrokeForCache&) { return nullptr; }
        void _DrawSymbol(IDisplaySymbol* symbolDef, TransformCP trans, ClipPlaneSetP clip) override {}
        void _DeleteSymbol(IDisplaySymbol*) override {}
        bool _FilterRangeIntersection(GeometrySourceCR element) override { return false; }
        void _CookDisplayParams(ElemDisplayParamsR, ElemMatSymbR) override {}
        void _SetupOutputs() override { SetIViewDraw(*m_IViewDraw); }
        NullContext(DgnDbR db) { m_dgnDb = &db; m_IViewDraw = nullptr; m_IDrawGeom = nullptr; m_ignoreViewRange = true; }
        };


    bool IsMatchTex(int x, int y, Point2d dimensions, const uint8_t* texP, const uint8_t* pattern, int width, int height, int subWidth, int subHeight, int nChannels = 3)
        {
        for (int i = 0; i < subWidth; ++i)
            for (int j = 0; j < subHeight; ++j)
                {
                if ((y + j) >= dimensions.y || (x + i) >= dimensions.x) return false;
                const uint8_t* origPixel = texP + (y+j)*dimensions.x * 3 + (x+i) * 3;
                const uint8_t* targetPixel = pattern + j*width * nChannels + i * nChannels;
                if (*origPixel != *targetPixel) return false;
                if (*(origPixel+1) != *(targetPixel+1)) return false;
                if (*(origPixel + 2) != *(targetPixel + 2)) return false;
                }
        return true;
        }

    bool FindMatching(int&x, int&y, int width, int height, Point2d dimensions, const uint8_t* texP, const uint8_t* pattern, int subWidth, int subHeight,int nChannels = 3)
        {
        for (int i = 0; i < dimensions.x; ++i)
            for (int j = 0; j < dimensions.y; ++j)
                {
                if (IsMatchTex(i, j, dimensions, texP, pattern, width, height,subWidth, subHeight, nChannels))
                    {
                    x = i;
                    y = j;
                    return true;
                    }
                }
        return false;
        }

    void AppendTextureToExisting(bvector<uint8_t>& tex, DPoint2d& uvMapBottomLeft, DPoint2d& uvMapTopRight, uint32_t width, uint32_t height, const uint8_t* data, ImageBuffer::Format fmt = ImageBuffer::Format::Rgba)
        {
        uint32_t oldWidth = 0, oldHeight = 0;
        bvector<uint8_t> oldTex;
        if (tex.size() > 0)
            {
            oldWidth = ((const uint32_t*)tex.data())[0];
            oldHeight = ((const uint32_t*)tex.data())[1];
            oldTex.insert(oldTex.end(), tex.begin(), tex.end());
            }
        uint32_t endWidth = width;
        uint32_t endHeight = height;
        if (width == 512 && height == 512)
            endWidth = endHeight = 128;

        int x, y;
        int nChannels;
        if (ImageBuffer::Format::Rgb == fmt)
            nChannels = 3;
        else
            nChannels = 4;
        int newWidth, newHeight;
        Point2d dim;
        dim.Init((int)oldWidth, (int)oldHeight);
        if (oldWidth >= width && oldHeight >= height && FindMatching(x, y, width, height, dim, oldTex.data() + 3 * sizeof(uint32_t), data,endWidth, endHeight, nChannels))
            {
            newWidth = oldWidth;
            newHeight = oldHeight;
            uvMapBottomLeft.x = (double)x / newWidth;
            uvMapBottomLeft.y = (double)(y + height) / newHeight;
            uvMapTopRight.x = (double)(x + width) / newWidth;
            uvMapTopRight.y = (double)y / newHeight;
            return;
            }
        else
            {
            if (oldWidth + endWidth <= 65535)
                {
                x = oldWidth;
                y = 0;
                newWidth = oldWidth + endWidth;
                newHeight = std::max(endHeight, oldHeight);
                }
            else
                {
                newWidth = std::max(endWidth, oldWidth);
                newHeight = oldHeight + endHeight;
                x = 0;
                y = oldHeight;
                }
            tex.resize(3 * sizeof(uint32_t) + newWidth*newHeight * 3, 0xFF);
            }
        ((uint32_t*)tex.data())[0] = newWidth;
        ((uint32_t*)tex.data())[1] = newHeight;
        ((uint32_t*)tex.data())[2] = 3;
        uint8_t* buffer = tex.data() + 3 * sizeof(uint32_t);

        //copy old texture data in outTex starting at 0,0
        for (size_t i = 0; i < oldWidth; ++i)
            {
            for (size_t j = 0; j < oldHeight; ++j)
                {
                const uint8_t* origPixel = oldTex.data()+3*sizeof(uint32_t)+ j*oldWidth * 3 + i * 3;
                uint8_t* targetPixel = buffer + j*newWidth * 3 + i * 3;
                memcpy(targetPixel, origPixel, 3);
                }
            }

        //copy new texture data in outTex starting at x, y
        for (size_t i = 0; i < endWidth; ++i)
            {
            for (size_t j = 0; j < endHeight; ++j)
                {
                const uint8_t* origPixel = data + j*width * nChannels + i * nChannels;
                uint8_t* targetPixel = buffer + (y + j)*newWidth * 3 + (i + x) * 3;
                memcpy(targetPixel, origPixel, 3);
                }
            }

        //compute uvs
        uvMapBottomLeft.x = (double)x / newWidth;
        uvMapBottomLeft.y = (double)(y + endHeight) / newHeight;
        uvMapTopRight.x = (double)(x + endWidth) / newWidth;
        uvMapTopRight.y = (double)y / newHeight;
        }

    DPoint2d RemapUVs(const DPoint2d& source, const DPoint2d& bottomLeft, const DPoint2d& topRight, const DPoint2d& maxUv)
        {
        DPoint2d uv = source;
        if (maxUv.x > 0)uv.x /= maxUv.x;
        uv.x = uv.x* (topRight.x - bottomLeft.x) + bottomLeft.x;
        if (fabs(uv.x) < 1e-5) uv.x = 0.0;
        if (fabs(uv.x - 1.0) < 1e-5) uv.x = 1.0;
        uv.x = fabs(uv.x);

        if (maxUv.y > 0)uv.y /= maxUv.y;
        uv.y = uv.y* (bottomLeft.y - topRight.y) + topRight.y;
        if (fabs(uv.y) < 1e-5) uv.y = 0.0;
        if (fabs(uv.y - 1.0) < 1e-5) uv.y = 1.0;
        uv.y = fabs(uv.y);
        return uv;
        }

    PolyfaceHeaderPtr CreatePolyfaceFromCurveOrSolid(ElementGeometryPtr curveOrSolid)
        {
        if (curveOrSolid == nullptr)
            return nullptr;

        IFacetOptionsPtr options = IFacetOptions::CreateForSurfaces();
        options->SetMaxPerFace(3);
        options->SetParamsRequired(true);
        options->SetParamMode(FACET_PARAM_Distance);
        options->SetNormalsRequired(true);

        IPolyfaceConstructionPtr constructor = IPolyfaceConstruction::Create(*options);

        switch (curveOrSolid->GetGeometryType())
            {
            case ElementGeometry::GeometryType::CurveVector:
                {
                CurveVectorPtr curve = curveOrSolid->GetAsCurveVector();
                if (curve->IsAnyRegionType())
                    constructor->AddRegion(*curve);
                break;
                }
            case ElementGeometry::GeometryType::SolidPrimitive:
                {
                constructor->AddSolidPrimitive(*curveOrSolid->GetAsISolidPrimitive());
                break;
                }
            case ElementGeometry::GeometryType::BsplineSurface:
                {
                constructor->Add(*curveOrSolid->GetAsMSBsplineSurface());
                break;
                }
            }

        return constructor->GetClientMeshPtr();
        }

    void MakeColorTile(bvector<uint8_t>& tex, int width, int height, RgbFactor color)
        {
        tex.resize(3*sizeof(int)+width*height * 3);
        int nChannels = 3;
        memcpy(&tex[0], &width, sizeof(int));
        memcpy(&tex[0]+sizeof(int), &height, sizeof(int));
        memcpy(&tex[0] + 2*sizeof(int), &nChannels, sizeof(int));
        for (size_t i = 0; i < width; ++i)
            for (size_t j = 0; j < height; ++j)
                {
                *(&tex[0]+3*sizeof(int) + (j*width * 3) + i * 3) = (uint8_t)(color.red * 255);
                *(&tex[0] + 3 * sizeof(int) + (j*width * 3) + i * 3 + 1) = (uint8_t)(color.green * 255);
                *(&tex[0] + 3 * sizeof(int) + (j*width * 3) + i * 3 + 2) = (uint8_t)(color.blue * 255);
                }
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsiclass                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    class DgnDbMeshExtractor : public InputExtractorBase
        {
        private:
            friend class                            DgnDbMeshExtractorCreator;

            PODPacketProxy<DPoint3d>                m_pointPacket;
            PODPacketProxy<int32_t>                m_indexPacket;
            PODPacketProxy<uint8_t>               m_metadataPacket;
            PODPacketProxy<uint8_t>               m_texPacket;
            PODPacketProxy<DPoint2d>              m_uvPacket;


            bool m_hasNext;

            int                                     m_defaultPtCount;
            DgnDbPtr                                m_database;
            DgnModelPtr                             m_model;
            DgnModel::const_iterator                m_currentElement;
            Transform                              m_scaleUnits;
            uint64_t                               m_nextTexId;

            bmap<int, uint64_t>    m_mapOfTexColorIds;
            bmap<uint64_t, uint64_t>    m_mapOfTexIds;

            size_t m_nelements;

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                  Elenie.Godzaridis  06/2016
            +---------------+---------------+---------------+---------------+---------------+------*/
            explicit                        DgnDbMeshExtractor(DgnDbSource& dgndbFile)
                {
                m_database = dgndbFile.GetDatabase();
                m_hasNext = false;
                m_model = m_database->Models().GetModel(m_database->Models().QueryFirstModelId());
                m_model->FillModel();
                m_currentElement = m_model->begin();
                m_hasNext = m_currentElement != m_model->end();
                DPoint3d scale = DPoint3d::From(1, 1, 1);
                m_database->Units().GetDgnGCS()->UorsFromCartesian(scale, scale);
                m_scaleUnits.InverseOf(Transform::FromScaleFactors(scale.x, scale.y, scale.z));
                m_nelements = 0;
                m_nextTexId = 1;
                }

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                 Elenie.Godzaridis   06/2016
            +---------------+---------------+---------------+---------------+---------------+------*/
            virtual void                    _Assign(PacketGroup&     rawEntities) override
                {
                m_pointPacket.AssignTo(rawEntities[0]);
                m_indexPacket.AssignTo(rawEntities[1]);
                m_metadataPacket.AssignTo(rawEntities[2]);
                m_texPacket.AssignTo(rawEntities[3]);
                m_uvPacket.AssignTo(rawEntities[4]);
                }

            virtual size_t              _GetPhysicalSize() override
                {
                return 0;
                }

            virtual size_t              _GetReadPosition() override
                {
                return 0;
                }

            /*---------------------------------------------------------------------------------**//**
            * @description
            * @bsimethod                                                  Elenie.Godzaridis   06/2016
            +---------------+---------------+---------------+---------------+---------------+------*/
            virtual void                    _Read() override
                {
                    m_pointPacket.SetSize(0);
                    m_indexPacket.SetSize(0);
                    m_metadataPacket.SetSize(0);
                    m_texPacket.SetSize(0);
                    m_uvPacket.SetSize(0);
                if (m_hasNext)
                    {
                    DgnElementCPtr elem = m_currentElement->second;
                   // bool filterElem = true;// elem->GetElementClass()->GetId() == 320;//357 || elem->GetElementClass()->GetId() == 354 || elem->GetElementClass()->GetId() == 328 || elem->GetElementClass()->GetId() == 325 || elem->GetElementClass()->GetId() == 265 || elem->GetElementClass()->GetId() == 320;//265; //341 || elem->GetElementClass()->GetId() == 344 || elem->GetElementClass()->GetId() == 388;
                    bool filterElem = elem->GetElementClass()->GetId() != 320;// && elem->GetElementClass()->GetId() != 273; //elem->GetElementClass()->GetId() == 257 || elem->GetElementClass()->GetId() == 251 || elem->GetElementClass()->GetId() == 260;
                    if (filterElem && nullptr != elem->ToGeometrySource3d())
                        {
                        Transform transformToWorld;
                        Placement3dCR placement = elem->ToGeometrySource3d()->GetPlacement();
                        transformToWorld = placement.GetTransform();
                        bvector<int32_t> indices;
                        bvector<DPoint3d> pts;
                        bvector<DPoint2d> uvs;
                        bvector<bvector<uint8_t>> tex;
                        bvector<int64_t> texIds;
                        bvector<int32_t> parts;
                        ElementGeometryCollection collection(*m_database, dynamic_cast<const GeometricElement3d*>(elem.get())->GetGeomStream());
                        size_t offset = 0;
                        bool hasTexture = false;
                        for (ElementGeometryPtr geometry : collection)
                            {
                            geometry->TransformInPlace(transformToWorld);
                            geometry->TransformInPlace(m_scaleUnits);
                            PolyfaceHeaderPtr mesh = geometry->GetAsPolyfaceHeader();
                            if (geometry->GetGeometryType() == ElementGeometry::GeometryType::SolidPrimitive || geometry->GetGeometryType() == ElementGeometry::GeometryType::CurveVector)
                                mesh = CreatePolyfaceFromCurveOrSolid(geometry);
                            if (!mesh.IsValid()) continue;

                            DPoint2d uvMapBottomLeft, uvMapTopRight;
                            bool currentTex = false;
                            bool texBasedOnColor = false;
                            int64_t currentTexId = -1;
                            ElemDisplayParams params(collection.GetElemDisplayParams()); //find whether element is textured
                            // if (params.GetSubCategoryId().IsValid())
                                {
                                NullContext ct(*m_database);
                                params.Resolve(ct);
                                DgnMaterialId id = params.GetMaterial();
                                RenderMaterialPtr renderMat = JsonRenderMaterial::Create(*m_database, id);
                                tex.resize(tex.size() + 1);
                                if (renderMat.IsValid())
                                    {
                                    RenderMaterialMapPtr      patternMap = renderMat->_GetMap(RENDER_MATERIAL_MAP_Pattern);
                                    if (patternMap.IsValid())
                                        {
                                        Json::Value     textureIdValue = ((JsonRenderMaterialMap*)patternMap.get())->GetValue()[RENDER_MATERIAL_TextureId];
                                        if (m_mapOfTexIds.count((uint64_t)textureIdValue.asInt()) == 0)
                                            {
                                            ImageBufferPtr data = patternMap->_GetImage(*m_database);

                                            AppendTextureToExisting(tex.back(), uvMapBottomLeft, uvMapTopRight, data->GetWidth(), data->GetHeight(), data->GetDataCP(), data->GetFormat());
                                            currentTexId = (int64_t)m_nextTexId;
                                            m_mapOfTexIds[textureIdValue.asUInt64()] = m_nextTexId++;
                                            }
                                        else currentTexId = (int64_t)m_mapOfTexIds[textureIdValue.asUInt64()];
                                        /*ImageBufferPtr data = patternMap->_GetImage(*m_database);
                                        int width = tex.size() == 0 ? 0 : ((uint32_t*)tex.data())[0], height = tex.size() == 0 ? 0 : ((uint32_t*)tex.data())[1];
                                        AppendTextureToExisting(tex, uvMapBottomLeft, uvMapTopRight, data->GetWidth(), data->GetHeight(), data->GetDataCP(), data->GetFormat());
                                        DPoint2d maxUv = DPoint2d::From(1, 1);
                                        DPoint2d newUvTopRight = DPoint2d::From((double)width / ((uint32_t*)tex.data())[0], 0);
                                        DPoint2d newUvBotLeft = DPoint2d::From(0, (double)height/ ((uint32_t*)tex.data())[1]);
                                        for (auto& uv : uvs)
                                        {
                                        uv = RemapUVs(uv, newUvBotLeft, newUvTopRight, maxUv);
                                        }*/
                                        currentTex = hasTexture = true;
                                        }
                                    else if (renderMat->_GetBool(RENDER_MATERIAL_FlagHasBaseColor))
                                        {
                                        RgbFactor diffuseColor = renderMat->_GetColor(RENDER_MATERIAL_Color);
                                        if (m_mapOfTexColorIds.count(/*diffuseColor.ToIntColor()*/0) == 0)
                                            {
                                            MakeColorTile(tex.back(), 4, 4, diffuseColor);
                                            currentTexId = (int64_t)m_nextTexId;
                                            m_mapOfTexColorIds[/*diffuseColor.ToIntColor()*/0] = m_nextTexId++;
                                            }
                                        else currentTexId = (int64_t)m_mapOfTexColorIds[/*diffuseColor.ToIntColor()*/0];
                                        /* int width = tex.size() == 0 ? 0 : ((uint32_t*)tex.data())[0], height = tex.size() == 0 ? 0 : ((uint32_t*)tex.data())[1];
                                         AppendTextureToExisting(tex, uvMapBottomLeft, uvMapTopRight,4, 4,colorTileRgb.data(), ImageBuffer::Format::Rgb);
                                         DPoint2d maxUv = DPoint2d::From(1, 1);
                                         DPoint2d newUvTopRight = DPoint2d::From((double)width / ((uint32_t*)tex.data())[0], 0);
                                         DPoint2d newUvBotLeft = DPoint2d::From(0, (double)height / ((uint32_t*)tex.data())[1]);
                                         for (auto& uv : uvs)
                                         {
                                         uv = RemapUVs(uv, newUvBotLeft, newUvTopRight, maxUv);
                                         }*/
                                        currentTex = hasTexture = true;
                                        texBasedOnColor = true;
                                        }
                                    if (mesh->GetParamCount() == 0)
                                        mesh->BuildPerFaceParameters(LocalCoordinateSelect::LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft);
                                    }
                                texIds.push_back(currentTexId);
                                }
                                mesh->Triangulate();
                                pts.insert(pts.end(), mesh->GetPointCP(), mesh->GetPointCP() + mesh->GetPointCount());
                                int nextPt = (int)pts.size() - (int)offset;
                                if (texBasedOnColor &&currentTex && mesh->GetParamCount() > pts.size()) pts.resize(mesh->GetParamCount());
                                uvs.resize(pts.size(), DPoint2d::From(0.0, 0.0));
                                PolyfaceVisitorPtr vis = PolyfaceVisitor::Attach(*mesh);
                                bvector<int> &pointIndex = vis->ClientPointIndex();
                                bvector<int> &param = vis->ClientParamIndex();
                                bmap<int, int> paramsMap;
                              /*  DPoint2d uvMax = DPoint2d::From(1.0, 1.0);
                                for (size_t i = 0; i < mesh->GetParamCount(); ++i)
                                    {
                                    uvMax.x = std::max(fabs(mesh->GetParamCP()[i].x), uvMax.x);
                                    uvMax.y = std::max(fabs(mesh->GetParamCP()[i].y), uvMax.y);
                                    }*/
                                for (vis->Reset(); vis->AdvanceToNextFace();)
                                    {
                                    int32_t idx[3] = { pointIndex[0], pointIndex[1], pointIndex[2] };
                                    if (currentTex)
                                        {
                                        for (size_t i = 0; i < 3; ++i)
                                            if (!texBasedOnColor && paramsMap.count(idx[i]) > 0 && paramsMap[idx[i]] != param[i])
                                                {
                                                if (nextPt + (int)offset >= pts.size())
                                                    {
                                                    pts.resize(nextPt + (int)offset + 1);
                                                    uvs.resize(pts.size(), DPoint2d::From(0.0, 0.0));
                                                    }
                                                pts[nextPt + (int)offset] = pts[idx[i] + (int)offset];
                                                idx[i] = nextPt;
                                                nextPt++;
                                                }
                                        for (size_t i = 0; i < 3; ++i)
                                            paramsMap[idx[i]] = param[i];
                                        uvs[idx[0] + (int)offset] = mesh->GetParamCP()[param[0]];//RemapUVs(mesh->GetParamCP()[param[0]], uvMapBottomLeft, uvMapTopRight, uvMax);
                                        uvs[idx[1] + (int)offset] = mesh->GetParamCP()[param[1]];//RemapUVs(mesh->GetParamCP()[param[1]], uvMapBottomLeft, uvMapTopRight, uvMax);
                                        uvs[idx[2] + (int)offset] = mesh->GetParamCP()[param[2]];//RemapUVs(mesh->GetParamCP()[param[2]], uvMapBottomLeft, uvMapTopRight, uvMax);
                                        }
                                    indices.push_back(idx[0] + 1 + (int)offset);
                                    indices.push_back(idx[1] + 1 + (int)offset);
                                    indices.push_back(idx[2] + 1 + (int)offset);
                                    }
                                pts.resize(nextPt + (int)offset);
                                uvs.resize(pts.size(), DPoint2d::From(0.0, 0.0));
                                offset = pts.size();
                                parts.push_back((int)indices.size());
                            }
                        m_nelements++;
                        if (!pts.empty() && !indices.empty())
                            {
                            if (elem->GetElementClass()->GetId() == 340 && pts.size() > 10000 && parts.size() == 1)
                                {
                                MTGGraph g;
                                bvector<int> componentPointsId;
                                CreateGraphFromIndexBuffer(&g, (const long*)&indices[0], (int)indices.size(), (int)pts.size(), componentPointsId, &pts[0]);
                                std::vector<DPoint3d> newPts;
                                newPts.insert(newPts.end(), pts.begin(), pts.end());
                                CGALEdgeCollapse(&g, newPts, elem->GetElementId().GetValue());
                                MTGGraph* graph = &g;
                                std::vector<int> faceIndices;
                                MTGMask visitedMask = graph->GrabMask();
                                bvector<DPoint3d> retainedPts;
                                bmap<DPoint3d, int, DPoint3dZYXTolerancedSortComparison> ptMap(DPoint3dZYXTolerancedSortComparison(10e-4, 0));
                                bvector<int> indices(pts.size(), -1);


                                MTGARRAY_SET_LOOP(edgeID, graph)
                                    {
                                    if (!graph->GetMaskAt(edgeID, visitedMask))
                                        {
                                        if (FastCountNodesAroundFace(graph, edgeID) != 3)
                                            {
                                            int vIndex = -1;
                                            graph->TryGetLabel(edgeID, 0, vIndex);
                                            assert(vIndex > 0);
                                            assert(vIndex <= (int)pts.size());
                                            if (indices[vIndex - 1] == -1)
                                                {
                                                if (ptMap.count(pts[vIndex - 1]) == 0)
                                                    {
                                                    retainedPts.push_back(pts[vIndex - 1]);
                                                    indices[vIndex - 1] = (int)retainedPts.size();
                                                    ptMap[pts[vIndex - 1]] = (int)retainedPts.size();
                                                    }
                                                else
                                                    {
                                                    indices[vIndex - 1] = ptMap[pts[vIndex - 1]];
                                                    }
                                                }
                                            int idx = indices[vIndex - 1];
                                            graph->TrySetLabel(edgeID, 0, idx);
                                            graph->SetMaskAt(edgeID, MTG_EXTERIOR_MASK);
                                            graph->SetMaskAt(graph->EdgeMate(edgeID), MTG_BOUNDARY_MASK);
                                            continue;
                                            }
                                        MTGARRAY_FACE_LOOP(faceID, graph, edgeID)
                                            {
                                            int vIndex = -1;
                                            graph->TryGetLabel(faceID, 0, vIndex);
                                            assert(vIndex > 0);
                                            assert(vIndex <= (int)pts.size());
                                            if (indices[vIndex - 1] == -1)
                                                {
                                                if (ptMap.count(pts[vIndex - 1]) == 0)
                                                    {
                                                    retainedPts.push_back(pts[vIndex - 1]);
                                                    indices[vIndex - 1] = (int)retainedPts.size();
                                                    ptMap[pts[vIndex - 1]] = (int)retainedPts.size();
                                                    }
                                                else
                                                    {
                                                    indices[vIndex - 1] = ptMap[pts[vIndex - 1]];
                                                    }
                                                }
                                            int idx = indices[vIndex - 1];
                                            faceIndices.push_back(idx);
                                            graph->SetMaskAt(faceID, visitedMask);
                                            if (graph->GetMaskAt(faceID, MTG_EXTERIOR_MASK)) graph->ClearMaskAt(faceID, MTG_EXTERIOR_MASK);
                                            if (graph->GetMaskAt(faceID, MTG_BOUNDARY_MASK)) graph->ClearMaskAt(faceID, MTG_BOUNDARY_MASK);
                                            graph->TrySetLabel(faceID, 0, idx);
                                            }
                                        MTGARRAY_END_FACE_LOOP(faceID, graph, edgeID)
                                        }
                                    }
                                MTGARRAY_END_SET_LOOP(edgeID, graph)
                                    graph->ClearMask(visitedMask);
                                graph->DropMask(visitedMask);
                                pts = retainedPts;
                                indices.clear();
                                indices.insert(indices.end(), faceIndices.begin(), faceIndices.end());
                                uvs.resize(pts.size(),DPoint2d::From(0.0,0.0));
                                parts.back() = (int)indices.size();
                                }
                            else if (pts.size() > 10000 && parts.size() >5)
                                {
                                size_t id = 0;
                                for (; id < parts.size() && parts[id] < 15000; id++) {}
                                parts.resize(id);
                                indices.resize(parts.back());
                                }
                            if (elem->GetElementClass()->GetId() != 320 || pts.size() < 30000)
                                {
                                memcpy(m_pointPacket.Edit(), &pts[0], pts.size()*sizeof(DPoint3d));
                                m_pointPacket.SetSize(pts.size());
                                memcpy(m_indexPacket.Edit(), &indices[0], indices.size() * sizeof(int32_t));
                                m_indexPacket.SetSize(indices.size());
                                Json::Value val(Json::objectValue);
                                val["elementId"] = elem->GetElementId().GetValue();
                                val["texId"] = Json::arrayValue;
                                bset<int64_t> texIdSet;
                                for (auto& texID : texIds)
                                    {
                                    assert(texID < 10000);
                                    val["texId"].append(texID);
                                    texIdSet.insert(texID);
                                    }
                                val["parts"] = Json::arrayValue;
                                for (auto& part : parts)
                                    val["parts"].append(part);
                                val["mapIds"] = Json::arrayValue;

                                for (auto& mapId : m_mapOfTexIds)
                                    {
                                    if (texIdSet.count(mapId.second) != 0)
                                        {
                                        Json::Value newEntry = Json::objectValue;
                                        newEntry["dgn"] = mapId.first;
                                        newEntry["SM"] = mapId.second;
                                        val["mapIds"].append(newEntry);
                                        }
                                    }
                                for (auto& mapId : m_mapOfTexColorIds)
                                    {
                                    if (texIdSet.count(mapId.second) != 0)
                                        {
                                        Json::Value newEntry = Json::objectValue;
                                        newEntry["dgn"] = ((uint64_t)0xFF00CECE << 32) | mapId.first;
                                        newEntry["SM"] = mapId.second;
                                        val["mapIds"].append(newEntry);
                                        }
                                    }
                                Utf8String jsonString(Json::FastWriter().write(val));
                                memcpy(m_metadataPacket.Edit(), jsonString.data(), jsonString.SizeInBytes());
                                m_metadataPacket.SetSize(jsonString.SizeInBytes());

                                if (hasTexture)
                                    {
                                    memcpy(m_uvPacket.Edit(), &uvs[0], uvs.size()*sizeof(DPoint2d));
                                    m_uvPacket.SetSize(uvs.size());

                                    bvector<uint8_t> serializedTexData;
                                    /*memcpy(m_texPacket.Edit(), &tex[0], tex.size()*sizeof(uint8_t));
                                    m_texPacket.SetSize(tex.size());*/
                                    for (auto& textureData : tex)
                                        {
                                        size_t offset = serializedTexData.size();
                                        serializedTexData.resize(serializedTexData.size()+textureData.size() + sizeof(size_t));
                                        size_t currentSize = textureData.size();
                                        memcpy(&serializedTexData[0] + offset, &currentSize, sizeof(size_t));
                                        if (currentSize > 0)
                                            memcpy(&serializedTexData[0] + offset+sizeof(size_t), &textureData[0], currentSize);
                                        }

                                    if (serializedTexData.size() > 0)
                                        {
                                        memcpy(m_texPacket.Edit(), &serializedTexData[0], serializedTexData.size()*sizeof(uint8_t));
                                        m_texPacket.SetSize(serializedTexData.size());
                                        }

                                    }
                                }
                            }
                        }
                    m_currentElement++;
                    }
                    m_hasNext =/* m_nelements < 20 && */m_currentElement != m_model->end();
//                m_headerPacket.SetSize(m_featureArray.GetHeaders().GetSize());
//                m_pointPacket.SetSize(m_featureArray.GetPoints().GetSize());
                }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool                    _Next                          () override
        {
        return m_hasNext;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Elenie.Godzaridis   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
    class DgnDbSourceCreator : public LocalFileSourceCreatorBase
    {
    virtual ExtensionFilter         _GetExtensions                         () const override
        {
        return L"*.dgndb";
        }    

    virtual bool                    _Supports                              (const LocalFileSourceRef&   pi_rSourceRef) const override
        {
        if(!DefaultSupports(pi_rSourceRef))
            return false;

        return true;
        }

    virtual SourceBase*             _Create                                (const LocalFileSourceRef&   pi_rSourceRef,
                                                                            Log&                 pi_warningLog) const override
        {
        return new DgnDbSource(pi_rSourceRef.GetPath());
        }
    };

    const SourceRegistry::AutoRegister<DgnDbSourceCreator> s_RegisterDgnDbFile;


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Elenie.Godzaridis   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
    class DgnDbMeshExtractorCreator : public InputExtractorCreatorMixinBase<DgnDbSource>
    {
    virtual bool                                _Supports                          (const DataType&             type) const override
        {
        return type.GetFamily() == MeshTypeFamilyCreator().Create();
        }


    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual RawCapacities                       _GetOutputCapacities(DgnDbSource&                 sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection) const override
        {
        return RawCapacities(1000000 * sizeof(DPoint3d), 5000000 * sizeof(int32_t), 10000 * sizeof(uint8_t), 4000 * 4000 * 3 * sizeof(uint8_t), 1000000 * sizeof(DPoint2d));
        }

    /*---------------------------------------------------------------------------------**//**
    * @description
    * @bsimethod                                                  Elenie.Godzaridis   06/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual InputExtractorBase*                 _Create(DgnDbSource&                 sourceBase,
                                                                                    const BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Source&                   source,
                                                                                    const ExtractionQuery&          selection,
                                                                                    const ExtractionConfig&         config,
                                                                                    Log&                            log) const override
        {
        return new DgnDbMeshExtractor(sourceBase);
        }
    };


    const ExtractorRegistry::AutoRegister<DgnDbMeshExtractorCreator> s_RegisterDgnDbMeshImporter;
    }//END UNNAMED NAMESPACE