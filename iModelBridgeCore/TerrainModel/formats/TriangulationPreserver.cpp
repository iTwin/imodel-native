/*--------------------------------------------------------------------------------------+
|
|     $Source: formats/TriangulationPreserver.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <TerrainModel/Formats/Formats.h>
#include "TriangulationPreserver.h"
#include <TerrainModel/Core/bcdtmInlines.h>
#include <TerrainModel/Core/DTMIterators.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

TriangulationPreserver::TriangulationPreserver (BcDTMR dtm) : m_dtm (&dtm), m_dtmObj (nullptr), m_stmDtm (nullptr)
    {
    m_useGraphicBreaks = true;
    m_numLinks = 0;
    m_numLinkErrors = 0;
    m_numLinkSwapFailed = 0;
    m_numLinksSwapped = 0;
    }

void TriangulationPreserver::Initialize ()
    {
    if (m_stmDtm.IsNull ())
        m_stmDtm = BcDTM::Create ();
    }

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
    }

void TriangulationPreserver::AddPoint (DPoint3dCR pt, int ptId)
    {
    AddPoints (&pt, 1, ptId);
    }

void TriangulationPreserver::AddTriangle (int* ptNums, int numPoints)
    {
    Initialize ();
    if (numPoints != 3)
        return;

    DPoint3d pts[4];
    pts[0] = m_pts[GetLocalId (ptNums[0])];
    pts[1] = m_pts[GetLocalId (ptNums[1])];
    pts[2] = m_pts[GetLocalId (ptNums[2])];
    pts[3] = pts[0];

    DTMUserTag dtmUserTag = 0;
    DTMFeatureId id;
//    m_dtm->AddLinearFeature (DTMFeatureType::Breakline, pts, 4, dtmUserTag, &id);
    m_stmDtm->AddLinearFeature (DTMFeatureType::GraphicBreak, pts, 4, dtmUserTag, &id);
    }

void TriangulationPreserver::Finish ()
    {
    if (m_stmDtm.IsNull ())
        return;

    // Recreate the triangulation.
    bcdtmObject_triangulateStmTrianglesDtmObject (m_stmDtm->GetTinHandle ());

    // Add the void features to the TM.
    DTMFeatureEnumerator features (*m_stmDtm);
    bvector<DPoint3d> pts;

    // Add the Voids/Holes/Islands.
    for (const auto& feature : features)
        {
        DTMFeatureId id;
        feature.GetFeaturePoints (pts);
        m_dtm->AddLinearFeature (feature.FeatureType (), pts.data (), (int)pts.size (), &id);
        }

    DTMPointArray boundary;
    m_stmDtm->GetBoundary (boundary);

    if (!boundary.empty ())
        {
        DTMFeatureId id;
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

    DTMMeshEnumerator meshEnum (*m_stmDtm);

    for (auto mesh : meshEnum)
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

    if (m_useGraphicBreaks)
        {
        // Triangulate the DTM.
        m_dtm->Triangulate ();
        return;
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
    }

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
