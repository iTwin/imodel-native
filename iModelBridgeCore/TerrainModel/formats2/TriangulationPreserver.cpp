/*--------------------------------------------------------------------------------------+
|
|     $Source: formats/TriangulationPreserver.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TerrainModel/Formats/Formats.h>
#include "TriangulationPreserver.h"
#include <TerrainModel/Core/bcdtmInlines.h>
#include <TerrainModel/Core/DTMIterators.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

const double MAXAREAFORVOIDORISLANDS = 0.001;

//=======================================================================================
// @bsimethod                                            Daryl.Holmwood      10/2017
//=======================================================================================
TriangulationPreserver::TriangulationPreserver (BcDTMR dtm) : m_dtm (&dtm), m_dtmObj (nullptr), m_stmDtm (nullptr)
    {
    m_useGraphicBreaks = true;
    m_numLinks = 0;
    m_numLinkErrors = 0;
    m_numLinkSwapFailed = 0;
    m_numLinksSwapped = 0;
    m_numPointsUsed = 0;
    }

//=======================================================================================
// @bsimethod                                            Daryl.Holmwood      10/2017
//=======================================================================================
void TriangulationPreserver::Initialize ()
    {
    if (m_stmDtm.IsNull ())
        m_stmDtm = BcDTM::Create ();
    }

//=======================================================================================
// @bsimethod                                            Daryl.Holmwood      10/2017
//=======================================================================================
void TriangulationPreserver::AddPoints (DPoint3dCP pts, int numPoints, int firstId)
    {
    if ((int)m_ptIdTolocalId.size () < firstId + numPoints)
        m_ptIdTolocalId.resize (firstId + numPoints, -1);

    m_pts.reserve (m_pts.size () + numPoints);

    for (int i = 0; i < numPoints; i++)
        {
        BeAssert (m_ptIdTolocalId[firstId] == -1);
        m_ptIdTolocalId[firstId++] = (int)m_pts.size ();
        m_pts.push_back (*pts++);
        }
    m_pointUsed.resize(m_pts.size());
    }

//=======================================================================================
// @bsimethod                                            Daryl.Holmwood      10/2017
//=======================================================================================
void TriangulationPreserver::AddPoint (DPoint3dCR pt, int ptId)
    {
    AddPoints (&pt, 1, ptId);
    }

//=======================================================================================
// @bsimethod                                            Daryl.Holmwood      10/2017
//=======================================================================================
void TriangulationPreserver::AddTriangle (int* ptNums, int numPoints)
    {
    Initialize ();
    if (numPoints != 3)
        return;

    DPoint3d pts[3];
    int num = GetLocalId(ptNums[0]);
    BeAssert(-1 != num);
    if (-1 == num)
        return;

    pts[0] = m_pts[num];

    num = GetLocalId(ptNums[1]);
    BeAssert(-1 != num);
    if (-1 == num)
        return;
    pts[1] = m_pts[GetLocalId (ptNums[1])];

    num = GetLocalId(ptNums[2]);
    BeAssert(-1 != num);
    if (-1 == num)
        return;
    pts[2] = m_pts[GetLocalId (ptNums[2])];

    int side = bcdtmMath_sideOf(pts[0].x, pts[0].y, pts[1].x, pts[1].y, pts[2].x, pts[2].y);

    // Make sure it is the right orientation.
    if (side > 0)
        std::swap(ptNums[2], ptNums[1]);
/*
    DTMDirection direction;
    double area;
    if (bcdtmMath_getPolygonDirectionP3D(pts, numPoints, &direction, &area) != DTM_SUCCESS)
        return;
    if (area < MAXAREAFORVOIDORISLANDS)
        return;
    long onLine;
    double X, Y;
    double d2 = bcdtmMath_distanceOfPointFromLine(&onLine, pts[0].x, pts[0].y, pts[1].x, pts[1].y, pts[2].x, pts[2].y, &X, &Y);

    if (d2 < 0.001)
        return;

    double d3 = bcdtmMath_distanceOfPointFromLine(&onLine, pts[1].x, pts[1].y, pts[2].x, pts[2].y, pts[0].x, pts[0].y, &X, &Y);

    if (d3 < 0.001)
        return;
    double d4 = bcdtmMath_distanceOfPointFromLine(&onLine, pts[2].x, pts[2].y, pts[0].x, pts[0].y, pts[1].x, pts[1].y, &X, &Y);

    if (d4 < 0.001)
        return;
*/

    for (int i = 0; i < 3; i++)
        {
        int localPtNum = GetLocalId(ptNums[i]);
        if (!m_pointUsed[localPtNum])
            {
            m_pointUsed[localPtNum] = true;
            m_numPointsUsed++;
            }
        m_pointIndex.push_back(localPtNum);
        }
    }


