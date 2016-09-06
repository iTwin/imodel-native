#define NOMINMAX
#include <stdint.h>
typedef uint8_t byte;
using namespace std;

class SMNodeGroup;
class SMNodeGroupMasterHeader;
#include <stdlib.h>
#include <Bentley\Bentley.h>
#include <ImagePP/h/ImageppAPI.h>
#undef static_assert
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/DesktopTools/WindowsKnownLocationsAdmin.h>
USING_NAMESPACE_BENTLEY_DGNPLATFORM
#include <TerrainModel/Core/bcdtmClass.h>
#include <ScalableMesh\IScalableMesh.h>
#undef static_assert
#include <mutex>
#include <ScalableMesh\IScalableMeshSourceCreator.h>
#include <ScalableMesh\ScalableMeshLib.h>
#include <ConceptDataAccess/ConceptDataAccessApi.h>
#include <json/json.h>
#include <ConceptStationBase/ConceptStationBase.h>
#include <ConceptStationDomain/ConceptStationDomain.h>
#include <ConceptStationBridge/ConceptStationBridgeApi.h>

CONCEPTSTATION_TYPEDEFS(OutboundRoad)
CONCEPTSTATION_TYPEDEFS(IntersectionCornerData)
CONCEPTSTATION_TYPEDEFS(SimpleRadiusCornerData)
CONCEPTSTATION_TYPEDEFS(RadiusAndTaperCornerData)
CONCEPTSTATION_TYPEDEFS(IntersectionCornerResult)
CONCEPTSTATION_TYPEDEFS(SimpleRadiusCornerResult)
CONCEPTSTATION_TYPEDEFS(RadiusAndTaperCornerResult)
CONCEPTSTATION_TYPEDEFS(IIntersectionSegmentDetailedDimensions)
CONCEPTSTATION_TYPEDEFS(CurveWithIndex)
CONCEPTSTATION_TYPEDEFS(IntersectionDetail)
CONCEPTSTATION_TYPEDEFS(DefaultTemplateProviderFactory)
CONCEPTSTATION_TYPEDEFS(TemplateProvider);
CONCEPTSTATION_TYPEDEFS(TransitionTemplateProvider);
CONCEPTSTATION_TYPEDEFS(DefaultTemplateProvider);
CONCEPTSTATION_TYPEDEFS(RoadSample);
CONCEPTSTATION_TYPEDEFS(RoadMeshGenerator);

CONCEPTSTATION_REF_COUNTED_PTR(IntersectionCornerData)
CONCEPTSTATION_REF_COUNTED_PTR(SimpleRadiusCornerData)
CONCEPTSTATION_REF_COUNTED_PTR(RadiusAndTaperCornerData)
CONCEPTSTATION_REF_COUNTED_PTR(IntersectionCornerResult)
CONCEPTSTATION_REF_COUNTED_PTR(SimpleRadiusCornerResult)
CONCEPTSTATION_REF_COUNTED_PTR(RadiusAndTaperCornerResult)
CONCEPTSTATION_REF_COUNTED_PTR(IIntersectionSegmentDetailedDimensions)
CONCEPTSTATION_REF_COUNTED_PTR(CurveWithIndex)
CONCEPTSTATION_REF_COUNTED_PTR(IntersectionDetail)
CONCEPTSTATION_REF_COUNTED_PTR(DefaultTemplateProviderFactory)

CONCEPTSTATION_REF_COUNTED_PTR(TemplateProvider);
CONCEPTSTATION_REF_COUNTED_PTR(DefaultTemplateProvider);
CONCEPTSTATION_REF_COUNTED_PTR(TransitionTemplateProvider);
CONCEPTSTATION_REF_COUNTED_PTR(RoadSample);
CONCEPTSTATION_REF_COUNTED_PTR(RoadMeshGenerator);

#include "RoadHelper.h"
#include "RoadIntersectionEditor.h"

#include "MeshGenerators/BuildingGenerator.h"
#include "MeshGenerators/DesignRoadGenerator.h"
#include "MeshGenerators/FurnitureGenerator.h"
#include "MeshGenerators/MeshGen.h"
#include "MeshGenerators/IntersectionGenerator.h"
#include "MeshGenerators/WaterbodyGenerator.h"
#include "RoadSegmentDetailedDimensions.h"

#include <ConceptStationDomain/IntersectionSizeChecker.h>
#include <ConceptStationDomain/MeshGenerators/MeshGen.h>

