#pragma warning(disable: 4018)
#pragma warning(disable: 4786)

#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <bcDTMBaseDef.h>
#include <dtmevars.h>

#include "bcDTMImpl.h"

#include <bcdtminlines.h>
#include <TerrainModel/Core/DTMIterators.h>
#include <TerrainModel/Core/TMTransformHelper.h>
#include <algorithm>

USING_NAMESPACE_BENTLEY_TERRAINMODEL


int bcdtmMark_checkForPointOnRegionHullDtmObject (BC_DTM_OBJ* dtmP, long pnt);

DTMFeatureInfo::DTMFeatureInfo (BcDTMP dtm, long feature) : m_dtm (dtm), m_feature (feature)
    {
    }

void DTMFeatureInfo::GetFeaturePoints (bvector<DPoint3d>& points) const
    {
    long dtmFeature = m_feature;

    // ToDo use a more direct way to do this.
    DPoint3d* featurePts = nullptr;
    long numFeaturePts;
    if (bcdtmData_copyInitialDtmFeaturePointsToPointArrayDtmObject (m_dtm->GetTinHandle(), dtmFeature, &featurePts, &numFeaturePts) == DTM_SUCCESS && featurePts)
        {
        points.resize (numFeaturePts);
        if (m_dtm->GetTransformHelper ())
            m_dtm->GetTransformHelper ()->convertPointsFromDTM (featurePts, numFeaturePts);
        memcpy (&points[0], featurePts, sizeof DPoint3d * numFeaturePts);
        free (featurePts);
        }
    else
        points.clear ();
    }

void DTMFeatureInfo::GetFeatureInfo (DTMFeatureType& featureType, DTMFeatureId& featureId, DTMUserTag& userTag) const
    {
    BC_DTM_FEATURE* dtmFeatureP = ftableAddrP (m_dtm->GetTinHandle (), m_feature);
    featureType = dtmFeatureP->dtmFeatureType;
    featureId = dtmFeatureP->dtmFeatureId;
    userTag = dtmFeatureP->dtmUserTag;
    }

DTMFeatureType DTMFeatureInfo::FeatureType () const
    {
    BC_DTM_FEATURE* dtmFeatureP = ftableAddrP (m_dtm->GetTinHandle (), m_feature);
    return dtmFeatureP->dtmFeatureType;
    }

DTMFeatureId DTMFeatureInfo::FeatureId () const
    {
    BC_DTM_FEATURE* dtmFeatureP = ftableAddrP (m_dtm->GetTinHandle (), m_feature);
    return dtmFeatureP->dtmFeatureId;
    return dtmFeatureP->dtmUserTag;
    }

DTMUserTag DTMFeatureInfo::UserTag () const
    {
    BC_DTM_FEATURE* dtmFeatureP = ftableAddrP (m_dtm->GetTinHandle (), m_feature);
    return dtmFeatureP->dtmUserTag;
    }

long DTMFeatureInfo::FeatureIndex () const
    {
    return m_feature;
    }

static int featureIdCompareFunction (const DTMFeatureId& id1P, const DTMFeatureId& id2P)
    {
    if (id1P < id2P) return -1;
    if (id1P > id2P) return 1;
    return(0);
    }

DTMFeatureInfo DTMFeatureEnumerator::iterator::operator* () const
    {
    if (m_p_vec->m_sort)
        return DTMFeatureInfo (m_p_vec->m_dtm.get(), m_p_vec->m_sortedIds[m_pos].featureNum);
    return DTMFeatureInfo (m_p_vec->m_dtm.get(), m_pos);
    }

const DTMFeatureEnumerator::iterator& DTMFeatureEnumerator::iterator::operator++ ()
    {
    if (!m_p_vec->MoveNext (m_pos, m_nextSourcePos))
        {
        m_pos = -2;
        m_nextSourcePos = -2;
        }
    return *this;
    }

DTMFeatureEnumerator::DTMFeatureEnumerator (BcDTMCR dtm)
    {
    m_dtm = const_cast<BcDTMP>(&dtm);
    m_dtmP = m_dtm->GetTinHandle ();
    m_isInitialized = false;
    m_sort = false;
    m_readSourceFeatures = true;
    ClearFilterByUserTag();
    ClearFilterByFeatureId ();
    IncludeAllFeatures ();
    }

DTMFeatureEnumeratorPtr DTMFeatureEnumerator::Create (BcDTMCR dtm)
    {
    return new DTMFeatureEnumerator (dtm);
    }


void DTMFeatureEnumerator::InitializeForSorting () const
    {
    if (m_sort)
        {
        m_sortedIds.resize (m_dtmP->numFeatures);

        for (long dtmFeature = 0; dtmFeature < m_dtmP->numFeatures; ++dtmFeature)
            {
            BC_DTM_FEATURE* dtmFeatureP = ftableAddrP (m_dtmP, dtmFeature);
            m_sortedIds[dtmFeature].featureId = dtmFeatureP->dtmFeatureId;
            m_sortedIds[dtmFeature].featureNum = dtmFeature;
            }
        std::sort (m_sortedIds.begin (), m_sortedIds.end (), &sortedFeatureId::compare);
        }
    }

void DTMFeatureEnumerator::Initialize () const
    {
    int dbg = DTM_TRACE_VALUE (0);
    /*
    **  Count Number Of Roll Back Features
    */
    long numCleanUpFeatures = 0;
    
    InitializeForSorting ();

    for (long dtmFeature = 0; dtmFeature < m_dtmP->numFeatures; ++dtmFeature)
        {
        BC_DTM_FEATURE* dtmFeatureP = ftableAddrP (m_dtmP, dtmFeature);

        if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots) numCleanUpFeatures++;
        }
    if (dbg) bcdtmWrite_message (0, 0, 0, "numCleanUpFeatures = %8ld", numCleanUpFeatures);

    /*
    **     Allocate Memory For Feature Ids
    */
    long numFeatureIds = numCleanUpFeatures;
    m_sourceFeatureIds.resize (numFeatureIds);

    if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of Roll Back Features = %8ld", numCleanUpFeatures);
    /*
    **     Populate Feature Ids
    */
    numFeatureIds = 0;
    for (long index = 0; index < m_dtmP->numFeatures; ++index)
        {
        long dtmFeature;
        if (m_sort)
            dtmFeature = m_sortedIds[index].featureNum;
        else
            dtmFeature = index;
        BC_DTM_FEATURE* dtmFeatureP = ftableAddrP (m_dtmP, dtmFeature);

        if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots)
            {
            m_sourceFeatureIds [numFeatureIds] = dtmFeatureP->dtmFeatureId;
            ++numFeatureIds;
            }
        }
    /*
    **     Sort Feature Ids
    */
    if (numCleanUpFeatures > 1 && !m_sort)
        std::sort (m_sourceFeatureIds.begin (), m_sourceFeatureIds.end (), &featureIdCompareFunction);
    }

