/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/CivilDTMHelpers.cpp $
|    $RCSfile: CivilDTMHelpers.cpp,v $
|   $Revision: 1.9 $
|       $Date: 2012/02/23 00:32:43 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../ImagePPHeaders.h"
#include "CivilDTMHelpers.h"
#include "..\Stores\SMStoreUtils.h"
#include <TerrainModel/Core/DTMIterators.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
namespace Plugin {

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CivilImportedTypes::CivilImportedTypes ()
    {
    static const DTMFeatureType POINT_TYPES[] =
        {
        //DTMFeatureType::RandomSpots,
        //DTMFeatureType::GroupSpots,
        // NTERAY: TR# 334042: SPOT type seem to alway be returned by the browse function and seem
        //         to include all other point types. I believe one can import either only spots or
        //         all other types but not both at the same time as points will be duplicated.
        DTMFeatureType::Spots,
        //DTMFeatureType::FeatureSpot,
        };
    static const size_t POINT_TYPES_QTY = sizeof(POINT_TYPES) / sizeof(POINT_TYPES[0]);

    static const DTMFeatureType LINEAR_TYPES[] =
        {
        DTMFeatureType::Breakline,
        DTMFeatureType::Hull,
        DTMFeatureType::ContourLine,
        DTMFeatureType::Void,
        DTMFeatureType::BreakVoid,
        DTMFeatureType::Island,
        DTMFeatureType::Hole,
        DTMFeatureType::Polygon,
        DTMFeatureType::ZeroSlopePolygon,
        DTMFeatureType::SoftBreakline,
        // Removed as it was not supported for now and was causing a memory leak.
        //DTMFeatureType::GraphicBreak
        };
    static const size_t LINEAR_TYPES_QTY = sizeof(LINEAR_TYPES) / sizeof(LINEAR_TYPES[0]);


    static const DTMFeatureType TIN_LINEAR_TYPES[] =
        {
        //DTMFeatureType::TinHull
        DTMFeatureType::TinPoint,
        DTMFeatureType::TinLine,
        //DTMFeatureType::Triangle,
        //DTMFeatureType::TriangleEdge,
        //DTMFeatureType::TriangleIndex,
        //DTMFeatureType::TriangleInfo,
        };
    static const size_t TIN_LINEAR_TYPES_QTY = sizeof(TIN_LINEAR_TYPES) / sizeof(TIN_LINEAR_TYPES[0]);


    static const DTMFeatureType MESH_TYPES[] =
        {
        //DTMFeatureType::TinHull
        //DTMFeatureType::TinPoint,
        //DTMFeatureType::TinLine,
        DTMFeatureType::Triangle,
        //DTMFeatureType::TriangleEdge,
        //DTMFeatureType::TriangleIndex,
        //DTMFeatureType::TriangleInfo,
        };
    static const size_t MESH_TYPES_QTY = sizeof(MESH_TYPES) / sizeof(MESH_TYPES[0]);

    struct IsValidFeatureType : public std::unary_function<DTMFeatureType, bool>
        {
        bool operator () (DTMFeatureType featureType) const
            { return DTM_ERROR != bcdtmData_testForValidDtmObjectExportFeatureType(featureType); }
        };

    linears.reserve(LINEAR_TYPES_QTY);
    points.reserve(POINT_TYPES_QTY);
    tinLinears.reserve(TIN_LINEAR_TYPES_QTY);
    mesh.reserve(MESH_TYPES_QTY);
    remove_copy_if(POINT_TYPES, POINT_TYPES + POINT_TYPES_QTY, back_inserter(points), not1(IsValidFeatureType()));
    remove_copy_if(LINEAR_TYPES, LINEAR_TYPES + LINEAR_TYPES_QTY, back_inserter(linears), not1(IsValidFeatureType()));
    remove_copy_if(TIN_LINEAR_TYPES, TIN_LINEAR_TYPES + TIN_LINEAR_TYPES_QTY, back_inserter(tinLinears), not1(IsValidFeatureType()));
    remove_copy_if(MESH_TYPES, MESH_TYPES + MESH_TYPES_QTY, back_inserter(mesh), not1(IsValidFeatureType()));
    }



/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const CivilImportedTypes& CivilImportedTypes::GetInstance ()
    {
    static const CivilImportedTypes SINGLETON;
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static int AccumulateLinearFeatureStatsCallback    (DTMFeatureType pi_FeatureType, DTMUserTag pi_FeatureTag, DTMFeatureId pi_FeatureId,
                                                    DPoint3d* pi_pPoints, size_t pi_PointQty,
                                                    void* pi_pUserArg)
    {
    LinearFeatureTypeInfo& linFeatureStats = *((LinearFeatureTypeInfo*)pi_pUserArg);
    ++linFeatureStats.m_linearCount;
    linFeatureStats.m_pointCount += pi_PointQty;

    return DTM_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static int CopyPointsCallback  (DTMFeatureType pi_FeatureType, DTMUserTag pi_FeatureTag, DTMFeatureId pi_FeatureId,
                                DPoint3d* pi_pPoints, size_t pi_PointQty,
                                void* pi_pUserArg)
    {
    HPU::Array<DPoint3d>& rPointArray = *((HPU::Array<DPoint3d>*)pi_pUserArg);
    rPointArray.Append(pi_pPoints, pi_pPoints + pi_PointQty);

    return DTM_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static int CopyFeaturesCallback(DTMFeatureType pi_FeatureType, DTMUserTag pi_FeatureTag, DTMFeatureId pi_FeatureId,
                                DPoint3d* pi_pPoints, size_t pi_PointQty,
                                void* pi_pUserArg)
    {
    IDTMFeatureArray<DPoint3d>& rFeatureList = *((IDTMFeatureArray<DPoint3d>*)pi_pUserArg);

    HPRECONDITION(pi_FeatureTag <= numeric_limits<ISMStore::FeatureHeader::group_id_type>::max());
    //Very bad converting int64 to uint32_t.... but require to changes the STM file format to fix....
    //Was already like that in V8i, probably not quite important since tag doesn`t seem to be used at all??
    rFeatureList.Append((ISMStore::FeatureType)pi_FeatureType,
                        pi_pPoints,
                        pi_pPoints + pi_PointQty,
                        (ISMStore::FeatureHeader::group_id_type)(pi_FeatureTag & 0xFFFFFFFF));

    return DTM_SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t ComputeMaxPointCounts (const LinearFeatureTypeInfoList& pi_rList)
    {
    struct TypeInfoPointCountComp
        {
        bool operator () (const LinearFeatureTypeInfo& pi_rLeft, const LinearFeatureTypeInfo& pi_rRight) const
            {
            return pi_rLeft.m_pointCount < pi_rRight.m_pointCount;
            }
        };

    size_t maxPtCount = 0;

    if (0 != pi_rList.size())
        {
        maxPtCount = max_element(pi_rList.begin(), pi_rList.end(), TypeInfoPointCountComp())->m_pointCount;
        assert(0 != maxPtCount);
        }

    return maxPtCount;
    }


/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t ComputeMaxFeatureCounts (const LinearFeatureTypeInfoList& pi_rList)
    {
    struct TypeInfoFeatureCountComp
        {
        bool operator () (const LinearFeatureTypeInfo& pi_rLeft, const LinearFeatureTypeInfo& pi_rRight) const
            {
            return pi_rLeft.m_linearCount < pi_rRight.m_linearCount;
            }
        };

    size_t maxFeatureCount = 0;

    if (0 != pi_rList.size())
        {
        maxFeatureCount = max_element(pi_rList.begin(), pi_rList.end(), TypeInfoFeatureCountComp())->m_linearCount;
        assert(0 != maxFeatureCount);
        }

    return maxFeatureCount;
    }


/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ComputeTotalCounts (LinearFeatureTypeInfoList&              po_rList,
                                const BcDTM&                           pi_rDTM,
                                const CivilImportedTypes::TypeIdList&   pi_rTypeIds)
    {
    CivilImportedTypes::TypeIdList::const_iterator TypeIdIt = pi_rTypeIds.begin();
    for (; TypeIdIt != pi_rTypeIds.end(); ++TypeIdIt)
        {
        LinearFeatureTypeInfo linFeatureStats(*TypeIdIt, 0, 0);

		if (*TypeIdIt == DTMFeatureType::Hull)
		{
			DTMFeatureStatisticsInfo info;
			const_cast<BcDTM&>(pi_rDTM).CalculateFeatureStatistics(info);
			if (!info.hasHull)
				continue;

			//we only care for explicit hull features. However, BrowseFeature *will* return a hull for all in tin state,
			//even though there's no source hull features. With CalculateFeatureStatistics we explicitly look for source hull features.
		}
        if (DTM_SUCCESS != const_cast<BcDTM&>(pi_rDTM).BrowseFeatures(*TypeIdIt, numeric_limits<long>::max(),
                                                                       &linFeatureStats, AccumulateLinearFeatureStatsCallback))
            {
            HASSERT(!"Unable to count features");
            return false;
            }

        if (0 != linFeatureStats.m_linearCount)
            {
            assert(0 != linFeatureStats.m_pointCount);
            po_rList.push_back(linFeatureStats);
            }
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointHandler::PointHandler (const BcDTM&                           pi_rDTM,
                            const CivilImportedTypes::TypeIdList&   typeIDList)
    :   m_rDTM(pi_rDTM),
        m_typeIDList(typeIDList),
        m_initialized(false),
        m_maxPtCount(0)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointHandler::ComputeCounts ()
    {
    if (m_initialized)
        return true;

    if (!ComputeTotalCounts(m_typesInfo, m_rDTM, m_typeIDList))
        return false;

    m_initialized = true;
    m_maxPtCount = ComputeMaxPointCounts(m_typesInfo);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointHandler::TypeInfoCIter PointHandler::TypesInfoBegin () const
    {
    assert(m_initialized);
    return m_typesInfo.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointHandler::TypeInfoCIter PointHandler::TypesInfoEnd () const
    {
    assert(m_initialized);
    return m_typesInfo.end();
    }


size_t PointHandler::GetMaxPointCount () const
    {
    assert(m_initialized);
    return m_maxPtCount;
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointHandler::Copy(TypeInfoCIter           pi_typeInfoIter,
                        HPU::Array<DPoint3d>&   po_pointArray) const
    {
    // Retrieve points of the specified type in point array
    if (DTM_SUCCESS != const_cast<BcDTM&>(m_rDTM).BrowseFeatures(pi_typeInfoIter->m_typeID, numeric_limits<long>::max(),
                                                                  &po_pointArray, CopyPointsCallback))
        {
        assert(!"Unable to gather points from source");
        return false;
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LinearHandler::LinearHandler   (const BcDTM&                           pi_rDTM,
                                const CivilImportedTypes::TypeIdList&   typeIDList)
    :   m_rDTM(pi_rDTM),
        m_typeIDList(typeIDList),
        m_initialized(false),
        m_maxPtCount(0),
        m_maxLinearCount(0)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool LinearHandler::ComputeCounts ()
    {
    if (m_initialized)
        return true;

    if (!ComputeTotalCounts(m_typesInfo, m_rDTM, m_typeIDList))
        return false;

    m_initialized = true;
    m_maxLinearCount = ComputeMaxFeatureCounts(m_typesInfo);
    m_maxPtCount = ComputeMaxPointCounts(m_typesInfo);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LinearHandler::TypeInfoCIter LinearHandler::TypesInfoBegin () const
    {
    assert(m_initialized);
    return m_typesInfo.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LinearHandler::TypeInfoCIter LinearHandler::TypesInfoEnd () const
    {
    assert(m_initialized);
    return m_typesInfo.end();
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t LinearHandler::GetMaxLinearCount () const
    {
    assert(m_initialized);
    return m_maxLinearCount;
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t LinearHandler::GetMaxPointCount () const
    {
    assert(m_initialized);
    return m_maxPtCount;
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool LinearHandler::Copy    (TypeInfoCIter               pi_typeInfoIter,
                                    IDTMFeatureArray<DPoint3d>& po_featureArray) const
    {
	if (pi_typeInfoIter->m_typeID == DTMFeatureType::Hull)
	{
		DTMFeatureStatisticsInfo info;
		const_cast<BcDTM&>(m_rDTM).CalculateFeatureStatistics(info);
		if (!info.hasHull)
			return true;

		//we only care for explicit hull features. However, BrowseFeature *will* return a hull for all in tin state,
		//even though there's no source hull features. With CalculateFeatureStatistics we explicitly look for source hull features.
	}
    // Retrieve features of the specified type in feature list
    if (DTM_SUCCESS != const_cast<BcDTM&>(m_rDTM).BrowseFeatures(pi_typeInfoIter->m_typeID, numeric_limits<long>::max(),
                                                                  &po_featureArray, CopyFeaturesCallback))
        {
        assert(!"Unable to gather features from source");
        return false;
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Mathieu.St-Pierre  08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MeshHandler::MeshHandler(const BcDTM&                          pi_rDTM,
                         const CivilImportedTypes::TypeIdList& typeIDList)
    : m_rDTM(pi_rDTM),
    m_typeIDList(typeIDList),
    m_initialized(false),
    m_maxPtCount(0), 
    m_maxPtIndicesCount(0)
    {    
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Mathieu.St-Pierre   09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshHandler::ComputeCounts()
    {
    if (m_initialized)
        return true;
    /*
    if (!ComputeTotalCounts(m_typesInfo, m_rDTM, m_typeIDList))
        return false;
        */

    //MeshFeatureTypeInfoList m_typesInfo;
    //size_t                  m_maxPtCount;
    //size_t                  m_maxPtIndicesCount;

    m_initialized = true;

    if (DTMState::Tin != const_cast<BcDTM&>(m_rDTM).GetDTMState())
        {
        DTMStatusInt status = const_cast<BcDTM&>(m_rDTM).Triangulate();
        assert(status == DTM_SUCCESS);
        }        
    /*
    m_maxPtCount = ComputeMaxPointCounts(m_typesInfo);    
    m_maxPtIndicesCount = ComputeMaxPointIndicesCounts(m_typesInfo);
    */

    m_maxPtCount = const_cast<BcDTM&>(m_rDTM).GetPointCount();
    m_maxPtIndicesCount = const_cast<BcDTM&>(m_rDTM).GetTrianglesCount() * 3;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Mathieu.St-Pierre   09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
#if 0
MeshHandler::TypeInfoCIter MeshHandler::TypesInfoBegin() const
    {
    assert(m_initialized);
    return m_typesInfo.begin();
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Mathieu.St-Pierre   09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
#if 0
MeshHandler::TypeInfoCIter MeshHandler::TypesInfoEnd() const
    {
    assert(m_initialized);
    return m_typesInfo.end();
    }
#endif

size_t MeshHandler::GetMaxPointCount() const
    {
    assert(m_initialized);
    return m_maxPtCount;
    }

size_t MeshHandler::GetMaxPtIndicesCount() const
    {
    assert(m_initialized);
    return m_maxPtIndicesCount;
    }

/*---------------------------------------------------------------------------------**//**
* @description
*
* @bsimethod                                                  Mathieu.St-Pierre   09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct MeshCallbackUserArg
    {
    HPU::Array<DPoint3d>* m_pointArray;
    HPU::Array<int32_t>*  m_ptIndicesArray;
    };


int TriangleMeshCallback(DTMFeatureType featureType, int numTriangles, int numMeshPoints, DPoint3d *meshPointsP, int numMeshFaces, long *meshFacesP, void *userP)
    {    
    MeshCallbackUserArg* userArg = (MeshCallbackUserArg*)userP;

    userArg->m_pointArray->Append(meshPointsP, meshPointsP + numMeshPoints);
    userArg->m_ptIndicesArray->Append(meshFacesP, meshFacesP + numMeshFaces);

    return DTM_SUCCESS;    
    }

bool MeshHandler::Copy(/*TypeInfoCIter           pi_typeInfoIter,*/
                        HPU::Array<DPoint3d>&  po_pointArray, 
                        HPU::Array<int32_t>&   po_ptIndicesArray) const
    {        
    assert(DTMState::Tin == const_cast<BcDTM&>(m_rDTM).GetDTMState());

    MeshCallbackUserArg userArg;

    userArg.m_pointArray = &po_pointArray;
    userArg.m_ptIndicesArray = &po_ptIndicesArray;

    DTMFenceParams fenceParams;

    int maxTriangles = m_rDTM.GetTrianglesCount() * 2;

    Bentley::TerrainModel::DTMMeshEnumeratorPtr en = Bentley::TerrainModel::DTMMeshEnumerator::Create(const_cast<BcDTM&>(m_rDTM));

    en->SetMaxTriangles(maxTriangles);

    for (PolyfaceQueryP polyFace : *en)
        {                
        po_pointArray.Append(polyFace->GetPointCP(), polyFace->GetPointCP() + polyFace->GetPointCount());
        po_ptIndicesArray.Append(polyFace->GetPointIndexCP(), polyFace->GetPointIndexCP() + polyFace->GetPointIndexCount());                
        }


    return true;
    }

} // END namespace Plugin
END_BENTLEY_SCALABLEMESH_NAMESPACE