//=======================================================================================
// @bsistruct                                            Daryl.Holmwood      10/2017
//=======================================================================================
struct DuplicateFeatureChecker
    {
    //=======================================================================================
    // @bsistruct                                            Daryl.Holmwood      10/2017
    //=======================================================================================
    struct DuplicateFeatureCheckerFeature
        {
        //=======================================================================================
        // @bsistruct                                            Daryl.Holmwood      10/2017
        //=======================================================================================
        struct Feature
            {
            private:
                DRange3d m_range;
                bvector<DPoint3d> m_pts;

                //=======================================================================================
                // @bsimethod                                            Daryl.Holmwood      10/2017
                //=======================================================================================
                void FixLineString(bvector<DPoint3d>& featurePts)
                    {
                    size_t leftmost = -1;
                    size_t index = 0;

                    DTMDirection direction;
                    double area;
                    if (bcdtmMath_getPolygonDirectionP3D(featurePts.data(), (long)featurePts.size(), &direction, &area) != DTM_SUCCESS)
                        return;

                    if (direction == DTMDirection::AntiClockwise)
                        std::reverse(featurePts.begin(), featurePts.end() - 1);

                    for (DPoint3d& pt : featurePts)
                        {
                        if (leftmost == -1)
                            leftmost = index;
                        else if (pt.x < featurePts[leftmost].x)
                            leftmost = index;
                        else if (pt.x == featurePts[leftmost].x && pt.y < featurePts[leftmost].y)
                            leftmost = index;
                        index++;
                        }

                    if (leftmost == 0)
                        return;
                    bvector<DPoint3d> newPts;

                    newPts.insert(newPts.begin(), featurePts.begin() + leftmost, featurePts.end() - 1);
                    newPts.insert(newPts.end(), featurePts.begin(), featurePts.begin() + (leftmost - 1));
                    featurePts.swap(newPts);
                    }
            public:
                //=======================================================================================
                // @bsimethod                                            Daryl.Holmwood      10/2017
                //=======================================================================================
                Feature(bvector<DPoint3d>& pts) : m_pts(pts)
                    {
                    FixLineString(m_pts);
                    m_range.InitFrom(m_pts);
                    }

                //=======================================================================================
                // @bsimethod                                            Daryl.Holmwood      10/2017
                //=======================================================================================
                bool IsEqual(const Feature& testFeature) const
                    {
                    if (m_range.IsEqual(testFeature.m_range, 0.001) &&
                        DPoint3d::AlmostEqual(m_pts, testFeature.m_pts, 0.001))
                        return true;
                    return false;
                    }
            };
        bvector<Feature> m_features;

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood      10/2017
        //=======================================================================================
        void Initialize(DTMFeatureType featureType, BcDTMCR dtm)
            {
            DTMFeatureEnumerator features(dtm);
            features.ExcludeAllFeatures();
            features.IncludeFeature(featureType);
            bvector<DPoint3d> pts;

            for (const auto& feature : features)
                {
                feature.GetFeaturePoints(pts);
                m_features.push_back(Feature(pts));
                }
            }

        //=======================================================================================
        // @bsimethod                                            Daryl.Holmwood      10/2017
        //=======================================================================================
        bool IsDuplicate(bvector<DPoint3d>& featurePts) const
            {
            Feature testFeature(featurePts);

            for (auto& feature : m_features)
                {
                if (feature.IsEqual(testFeature))
                    return true;
                }
            return false;
            }
        };
    bmap<DTMFeatureType, DuplicateFeatureCheckerFeature> m_featureTypes;
    BcDTMCR m_dtm;

private:
    //=======================================================================================
    // @bsimethod                                            Daryl.Holmwood      10/2017
    //=======================================================================================
    bool DoesFeatureExistInternal( DTMFeatureType featureType, bvector<DPoint3d>& featurePts)
        {
        if (m_featureTypes.find(featureType) == m_featureTypes.end())
            m_featureTypes[featureType].Initialize(featureType, m_dtm);

        return m_featureTypes[featureType].IsDuplicate(featurePts);
        }
public:
    //=======================================================================================
    // @bsimethod                                            Daryl.Holmwood      10/2017
    //=======================================================================================
    DuplicateFeatureChecker(BcDTMCR dtm) : m_dtm(dtm)
        { }

    //=======================================================================================
    // @bsimethod                                            Daryl.Holmwood      10/2017
    //=======================================================================================
    bool DoesFeatureExist(DTMFeatureType featureType, bvector<DPoint3d>& featurePts)
        {
        if (DoesFeatureExistInternal(featureType, featurePts))
            return true;

        if (featureType == DTMFeatureType::Void && DoesFeatureExistInternal(DTMFeatureType::DrapeVoid, featurePts))
            return true;

        if (featureType == DTMFeatureType::Void && DoesFeatureExistInternal(DTMFeatureType::Hole, featurePts))
            return true;

        if (featureType == DTMFeatureType::Void && DoesFeatureExistInternal(DTMFeatureType::BreakVoid, featurePts))
            return true;

        return false;
        }
    };