USING_NAMESPACE_BENTLEY_CONCEPTCIVIL
USING_NAMESPACE_BENTLEY_CONCEPTSTATION
USING_NAMESPACE_BENTLEY_SQLITE
#pragma warning(disable:4189)

DgnDbPtr mainProject;
Transform s_unitsTrans;
std::mutex dgndbMutex;

void DrawPolyline(bvector<PolyfaceHeaderPtr>& result, bvector<ImageBufferPtr>& textures, DgnElementCP sourceElem, CurveVectorCP horizontalAlignment, CurveVectorCP verticalAlignment, double samplingFactor = 1);

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
class AppHost : public DgnPlatformLib::Host
    {

    protected:

        virtual void _SupplyProductName(Utf8StringR str) override { str.assign("Test"); }
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override
            {
            return *new WindowsKnownLocationsAdmin();
            }

        virtual BeSQLite::L10N::SqlangFiles  _SupplySqlangFiles() override
            {
            return L10N::SqlangFiles(BeFileName());
            }
        virtual DgnPlatformLib::Host::GeoCoordinationAdmin&      _SupplyGeoCoordinationAdmin() override
            {

            WString geocoordinateDataPath(L".\\GeoCoordinateData\\");

            return *DgnGeoCoordinationAdmin::Create(BeFileName(geocoordinateDataPath.c_str()));
            }
    };

AppHost libHost;

void BuildSubResolutionRoad(bvector<PolyfaceHeaderPtr>& result, bvector<ImageBufferPtr>& textures, JsonValueR resInfo, DgnElementCP sourceElem, JsonValueCR config, DRange3d nodeExt, DRange3d elemRange, size_t totalNElements)
    {
    RoadSegmentCP roadSegment = (RoadSegmentCP)sourceElem;

    double fraction = /*std::max(*/std::min(elemRange.XLength() / nodeExt.XLength(), elemRange.YLength() / nodeExt.YLength())/*, elemRange.ZLength() / nodeExt.ZLength())*/;

    double distance = 15.0 + (2 * 1/fraction);


    RoadSegmentInfoPtr info = RoadSegmentInfo::Create(*roadSegment);

    if (!info.IsValid()) return;

    if (fraction < 0.05 || totalNElements > 50)
        {
        if (info->GetPartialHorizontalAlignmentCP() != nullptr && info->GetPartialVerticalAlignmentCP() != nullptr)
        DrawPolyline(result, textures, sourceElem, info->GetPartialHorizontalAlignmentCP(), info->GetPartialVerticalAlignmentCP());
        return;
        }
    
    RoadMeshGeneratorPtr designerPtr = RoadMeshGenerator::Create();

    if (distance > info->GetPartialHorizontalAlignmentCP()->Length() / 2.0) distance = info->GetPartialHorizontalAlignmentCP()->Length() / 2.0;
    designerPtr->SetDropInterval(distance);

    designerPtr->SetHorizontalAlignment(info->GetPartialHorizontalAlignmentCP());
    designerPtr->SetVerticalAlignment(info->GetPartialVerticalAlignmentCP());
    designerPtr->SetCreateSkirts(false); 

    DefaultTemplateProviderPtr templateProvider = DefaultTemplateProvider::Create(*info->GetGeometryTemplateCP());
    bvector<bvector<DPoint3d>> endConditionLineStrings;
    designerPtr->BuildRoadPolyfaces(*templateProvider, 0.0, designerPtr->GetLength(), endConditionLineStrings);

    RoadMeshResult res = designerPtr->RetrievePolyfaces();
    IFacetOptionsPtr  options = IFacetOptions::Create();

/*    Transform transformToWorld;
    Placement3dCR placement = sourceElem->ToGeometrySource3d()->GetPlacement();
    transformToWorld = placement.GetTransform();*/

    for (auto const& polyface : res.GetMeshedComponent())
        {
        IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::New(*options);
        DgnMaterialId matId = ConceptualMaterials::QueryMaterialId(*mainProject, polyface.second);
        RenderMaterialPtr renderMat = JsonRenderMaterial::Create(*mainProject, matId);
        if (renderMat.IsValid())
            {
            RenderMaterialMapPtr      patternMap = renderMat->_GetMap(RENDER_MATERIAL_MAP_Pattern);
            if (patternMap.IsValid())
                {
                ImageBufferPtr data = patternMap->_GetImage(*mainProject);
                textures.push_back(data);
                }
            else textures.push_back(nullptr);
            if (polyface.first->GetParamCount() == 0)
                polyface.first->BuildPerFaceParameters(LocalCoordinateSelect::LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft);
            }
        else textures.push_back(nullptr);
        //polyface.first->Transform(transformToWorld);
        builder->AddPolyface(*polyface.first);
        result.push_back(builder->GetClientMeshPtr());
        }
    }

