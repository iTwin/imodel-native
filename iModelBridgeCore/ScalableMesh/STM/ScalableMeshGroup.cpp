#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h" 

#include "InternalUtilityFunctions.h"
#include "ScalableMesh.h"
#include "ScalableMeshQuadTreeBCLIBFilters.h"
#include "ScalableMeshQuadTreeQueries.h"

#include <ScalableMesh/GeoCoords/GCS.h>
#include <STMInternal/GeoCoords/WKTUtils.h>
#include <ScalableMesh/GeoCoords/Reprojection.h>


#include "ScalableMeshQuery.h"
#include "ScalableMeshSourcesPersistance.h"

#include <ScalableMesh/IScalableMeshPolicy.h>
#include <ScalableMesh\IScalableMeshSourceCollection.h>
#include <ScalableMesh\IScalableMeshDocumentEnv.h>
#include <ScalableMesh\IScalableMeshGroundExtractor.h>
#include <ScalableMesh\IScalableMeshSourceImportConfig.h>
#include <ScalableMesh\IScalableMeshSources.h>

#ifndef LINUX_SCALABLEMESH_BUILD
#include <CloudDataSource/DataSourceManager.h>
#endif

#include "ScalableMeshDraping.h"
#include "ScalableMeshVolume.h"

#include "Edits\ClipRegistry.h"
#include "Stores\SMStreamingDataStore.h"

#include <Vu\VuApi.h>
#include <Vu\vupoly.fdf>
#include "vuPolygonClassifier.h"
#include <ImagePP\all\h\HIMMosaic.h>
#include "LogUtils.h"
#include "ScalableMeshEdit.h"
#include <ScalableMesh/ScalableMeshLib.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
#include "MosaicTextureProvider.h"
#include "ScalableMeshGroup.h"



using namespace ISMStore;
USING_NAMESPACE_IMAGEPP
USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT
USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

int64_t          ScalableMeshGroup::_GetPointCount()
    {
    int64_t ptCount = 0;

    for (auto& member : m_members)
        ptCount += member->GetPointCount();
    return ptCount;
    }

uint64_t          ScalableMeshGroup::_GetNodeCount()
    {
    uint64_t nodeCount = 0;

    for (auto& member : m_members)
        nodeCount += member->GetNodeCount();
    return nodeCount;
    }

bool          ScalableMeshGroup::_IsTerrain()
    {
    for (auto& member : m_members)
        if (member->IsTerrain()) return true;

    return false;
    }

bool          ScalableMeshGroup::_IsTextured()
    {
    for (auto& member : m_members)
        if (member->IsTextured()) return true;

    return false;
    }

StatusInt ScalableMeshGroup::_GetTextureInfo(IScalableMeshTextureInfoPtr& textureInfo) const
    {
    return ERROR;
    }

BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  ScalableMeshGroup::_GetDTMInterface(DTMAnalysisType type)
    {
    if (!m_smGroupsDTM[type].IsValid())
        {
        for (int i = 0; i < (int)DTMAnalysisType::Qty; ++i)
            {
            m_smGroupsDTM[i] = ScalableMeshGroupDTM::Create(this);
            m_smGroupsDTM[i]->SetAnalysisType((DTMAnalysisType)i);
            }
        }
    return m_smGroupsDTM[type].get();
    }

BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*  ScalableMeshGroup::_GetDTMInterface(DMatrix4d& storageToUors, DTMAnalysisType type)
    {
    if (!m_smGroupsDTM[type].IsValid())
        {
        for (int i = 0; i < (int)DTMAnalysisType::Qty; ++i)
            {
            m_smGroupsDTM[i] = ScalableMeshGroupDTM::Create(this);
            m_smGroupsDTM[i]->SetAnalysisType((DTMAnalysisType)i);
            }
        }
    return m_smGroupsDTM[type].get();
    }

DTMStatusInt     ScalableMeshGroup::_GetRange(DRange3dR range)
    {

    DRange3d extRange = DRange3d::NullRange();

    for (auto& member : m_members)
        {
        DRange3d memberRange;
        member->GetRange(memberRange);
        extRange.Extend(memberRange);
        }

    range = extRange;
    return DTM_SUCCESS;
    }

StatusInt         ScalableMeshGroup::_GetBoundary(bvector<DPoint3d>& boundary)
    {
    if (m_members.empty()) return ERROR;

    return m_members.front()->GetBoundary(boundary);
    }


// Inherited from IMRDTM                   
Count                  ScalableMeshGroup::_GetCountInRange(const DRange2d& range, const CountType& type, const uint64_t& maxNumberCountedPoints) const
    {
    if (m_members.empty()) return Count(0, 0);

    return m_members.front()->GetCountInRange(range, type, maxNumberCountedPoints);
    }

int64_t                ScalableMeshGroup::_GetBreaklineCount() const
    {
    return 0;
    }