bool DTMFeatureEnumerator::MoveNext (long& m_index, size_t& m_nextSourceFeatureIndex) const
    {
    if (!m_isInitialized)
        Initialize ();
    do
        {
        m_index++;
        if (m_index >= m_dtmP->numFeatures)
            return false;
        long dtmFeature;

        if (m_sort)
            dtmFeature = m_sortedIds[m_index].featureNum;
        else
            dtmFeature = m_index;

        BC_DTM_FEATURE* dtmFeatureP = ftableAddrP (m_dtmP, dtmFeature);

        // Check if this is an invalid feature state.
        if (!((dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin || dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError || dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray) && dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots))
            continue;

        // ToDo Check the feature type matches what the user wants
        bool isSourceFeature = true;

        if (m_filterByFeatureId && m_featureIdFilter != dtmFeatureP->dtmFeatureId)
            continue;

        if ((m_userTagLow <= m_userTagHigh) && (
            m_userTagLow > dtmFeatureP->dtmUserTag ||
            m_userTagHigh < dtmFeatureP->dtmUserTag))
            continue;

        if (m_filterByFeatureType)
            {
            unsigned int featureType = (unsigned int)dtmFeatureP->dtmFeatureType;
            FeatureTypeFilter filter;
            if (featureType < m_featureTypeFilter.size ())
                filter = m_featureTypeFilter[featureType];
            else
                filter = FeatureTypeFilter::Default;

            if (filter == FeatureTypeFilter::Exclude || (filter == FeatureTypeFilter::Default && m_excludeFeatureTypeByDefault))
                continue;
            }
        else if (m_excludeFeatureTypeByDefault)
            continue;

        if (m_sort)
            {
            if (dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
                {
                while (m_nextSourceFeatureIndex < m_sourceFeatureIds.size () && m_sourceFeatureIds[m_nextSourceFeatureIndex] < dtmFeatureP->dtmFeatureId)
                    m_nextSourceFeatureIndex++;

                isSourceFeature = m_nextSourceFeatureIndex >= m_sourceFeatureIds.size () || m_sourceFeatureIds[m_nextSourceFeatureIndex] != dtmFeatureP->dtmFeatureId;
                }
            }
        else
            {
            if (dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
                {
                if (m_sourceFeatureIds.size () == 1)
                    {
                    if (dtmFeatureP->dtmFeatureId == m_sourceFeatureIds[0])
                        {
                        isSourceFeature = false;
                        }
                    }
                else if (m_sourceFeatureIds.size () > 1)
                    {
                    DTMFeatureId* featureIdsP = &m_sourceFeatureIds[0];
                    DTMFeatureId* feat1P = featureIdsP;
                    DTMFeatureId* feat2P = &featureIdsP[m_sourceFeatureIds.size () - 1];
                    if (dtmFeatureP->dtmFeatureId == *feat1P || dtmFeatureP->dtmFeatureId == *feat2P)
                        isSourceFeature = false;
                    else
                        {
                        DTMFeatureId *feat3P = &featureIdsP[((long)(feat1P - featureIdsP) + (long)(feat2P - featureIdsP)) / 2];
                        while (feat3P != feat1P && feat3P != feat2P)
                            {
                            if (dtmFeatureP->dtmFeatureId == *feat3P)
                                {
                                isSourceFeature = false;
                                feat1P = feat2P = feat3P;
                                }
                            else
                                {
                                if (dtmFeatureP->dtmFeatureId < *feat3P) feat2P = feat3P;
                                else if (dtmFeatureP->dtmFeatureId > *feat3P) feat1P = feat3P;
                                feat3P = &featureIdsP[((long)(feat1P - featureIdsP) + (long)(feat2P - featureIdsP)) / 2];
                                }
                            }
                        }
                    }
                }
            }
        if (isSourceFeature)
            break;
        } while (true);
        return true;
    }

DTMFeatureEnumerator::iterator DTMFeatureEnumerator::begin () const
    {
    long index = -1;
    size_t nextSourceFeatureIndex = 0;
    if (MoveNext (index, nextSourceFeatureIndex))
        return iterator (this, index, nextSourceFeatureIndex);
    return end ();
    }

DTMFeatureEnumerator::iterator DTMFeatureEnumerator::end () const
    {
    return iterator (this, -2, -2);
    }

//void DTMFeatureEnumerator::GetFeaturePoints (bvector<DPoint3d>& points) const
//    {
//    long dtmFeature;
//
//    if (m_sort)
//        dtmFeature = m_sortedIds[m_index].featureNum;
//    else
//        dtmFeature = m_index;
//
//    // ToDo use a more direct way to do this.
//    DPoint3d* featurePts = nullptr;
//    long numFeaturePts;
//    if (bcdtmData_copyInitialDtmFeaturePointsToPointArrayDtmObject (m_dtmP, dtmFeature, (DPoint3d**)featurePts, &numFeaturePts) == DTM_SUCCESS && featurePts)
//        {
//        points.reserve (numFeaturePts);
//        memcpy (&points[0], featurePts, sizeof DPoint3d * numFeaturePts);
//        free (featurePts);
//        }
//    else
//        points.clear ();
//    }
//
///*-------------------------------------------------------------------+
//|                                                                    |
//|                                                                    |
//|                                                                    |
//+-------------------------------------------------------------------*/
//int bcdtmExtData_browseAllSourceFeaturesDtmObject
//(
//BC_DTM_OBJ   *dtmP,
//bool sorted,
//browseAllSourceCallback callbackFunctionP,
//void* userP
//)
//    {
//    int ret = DTM_SUCCESS, dbg = 0;
//    long dtmFeature, numFeatureIds = 0, numCleanUpFeatures, index;
//    BC_DTM_FEATURE *dtmFeatureP;
//    DTMFeatureId *feat1P, *feat2P, *feat3P, *featureIdsP = NULL;
//    bvector<sortedFeatureId> sortedIds;
//    bool isSourceFeature;
//    /*
//    ** Write Entry Message
//    */
//    if (dbg)
//        {
//        bcdtmWrite_message (0, 0, 0, "Browsing Source Features");
//        bcdtmWrite_message (0, 0, 0, "dtmP   = %p", dtmP);
//        bcdtmWrite_message (0, 0, 0, "sorted = %d", sorted);
//        bcdtmWrite_message (0, 0, 0, "callbackFunctionP = %d", callbackFunctionP);
//        bcdtmWrite_message (0, 0, 0, "userP = %p", userP);
//        }
//    /*
//    ** Check For Valid Dtm Objects
//    */
//    if (bcdtmObject_testForValidDtmObject (dtmP)) goto errexit;
//    /*
//    ** Write Entry Message
//    */
//    if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Tin Features That Are Roll Back Features");
//    /*
//    ** Only Process If Features Present
//    */
//    if (dtmP->numFeatures > 0)
//        {
//        /*
//        **  Count Number Of Roll Back Features
//        */
//        numCleanUpFeatures = 0;
//        if (sorted)
//            sortedIds.resize (dtmP->numFeatures);
//        for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
//            {
//            dtmFeatureP = ftableAddrP (dtmP, dtmFeature);
//            if (sorted)
//                {
//                sortedIds[dtmFeature].featureId = dtmFeatureP->dtmFeatureId;
//                sortedIds[dtmFeature].featureNum = dtmFeature;
//                }
//
//            if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots) ++numCleanUpFeatures;
//            }
//        if (dbg) bcdtmWrite_message (0, 0, 0, "numCleanUpFeatures = %8ld", numCleanUpFeatures);
//
//        /*
//        **     Allocate Memory For Feature Ids
//        */
//        numFeatureIds = numCleanUpFeatures;
//        featureIdsP = (DTMFeatureId *)malloc (numFeatureIds * sizeof(DTMFeatureId));
//        if (featureIdsP == NULL)
//            {
//            bcdtmWrite_message (0, 0, 0, "Memory Allocation Failure");
//            goto errexit;
//            }
//        if (dbg) bcdtmWrite_message (0, 0, 0, "Number Of Roll Back Features = %8ld", numCleanUpFeatures);
//        /*
//        **     Populate Feature Ids
//        */
//        if (sorted)
//            std::sort (sortedIds.begin (), sortedIds.end (), &sortedFeatureId::compare);
//        numFeatureIds = 0;
//        for (index = 0; index < dtmP->numFeatures; ++index)
//            {
//            if (sorted)
//                dtmFeature = sortedIds[index].featureNum;
//            else
//                dtmFeature = index;
//            dtmFeatureP = ftableAddrP (dtmP, dtmFeature);
//            if (dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback && dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots)
//                {
//                *(featureIdsP + numFeatureIds) = dtmFeatureP->dtmFeatureId;
//                ++numFeatureIds;
//                }
//            }
//        /*
//        **     Sort Feature Ids
//        */
//        if (numCleanUpFeatures > 1) qsort (featureIdsP, numFeatureIds, sizeof(DTMFeatureId), (int (*)(const void*, const void*))bcdtmExtData_featureIdCompareFunction);
//        /*
//        **     Mark Tin Features That Have The Same Feature Id As A Roll Back Feature
//        */
//        for (dtmFeature = 0; dtmFeature < dtmP->numFeatures; ++dtmFeature)
//            {
//            dtmFeatureP = ftableAddrP (dtmP, dtmFeature);
//            if ((dtmFeatureP->dtmFeatureState == DTMFeatureState::Data || dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin || dtmFeatureP->dtmFeatureState == DTMFeatureState::TinError || dtmFeatureP->dtmFeatureState == DTMFeatureState::Rollback || dtmFeatureP->dtmFeatureState == DTMFeatureState::PointsArray) && dtmFeatureP->dtmFeatureType != DTMFeatureType::RandomSpots)
//                {
//                isSourceFeature = true;
//                if (dtmFeatureP->dtmFeatureState != DTMFeatureState::Rollback)
//                    {
//                    if (numCleanUpFeatures == 1)
//                        {
//                        if (dtmFeatureP->dtmFeatureId == *featureIdsP)
//                            {
//                            isSourceFeature = false;
//                            }
//                        }
//                    else if (numCleanUpFeatures > 1)
//                        {
//                        feat1P = featureIdsP;
//                        feat2P = featureIdsP + numCleanUpFeatures - 1;
//                        if (dtmFeatureP->dtmFeatureId == *feat1P || dtmFeatureP->dtmFeatureId == *feat2P)
//                            {
//                            isSourceFeature = false;
//                            }
//                        else
//                            {
//                            feat3P = featureIdsP + ((long)(feat1P - featureIdsP) + (long)(feat2P - featureIdsP)) / 2;
//                            while (feat3P != feat1P && feat3P != feat2P)
//                                {
//                                if (dtmFeatureP->dtmFeatureId == *feat3P)
//                                    {
//                                    isSourceFeature = false;
//                                    feat1P = feat2P = feat3P;
//                                    }
//                                else
//                                    {
//                                    if (dtmFeatureP->dtmFeatureId < *feat3P) feat2P = feat3P;
//                                    else if (dtmFeatureP->dtmFeatureId > *feat3P) feat1P = feat3P;
//                                    feat3P = featureIdsP + ((long)(feat1P - featureIdsP) + (long)(feat2P - featureIdsP)) / 2;
//                                    }
//                                }
//                            }
//                        }
//                    }
//                if (isSourceFeature)
//                    {
//                    if (callbackFunctionP)
//                        callbackFunctionP (dtmFeature, dtmFeatureP->dtmFeatureType, dtmFeatureP->dtmFeatureId, dtmFeatureP->dtmUserTag, userP);
//                    }
//                }
//            }
//        }
//    /*
//    ** Clean Up
//    */
//cleanup:
//    if (featureIdsP != NULL) { free (featureIdsP); featureIdsP = NULL; }
//    /*
//    ** Job Completed
//    */
//    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Browsing source features Completed");
//    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Browsing source features Error");
//    return(ret);
//    /*
//    ** Error Exit
//    */
//errexit:
//    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
//    goto cleanup;
//    }
//// Browse for all source features
//// 1. Create the arrray of features which are in rollback.
//// 2. Browse through all features. If the feature type is TIN/TIN_ERROR/Rollback, if it isn't rollback check that there isn't a rollback.
//// 3. Call callback with featureNum/Type/Id/UserTag
//// 4. Create function to get original points from featureNum.
//
///*-------------------------------------------------------------------+
//|                                                                    |
//|                                                                    |
//|                                                                    |
//+-------------------------------------------------------------------*/
//int bcdtmExtData_getSourceFeaturePoints (BC_DTM_OBJ* dtmP, long dtmFeature, DPoint3d** featurePtsPP, long* numFeaturePtsP)
//    {
//    int ret = DTM_SUCCESS;
//    int dbg = 0;
//    /*
//    ** Check For Valid Dtm Objects
//    */
//    if (bcdtmObject_testForValidDtmObject (dtmP)) goto errexit;
//
//    if (dtmFeature < 0 || dtmFeature > dtmP->numFeatures)
//        {
//        bcdtmWrite_message (1, 0, 0, "PointNumber out of range");
//        goto errexit;
//        }
//
//    if (bcdtmData_copyInitialDtmFeaturePointsToPointArrayDtmObject (dtmP, dtmFeature, (DPoint3d**)featurePtsPP, numFeaturePtsP)) goto errexit;
//cleanup:
//    /*
//    ** Job Completed
//    */
//    if (dbg && ret == DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Browsing source features Completed");
//    if (dbg && ret != DTM_SUCCESS) bcdtmWrite_message (0, 0, 0, "Browsing source features Error");
//    return(ret);
//    /*
//    ** Error Exit
//    */
//errexit:
//    if (ret == DTM_SUCCESS) ret = DTM_ERROR;
//    goto cleanup;
//    }

const DTMMeshEnumerator::iterator& DTMMeshEnumerator::iterator::operator++ ()
    {
    m_polyface = nullptr;
    if (!m_p_vec->MoveNext (m_pos, m_pos2))
        m_pos = -2;
    return *this;
    }

DTMMeshEnumerator::DTMMeshEnumerator (BcDTMR dtm) : m_dtm (&dtm)
    {
    m_dtmP = dtm.GetTinHandle ();
    clipDtmP = nullptr;
    m_initialized = false;
    maxTriangles = 50000;
    m_tilingMode = false;
    zAxisFactor = 1;
//D    m_polyface = PolyfaceHeader::CreateFixedBlockIndexed (3);
    }

DTMMeshEnumeratorPtr DTMMeshEnumerator::Create (BcDTMR dtm)
    {
    return new DTMMeshEnumerator (dtm);
    }
DTMMeshEnumerator::~DTMMeshEnumerator ()
    {
    Reset ();
    }
/*
if (fenceType != DTMFenceType::Block)
{
bcdtmWrite_message (2, 0, 0, "DTM Fence Block is the only supported fence option.");
goto errexit;
}
// Validate Mesh Size
if (maxTriangles <= 0) maxTriangles = 50000;
if (maxTriangles > m_dtmP->numTriangles) maxTriangles = m_dtmP->numTriangles;
if (dbg) bcdtmWrite_message (0, 0, 0, "Reset maxTriangles To = %8ld", maxTriangles);
// Validate Normal Vector Option
if (vectorOption < 1 || vectorOption > 2) vectorOption = 2;
// Validate z Axis Factor
if (zAxisFactor <= 0.0) zAxisFactor = 1.0;
*/

DTMStatusInt DTMMeshEnumerator::Initialize () const
    {
    DTMStatusInt ret = DTM_SUCCESS;
    long dbg = DTM_TRACE_VALUE (0), tdbg = DTM_TIME_VALUE (0);
    long numTriangles;
    long startTime;
    DTMFenceType fenceType = m_fence.fenceType;
    bool useFence = fenceType != DTMFenceType::None;
    DTMFenceOption fenceOption = m_fence.fenceOption;
    DPoint3dCP fencePtsP = m_fence.points;
    DPoint3dCP pntP;
    long  pointMarkOffset = 0, numMarked = 0;
    long c1 = 0, c2 = 0;
    BeAssert (!m_initialized);
    if (m_initialized)
        return DTM_ERROR;

    m_initialized = true;
    long numFencePts = m_fence.numPoints;
    /*
    ** Write Entry Message
    */
    if (dbg)
        {
        startTime = bcdtmClock ();
        bcdtmWrite_message (0, 0, 0, "Interrupt Loading Triangle Shade Mesh");
        bcdtmWrite_message (0, 0, 0, "Dtm Object     = %p", m_dtmP);
        bcdtmWrite_message (0, 0, 0, "maxTriangles   = %8ld", maxTriangles);
        bcdtmWrite_message (0, 0, 0, "useFence       = %8ld", useFence);
        bcdtmWrite_message (0, 0, 0, "fenceType      = %8ld", fenceType);
        bcdtmWrite_message (0, 0, 0, "fenceOption    = %8ld", fenceOption);
        bcdtmWrite_message (0, 0, 0, "fencePtsP      = %p", fencePtsP);
        bcdtmWrite_message (0, 0, 0, "numFencePts    = %8ld", numFencePts);
        }
    /*
    ** Validate Fence
    */
    if (useFence == true && (fencePtsP == NULL || numFencePts <= 2)) useFence = false;
    if (useFence == true && (fencePtsP->x != (fencePtsP + numFencePts - 1)->x || fencePtsP->y != (fencePtsP + numFencePts - 1)->y)) useFence = false;
    if (useFence)
        {
        if (fenceType != DTMFenceType::Block && fenceType != DTMFenceType::Shape) fenceType = DTMFenceType::Block;
        if (fenceOption != DTMFenceOption::Inside && fenceOption != DTMFenceOption::Outside && fenceOption != DTMFenceOption::Overlap) fenceOption = DTMFenceOption::Overlap;
        }
    /*
    ** Test For Valid Dtm Object
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "Checking For Valid Dtm Object");
    if (bcdtmObject_testForValidDtmObject (m_dtmP)) goto errexit;
    /*
    **  Check If DTM Is In Tin State
    */
    if (m_dtmP->dtmState != DTMState::Tin)
        {
        bcdtmWrite_message (2, 0, 0, "Method Requires Triangulated DTM");
        goto errexit;
        }
    /*
    ** Check For Voids In The Triangulation
    */
    voidsInDtm = false;
    bcdtmList_testForVoidsInDtmObject (m_dtmP, (long*)&voidsInDtm);
    if (dbg) bcdtmWrite_message (0, 0, 0, "voidsInDtm = %2ld", voidsInDtm);
    /*
    ** Initialise
    */
    leftMostPnt = startPnt = 0;
    lastPnt = m_dtmP->numPoints - 1;
    numTriangles = 0;
    /*
    ** Write Start Scan Point And Last Scan Point
    */
    if (dbg) bcdtmWrite_message (0, 0, 0, "startPnt = %8ld lastPnt = %8ld", startPnt, lastPnt);
    /*
    **  Build Clipping Dtm For Fence
    */
    if (useFence == true)
        {
        if (bcdtmClip_buildClippingTinFromFencePointsDtmObject (&clipDtmP, fencePtsP, numFencePts)) goto errexit;
        clipDtm = BcDTM::CreateFromDtmHandle (clipDtmP);
        if (fabs (m_dtmP->xMin - clipDtmP->xMin) < m_dtmP->ppTol) clipDtmP->xMin = m_dtmP->xMin;
        if (fabs (m_dtmP->xMax - clipDtmP->xMax) < m_dtmP->ppTol) clipDtmP->xMax = m_dtmP->xMax;
        if (fabs (m_dtmP->yMin - clipDtmP->yMin) < m_dtmP->ppTol) clipDtmP->yMin = m_dtmP->yMin;
        if (fabs (m_dtmP->yMax - clipDtmP->yMax) < m_dtmP->ppTol) clipDtmP->yMax = m_dtmP->yMax;
        if (fenceType == DTMFenceType::Block && m_dtmP->xMin >= clipDtmP->xMin && m_dtmP->xMax <= clipDtmP->xMax &&  m_dtmP->yMin >= clipDtmP->yMin && m_dtmP->yMax <= clipDtmP->yMax) useFence = false;
        if (dbg) bcdtmWrite_message (0, 0, 0, "useFence = %2ld ** fenceType = %2ld", useFence, fenceType);
        if (!m_tilingMode)
            {
            //  Mark Points Immediately External To Fence
            m_pointMark = 1;// m_dtmP->numPoints * 2 + pointMarkOffset;
            startTime = bcdtmClock ();
            if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Points External To Fence");
            if (bcdtmLoad_markTinPointsExternalToFenceDtmObject (m_dtmP, clipDtmP, m_pointMark, &leftMostPnt, &numMarked)) goto errexit;
            if (tdbg)
                {
                bcdtmWrite_message (0, 0, 0, "leftMostPnt = %8ld leftMostPnt->x = %12.5lf", leftMostPnt, pointAddrP (m_dtmP, leftMostPnt)->x);
                bcdtmWrite_message (0, 0, 0, "** Time To Mark %6ld Tin Points External To Fence = %8.3lf Seconds", numMarked, bcdtmClock_elapsedTime (bcdtmClock (), startTime));
                }
            //  if( numMarked == 0 ) useFence = FALSE ;

            //  Mark Points Internal To Fence
            }
        else
            m_pointMark = 1;
        /*
        **  Find Points Immediately Before And After Fence
        */
        startTime = bcdtmClock ();
        bcdtmFind_binaryScanDtmObject (m_dtmP, clipDtmP->xMin, &startPnt);
        while (startPnt > 0 && pointAddrP (m_dtmP, startPnt)->x >= clipDtmP->xMin) --startPnt;
        if (pointAddrP (m_dtmP, startPnt)->x < clipDtmP->xMin) ++startPnt;
        bcdtmFind_binaryScanDtmObject (m_dtmP, clipDtmP->xMax, &lastPnt);
        while (lastPnt < m_dtmP->numPoints - 1 && pointAddrP (m_dtmP, lastPnt)->x <= clipDtmP->xMax) ++lastPnt;
        if (tdbg)
            {
            bcdtmWrite_message (0, 0, 0, "startPnt = %8ld startPnt->x = %12.5lf", startPnt, pointAddrP (m_dtmP, startPnt)->x);
            bcdtmWrite_message (0, 0, 0, "lastPnt  = %8ld lastPnt->x  = %12.5lf", lastPnt, pointAddrP (m_dtmP, lastPnt)->x);
            bcdtmWrite_message (0, 0, 0, "clipDtmP->xMin  = %12.5lf clipDtmP->xMax  = %12.5lf", clipDtmP->xMin, clipDtmP->xMax);
            bcdtmWrite_message (0, 0, 0, "lastPnt  = %8ld lastPnt->x  = %12.5lf", lastPnt, pointAddrP (m_dtmP, lastPnt)->x);
            bcdtmWrite_message (0, 0, 0, "** Index Time 00 = %8.3lf Seconds", bcdtmClock_elapsedTime (bcdtmClock (), startTime));
            }
        /*
        **  Mark Tin Points Within Block Fence
        */
        if (fenceType == DTMFenceType::Block)
            {
            if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Points Within Fence Block");
            for (long pnt1 = startPnt; pnt1 <= lastPnt; ++pnt1)
                {
                pntP = pointAddrP (m_dtmP, pnt1);

                if (m_tilingMode)
                    {
                    if (pntP->x > clipDtmP->xMax || pntP->y > clipDtmP->yMax)
                        {
                        c1++;
                        nodeAddrP (m_dtmP, pnt1)->sPtr = 2;
                        }
                    else if (pntP->x >= clipDtmP->xMin && pntP->y >= clipDtmP->yMin)
                        {
                        c2++;
                        nodeAddrP (m_dtmP, pnt1)->sPtr = 1;
                        }
                    }
                else
                    if (pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
                        pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax) nodeAddrP (m_dtmP, pnt1)->sPtr = m_pointMark;
                }
            }
        /*
        **  Mark Tin Points Within Shape Fence
        */
        else if (fenceType == DTMFenceType::Shape)
            {
            if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Points Within Fence Shape");
            for (long pnt1 = startPnt; pnt1 <= lastPnt; ++pnt1)
                {
                pntP = pointAddrP (m_dtmP, pnt1);
                if (pntP->x >= clipDtmP->xMin && pntP->x <= clipDtmP->xMax &&
                    pntP->y >= clipDtmP->yMin && pntP->y <= clipDtmP->yMax)
                    {
                    long fndType, tinPnt1, tinPnt2, tinPnt3;
                    if (bcdtmFind_triangleDtmObject (m_dtmP, pntP->x, pntP->y, &fndType, &tinPnt1, &tinPnt2, &tinPnt3)) goto errexit;
                    if (fndType) nodeAddrP (m_dtmP, pnt1)->sPtr = m_pointMark;
                    }
                }
            }
        /*
        **  Write Start Scan Point And Last Scan Point
        */
        if (dbg) bcdtmWrite_message (0, 0, 0, "startPnt = %8ld lastPnt = %8ld", startPnt, lastPnt);

        if (m_tilingMode)
            leftMostPnt = startPnt;
        }
    if (m_regionMode != RegionMode::Normal)
        {
        ScanAndMarkRegions ();
        }
    /*
    ** Allocate Memory For Mesh Faces
    */
    meshFaces.resize (maxTriangles * 3);
    m_useFence = useFence;
    return DTM_SUCCESS;
errexit:
    if (m_dtmP->dtmState == DTMState::Tin)
        {
        bcdtmList_nullTptrValuesDtmObject (m_dtmP);
        bcdtmList_nullSptrValuesDtmObject (m_dtmP);
        }
    return DTM_ERROR;
    }

void DTMMeshEnumerator::ScanAndMarkRegions () const
    {  
    m_regionPointMask.resize (m_dtmP->numPoints);
    std::fill_n(m_regionPointMask.begin(), m_dtmP->numPoints, false);
    DTMFeatureEnumeratorPtr regionEnumerator = DTMFeatureEnumerator::Create (*m_dtm.get ());
    regionEnumerator->ExcludeAllFeatures ();
    regionEnumerator->IncludeFeature (DTMFeatureType::Region);
    regionEnumerator->SetReadSourceFeatures (false);
      
    if (m_regionMode == RegionMode::RegionFeatureId)
        regionEnumerator->SetFeatureIdFilter (m_regionFeatureId);
    else if (m_regionMode == RegionMode::RegionUserTag)
        regionEnumerator->SetUserTagFilter (m_regionUserTag);

    long  minPnt = m_dtmP->numPoints + 1, maxPnt = -1;
    for (DTMFeatureInfo info : *regionEnumerator)
        {
        long  tmpMinPnt = 0, tmpMaxPnt = 0;

        ScanAndMarkRegion(info.FeatureIndex(), tmpMinPnt, tmpMaxPnt);
        if (minPnt > tmpMinPnt)
            minPnt = tmpMinPnt;
        if (maxPnt < tmpMaxPnt)
            maxPnt = tmpMaxPnt;
        }
    bcdtmList_nullTptrValuesDtmObject (m_dtmP);

    if (m_regionMode != RegionMode::NonRegion)
        {
        if (leftMostPnt < minPnt)
            leftMostPnt = minPnt;
        if (lastPnt > maxPnt)
            lastPnt = maxPnt;
        }
    }

void DTMMeshEnumerator::ScanAndMarkRegion (long featureIndex, long& minPnt, long& maxPnt) const
    {
    bool markValue = true;
    long startPnt;

    if (bcdtmList_copyDtmFeatureToTptrListDtmObject (m_dtmP, featureIndex, &startPnt) == DTM_SUCCESS)
        {
        BC_DTM_OBJ* dtmP = m_dtmP;
        long numMarked;
        long* minPntP = &minPnt;
        long* maxPntP = &maxPnt;
        long* numMarkedP = &numMarked;
        int regionOption = 2; // Needs to be added, Add areas inside internal Regions.
        long ret = DTM_SUCCESS, dbg = DTM_TRACE_VALUE (0);
        long priorPnt, scanPnt, nextPnt, antPnt, clPnt, clPtr, firstPnt, lastPnt;
        /*
        ** Initialise
        */
        firstPnt = lastPnt = dtmP->nullPnt;
        *minPntP = dtmP->numPoints - 1;
        *maxPntP = 0;
        *numMarkedP = 0;
        long mark = -10;
        /*
        ** Check Start Point tPtr List Is Not Null
        */
        if (nodeAddrP (dtmP, startPnt)->tPtr == dtmP->nullPnt)
            {
            bcdtmWrite_message (2, 0, 0, "Tptr List Start Point Is Null");
            return;
            }
        /*
        ** Scan Around Tptr Polygon And Mark Internal Points And Create Internal Tptr List
        */
        if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Internal To Tptr Polygon");
        priorPnt = startPnt;
        scanPnt = nodeAddrP (dtmP, startPnt)->tPtr;
        do
            {
            if (scanPnt < *minPntP) *minPntP = scanPnt;
            if (scanPnt > *maxPntP) *maxPntP = scanPnt;
            antPnt = nextPnt = nodeAddrP(dtmP, scanPnt)->tPtr;
            if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, scanPnt, antPnt)) < 0)
                return;// ToDo....
            while (antPnt != priorPnt)
                {
                /*
                **     Check For Point On Internal Region Hull
                */
                if (nodeAddrP (dtmP, antPnt)->tPtr == dtmP->nullPnt && regionOption == 2)
                    {
                    if (bcdtmMark_checkForPointOnRegionHullDtmObject (dtmP, antPnt))
                        {
                        nodeAddrP (dtmP, antPnt)->tPtr = dtmP->numPoints;
                        if (antPnt < *minPntP) *minPntP = antPnt;
                        if (antPnt > *maxPntP) *maxPntP = antPnt;
                        }
                    }
                if (nodeAddrP (dtmP, antPnt)->tPtr == dtmP->nullPnt)
                    {
                    nodeAddrP (dtmP, antPnt)->tPtr = mark;
                    m_regionPointMask[antPnt] = markValue;
                    ++(*numMarkedP);
                    if (antPnt < *minPntP) *minPntP = antPnt;
                    if (antPnt > *maxPntP) *maxPntP = antPnt;
                    clPtr = nodeAddrP (dtmP, antPnt)->cPtr;
                    while (clPtr != dtmP->nullPtr)
                        {
                        clPnt = clistAddrP (dtmP, clPtr)->pntNum;
                        clPtr = clistAddrP (dtmP, clPtr)->nextPtr;
                        /*
                        **           Check For Point On Internal Region Hull
                        */
                        if (nodeAddrP (dtmP, clPnt)->tPtr == dtmP->nullPnt && regionOption == 2)
                            {
                            if (bcdtmMark_checkForPointOnRegionHullDtmObject (dtmP, clPnt))
                                {
                                nodeAddrP (dtmP, clPnt)->tPtr = dtmP->numPoints;
                                if (clPnt < *minPntP) *minPntP = clPnt;
                                if (clPnt > *maxPntP) *maxPntP = clPnt;
                                }
                            }
                        /*
                        **           Mark Point
                        */
                        if (nodeAddrP (dtmP, clPnt)->tPtr == dtmP->nullPnt)
                            {
                            if (lastPnt == dtmP->nullPnt)
                                {
                                firstPnt = lastPnt = clPnt;
                                }
                            else
                                {
                                nodeAddrP (dtmP, lastPnt)->tPtr = clPnt; lastPnt = clPnt;
                                }
                            nodeAddrP (dtmP, clPnt)->tPtr = clPnt;
                            }
                        }
                    }
                if ((antPnt = bcdtmList_nextAntDtmObject (dtmP, scanPnt, antPnt)) < 0)
                    return; // ToDo check exit goto errexit;
                }
            priorPnt = scanPnt;
            scanPnt = nextPnt;
            } while (priorPnt != startPnt);
        /*
        ** Scan Tptr List And Mark Connected Points
        */
        if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Tptr List Points");
        while (firstPnt != lastPnt)
            {
            nextPnt = nodeAddrP (dtmP, firstPnt)->tPtr;
            nodeAddrP (dtmP, firstPnt)->tPtr = mark;
            m_regionPointMask[firstPnt] = markValue;
            ++(*numMarkedP);
            if (firstPnt < *minPntP) *minPntP = firstPnt;
            if (firstPnt > *maxPntP) *maxPntP = firstPnt;
            clPtr = nodeAddrP (dtmP, firstPnt)->cPtr;
            while (clPtr != dtmP->nullPtr)
                {
                clPnt = clistAddrP (dtmP, clPtr)->pntNum;
                clPtr = clistAddrP (dtmP, clPtr)->nextPtr;
                /*
                **     Check For Point On Internal Region Hull
                */
                if (nodeAddrP (dtmP, clPnt)->tPtr == dtmP->nullPnt && regionOption == 2)
                    {
                    if (bcdtmMark_checkForPointOnRegionHullDtmObject (dtmP, clPnt))
                        {
                        nodeAddrP (dtmP, clPnt)->tPtr = dtmP->numPoints;
                        if (clPnt < *minPntP) *minPntP = clPnt;
                        if (clPnt > *maxPntP) *maxPntP = clPnt;
                        }
                    }
                /*
                **     Mark Point
                */
                if (nodeAddrP (dtmP, clPnt)->tPtr == dtmP->nullPnt)
                    {
                    nodeAddrP (dtmP, lastPnt)->tPtr = clPnt;
                    lastPnt = clPnt;
                    nodeAddrP (dtmP, clPnt)->tPtr = clPnt;
                    }
                }
            firstPnt = nextPnt;
            }
        /*
        ** Mark Last Point
        */
        if (dbg) bcdtmWrite_message (0, 0, 0, "Marking Last Point");
        if (lastPnt != dtmP->nullPnt)
            {
            nodeAddrP (dtmP, lastPnt)->tPtr = mark;
            m_regionPointMask[lastPnt] = markValue;
            ++(*numMarkedP);
            if (lastPnt < *minPntP) *minPntP = lastPnt;
            if (lastPnt > *maxPntP) *maxPntP = lastPnt;
            }
        }
    }

