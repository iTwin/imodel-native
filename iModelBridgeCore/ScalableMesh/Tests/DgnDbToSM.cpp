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

void BuildSubResolutionRoad(PolyfaceHeaderPtr& result, JsonValueR resInfo, DgnElementCP sourceElem, JsonValueCR config, DRange3d nodeExt, DRange3d elemRange)
    {
    RoadSegmentCP roadSegment = (RoadSegmentCP)sourceElem;

    double fraction = /*std::max(*/std::min(elemRange.XLength() / nodeExt.XLength(), elemRange.YLength() / nodeExt.YLength())/*, elemRange.ZLength() / nodeExt.ZLength())*/;

    double distance = 15.0 + (2 * 1/fraction);

    RoadSegmentInfoPtr info = RoadSegmentInfo::Create(*roadSegment);
    
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
    IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::New(*options);


    for (auto const& polyface : res.GetMeshedComponent())
        builder->AddPolyface(*polyface.first);

    for (auto const& polyface : res.GetMeshedEndConditions())
        builder->AddPolyface(*polyface.first);

    result = builder->GetClientMeshPtr();
    }

void OpenProject()
    {
    DbResult openStatus;
    BeFileName fileName = BeFileName(L"E:\\HRGreen_Demo.dgndb");
    mainProject = DgnDb::OpenDgnDb(&openStatus, fileName, DgnDb::OpenParams(Db::OpenMode::Readonly));
    DPoint3d scale = DPoint3d::From(1, 1, 1);
    mainProject->Units().GetDgnGCS()->UorsFromCartesian(scale, scale);
    s_unitsTrans.InverseOf(Transform::FromScaleFactors(scale.x, scale.y, scale.z));
    }

void GetIndexedMesh(bvector<DPoint3d>& pts, bvector<int32_t>& indexes, PolyfaceHeaderPtr meshP)
    {
    meshP->Triangulate();
    pts.insert(pts.end(), meshP->GetPointCP(), meshP->GetPointCP() + meshP->GetPointCount());
    PolyfaceVisitorPtr vis = PolyfaceVisitor::Attach(*meshP);
    bvector<int> &pointIndex = vis->ClientPointIndex();
    for (vis->Reset(); vis->AdvanceToNextFace();)
        {
        indexes.push_back(pointIndex[0] + 1);
        indexes.push_back(pointIndex[1] + 1);
        indexes.push_back(pointIndex[2] + 1);
        }
    }

bool FilterElement(bool& shouldCreateGraph, bvector<bvector<DPoint3d>>& newMeshPts, bvector<bvector<int32_t>>& newMeshIndexes, bvector<Utf8String>& newMeshMetadata, const bvector<ScalableMesh::IScalableMeshMeshPtr>& submeshes, const bvector<Utf8String>& meshMetadata, DRange3d nodeExt)
    {
    DgnModelPtr model;
        {
        std::lock_guard<std::mutex> lock(dgndbMutex);
        model = mainProject->Models().GetModel(mainProject->Models().QueryFirstModelId());
        model->FillModel();
        }
    for (size_t i = 0; i < submeshes.size(); ++i)
        {
        Json::Value val;
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(meshMetadata[i], val);
        if (!parsingSuccessful) continue;
       auto elementCP = model->FindElementById(DgnElementId((uint64_t)(val["elementId"].asInt64())));
        if (elementCP == nullptr) continue;
        if (RoadSegment::QueryClassId(*mainProject) == DgnClassId(elementCP->GetElementClass()->GetId()))
            {
            PolyfaceHeaderPtr subMeshPoly;
            Json::Value result;
            result["elementId"] = val["elementId"].asInt64();

            DRange3d elemRange = DRange3d::From(submeshes[i]->GetPolyfaceQuery()->GetPointCP(), (int)submeshes[i]->GetNbPoints());

            dgndbMutex.lock();
            BuildSubResolutionRoad(subMeshPoly,result, elementCP, val, nodeExt, elemRange);
            subMeshPoly->Transform(s_unitsTrans);
            dgndbMutex.unlock();

            bvector<DPoint3d> pts;
            bvector<int32_t> indexes;
            Utf8String metadata = Json::FastWriter().write(result);
            GetIndexedMesh(pts, indexes, subMeshPoly);
            newMeshPts.push_back(pts);
            newMeshIndexes.push_back(indexes);
            newMeshMetadata.push_back(metadata);
            }
        else /*if (IntersectionSegment::QueryClassId(*mainProject) == DgnClassId(elementCP->GetElementClass()->GetId()) && submeshes[i].IsValid() && submeshes[i]->GetNbFaces() > 0)*/
            {
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
            }

        }
    shouldCreateGraph = false;
    return true;
    }

AppHost libHost;
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
    BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreatorPtr creatorPtr(BENTLEY_NAMESPACE_NAME::ScalableMesh::IScalableMeshSourceCreator::GetFor(L"e:\\output\\test_dgndb_lod_hr.stm", createStatus));
    if (!mainProject.IsValid()) OpenProject();
    if (creatorPtr == 0)
        {
        printf("ERROR : cannot create STM file\r\n");
        }
    BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMSourcePtr srcPtr = BENTLEY_NAMESPACE_NAME::ScalableMesh::IDTMLocalFileSource::Create(BENTLEY_NAMESPACE_NAME::ScalableMesh::DTM_SOURCE_DATA_MESH, L"E:\\HRGreen_Demo.dgndb");
    creatorPtr->EditSources().Add(srcPtr);
    creatorPtr->SetUserFilterCallback(&FilterElement);
    creatorPtr->Create(true);
    creatorPtr->SaveToFile();
    creatorPtr = nullptr;

    std::cout << "THE END" << std::endl;
    return 0;
}