ScalableMeshCompressionType   ScalableMeshGroup::_GetCompressionType() const
    {
    return ScalableMeshCompressionType::SCM_COMPRESSION_DEFLATE;
    }

int                    ScalableMeshGroup::_GetNbResolutions() const
    {
    if (m_members.empty()) return 0;

    return m_members.front()->GetNbResolutions();
    }

size_t                    ScalableMeshGroup::_GetTerrainDepth() const
    {
    size_t maxDepth = 0;
    for (auto& member : m_members)
        {
        if (member->IsTerrain()) maxDepth += member->GetTerrainDepth();
        }

    return maxDepth;

    }

const GeoCoords::GCS&  ScalableMeshGroup::_GetGCS() const
    {

    return m_members.front()->GetGCS();
    }

ScalableMeshState             ScalableMeshGroup::_GetState() const
    {
    return SCM_STATE_UP_TO_DATE;
    }

bool                   ScalableMeshGroup::_IsReadOnly() const
    {
    return m_members.front()->IsReadOnly();
    }

bool                   ScalableMeshGroup::_IsShareable() const
    {
    return m_members.front()->IsShareable();
    }

int                    ScalableMeshGroup::_GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSCPtr& targetGCS) const
    {
    return ERROR;
    }


void                               ScalableMeshGroup::_RegenerateClips(bool forceRegenerate)
{
}
uint64_t                           ScalableMeshGroup::_AddClip(const DPoint3d* pts, size_t ptsSize)
    {
    return 0;
    }

bool                               ScalableMeshGroup::_ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID)
    {
    auto clipPrim = ICurvePrimitive::CreateLineString(pts, ptsSize);
    CurveVectorPtr clipVecPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, clipPrim);

    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (m_regions[i].hasRestrictedRegion)
            {
            auto curvePtr = ICurvePrimitive::CreateLineString(m_regions[i].region);
            CurveVectorPtr curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);

            CurveVectorPtr intersection = CurveVector::AreaIntersection(*curveVectorPtr, *clipVecPtr);


#ifndef VANCOUVER_API
            if (!intersection.IsNull())
                {
                bvector<bvector<DPoint3d>> loops;
                if (intersection->CollectLinearGeometry(loops) && !loops.empty())
                    if (m_members[i]->ModifyClip(loops.front().data(), loops.front().size(), clipID)) return true;
                }
#else
            assert(!"Reactivate on Vancouver");
#endif
            }
        }

    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (!m_regions[i].hasRestrictedRegion && m_members[i]->IsTerrain())
            {
            if (m_members[i]->ModifyClip(pts, ptsSize, clipID)) return true;
            }
        }

    return false;
    }