//=======================================================================================
// @bsimethod                                            Daryl.Holmwood      10/2017
//=======================================================================================
BcDTMPtr TriangulationPreserver::Finish ()
    {
    if (m_stmDtm.IsNull ())
        return m_dtm;

    if (m_numPointsUsed != (int)m_pts.size())
        {
        bvector<int> m_ptMapper;
        m_ptMapper.resize(m_pts.size(), -1);
        int nextNum = 0;
        for (int i = 0; i < (int)m_pts.size(); i++)
            {
            if (m_pointUsed[i])
                {
                if (nextNum != i)
                    m_pts[nextNum] = m_pts[i];

                m_ptMapper[i] = nextNum;
                nextNum++;
                }
            }
        m_pts.resize(nextNum);
        for (auto& pt : m_pointIndex)
            {
            pt = m_ptMapper[pt];
            BeAssert(-1 != pt);
            }
        }

    bcdtmObject_storeTrianglesInDtmObject(m_stmDtm->GetTinHandle(), DTMFeatureType::GraphicBreak, m_pts.data(), (int)m_pts.size(), m_pointIndex.data(), (int)m_pointIndex.size() / 3);

    // Recreate the triangulation.
    if (bcdtmObject_triangulateStmTrianglesDtmObject(m_stmDtm->GetTinHandle()) != DTM_SUCCESS)
        {
        m_dtm->AddPoints(m_pts.data(), (int)m_pts.size());
        m_dtm->Triangulate();
        return m_dtm;
        }

    if (m_dtm->GetPointCount() == 0)
        return m_stmDtm;

    DuplicateFeatureChecker duplicateFeatureChecker(*m_dtm);
    // Add the void features to the TM.
    DTMFeatureEnumerator features (*m_stmDtm);
    bvector<DPoint3d> pts;
    // Add the Voids/Holes/Islands.
    for (const auto& feature : features)
        {
        DTMFeatureId id;
        feature.GetFeaturePoints (pts);

        if (pts.size () <= 2)
            continue;
        if (pts.size () == 3)
             {
            int side = bcdtmMath_sideOf (pts[0].x, pts[0].y, pts[1].x, pts[1].y, pts[2].x, pts[2].y);
            if (side == 0)
                continue;
            }

        DTMDirection direction;
        double area;
        if (bcdtmMath_getPolygonDirectionP3D (pts.data(), (long)pts.size (), &direction, &area) != DTM_SUCCESS)
            continue;

        if (area < MAXAREAFORVOIDORISLANDS)
            continue;


        if(!duplicateFeatureChecker.DoesFeatureExist(feature.FeatureType (), pts))
            m_dtm->AddLinearFeature (feature.FeatureType (), pts.data (), (int)pts.size (), &id);
        }

    DTMPointArray boundary;
    m_stmDtm->GetBoundary (boundary);

    if (!boundary.empty ())
        {
        DTMFeatureId id;
        BcDTMPtr boundaryFixerDtm = BcDTM::Create();

        boundaryFixerDtm->AddLinearFeature (DTMFeatureType::Breakline, boundary.data (), (int)boundary.size (), &id);
        DTMFeatureEnumerator voidFeatures (*m_dtm);
        voidFeatures.ExcludeAllFeatures();
        voidFeatures.IncludeFeature(DTMFeatureType::Void);
        voidFeatures.IncludeFeature(DTMFeatureType::DrapeVoid);
        voidFeatures.IncludeFeature(DTMFeatureType::BreakVoid);
        voidFeatures.IncludeFeature(DTMFeatureType::Hull);
        voidFeatures.IncludeFeature(DTMFeatureType::Island);
        voidFeatures.IncludeFeature(DTMFeatureType::Hole);

        bvector<DPoint3d> pts;
        // Add the Voids/Holes/Islands.
        for (const auto& feature : voidFeatures)
            {
            DTMFeatureId id;
            feature.GetFeaturePoints (pts);
            boundaryFixerDtm->AddLinearFeature(DTMFeatureType::Breakline, pts.data(), (int)pts.size(), &id);
            }
        boundaryFixerDtm->Triangulate();
        boundaryFixerDtm->RemoveNoneFeatureHullLines();

        boundaryFixerDtm->GetBoundary (boundary);
        m_dtm->RemoveHull();
        m_dtm->AddLinearFeature (DTMFeatureType::Hull, boundary.data (), (int)boundary.size (), &id);
        }

    // For now we will just add the Graphic Breaks, so no need to triangulate here.
    //// Triangulate the DTM.
    if (!m_useGraphicBreaks)
        {
        // Adds the points, most should already be in the DTM, if if there isn't then the triangulation wont be the same.
        m_dtm->AddPoints (m_pts);
        m_dtm->Triangulate ();
        }
    else
        {
        m_dtm->AddPoints(m_pts);
        }

    if (0)
        {
        DTMMeshEnumeratorPtr meshEnum = DTMMeshEnumerator::Create (*m_stmDtm);

        for (auto mesh : *meshEnum)
            {
            PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*mesh, false);

            if (m_useGraphicBreaks)
                {
                bvector<DPoint3d> &facePoint = visitor->Point ();
                for (visitor->Reset (); visitor->AdvanceToNextFace ();)
                    {
                    DTMFeatureId id;
                    m_dtm->AddLinearFeature (DTMFeatureType::GraphicBreak, facePoint.data (), (int)facePoint.size (), &id);
                    }
                }
            else
                {
                bvector<long> ptMapper;
                // Create
                DPoint3dCP pts = mesh->GetPointCP ();
                for (int i = 0; i < (int)mesh->GetPointCount (); i++)
                    {
                    long ptNum = 0;
                    if (bcdtmFind_closestPointDtmObject (m_dtmObj, pts[i].x, pts[i].y, &ptNum) == 1)
                        ptMapper[i] = ptNum;
                    else
                        {
                        // Error....
                        BeAssert (false);
                        }
                    }
                for (visitor->Reset (); visitor->AdvanceToNextFace ();)
                    {
                    const int* ptIndexes = visitor->GetClientPointIndexCP ();
                    long ptNums[3];
                    ptNums[0] = ptMapper[ptIndexes[0]];
                    ptNums[1] = ptMapper[ptIndexes[1]];
                    ptNums[2] = ptMapper[ptIndexes[2]];
                    CheckTriangle (ptNums, 3);
                    }
                }

            }
        }

    if (m_useGraphicBreaks)
        {
        // Triangulate the DTM.
        m_dtm->Triangulate ();
        return m_dtm;
        }

    long numPoints = m_dtmObj->numPoints;
    for (int i = 0; i < 10; i++)
        {
        bool hasSwapped = false;
        m_numLinkSwapFailed = 0;
        for (MissingLink& link : m_missingLinks)
            {
            if (!link.swapped)
                {
                bcdtmInsert_swapTinLinesThatIntersectInsertLineDtmObject (m_dtmObj, link.ptNums[0], link.ptNums[1], false);
                if (numPoints != m_dtmObj->numPoints)
                    numPoints = 0;
                if (!bcdtmList_testLineDtmObject (m_dtmObj, link.ptNums[0], link.ptNums[1]))
                    {
                    // Swap Failed.
                    m_numLinkSwapFailed++;
                    }
                else
                    {
                    link.swapped = true;
                    hasSwapped = true;
                    m_numLinksSwapped++;
                    }
                }
            }
        if (m_numLinkSwapFailed == 0 || !hasSwapped)
            break;
        }

    int m_failed2 = 0;
    BcDTMPtr l = BcDTM::Create ();
    for (MissingLink& link : m_missingLinks)
        {
        if (!bcdtmList_testLineDtmObject (m_dtmObj, link.ptNums[0], link.ptNums[1]))
            {
            DPoint3d pts[2];
            pts[0] = *pointAddrP (m_dtmObj, link.ptNums[0]);
            pts[1] = *pointAddrP (m_dtmObj, link.ptNums[1]);
            DTMFeatureId id;
            l->AddLinearFeature (DTMFeatureType::SoftBreakline, pts, 2, &id);
            m_failed2++;
            }
        }