void DTMMeshEnumerator::Reset ()
        {
        if (m_initialized)
            {
            long dbg = DTM_TRACE_VALUE (0);
            DTMFenceType fenceType = m_fence.fenceType;
            bool useFence = m_useFence;

            m_initialized = false;
            if (m_dtmP->dtmState == DTMState::Tin) bcdtmList_nullTptrValuesDtmObject (m_dtmP);
            if (useFence && m_dtmP->dtmState == DTMState::Tin) for (long node = leftMostPnt; node <= lastPnt; ++node) nodeAddrP (m_dtmP, node)->sPtr = m_dtmP->nullPnt;
            }
        }


int DTMMeshEnumerator::bcdtmList_isPtInsideFeature(BC_DTM_OBJ *dtmP, long P1, long testPnt, long featureNum) const
    {
    int numFound = 0;
    enum
        {
        unknown, inside, outside
        } state = unknown;
    bool foundPt = false;
    long clPtr;
    /*
    ** Test For Void Hull Line
    */
    clPtr = nodeAddrP(dtmP, P1)->cPtr;
    while (clPtr != dtmP->nullPtr)
        {
        long P2 = clistAddrP(dtmP, clPtr)->pntNum;

        if (testPnt == P2)
            {
            foundPt = true;
            if (state != unknown)
                return state == inside;
            }
        else
            {
            if (bcdtmList_testForRegionLineDtmObject(dtmP, P2, P1, featureNum))
                {
                state = inside;
                if (foundPt)
                    return false;
                }
            else if (bcdtmList_testForRegionLineDtmObject(dtmP, P1, P2, featureNum))
                {
                state = outside;
                if (foundPt)
                    return true;
                }
            }
        clPtr = clistAddrP(dtmP, clPtr)->nextPtr;
        }
    BeAssert(foundPt);
    /*
    ** Job Completed
    */
    return false;

    }