void DrawPolyline(bvector<PolyfaceHeaderPtr>& result, bvector<ImageBufferPtr>& textures, DgnElementCP sourceElem, CurveVectorCP horizontalAlignment, CurveVectorCP verticalAlignment, double samplingFactor)
    {
    IFacetOptionsPtr  options = IFacetOptions::Create();
    IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::New(*options);
    bvector<bvector<bvector<DPoint3d>>> pointsOnLine, elevationPointsOnLine;
    horizontalAlignment->CollectLinearGeometry(pointsOnLine);
    verticalAlignment->CollectLinearGeometry(elevationPointsOnLine);

    bvector<DPoint3d> polygon;
    DVec3d vec;
    int step = (int)floor(pointsOnLine[0][0].size() / samplingFactor * pointsOnLine[0][0].size());
    for (size_t i = 0; i < pointsOnLine[0][0].size(); ++i)
        {
        DPoint3d pt = pointsOnLine[0][0][i];
        if (elevationPointsOnLine[0][0].size() > i) pt.z = elevationPointsOnLine[0][0][i].z;
        if (i == 1)
            {
            DVec3d vecSource = DVec3d::FromStartEnd(pointsOnLine[0][0][i], pointsOnLine[0][0][i-1]);
            DVec3d vecTarget = vecSource;
            vecTarget.Negate();
            vecTarget.Normalize();
            vecTarget.Add(DVec3d::From(0, 0.0001, 0));
            vec = DVec3d::FromRotate90Towards(vecSource, vecTarget);
            vec.Normalize();
            vec.Scale(10);
            }
        //if (i % step != 0) continue;
        polygon.push_back(pt);
        }
    for (int j = (int)pointsOnLine[0][0].size() - 1; j >= 0; --j)
        {
        DPoint3d pt = pointsOnLine[0][0][j];
        if (elevationPointsOnLine[0][0].size() > j) pt.z = elevationPointsOnLine[0][0][j].z;
        pt.SumOf(pt, vec);
        polygon.push_back(pt);
        }
    if (polygon.size() > 0)
        {
        polygon.push_back(polygon.front());
        //builder->AddPolyface(*mesh);
        builder->AddTriangulation(polygon);
        result.push_back(builder->GetClientMeshPtr());
        textures.push_back(nullptr);
        }
    }