bool                               ScalableMeshGroup::_AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, bool alsoAddOnTerrain)
    {
    if (ptsSize == 0) return false;
    auto clipPrim = ICurvePrimitive::CreateLineString(pts, ptsSize);
    CurveVectorPtr clipVecPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, clipPrim);

    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (m_regions[i].hasRestrictedRegion)
            {
            auto curvePtr = ICurvePrimitive::CreateLineString(m_regions[i].region);
            CurveVectorPtr curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);
            
            CurveVectorPtr intersection = CurveVector::AreaIntersection(*curveVectorPtr, *clipVecPtr);
            if (!intersection.IsNull() && curveVectorPtr->PointInOnOutXY(pts[0]) != CurveVector::InOutClassification::INOUT_Out)
                {
#ifndef VANCOUVER_API
                bvector<bvector<DPoint3d>> loops;
                if(intersection->CollectLinearGeometry(loops) && !loops.empty())
                    if (m_members[i]->AddClip(loops.front().data(), loops.front().size(), clipID)) return true;
#else
                assert(!"Reactivate on Vancouver");
#endif
                }
            }
        }

    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (!m_regions[i].hasRestrictedRegion && m_members[i]->IsTerrain())
            {
            if (m_members[i]->AddClip(pts, ptsSize, clipID)) return true;
            }
        }

    return false;
    }

 void                               ScalableMeshGroup::_SetClipOnOrOff(uint64_t id, bool isActive)
    {
    for (auto& member : m_members)
        member->SetClipOnOrOff(id, isActive);
    }
 void                               ScalableMeshGroup::_GetIsClipActive(uint64_t id, bool& isActive)
    {
    for (auto& member : m_members)
        member->GetIsClipActive(id, isActive);
    }

 void                               ScalableMeshGroup::_GetClipType(uint64_t id, SMNonDestructiveClipType& type)
    {
    assert(false);
    }

 bool                               ScalableMeshGroup::_AddClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
     {
     auto clipPrim = ICurvePrimitive::CreateLineString(pts, ptsSize);
     CurveVectorPtr clipVecPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, clipPrim);

     for (size_t i = 0; i < m_members.size(); ++i)
         {
         if (m_regions[i].hasRestrictedRegion)
             {
             auto curvePtr = ICurvePrimitive::CreateLineString(m_regions[i].region);
             CurveVectorPtr curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);

             CurveVectorPtr intersection = CurveVector::AreaIntersection(*curveVectorPtr, *clipVecPtr);
             if (!intersection.IsNull() && curveVectorPtr->PointInOnOutXY(pts[0]) != CurveVector::InOutClassification::INOUT_Out)
                 {
#ifndef VANCOUVER_API
                 bvector<bvector<DPoint3d>> loops;
                 if (intersection->CollectLinearGeometry(loops) && !loops.empty())
                     if (m_members[i]->AddClip(loops.front().data(), loops.front().size(), clipID, geom, type, isActive)) return true;
#else
                 assert(!"Reactivate on Vancouver");
#endif
                 }
             }
         }

     for (size_t i = 0; i < m_members.size(); ++i)
         {
         if (!m_regions[i].hasRestrictedRegion && m_members[i]->IsTerrain())
             {
             if (m_members[i]->AddClip(pts, ptsSize, clipID, geom, type, isActive)) return true;
             }
         }

     return false;
     }

 bool                               ScalableMeshGroup::_ModifyClip(const DPoint3d* pts, size_t ptsSize, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
     {
     auto clipPrim = ICurvePrimitive::CreateLineString(pts, ptsSize);
     CurveVectorPtr clipVecPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, clipPrim);

     for (size_t i = 0; i < m_members.size(); ++i)
         {
         if (m_regions[i].hasRestrictedRegion)
             {
             auto curvePtr = ICurvePrimitive::CreateLineString(m_regions[i].region);
             CurveVectorPtr curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);

             CurveVectorPtr intersection = CurveVector::AreaIntersection(*curveVectorPtr, *clipVecPtr);
             if (!intersection.IsNull() && curveVectorPtr->PointInOnOutXY(pts[0]) != CurveVector::InOutClassification::INOUT_Out)
                 {
#ifndef VANCOUVER_API
                 bvector<bvector<DPoint3d>> loops;
                 if (intersection->CollectLinearGeometry(loops) && !loops.empty())
                     if (m_members[i]->ModifyClip(loops.front().data(), loops.front().size(), clipID, geom, type, isActive)) return true;
#else
                 assert(!"Reactivate on Vancouver");
#endif
                 }
             }
         }

     for (size_t i = 0; i < m_members.size(); ++i)
         {
         if (!m_regions[i].hasRestrictedRegion && m_members[i]->IsTerrain())
             {
             if (m_members[i]->ModifyClip(pts, ptsSize, clipID, geom, type, isActive)) return true;
             }
         }

     return false;
     }


 bool                               ScalableMeshGroup::_AddClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
 {


     return false;
 }

 bool                               ScalableMeshGroup::_ModifyClip(const ClipVectorPtr& clip, uint64_t clipID, SMClipGeometryType geom, SMNonDestructiveClipType type, bool isActive)
 {


     return false;
 }

bool                               ScalableMeshGroup::_RemoveClip(uint64_t clipID)
    {
    for (auto& member : m_members)
        member->RemoveClip(clipID);
    return true;
    }

bool                               ScalableMeshGroup::_GetClip(uint64_t clipID, bvector<DPoint3d>& clipData)
{
    for (auto& member : m_members)
        if(member->GetClip(clipID, clipData))
            return true;

    return false;
}

bool                               ScalableMeshGroup::_IsInsertingClips()
    {
    bool isInsertingClips = false;
    for (auto& member : m_members)
        {
        bool memberIsInsertingClips = member->IsInsertingClips();
        BeAssert(!isInsertingClips || memberIsInsertingClips); // If one member is inserting clips, all of them should be adding clips
        isInsertingClips = memberIsInsertingClips;
        }
    return isInsertingClips;
    }

void                               ScalableMeshGroup::_SetIsInsertingClips(bool toggleInsertMode)
    {
    for (auto& member : m_members)
        member->SetIsInsertingClips(toggleInsertMode);
    }


void                               ScalableMeshGroup::_ModifyClipMetadata(uint64_t clipId, double importance, int nDimensions)
    {
    for (auto& member : m_members)
        member->ModifyClipMetadata(clipId, importance, nDimensions);
    }

void                               ScalableMeshGroup::_GetAllClipsIds(bvector<uint64_t>& allClipIds)
    {
    for (auto& member : m_members)
        {
        bvector<uint64_t> tempIds;
        member->GetAllClipIds(tempIds);
        allClipIds.insert(allClipIds.end(), tempIds.begin(), tempIds.end());
        }
    }

void                               ScalableMeshGroup::_SynchronizeClipData(const bvector<bpair<uint64_t, bvector<DPoint3d>>>& listOfClips, const bvector<bpair<uint64_t, bvector<bvector<DPoint3d>>>>& listOfSkirts)
    {

    }