// This is a simple test as most cases they will only be one region scanned.
int DTMMeshEnumerator::bcdtmList_testTriangleInsideRegionDtmObject(BC_DTM_OBJ *dtmP, long P1, long P2, long P3) const
/*
** This Function Tests If The Line P1-P2 is A Void Or Hole Hull Line
*/
    {
    long clPtr;
    /*
    ** Test For Void Hull Line
    */
    clPtr = nodeAddrP(dtmP, P1)->fPtr;
    while (clPtr != dtmP->nullPtr)
        {
        long dtmFeature = flistAddrP(dtmP, clPtr)->dtmFeature;

        if (ftableAddrP(dtmP, dtmFeature)->dtmFeatureType == DTMFeatureType::Region)
            {
            bool addRegion = false;
            if (m_regionMode == RegionMode::RegionUserTag)
                {
                if (ftableAddrP(dtmP, flistAddrP(dtmP, clPtr)->dtmFeature)->dtmUserTag == m_regionUserTag)
                    addRegion = true;
                }
            else if (m_regionMode == RegionMode::RegionFeatureId)
                {
                if (ftableAddrP(dtmP, flistAddrP(dtmP, clPtr)->dtmFeature)->dtmFeatureId == m_regionFeatureId)
                    addRegion = true;
                }
            else
                addRegion = true;

            if (addRegion)
                {
                long testPnt = P2;
                if (bcdtmList_testForRegionLineDtmObject(dtmP, P1, P2, dtmFeature))
                    testPnt = P3;
                if (bcdtmList_isPtInsideFeature(dtmP, P1, testPnt, dtmFeature))
                    return true;
                }

            }
        clPtr = flistAddrP(dtmP, clPtr)->nextPtr;
        }
    /*
    ** Job Completed
    */
    return false;
    }