void BuildSubResolutionBridge(bvector<PolyfaceHeaderPtr>& result, bvector<ImageBufferPtr>& textures, JsonValueR resInfo, DgnElementCP sourceElem, JsonValueCR config, DRange3d nodeExt, DRange3d elemRange, size_t totalNElements)
    {
    BridgeSegmentCP bridgeSegment = (BridgeSegmentCP)sourceElem;
    double fraction =  std::min(elemRange.XLength() / nodeExt.XLength(), elemRange.YLength() / nodeExt.YLength())/*, elemRange.ZLength() / nodeExt.ZLength())*/;

    double distance = 15.0 + (2 * 1 / fraction);

    RoadSegmentInfoPtr info = RoadSegmentInfo::Create(*bridgeSegment);
    BridgeSuperstructureDimensionsCP pSuperstructure = bridgeSegment->GetSuperstructureDimensions();

    CrossSectionTemplateCPtr csTemplate = CrossSectionTemplate::Get(*mainProject, CrossSectionTemplate::Scenery::GetSceneryDeckTemplateId(*mainProject, 1, 1));

    if (fraction < 0.05 || totalNElements > 50)
        {
        if (info->GetPartialHorizontalAlignmentCP() != nullptr && info->GetPartialVerticalAlignmentCP() != nullptr)
        DrawPolyline(result, textures, sourceElem, info->GetPartialHorizontalAlignmentCP(), info->GetPartialVerticalAlignmentCP());
        return;
        }
    else if (fraction < 0.25 || totalNElements > 30)
        {
        info->SetCrossSectionTemplateId(CrossSectionTemplate::Scenery::GetSceneryDeckTemplateId(*mainProject, 1, 1));
        info->SetCrossSectionTemplate(csTemplate.get());
        }



    CurveVectorPtr fullHorizontalAlignment = info->GetHorizontalAlignmentCP()->Clone();
    CurveVectorPtr fullVerticalAlignment = (nullptr != info->GetVerticalAlignmentCP()) ? info->GetVerticalAlignmentCP()->Clone() : nullptr;

    bvector<ConceptualElementGeometry> superstructureGeometryParts;
    bvector<bpair<ConceptualSubCategoryType, ElementGeometryPtr>> startAbutmentGeometryParts;
    bvector<bpair<ConceptualSubCategoryType, ElementGeometryPtr>> endAbutmentGeometryParts;
    bvector<bvector<bpair<ConceptualSubCategoryType, ElementGeometryPtr>>> pierGeometryParts;

    bvector<BridgeSupportElementPtr> supportElements;
    bvector<BridgePierCP> piers;
    BridgeAbutmentCP startAbutment = nullptr, endAbutment = nullptr;
    
    DPoint3d origin;
    fullHorizontalAlignment->GetStartPoint(origin);
    Transform worldToElement = Transform::From(DPoint3d::FromScale(origin, -1));
    CurveVectorPtr roadSegmentHorizAlignmentInElementCoords = info->GetPartialHorizontalAlignmentCP()->Clone();
    roadSegmentHorizAlignmentInElementCoords->TransformInPlace(worldToElement);
    Transform transformToWorld;
    Placement3dCR placement = sourceElem->ToGeometrySource3d()->GetPlacement();
    transformToWorld = placement.GetTransform();

    for (ConceptualElementKey bridgeSupportKey : bridgeSegment->QueryOrderedSupports())
        {
        auto supportElmPtr = BridgeSupportElement::GetForEdit(bridgeSegment->GetDgnDb(), bridgeSupportKey.GetElementId());
        supportElements.push_back(supportElmPtr);

        if (BridgePierP pierElm = dynamic_cast<BridgePierP>(supportElmPtr.get()))
            piers.push_back(pierElm);
        else if (BridgeAbutmentP abutmentElm = dynamic_cast<BridgeAbutmentP>(supportElmPtr.get()))
            {
            BeAssert(!endAbutment);

            if (!startAbutment)
                startAbutment = abutmentElm;
            else
                endAbutment = abutmentElm;
            }
        else
            {
            BeAssert(false);
            }
        }

    if (startAbutment == nullptr || endAbutment == nullptr) return;
    BeFileName sectionLibPath =BeFileName( L".\\");
    sectionLibPath.AppendUtf8("SectionLibs/default.cs1"); 
    ConceptualDrapePtr draperPtr = ConceptualDrape::Create(bridgeSegment->GetDgnDb());
    BridgeMeshGeneratorPtr bridgeGenPtr = BridgeMeshGenerator::Create(sectionLibPath, true);
    IFacetOptionsPtr  options = IFacetOptions::Create();

    if (distance > info->GetPartialHorizontalAlignmentCP()->Length() / 2.0) distance = info->GetPartialHorizontalAlignmentCP()->Length() / 2.0;
    bridgeGenPtr->SetDropInterval(distance);
    if (SUCCESS == bridgeGenPtr->Generate(nullptr, *pSuperstructure, *startAbutment, *endAbutment, piers, *draperPtr,
        *info, roadSegmentHorizAlignmentInElementCoords,
        superstructureGeometryParts, startAbutmentGeometryParts, endAbutmentGeometryParts, pierGeometryParts, nullptr, nullptr, nullptr))
        {
        for (auto& part : superstructureGeometryParts)
            {
            if (!part.m_elementGeometryPtr->GetAsPolyfaceHeader().IsValid()) continue;
            PolyfaceHeaderPtr mesh = part.m_elementGeometryPtr->GetAsPolyfaceHeader();
            IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::New(*options);
            DgnMaterialId matId = ConceptualMaterials::QueryMaterialId(*mainProject, part.m_conceptualMaterialType);
            RenderMaterialPtr renderMat = JsonRenderMaterial::Create(*mainProject, matId);
            if (renderMat.IsValid())
                {
                RenderMaterialMapPtr      patternMap = renderMat->_GetMap(RENDER_MATERIAL_MAP_Pattern);
                if (patternMap.IsValid())
                    {
                    ImageBufferPtr data = patternMap->_GetImage(*mainProject);
                    textures.push_back(data);
                    }
                else textures.push_back(nullptr);
                if (mesh->GetParamCount() == 0)
                    mesh->BuildPerFaceParameters(LocalCoordinateSelect::LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft);
                }
            else textures.push_back(nullptr);
            mesh->Transform(transformToWorld);
             builder->AddPolyface(*mesh);
            result.push_back(builder->GetClientMeshPtr());
            }
        }
    }