bool                               ScalableMeshGroup::_ModifySkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID)
    {
    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (m_regions[i].hasRestrictedRegion)
            {
            //bool allPointsInRegion = true;
            auto curvePtr = ICurvePrimitive::CreateLineString(m_regions[i].region);
            CurveVectorPtr curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);
           /* for (auto& lineString : skirt)
                {
                for (auto& pt : lineString)
                    if (curveVectorPtr->PointInOnOutXY(pt) == CurveVector::InOutClassification::INOUT_Out)
                        {
                        allPointsInRegion = false;
                        break;
                        }
                if (!allPointsInRegion) break;
                }*/
            if (curveVectorPtr->PointInOnOutXY(skirt[0][0]) != CurveVector::InOutClassification::INOUT_Out)
                {
                return (m_members[i]->ModifySkirt(skirt, skirtID));
                }
            }
        }

    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (!m_regions[i].hasRestrictedRegion)
            {
            if (m_members[i]->ModifySkirt(skirt, skirtID)) return true;
            }
        }
    return false;
    }

bool                               ScalableMeshGroup::_AddSkirt(const bvector<bvector<DPoint3d>>& skirt, uint64_t skirtID, bool alsoAddOnTerrain)
    {
    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (m_regions[i].hasRestrictedRegion)
            {
            //bool allPointsInRegion = true;
            auto curvePtr = ICurvePrimitive::CreateLineString(m_regions[i].region);
            CurveVectorPtr curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);
          /*  for (auto& lineString : skirt)
                {
                for (auto& pt : lineString)
                    if (curveVectorPtr->PointInOnOutXY(pt) == CurveVector::InOutClassification::INOUT_Out)
                        {
                        allPointsInRegion = false;
                        break;
                        }
                if (!allPointsInRegion) break;
                }*/
            if (curveVectorPtr->PointInOnOutXY(skirt[0][0]) != CurveVector::InOutClassification::INOUT_Out)
                {
               return (m_members[i]->AddSkirt(skirt, skirtID));
                }
            }
        }

    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (!m_regions[i].hasRestrictedRegion)
            {
            if (m_members[i]->AddSkirt(skirt, skirtID)) return true;
            }
        }
    return false;
    }

bool                               ScalableMeshGroup::_GetSkirt(uint64_t skirtID, bvector<bvector<DPoint3d>>& skirt)
    {
    for (auto& member : m_members)
        {
        if (member->GetSkirt(skirtID, skirt)) break;
        }
    return !skirt.empty();
    }

bool                               ScalableMeshGroup::_RemoveSkirt(uint64_t skirtID)
    {
    for (auto& member : m_members)
        member->RemoveSkirt(skirtID);
    return true;
    }


BentleyStatus                      ScalableMeshGroup::_SetReprojection(GeoCoordinates::BaseGCSCR targetCS, TransformCR approximateTransform)
    {
    return ERROR;
    }

#ifdef VANCOUVER_API
BentleyStatus                      ScalableMeshGroup::_Reproject(GeoCoordinates::BaseGCSCP targetCS, DgnModelRefP dgnModel)
    {
    return ERROR;
    }
#else
BentleyStatus                      ScalableMeshGroup::_Reproject(DgnGCSCP targetCS, DgnDbR dgnProject)
    {
    return ERROR;
    }

#endif

Transform                          ScalableMeshGroup::_GetReprojectionTransform() const
    {
    return m_members.front()->GetReprojectionTransform();
    }

SMStatus                      ScalableMeshGroup::_DetectGroundForRegion(BeFileName& createdTerrain, const BeFileName& coverageTempDataFolder, const bvector<DPoint3d>& coverageData, uint64_t id, IScalableMeshGroundPreviewerPtr groundPreviewer, BaseGCSCPtr& destinationGcs, bool limitResolution)
    {
    for (auto& member : m_members)
        {
        if (!member->IsTerrain()) return member->DetectGroundForRegion(createdTerrain, coverageTempDataFolder, coverageData, id, groundPreviewer, destinationGcs, limitResolution);
        }

    return SMStatus::S_SUCCESS;
    }

BentleyStatus                   ScalableMeshGroup::_CreateCoverage(const bvector<DPoint3d>& coverageData, uint64_t id, const Utf8String& coverageName)
    {
    for (auto& member : m_members)
        {
        if (!member->IsTerrain()) return member->CreateCoverage(coverageData, id, coverageName);
        }

    return SUCCESS;
    }

void                           ScalableMeshGroup::_GetAllCoverages(bvector<bvector<DPoint3d>>& coverageData)
    {
    for (auto& member : m_members)
        {
        if (!member->IsTerrain()) return member->GetAllCoverages(coverageData);
        }
    }

void                               ScalableMeshGroup::_GetCoverageIds(bvector<uint64_t>& ids) const
    {
    for (auto& member : m_members)
        {
        if (!member->IsTerrain()) return member->GetCoverageIds(ids);
        }
    }