int DTMMeshEnumerator::bcdtmList_testForRegionLineDtmObject(BC_DTM_OBJ *dtmP, long P1, long P2, long featureNum) const
/*
** This Function Tests If The Line P1-P2 is A Void Or Hole Hull Line
*/
    {
    long clPtr;
    /*
    ** Test For Void Hull Line
    */
    clPtr = nodeAddrP (dtmP, P1)->fPtr;
    while (clPtr != dtmP->nullPtr)
        {
        if (flistAddrP (dtmP, clPtr)->nextPnt == P2)
            {
            if (featureNum == -1)
                {
                if (ftableAddrP(dtmP, flistAddrP(dtmP, clPtr)->dtmFeature)->dtmFeatureType == DTMFeatureType::Region)
                    {
                    if (m_regionMode == RegionMode::RegionUserTag)
                        {
                        if (ftableAddrP(dtmP, flistAddrP(dtmP, clPtr)->dtmFeature)->dtmUserTag == m_regionUserTag)
                            return 1;
                        }
                    else if (m_regionMode == RegionMode::RegionFeatureId)
                        {
                        if (ftableAddrP(dtmP, flistAddrP(dtmP, clPtr)->dtmFeature)->dtmFeatureId == m_regionFeatureId)
                            return 1;
                        }
                    else
                        return(1);
                    }
                }
                else
                {
                if (featureNum == flistAddrP(dtmP, clPtr)->dtmFeature)
                    return 1;
                }
            }
        clPtr = flistAddrP (dtmP, clPtr)->nextPtr;
        }
    /*
    ** Job Completed
    */
    return(0);
    }