//    l->Save (L"d:\\temp\\failedToSwap.tin");
//    m_dtm->Save (L"d:\\temp\\newDTM.tin");
    return m_dtm;
    }

//=======================================================================================
// @bsimethod                                            Daryl.Holmwood      10/2017
//=======================================================================================
void TriangulationPreserver::MatchEdges()
    {
    bmap<uint64_t, bool> edgeMap;

    for (size_t i = 0; i < m_pointIndex.size(); i++)
        {
        size_t endPointIndex = i + 1;
        if (i % 3 == 2) endPointIndex -= 3;

        int p1 = m_pointIndex[i];
        int p2 = m_pointIndex[endPointIndex];

        auto it = edgeMap.find((uint64_t)p2 << 32 | p1);

        if (it != edgeMap.end())
            {
            BeAssert(!it->second);
            it->second = true;
            }
        else
            {
            it = edgeMap.find((uint64_t)p1 << 32 | p2);

            if (it != edgeMap.end())
                BeAssert(false);
            else
                {
                edgeMap[(uint64_t)p1 << 32 | p2] = false;
                }
            }
        }

    BcDTMPtr joinDTM = BcDTM::Create();

    for (auto it : edgeMap)
        {
        if (it.second)
            continue;

        DTMFeatureId featureId;
        DPoint3d pts[2];
        pts[0] = m_pts[it.first >> 32];
        pts[1] = m_pts[it.first && 0xffffffff];

        joinDTM->AddLinearFeature(DTMFeatureType::Breakline, pts, 2, &featureId);
        }
    int nFeatures, nJoinedFatures;
    joinDTM->JoinFeatures(DTMFeatureType::Breakline, &nFeatures, &nJoinedFatures, 0);

    m_stmDtm = BcDTM::Create();

    // Add the void features to the TM.
    DTMFeatureEnumerator features(*joinDTM);
    bvector<DPoint3d> pts;
    // Add the Voids/Holes/Islands.
    for (const auto& feature : features)
        {
        DTMFeatureId id;
        feature.GetFeaturePoints(pts);

        if (pts.size() <= 2)
            continue;

        DTMDirection direction;
        double area;
        if (bcdtmMath_getPolygonDirectionP3D(pts.data(), (long)pts.size(), &direction, &area) != DTM_SUCCESS)
            continue;
        if (area < MAXAREAFORVOIDORISLANDS)
            continue;

        if (direction == DTMDirection::AntiClockwise)
            m_stmDtm->AddLinearFeature(DTMFeatureType::Island, pts.data(), (int)pts.size(), &id);
        if (direction == DTMDirection::Clockwise)
            m_stmDtm->AddLinearFeature(DTMFeatureType::Void, pts.data(), (int)pts.size(), &id);
        }


    }

//=======================================================================================
// @bsimethod                                            Daryl.Holmwood      10/2017
//=======================================================================================
void TriangulationPreserver::CheckTriangle (long* ptNums, int numPts)
    {
    int firstPt = ptNums[numPts - 1];
    for (int i = 0; i < 3; i++)
        {
        int nextPt = *ptNums++;
        m_numLinks++;
        if (firstPt == -1 || nextPt == -1)
            {
            m_numLinkErrors++;
            }
        else
            {
            if (!bcdtmList_testLineDtmObject (m_dtmObj, firstPt, nextPt))
                {
                m_missingLinks.push_back (MissingLink (firstPt, nextPt));
                }
            else
                {
                // Mark this edge as done.
                }
            firstPt = nextPt;
            }
        }
    }