void                               ScalableMeshGroup::_GetCoverageName(Utf8String& name, uint64_t id) const
    {
    assert(!"Should not be called");    
    }

BentleyStatus                      ScalableMeshGroup::_DeleteCoverage(uint64_t id)
    {
    for (auto& member : m_members)
        {
        if (!member->IsTerrain()) return member->DeleteCoverage(id);
        }

    return SUCCESS;
    } 

#ifdef WIP_MESH_IMPORT

void ScalableMeshGroup::_GetAllTextures(bvector<IScalableMeshTexturePtr>& textures)
    {
    assert(!"Not yet implemented");
    }
#endif

void ScalableMeshGroup::_SetGroupSelectionFromPoint(DPoint3d firstPoint)
{
    for (size_t i = 0; i < m_members.size(); ++i)
    {
        if (m_regions[i].hasRestrictedRegion)
        {
            if (IsWithinRegion(m_members[i], &firstPoint, 1))
                return SelectMember(m_members[i]);
        }
    }

    for (size_t i = 0; i < m_members.size(); ++i)
    {
        if (!m_regions[i].hasRestrictedRegion)
        {
            return SelectMember(m_members[i]);
        }
    }
}

void ScalableMeshGroup::_ClearGroupSelection()
    {
    ClearSelection();
    }

bool ScalableMeshGroup::IsRegionRestricted(IScalableMesh* sMesh)
    {
    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (m_members[i] == sMesh && m_regions[i].hasRestrictedRegion) return true;
        }
    return false;
    }

bool ScalableMeshGroup::IsWithinRegion(IScalableMesh* sMesh,const DPoint3d* pts, size_t nOfPts)
    {
    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (m_members[i] == sMesh && m_regions[i].hasRestrictedRegion)
            {
            bool allPointsInRegion = true;
            auto curvePtr = ICurvePrimitive::CreateLineString(m_regions[i].region);
            CurveVectorPtr curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);
            for (size_t j = 0; j < nOfPts; ++j)
                {
                DPoint3d pt = pts[j];
                if (curveVectorPtr->PointInOnOutXY(pt) == CurveVector::InOutClassification::INOUT_Out)
                    {
                    allPointsInRegion = false;
                    break;
                    }
                }
                
            if (allPointsInRegion) return true;
            }
        }
    return false;
    }

void ScalableMeshGroup::AddMember(IScalableMeshPtr& sMesh, bool isRegionRestricted, const DPoint3d* region, size_t nOfPtsInRegion)
    {
    RegionInfo reg;
    reg.hasRestrictedRegion = isRegionRestricted;
    if (isRegionRestricted)
        {
        reg.region.resize(nOfPtsInRegion);
        reg.region.assign(region, region + nOfPtsInRegion);
        }

    m_members.push_back(sMesh.get());
    m_regions.push_back(reg);
    }

void ScalableMeshGroup::RemoveMember(IScalableMeshPtr& sMesh)
    {
    auto SMPos = std::find(m_members.begin(), m_members.end(), sMesh.get());
    if (SMPos == m_members.end()) return;

    size_t idx = std::distance(m_members.begin(), SMPos);
    m_members.erase(SMPos);

    m_regions.erase(m_regions.begin() + idx);
    }

void ScalableMeshGroup::SelectMember(IScalableMesh* sMesh)
{
    m_selection = sMesh;
    bvector<CurveVectorPtr> listOfRestrictions;
    for (size_t i = 0; i < m_members.size(); ++i)
    {
        if (m_members[i] == sMesh && m_regions[i].hasRestrictedRegion)
        {
            auto curvePtr = ICurvePrimitive::CreateLineString(m_regions[i].region);
            CurveVectorPtr curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, curvePtr);
            listOfRestrictions.push_back(curveVectorPtr);
        }
    }

    if (listOfRestrictions.empty())
    {
        for (size_t i = 0; i < m_members.size(); ++i)
        {
            if (m_regions[i].hasRestrictedRegion)
            {
                auto curvePtr = ICurvePrimitive::CreateLineString(m_regions[i].region);
                CurveVectorPtr curveVectorPtr = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Inner, curvePtr);
                listOfRestrictions.push_back(curveVectorPtr);
            }
        }
    }

    for (int i = 0; i < (int)DTMAnalysisType::Qty; ++i)
    {
        ScalableMeshDraping* draping = (ScalableMeshDraping*)sMesh->GetDTMInterface((DTMAnalysisType)i)->GetDTMDraping();
        draping->SetRegionRestrictions(listOfRestrictions);
    }
}

void ScalableMeshGroup::ClearSelection()
{
    if (m_selection != nullptr)
    {
        for (int i = 0; i < (int)DTMAnalysisType::Qty; ++i)
        {
            ScalableMeshDraping* draping = (ScalableMeshDraping*)m_selection->GetDTMInterface((DTMAnalysisType)i)->GetDTMDraping();
            draping->ClearRegionRestrictions();
        }
    }
    m_selection = nullptr;
}