bool DTMMeshEnumerator::bcdtmList_testForRegionTriangleDtmObject(BC_DTM_OBJ *dtmP, std::vector<bool>& pointMask, long P1, long P2, long P3) const
/*
** This Function Tests For A Void Triangle
*/
    {
    /*
    ** If Any Points are Void Points Then Triangle Is A Void triangle
    */
    if (pointMask[P1] || pointMask[P2] || pointMask[P3])
        return true;
    /*
    ** If any Lines are Void Lines Then Triangle Is A Void triangle
    */
    if (bcdtmList_testForRegionLineDtmObject (dtmP, P2, P1)) return true;
    if (bcdtmList_testForRegionLineDtmObject (dtmP, P3, P2)) return true;
    if (bcdtmList_testForRegionLineDtmObject (dtmP, P1, P3)) return true;

    if (bcdtmList_testTriangleInsideRegionDtmObject(dtmP, P1, P2, P3)) return true;

    return false;
    }

    bool DTMMeshEnumerator::MoveNext (long& pnt1, long& pnt2) const
        {
        if (!m_initialized) Initialize ();

        long dbg = DTM_TRACE_VALUE (0), tdbg = DTM_TIME_VALUE (0);
        long  pnt3, clPtr, numTriangles = 0;
        bool voidTriangle;
        int* faceP;
        DTM_CIR_LIST  *clistP;
        DTM_TIN_NODE  *node1P, *node2P, *node3P;
        DTMFenceType fenceType = m_fence.fenceType;
        bool usePnt2 = pnt1 != -1;
        bool useFence = m_useFence;

        if (m_dtmP->dtmState != DTMState::Tin)
            return false;
        if (pnt1 == -1)
            pnt1 = leftMostPnt;
        // Reset the pointers/counters.
        faceP = &meshFaces[0];
        numTriangles = 0;
        /*
        ** Scan DTM And Accumulate Triangle Mesh
        */
        if (dbg) bcdtmWrite_message (0, 0, 0, "Scanning DTM Triangles");
        for (; pnt1 <= lastPnt; ++pnt1)
            {
            node1P = nodeAddrP (m_dtmP, pnt1);
            if (useFence == false || (useFence == true && node1P->sPtr == m_pointMark))
                {
                if ((clPtr = node1P->cPtr) != m_dtmP->nullPtr)
                    {
                    if (!usePnt2)
                        {
                        if ((pnt2 = bcdtmList_nextAntDtmObject (m_dtmP, pnt1, clistAddrP (m_dtmP, clPtr)->pntNum)) < 0) goto errexit;
                        }
                    else
                        {
                        usePnt2 = false;
                        do
                            {
                            clistP = clistAddrP (m_dtmP, clPtr);
                            pnt3 = clistP->pntNum;
                            clPtr = clistP->nextPtr;
                            } while (pnt3 != pnt2);
                        }
                    //if (pnt2 == -1 && (pnt2 = bcdtmList_nextAntDtmObject (m_dtmP, pnt1, clistAddrP (m_dtmP, clPtr)->pntNum)) < 0) goto errexit;
                    //else
                    //    {
                    //    do
                    //        {
                    //        clistP = clistAddrP (m_dtmP, clPtr);
                    //        pnt3 = clistP->pntNum;
                    //        clPtr = clistP->nextPtr;
                    //        } while (pnt3 != pnt2);
                    //    }
                    node2P = nodeAddrP (m_dtmP, pnt2);
                    while (clPtr != m_dtmP->nullPtr)
                        {
                        clistP = clistAddrP (m_dtmP, clPtr);
                        pnt3 = clistP->pntNum;
                        clPtr = clistP->nextPtr;
                        node3P = nodeAddrP (m_dtmP, pnt3);
                        if (node1P->hPtr != pnt2)
                            {
                            if (pnt2 > pnt1 && pnt3 > pnt1 || (useFence == true && (node2P->sPtr != 1 || pnt2 > pnt1) && (node3P->sPtr != 1 || pnt3 > pnt1)))
                                {
                                /*
                                **                 Test For Void Triangle
                                */
                                voidTriangle = false;

                                if (node2P->sPtr == 2 || node3P->sPtr == 2 || pnt2 > lastPnt || pnt3 > lastPnt)
                                    voidTriangle = true;

                                if (!voidTriangle && voidsInDtm == true)
                                    {
                                    long voidTriangleLong;
                                    if (bcdtmList_testForVoidTriangleDtmObject (m_dtmP, pnt1, pnt2, pnt3, (long*)&voidTriangleLong)) goto errexit;
                                    voidTriangle = voidTriangleLong != 0;
                                    }
                                if (!voidTriangle && m_regionMode != RegionMode::Normal)
                                    {
                                    voidTriangle = bcdtmList_testForRegionTriangleDtmObject (m_dtmP, m_regionPointMask, pnt1, pnt2, pnt3);
                                    if (m_regionMode != RegionMode::NonRegion)
                                        voidTriangle = !voidTriangle;
                                    }
                                /*
                                **                 Process If None Void Triangle
                                */
                                if (voidTriangle == false)
                                    {
                                    *faceP = pnt3; ++faceP;
                                    *faceP = pnt2; ++faceP;
                                    *faceP = pnt1; ++faceP;
                                    ++numTriangles;
                                    /*
                                    **                    Check For Maximum Load Triangles
                                    */
                                    if (numTriangles == maxTriangles)
                                        {
                                        pnt2 = pnt3;
                                        return true;
                                        }
                                    }
                                }
                            }
                        /*
                        **           Continue Scan For Triangles
                        */
                        pnt2 = pnt3;
                        node2P = node3P;
                        }
                    }
                }
            }
        meshFaces.resize (numTriangles * 3);
        /*
        ** Check For Unloaded Triangles
        */
        return numTriangles > 0;
    errexit:
        return false;
        }

