#pragma once

#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

class TriangulationPreserver
    {
    BcDTMPtr m_dtm;
    BC_DTM_OBJ* m_dtmObj;
    BcDTMPtr m_stmDtm;
    bvector<DPoint3d> m_pts;
    bvector<int> m_ptIdTolocalId;
    bvector<int> m_pointIndex;
    std::vector<bool> m_pointUsed;
    int m_numPointsUsed;
    bool m_useGraphicBreaks;

    struct MissingLink
        {
        int ptNums[2];
        bool swapped;

        MissingLink ()
            {
            swapped = false;
            }
        MissingLink (int pt1, int pt2)
            {
            swapped = false;
            ptNums[0] = pt1;
            ptNums[1] = pt2;
            }
        };
    bvector<MissingLink> m_missingLinks;

    int m_numLinks;
    int m_numLinkErrors;
    int m_numLinkSwapFailed;
    int m_numLinksSwapped;
    public:
        TriangulationPreserver (BcDTMR dtm);

        void Initialize ();
        void AddPoints (DPoint3dCP pts, int numPoints, int firstId = 0);
        void AddPoint (DPoint3dCR pt, int ptId);
        void AddTriangle (int* ptNums, int numPoints);

        bool HasPoint (int ptId)
            {
            if (ptId < 0 || ptId >= (int)m_ptIdTolocalId.size ())
                return false;
            return m_ptIdTolocalId[ptId] != -1;
            }
        int GetLocalId (int ptId)
            {
            BeAssert (ptId >= 0 && ptId < (int)m_ptIdTolocalId.size ());
            if (ptId < 0 || ptId >= (int)m_ptIdTolocalId.size ())
                return -1;
            BeAssert (m_ptIdTolocalId[ptId] != -1);
            return m_ptIdTolocalId[ptId];
            }
        //int FindPointNum (DPoint3dCR pt);
        //void AddTriangle (DPoint3dCP pts, int numPts);
        //void AddTriangle (const long* pts, int numPts);
        BcDTMPtr Finish ();

        void CheckTriangle (long* ptNums, int numPoints);
    };

END_BENTLEY_TERRAINMODEL_NAMESPACE