IScalableMesh* ScalableMeshGroup::GetSelection()
{
    return m_selection;
}

ScalableMeshGroup::ScalableMeshGroup()
    {
    m_selection = nullptr;
}


ScalableMeshGroupDTM::ScalableMeshGroupDTM(IScalableMesh* scMesh)
    {
    if (dynamic_cast<ScalableMeshGroup*>(scMesh) != nullptr)
        m_group = dynamic_cast<ScalableMeshGroup*>(scMesh);
    }

void ScalableMeshGroupDTM::SetStorageToUors(DMatrix4d& storageToUors)
    {
    m_transformToUors.InitFrom(storageToUors);
    }

/*-------------------Methods inherited from IDTM-----------------------------*/
int64_t ScalableMeshGroupDTM::_GetPointCount()
    {
    return m_group->GetPointCount();
    }

BcDTMP ScalableMeshGroupDTM::_GetBcDTM()
    {
    return 0;
    };

DTMStatusInt ScalableMeshGroupDTM::_GetBoundary(DTMPointArray& result)
    {
    return m_group->GetBoundary(result) == SUCCESS ? DTM_SUCCESS : DTM_ERROR;
    }

IDTMDrapingP ScalableMeshGroupDTM::_GetDTMDraping()
    {
    return this;
    }

IDTMDrainageP    ScalableMeshGroupDTM::_GetDTMDrainage()
    {
    assert(0);
    return 0;
    }

IDTMContouringP  ScalableMeshGroupDTM::_GetDTMContouring()
    {
    assert(0);
    return 0;
    }

DTMStatusInt ScalableMeshGroupDTM::_CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints)
    {
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!scalableMesh->IsTerrain()) continue;
        if (!m_group->IsRegionRestricted(scalableMesh) || m_group->IsWithinRegion(scalableMesh, pts, numPoints))
            {
            return scalableMesh->GetDTMInterface(m_type)->CalculateSlopeArea(flatArea, slopeArea, pts, numPoints);
            }
        }
    return DTM_SUCCESS;
    }

DTMStatusInt ScalableMeshGroupDTM::_CalculateSlopeArea(double& flatArea, double& slopeArea, DPoint3dCP pts, int numPoints, DTMAreaValuesCallback progressiveCallback, DTMCancelProcessCallback isCancelledCallback)
    {
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!scalableMesh->IsTerrain()) continue;
        if (!m_group->IsRegionRestricted(scalableMesh) || m_group->IsWithinRegion(scalableMesh, pts, numPoints))
            {
            return scalableMesh->GetDTMInterface(m_type)->CalculateSlopeArea(flatArea, slopeArea, pts, numPoints, progressiveCallback, isCancelledCallback);
            }
        }

    return DTM_SUCCESS;
    }

    DTMStatusInt ScalableMeshGroupDTM::_ExportToGeopakTinFile(WCharCP fileNameP, TransformCP transformation)
    {
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!scalableMesh->IsTerrain()) continue;

        BeFileName terrainPath = BeFileName(((ScalableMeshBase*)scalableMesh)->GetPath());
        BeFileName newName = BeFileName(fileNameP);
        
#ifndef VANCOUVER_API
        WString ext = newName.GetExtension();
        newName = newName.GetDirectoryName();        
        newName.AppendToPath(BeFileName(BeFileName(fileNameP).GetFileNameWithoutExtension()));
#else                     
        WString ext = BeFileName::GetExtension(newName.c_str());
        newName.Clear();
        newName.AppendToPath(BeFileName::GetDirectoryName(newName.c_str()).c_str());
        newName.AppendToPath(BeFileName::GetFileNameWithoutExtension(fileNameP).c_str());
#endif        

        size_t position = terrainPath.FindI(L"terrain");
        if (position != std::string::npos)
            {
            BeFileName subTerrainPath = BeFileName(terrainPath.substr(position).c_str());
            newName.append(L"_");
#ifndef VANCOUVER_API
            newName.AppendString(subTerrainPath.GetFileNameWithoutExtension().c_str());
#else
            newName.AppendString(BeFileName::GetFileNameWithoutExtension(subTerrainPath).c_str());
#endif
            }
        else 
            {
#ifndef VANCOUVER_API
            newName.AppendString(terrainPath.GetFileNameWithoutExtension().c_str());
#else
            newName.AppendString(BeFileName::GetFileNameWithoutExtension(terrainPath).c_str());
#endif
            }
        newName.AppendExtension(ext.c_str());

        scalableMesh->GetDTMInterface(m_type)->ExportToGeopakTinFile(newName.c_str(), transformation);
        }

    return DTM_SUCCESS;

    }