DTMMeshEnumerator::iterator DTMMeshEnumerator::begin () const
    {
    long index = -1;
    long pnt2 = -1;
    if (MoveNext (index, pnt2))
        return iterator (this, index, pnt2);
    return end ();
    }

DTMMeshEnumerator::iterator DTMMeshEnumerator::end () const
    {
    return iterator (this, -2, -1);
    }

PolyfaceQueryP DTMMeshEnumerator::iterator::operator* () const
    {
    long dbg = DTM_TRACE_VALUE (0);
    DTM_TIN_NODE  *nodeP;
    DPoint3dP p3dP;
    DVec3dP normP;
    DPoint3dCP pntP;
    long meshVectorsSize = 0;
    long node;
    double dz;
    long numMeshPts, minTptrPnt, maxTptrPnt;
    BC_DTM_OBJ* m_dtmP = m_p_vec->m_dtm->GetTinHandle ();
    long nullPnt = m_dtmP->nullPnt;
    /*
    ** Mark Mesh Points
    */
    minTptrPnt = m_dtmP->numPoints;
    maxTptrPnt = -1;
    BlockedVectorDPoint3dR points = m_p_vec->meshPoints;
    BlockedVectorDVec3dR normals = m_p_vec->meshNormals;

    if (!m_p_vec->m_useRealPointIndexes)
        {
        numMeshPts = 0;
        for (long face : m_p_vec->meshFaces)
            {
            nodeP = nodeAddrP(m_dtmP, face);
            if (nodeP->tPtr == nullPnt)
                {
                if (face < minTptrPnt) minTptrPnt = face;
                else if (face > maxTptrPnt) maxTptrPnt = face;
                nodeP->tPtr = ++numMeshPts;
                }
            }
        if (dbg) bcdtmWrite_message(0, 0, 0, "minTptrPoint = %8ld maxTptrPoint = %8ld", minTptrPnt, maxTptrPnt);

        if (maxTptrPnt == -1) maxTptrPnt = minTptrPnt;
        /*
        **                       Allocate Memory For Mesh Points
        */
        if (dbg) bcdtmWrite_message(0, 0, 0, "Populating Mesh Points ** numMeshPts = %8ld", numMeshPts);
        points.resize(numMeshPts);
        normals.resize(numMeshPts);
        /*
        **                       Populate Mesh Points Array And Mesh Vectors Array
        */
        for (node = minTptrPnt; node <= maxTptrPnt; ++node)
            {
            nodeP = nodeAddrP(m_dtmP, node);
            if (nodeP->tPtr > 0 && nodeP->tPtr < m_dtmP->nullPnt)
                {
                p3dP = points.GetPtr() + (nodeP->tPtr - 1);
                normP = normals.GetPtr() + (nodeP->tPtr - 1);
                pntP = pointAddrP(m_dtmP, node);
                p3dP->x = pntP->x;
                p3dP->y = pntP->y;
                if (m_p_vec->zAxisFactor != 1)
                    {
                    dz = (pntP->z - m_dtmP->zMin) * m_p_vec->zAxisFactor;
                    p3dP->z = m_dtmP->zMin + dz;
                    }
                else
                    p3dP->z = pntP->z;

                // Convert Point.
                //            p3dP++;

                bcdtmLoad_calculateNormalVectorForTriangleVertexDtmObject(m_dtmP, node, 2, m_p_vec->zAxisFactor, normP);
                // Rotate normal....
                //            ++normP;
                }
            }
        /*
        **                       Reset Point Indexes In Mesh Faces
        */
        for (int& ptIndex : m_p_vec->meshFaces)
            ptIndex = nodeAddrP(m_dtmP, ptIndex)->tPtr;
        }
    else
        {
        numMeshPts = 0;
        for (long face : m_p_vec->meshFaces)
            {
            nodeP = nodeAddrP(m_dtmP, face);
            if (nodeP->tPtr == nullPnt)
                {
                if (face < minTptrPnt) minTptrPnt = face;
                else if (face > maxTptrPnt) maxTptrPnt = face;
                }
            }
        if (dbg) bcdtmWrite_message(0, 0, 0, "minTptrPoint = %8ld maxTptrPoint = %8ld", minTptrPnt, maxTptrPnt);

        if (maxTptrPnt == -1) maxTptrPnt = minTptrPnt;
        }
    /*
    **                       Null Tptr Values
    */
    for (node = minTptrPnt; node <= maxTptrPnt; ++node)
        nodeAddrP (m_dtmP, node)->tPtr = nullPnt;
    if (!m_polyface)
        m_polyface = new PolyfaceQueryCarrier (3, false, (size_t)m_p_vec->meshFaces.size (), (size_t)points.size (), points.GetCP (), m_p_vec->meshFaces.GetCP (), normals.size (), normals.GetCP (), m_p_vec->meshFaces.GetCP ());
    else
        *m_polyface = PolyfaceQueryCarrier (3, false, (size_t)m_p_vec->meshFaces.size (), (size_t)points.size (), points.GetCP (), m_p_vec->meshFaces.GetCP (), normals.size (), normals.GetCP (), m_p_vec->meshFaces.GetCP ());
    return m_polyface;
    }

DRange3d DTMMeshEnumerator::iterator::GetRange () const
    {
    DRange3d range;
    range.Init ();

    if (!m_p_vec->meshFaces.empty ())
        {
        BC_DTM_OBJ* m_dtmP = m_p_vec->m_dtm->GetTinHandle ();
        long nullPnt = m_dtmP->nullPnt;
        /*
        ** Mark Mesh Points
        */
        for (long face : m_p_vec->meshFaces)
            {
            range.Extend (*pointAddrP (m_dtmP, face));
            }

        if (m_p_vec->zAxisFactor != 1)
            {
            double dz = (range.low.z - m_dtmP->zMin) * m_p_vec->zAxisFactor;
            range.low.z = m_dtmP->zMin + dz;
            dz = (range.high.z - m_dtmP->zMin) * m_p_vec->zAxisFactor;
            range.high.z = m_dtmP->zMin + dz;
            }
        }
    return range;
    }

DRange3d DTMMeshEnumerator::GetRange () const
    {
    DRange3d range;
    range.Init ();

    for (iterator i = begin (); i != end (); ++i)
        {
        range.Extend (i.GetRange ());
        }
    
    return range;
    
    }