void BuildSubResolutionPier(bvector<PolyfaceHeaderPtr>& result, bvector<ImageBufferPtr>& textures, JsonValueR resInfo, DgnElementCP sourceElem, JsonValueCR config, DRange3d nodeExt, DRange3d elemRange)
    {}

void OpenProject()
    {
    DbResult openStatus;
    BeFileName fileName = BeFileName(L"E:\\Colorado.dgndb");
    mainProject = DgnDb::OpenDgnDb(&openStatus, fileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    DPoint3d scale = DPoint3d::From(1, 1, 1);
    mainProject->Units().GetDgnGCS()->UorsFromCartesian(scale, scale);
    s_unitsTrans.InverseOf(Transform::FromScaleFactors(scale.x, scale.y, scale.z));
    CrossSectionTemplate::Scenery::GetSceneryDeckTemplateId(*mainProject, 1, 1);
    }


DPoint2d RemapUVs(const DPoint2d& source, const DPoint2d& maxUv)
    {
    DPoint2d uv = source;
    if (maxUv.x > 0)uv.x /= maxUv.x;
    if (fabs(uv.x) < 1e-5) uv.x = 0.0;
    if (fabs(uv.x - 1.0) < 1e-5) uv.x = 1.0;
    uv.x = fabs(uv.x);

    if (maxUv.y > 0)uv.y /= maxUv.y;
    if (fabs(uv.y) < 1e-5) uv.y = 0.0;
    if (fabs(uv.y - 1.0) < 1e-5) uv.y = 1.0;
    uv.y = fabs(uv.y);
    return uv;
    }

void GetIndexedMesh(bvector<DPoint3d>& pts, bvector<int32_t>& indexes,bvector<DPoint2d>& uvs, PolyfaceHeaderPtr meshP)
    {
    meshP->Triangulate();
    pts.insert(pts.end(), meshP->GetPointCP(), meshP->GetPointCP() + meshP->GetPointCount());
    PolyfaceVisitorPtr vis = PolyfaceVisitor::Attach(*meshP);
    bvector<int> &pointIndex = vis->ClientPointIndex();
    int nextPt = (int)pts.size();
    if (meshP->GetParamCount() > pts.size()) pts.resize(meshP->GetParamCount());
    if (meshP->GetParamCount() > 0) uvs.resize(pts.size());
    bvector<int> &param = vis->ClientParamIndex();
    bmap<int, int> paramsMap;
    DPoint2d uvMax = DPoint2d::From(1.0, 1.0);
    for (size_t i = 0; i < meshP->GetParamCount(); ++i)
        {
        uvMax.x = std::max(fabs(meshP->GetParamCP()[i].x), uvMax.x);
        uvMax.y = std::max(fabs(meshP->GetParamCP()[i].y), uvMax.y);
        }
    for (vis->Reset(); vis->AdvanceToNextFace();)
        {
        int32_t idx[3] = { pointIndex[0], pointIndex[1], pointIndex[2] };
        if (meshP->GetParamCount() > 0)
            {
            for (size_t i = 0; i < 3; ++i)
                if (paramsMap.count(idx[i]) > 0 && paramsMap[idx[i]] != param[i])
                    {
                    if (nextPt >= pts.size())
                        {
                        pts.resize(nextPt + 1);
                        uvs.resize(pts.size(), DPoint2d::From(0.0, 0.0));
                        }
                    pts[nextPt] = pts[idx[i]];
                    idx[i] = nextPt;
                    nextPt++;
                    }
            for (size_t i = 0; i < 3; ++i)
                paramsMap[idx[i]] = param[i];
            uvs[idx[0]] = RemapUVs(meshP->GetParamCP()[param[0]], uvMax);
            uvs[idx[1]] = RemapUVs(meshP->GetParamCP()[param[1]], uvMax);
            uvs[idx[2]] = RemapUVs(meshP->GetParamCP()[param[2]], uvMax);
            }
        indexes.push_back(idx[0] + 1);
        indexes.push_back(idx[1] + 1);
        indexes.push_back(idx[2] + 1);
        }
    pts.resize(nextPt);
    uvs.resize(pts.size(), DPoint2d::From(0.0, 0.0));
    }

void ReadTexture(bvector<uint8_t>&tex, ImageBufferPtr& imageData)
    {
    if (imageData.get() == nullptr) return;
    int nChannels;
    if (ImageBuffer::Format::Rgb == imageData->GetFormat())
        nChannels = 3;
    else
        nChannels = 4;

    int newWidth = imageData->GetWidth();
    int newHeight = imageData->GetHeight();

    tex.resize(3 * sizeof(uint32_t) + newWidth*newHeight * 3, 0xFF);

    ((uint32_t*)tex.data())[0] = newWidth;
    ((uint32_t*)tex.data())[1] = newHeight;
    ((uint32_t*)tex.data())[2] = 3;
    uint8_t* buffer = tex.data() + 3 * sizeof(uint32_t);

    for (size_t i = 0; i < newWidth; ++i)
        {
        for (size_t j = 0; j < newHeight; ++j)
            {
            const uint8_t* origPixel = imageData->GetDataCP() + j*newWidth * nChannels + i * nChannels;
            uint8_t* targetPixel = buffer + (j)*newWidth * 3 + (i) * 3;
            memcpy(targetPixel, origPixel, 3);
            }
        }
    }

bool FilterElement(bool& shouldCreateGraph, bvector<bvector<DPoint3d>>& newMeshPts, bvector<bvector<int32_t>>& newMeshIndexes, bvector<Utf8String>& newMeshMetadata, bvector<bvector<DPoint2d>>& newUvs, bvector<bvector<uint8_t>>& newTex, const bvector<ScalableMesh::IScalableMeshMeshPtr>& submeshes, const bvector<Utf8String>& meshMetadata, DRange3d nodeExt)
    {
    DgnModelPtr model;
        {
        std::lock_guard<std::mutex> lock(dgndbMutex);
        model = mainProject->Models().GetModel(mainProject->Models().QueryFirstModelId());
        model->FillModel();
        }
    bset<int64_t> processedElems;
    bset<int64_t> uniqueElems;
    for (size_t i = 0; i < submeshes.size(); ++i)
        {
        Json::Value val;
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(meshMetadata[i], val);
        if (!parsingSuccessful) continue;
        uniqueElems.insert(val["elementId"].asInt64());
        }
    for (size_t i = 0; i < submeshes.size(); ++i)
        {
        Json::Value val;
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(meshMetadata[i], val);
        if (!parsingSuccessful) continue;
        bool wasProcessed = false;
        if (processedElems.count(val["elementId"].asInt64()) > 0) wasProcessed = true;
        else processedElems.insert(val["elementId"].asInt64());
       auto elementCP = model->FindElementById(DgnElementId((uint64_t)(val["elementId"].asInt64())));
        if (elementCP == nullptr) continue;
        DgnPlatformLib::AdoptHost(libHost);
        dgndbMutex.lock();
        if (RoadSegment::QueryClassId(*mainProject) == DgnClassId(elementCP->GetElementClass()->GetId()))
            {
            dgndbMutex.unlock();
            if (wasProcessed) continue;
            bvector<PolyfaceHeaderPtr> subMeshPoly;
            bvector<ImageBufferPtr> textures;
            Json::Value result;
            result["elementId"] = val["elementId"].asInt64();

            DRange3d elemRange = DRange3d::From(submeshes[i]->GetPolyfaceQuery()->GetPointCP(), (int)submeshes[i]->GetNbPoints());

            dgndbMutex.lock();
            BuildSubResolutionRoad(subMeshPoly, textures, result, elementCP, val, nodeExt, elemRange, uniqueElems.size());
            dgndbMutex.unlock();

            bvector<DPoint3d> pts;
            bvector<int32_t> indexes;
            Utf8String metadata = Json::FastWriter().write(result);
            int n = 0;
            for (auto& componentMesh : subMeshPoly)
                {
                componentMesh->Transform(s_unitsTrans);
                bvector<DPoint2d> uvs;
                GetIndexedMesh(pts, indexes, uvs, componentMesh);
                newMeshPts.push_back(pts);
                newMeshIndexes.push_back(indexes);
                newMeshMetadata.push_back(metadata);
                newUvs.push_back(uvs);
                bvector<uint8_t> tex;
                ReadTexture(tex, textures[n]);
                newTex.push_back(tex);
                ++n;
                }
            }
        else if (Furniture::QueryClassId(*mainProject) == DgnClassId(elementCP->GetElementClass()->GetId()) && submeshes[i].IsValid() && submeshes[i]->GetNbFaces() > 0) 
            {
            dgndbMutex.unlock();
            }
        else if (Marking::QueryClassId(*mainProject) == DgnClassId(elementCP->GetElementClass()->GetId()) && submeshes[i].IsValid() && submeshes[i]->GetNbFaces() > 0)
            {
            dgndbMutex.unlock();
            }

        else if (Waterway::QueryClassId(*mainProject) == DgnClassId(elementCP->GetElementClass()->GetId()) && submeshes[i].IsValid() && submeshes[i]->GetNbFaces() > 0)
            {
            dgndbMutex.unlock();
            }

        else if (BridgeSegment::QueryClassId(*mainProject) == DgnClassId(elementCP->GetElementClass()->GetId()) && submeshes[i].IsValid() && submeshes[i]->GetNbFaces() > 0)
            {
            dgndbMutex.unlock();
            if (wasProcessed) continue;
            bvector<PolyfaceHeaderPtr> subMeshPoly;
            bvector<ImageBufferPtr> textures;
            Json::Value result;
            result["elementId"] = val["elementId"].asInt64();

            DRange3d elemRange = DRange3d::From(submeshes[i]->GetPolyfaceQuery()->GetPointCP(), (int)submeshes[i]->GetNbPoints());

            dgndbMutex.lock();
            BuildSubResolutionBridge(subMeshPoly, textures, result, elementCP, val, nodeExt, elemRange, uniqueElems.size());
            dgndbMutex.unlock();

            bvector<DPoint3d> pts;
            bvector<int32_t> indexes;
            Utf8String metadata = Json::FastWriter().write(result);
            int n = 0;
            for (auto& componentMesh : subMeshPoly)
                {
                componentMesh->Transform(s_unitsTrans);
                bvector<DPoint2d> uvs;
                GetIndexedMesh(pts, indexes, uvs, componentMesh);
                newMeshPts.push_back(pts);
                newMeshIndexes.push_back(indexes);
                newMeshMetadata.push_back(metadata);
                newUvs.push_back(uvs);
                bvector<uint8_t> tex;
                ReadTexture(tex, textures[n]);
                newTex.push_back(tex);
                ++n;
                }
            }

        else if (BridgePier::QueryClassId(*mainProject) == DgnClassId(elementCP->GetElementClass()->GetId()) && submeshes[i].IsValid() && submeshes[i]->GetNbFaces() > 0)
            {
            dgndbMutex.unlock();
            if (wasProcessed) continue;
            bvector<PolyfaceHeaderPtr> subMeshPoly;
            bvector<ImageBufferPtr> textures;
            Json::Value result;
            result["elementId"] = val["elementId"].asInt64();

            DRange3d elemRange = DRange3d::From(submeshes[i]->GetPolyfaceQuery()->GetPointCP(), (int)submeshes[i]->GetNbPoints());

            dgndbMutex.lock();
            BuildSubResolutionPier(subMeshPoly, textures, result, elementCP, val, nodeExt, elemRange);
            dgndbMutex.unlock();

            bvector<DPoint3d> pts;
            bvector<int32_t> indexes;
            Utf8String metadata = Json::FastWriter().write(result);
            int n = 0;
            for (auto& componentMesh : subMeshPoly)
                {
                componentMesh->Transform(s_unitsTrans);
                bvector<DPoint2d> uvs;
                GetIndexedMesh(pts, indexes, uvs, componentMesh);
                newMeshPts.push_back(pts);
                newMeshIndexes.push_back(indexes);
                newMeshMetadata.push_back(metadata);
                newUvs.push_back(uvs);
                bvector<uint8_t> tex;
                ReadTexture(tex, textures[n]);
                newTex.push_back(tex);
                ++n;
                }
            }
        else /*if (IntersectionSegment::QueryClassId(*mainProject) == DgnClassId(elementCP->GetElementClass()->GetId()) && submeshes[i].IsValid() && submeshes[i]->GetNbFaces() > 0)*/
            {
            dgndbMutex.unlock();
            DRange3d elemRange = DRange3d::From(submeshes[i]->GetPolyfaceQuery()->GetPointCP(), (int)submeshes[i]->GetNbPoints());
            double fraction = std::min(elemRange.XLength() / nodeExt.XLength(), elemRange.YLength() / nodeExt.YLength());
            if ((int)submeshes[i]->GetNbPoints() > 300 && fraction < 0.05) continue;
            Json::Value result;
            result["elementId"] = val["elementId"].asInt64();
            Utf8String metadata = Json::FastWriter().write(result);
            newMeshMetadata.push_back(metadata);
            bvector<DPoint3d> pts(submeshes[i]->GetNbPoints());
            bvector<int32_t> indexes(submeshes[i]->GetNbFaces()*3);
            memcpy(&pts[0], submeshes[i]->GetPolyfaceQuery()->GetPointCP(), submeshes[i]->GetNbPoints()*sizeof(DPoint3d));
            memcpy(&indexes[0], submeshes[i]->GetPolyfaceQuery()->GetPointIndexCP(), submeshes[i]->GetPolyfaceQuery()->GetPointIndexCount() *sizeof(int32_t));
            newMeshPts.push_back(pts);
            newMeshIndexes.push_back(indexes);
            newUvs.push_back(bvector<DPoint2d>());
            newTex.push_back(bvector<uint8_t>());
            }
        DgnPlatformLib::ForgetHost();
        }
    shouldCreateGraph = false;
    return true;
    }


void ExportRoadsToFile(const char* fileName)
    {
    FILE* f = fopen(fileName, "wb");
    auto model = mainProject->Models().GetModel(mainProject->Models().QueryFirstModelId());
    model->FillModel();
    for (auto element = model->begin(); element != model->end(); element++)
        {
        DgnElementCPtr elem = element->second;
        if (nullptr != elem->ToGeometrySource3d() && (RoadSegment::QueryClassId(*mainProject) == DgnClassId(elem->GetElementClass()->GetId()) || IntersectionSegment::QueryClassId(*mainProject) == DgnClassId(elem->GetElementClass()->GetId())))
            {
            Transform transformToWorld;
            Placement3dCR placement = elem->ToGeometrySource3d()->GetPlacement();
            transformToWorld = placement.GetTransform();
            bvector<int32_t> indices;
            bvector<DPoint3d> pts;
            ElementGeometryCollection collection(*mainProject, dynamic_cast<const GeometricElement3d*>(elem.get())->GetGeomStream());
            size_t offset = 0;
            for (ElementGeometryPtr geometry : collection)
                {
                geometry->TransformInPlace(transformToWorld);
                PolyfaceHeaderPtr mesh = geometry->GetAsPolyfaceHeader();
                if (!mesh.IsValid()) continue;

                mesh->Triangulate();
                pts.insert(pts.end(), mesh->GetPointCP(), mesh->GetPointCP() + mesh->GetPointCount());
                PolyfaceVisitorPtr vis = PolyfaceVisitor::Attach(*mesh);
                bvector<int> &pointIndex = vis->ClientPointIndex();
                for (vis->Reset(); vis->AdvanceToNextFace();)
                    {
                    indices.push_back(pointIndex[0] + 1 + (int)offset);
                    indices.push_back(pointIndex[1] + 1 + (int)offset);
                    indices.push_back(pointIndex[2] + 1 + (int)offset);
                    }
                offset = pts.size();
                }
            fwrite(&offset, sizeof(size_t), 1, f);
            if (pts.size() > 0)
                fwrite(&pts[0], sizeof(DPoint3d), offset, f);
            size_t nIndices = indices.size();
            fwrite(&nIndices, sizeof(size_t), 1, f);
            if (indices.size() > 0)
                fwrite(&indices[0], sizeof(int32_t), nIndices, f);
            }
        }

    fclose(f);
    }


int wmain(int argc, wchar_t* argv[])
{
_set_error_mode(_OUT_TO_MSGBOX);

DgnPlatformLib::Initialize(libHost, false);
struct  SMHost : ScalableMesh::ScalableMeshLib::Host
    {
    SMHost()
        {}
    ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
        {
        return *new ScalableMesh::ScalableMeshAdmin();
        };
    };
    ScalableMesh::ScalableMeshLib::Initialize(*new SMHost());
    BeFileName geocoordinateDataPath(L".\\GeoCoordinateData\\");
    GeoCoordinates::BaseGCS::Initialize(geocoordinateDataPath.c_str());
    DgnDomains::RegisterDomain(ConceptualDomain::GetDomain());

    //create a scalable mesh
    StatusInt createStatus;
    //BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMesh::SetUserFilterCallback(&FilterElement);
    BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr creatorPtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(L"e:\\output\\test_dgndb_colorado.stm", createStatus));

    //BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshPtr creatorPtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMesh::GetFor(L"e:\\output\\coloradoDesign.stm", true, true, createStatus));
    if (!mainProject.IsValid()) OpenProject();
    if (creatorPtr == 0)
        {
        printf("ERROR : cannot create STM file\r\n");
        }
    BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMSourcePtr srcPtr = BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMLocalFileSource::Create(BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_DATA_MESH, L"E:\\Colorado.dgndb");
    creatorPtr->EditSources().Add(srcPtr);
    creatorPtr->SetUserFilterCallback(&FilterElement);
    //creatorPtr->ReFilter();
    creatorPtr->Create(true);
    creatorPtr->SaveToFile();
    creatorPtr = nullptr;

    std::cout << "THE END" << std::endl;
    return 0;
}