bool ScalableMeshGroupDTM::_GetTransformation(TransformR transformation)
    {
    transformation = m_transformToUors;
    return true;
    }

DTMStatusInt ScalableMeshGroupDTM::_GetTransformDTM(DTMPtr& transformedDTM, TransformCR)
    {
    return DTM_ERROR;
    }

DTMStatusInt ScalableMeshGroupDTM::_GetRange(DRange3dR range)
    {
    m_group->GetRange(range);
    return DTM_SUCCESS;
    }

IDTMVolumeP ScalableMeshGroupDTM::_GetDTMVolume()
    {
    return this;
    }


DTMStatusInt ScalableMeshGroupDTM::_DrapePoint(double* elevationP, double* slopeP, double* aspectP, DPoint3d triangle[3], int& drapedTypeP, DPoint3dCR point)
    {
    if (m_group->GetSelection() != nullptr)
    {
        return m_group->GetSelection()->GetDTMInterface(m_type)->GetDTMDraping()->DrapePoint(elevationP, slopeP, aspectP, triangle, drapedTypeP, point);
    }
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (m_group->IsRegionRestricted(scalableMesh))
            {
            if (m_group->IsWithinRegion(scalableMesh, &point, 1))
            if (DTM_SUCCESS == scalableMesh->GetDTMInterface(m_type)->GetDTMDraping()->DrapePoint(elevationP, slopeP, aspectP, triangle, drapedTypeP, point)) return DTM_SUCCESS;
            }
        }

    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!m_group->IsRegionRestricted(scalableMesh))
            {
            if (DTM_SUCCESS == scalableMesh->GetDTMInterface(m_type)->GetDTMDraping()->DrapePoint(elevationP, slopeP, aspectP, triangle, drapedTypeP, point)) return DTM_SUCCESS;
            }
        }

    return DTM_ERROR;
    }

DTMStatusInt ScalableMeshGroupDTM::_DrapeLinear(DTMDrapedLinePtr& ret, DPoint3dCP pts, int numPoints)
    {

    if (m_group->GetSelection() != nullptr)
    {
        return m_group->GetSelection()->GetDTMInterface(m_type)->GetDTMDraping()->DrapeLinear(ret, pts, numPoints);
    }

    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (m_group->IsRegionRestricted(scalableMesh))
            {
            if (m_group->IsWithinRegion(scalableMesh, pts, numPoints))
                if (DTM_SUCCESS == scalableMesh->GetDTMInterface(m_type)->GetDTMDraping()->DrapeLinear(ret, pts, numPoints)) return DTM_SUCCESS;
            }
        }

    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!m_group->IsRegionRestricted(scalableMesh))
            {
            if (DTM_SUCCESS == scalableMesh->GetDTMInterface(m_type)->GetDTMDraping()->DrapeLinear(ret, pts, numPoints)) return DTM_SUCCESS;
            }
        }

    return DTM_ERROR;

    }

bool ScalableMeshGroupDTM::_DrapeAlongVector(DPoint3d* endPt, double *slope, double *aspect, DPoint3d triangle[3], int *drapedType, DPoint3dCR point, double directionOfVector, double slopeOfVector)
    {

    if (m_group->GetSelection() != nullptr)
    {
        return m_group->GetSelection()->GetDTMInterface(m_type)->GetDTMDraping()->DrapeAlongVector(endPt, slope, aspect, triangle, drapedType, point, directionOfVector, slopeOfVector);
    }
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (m_group->IsRegionRestricted(scalableMesh) && m_group->IsWithinRegion(scalableMesh, &point, 1))
            {
            if (scalableMesh->GetDTMInterface(m_type)->GetDTMDraping()->DrapeAlongVector(endPt, slope, aspect, triangle, drapedType, point, directionOfVector, slopeOfVector)) 
                if (m_group->IsWithinRegion(scalableMesh, endPt, 1))
                return true;
            return false;
            }
        }

    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!m_group->IsRegionRestricted(scalableMesh))
            {
            if (scalableMesh->GetDTMInterface(m_type)->GetDTMDraping()->DrapeAlongVector(endPt, slope, aspect, triangle, drapedType, point, directionOfVector, slopeOfVector)) return true;
            }
        }

    return false;
    }

bool ScalableMeshGroupDTM::_ProjectPoint(DPoint3dR pointOnDTM, DMatrix4dCR w2vMap, DPoint3dCR testPoint)
    {

    if (m_group->GetSelection() != nullptr)
    {
        return m_group->GetSelection()->GetDTMInterface(m_type)->GetDTMDraping()->ProjectPoint(pointOnDTM, w2vMap, testPoint);
    }
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (m_group->IsRegionRestricted(scalableMesh))
            {
            if (DTM_SUCCESS == scalableMesh->GetDTMInterface(m_type)->GetDTMDraping()->ProjectPoint(pointOnDTM, w2vMap, testPoint)) 
                if (m_group->IsWithinRegion(scalableMesh, &pointOnDTM, 1))
                return DTM_SUCCESS;
            }
        }

    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!m_group->IsRegionRestricted(scalableMesh))
            {
            if (DTM_SUCCESS == scalableMesh->GetDTMInterface(m_type)->GetDTMDraping()->ProjectPoint(pointOnDTM, w2vMap, testPoint)) return DTM_SUCCESS;
            }
        }

    return DTM_ERROR;
    }

bool ScalableMeshGroupDTM::_IntersectRay(bvector<DTMRayIntersection>& pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint) 
    {
    return false;
    }

bool ScalableMeshGroupDTM::_IntersectRay(DPoint3dR pointOnDTM, DVec3dCR direction, DPoint3dCR testPoint)
    {

    double minParam = DBL_MAX;
    DPoint3d minPt = testPoint;

    DRay3d ray = DRay3d::FromOriginAndVector(testPoint, direction);

    if (m_group->GetSelection() != nullptr)
    {
        return m_group->GetSelection()->GetDTMInterface(m_type)->GetDTMDraping()->IntersectRay(pointOnDTM, direction, testPoint);
    }
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (m_group->IsRegionRestricted(scalableMesh))
            {
            DPoint3d currentPt;
            if (scalableMesh->GetDTMInterface(m_type)->GetDTMDraping()->IntersectRay(currentPt, direction, testPoint))
                {

                DPoint3d pt;
                double param;
                if (ray.ProjectPointBounded(pt, param, currentPt) && param < minParam && param > -1e-8)
                    {
                    if (m_group->IsWithinRegion(scalableMesh, &currentPt, 1))
                    {
                        minPt = currentPt;
                        minParam = param;
                    }
                    }
                }
            }
        }

    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!m_group->IsRegionRestricted(scalableMesh))
            {
            DPoint3d currentPt;
            if (scalableMesh->GetDTMInterface(m_type)->GetDTMDraping()->IntersectRay(currentPt, direction, testPoint))
                {
                DPoint3d pt;
                double param;
                if (ray.ProjectPointBounded(pt, param, currentPt) && param < minParam && param > -1e-8)
                    {
                    bool isOutsideRegions = true;
                    for (auto& scalableMeshRegion : m_group->GetMembers())
                    {
                        if (m_group->IsRegionRestricted(scalableMeshRegion) && m_group->IsWithinRegion(scalableMeshRegion, &currentPt, 1))
                        {
                            isOutsideRegions = false;
                            break;
                        }
                    }
                    if (isOutsideRegions)
                    {
                        minPt = currentPt;
                        minParam = param;
                    }
                    }
                }
            }
        }

    pointOnDTM = minPt;
    return minParam < DBL_MAX;
    }

//volume calls

DTMStatusInt ScalableMeshGroupDTM::_ComputeCutFillVolume(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh)
    {
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!scalableMesh->IsTerrain()) continue;
        double memberCut, memberFill, memberVolume;
        DTMStatusInt code = scalableMesh->GetDTMInterface(m_type)->GetDTMVolume()->ComputeCutFillVolume(&memberCut, &memberFill, &memberVolume, mesh);

        if (memberVolume > 0)
            {
            if(cut != nullptr) *cut = memberCut;
            if (fill != nullptr) *fill = memberFill;
            if (volume != nullptr) *volume = memberVolume;
            return code;
            }
        }

    return DTM_SUCCESS;
    }

DTMStatusInt ScalableMeshGroupDTM::_ComputeCutFillVolumeClosed(double* cut, double* fill, double* volume, PolyfaceHeaderCP mesh)
    {
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!scalableMesh->IsTerrain()) continue;
        double memberCut, memberFill, memberVolume;
        DTMStatusInt code = scalableMesh->GetDTMInterface(m_type)->GetDTMVolume()->ComputeCutFillVolumeClosed(&memberCut, &memberFill, &memberVolume, mesh);

        if (memberVolume > 0)
            {
            if (cut != nullptr) *cut = memberCut;
            if (fill != nullptr) *fill = memberFill;
            if (volume != nullptr) *volume = memberVolume;
            return code;
            }
        }

    return DTM_SUCCESS;
    }

bool ScalableMeshGroupDTM::_RestrictVolumeToRegion(uint64_t regionId)
    {
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!scalableMesh->IsTerrain()) continue;
        if (scalableMesh->GetDTMInterface(m_type)->GetDTMVolume()->RestrictVolumeToRegion(regionId)) return true;
        }

    return false;
    }

void ScalableMeshGroupDTM::_RemoveAllRestrictions()
    {
    for (auto& scalableMesh : m_group->GetMembers())
        {
        if (!scalableMesh->IsTerrain()) continue;
        scalableMesh->GetDTMInterface(m_type)->GetDTMVolume()->RemoveAllRestrictions();
        }
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
