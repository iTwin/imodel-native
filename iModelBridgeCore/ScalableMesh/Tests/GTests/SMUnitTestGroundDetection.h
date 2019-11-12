/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#define NOMINMAX

#include <Bentley/Bentley.h>
#include <Bentley/BeTest.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshProgressiveQuery.h>
#include "SMUnitTestUtil.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

class GroundDetectionTester
    {
    private : 

        IScalableMeshPtr  m_smPtr;
        bvector<DPoint3d> m_groundArea;
        uint64_t          m_expectedGroundPts;

/*
        double           m_minScreenPixelsPerPoint = 800;
        double           m_maxPixelError = 1;
        double           m_rootToViewMatrix[4][4];
        ClipVectorPtr    m_clipVector;
        bool             m_displayTexture;
        
        bool             m_waitQueryComplete = true;        
        bvector<double>  m_expectedResults;


        IScalableMeshDisplayCacheManagerPtr     m_displayCacheManager;
        IScalableMeshProgressiveQueryEnginePtr  m_progressiveQueryEngine;

        IScalableMeshProgressiveQueryEnginePtr GetProgressiveQueryEngine();

        void VerifyDisplayNodeFunctions(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes) const;
        void VerifyCurrentlyViewedNodesFunctions(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes);
*/
    public : 

        GroundDetectionTester();

        virtual ~GroundDetectionTester();

        bool SetQueryParams(const BeFileName& smFileName, const bvector<DPoint3d>& groundArea, uint64_t expectedGroundPts);

        void DoDetection();
/*
        void SetActiveClips(const bset<uint64_t>& clips);
        void GetActiveClips(bset<uint64_t>& clips);
*/

    };

class ScalableMeshTestGroundDetection : public ::testing::TestWithParam<std::tuple<BeFileName, bvector<DPoint3d>, uint64_t>>
    {
    protected:

        BeFileName        m_filename;
        bvector<DPoint3d> m_groundArea;
        uint64_t          m_expectedGroundPts;
        
    public:

        virtual void SetUp() {
            auto paramList = GetParam();
            m_filename = std::get<0>(paramList);
            m_groundArea = std::get<1>(paramList);
            m_expectedGroundPts = std::get<2>(paramList);            
            }

        virtual void TearDown() { }
        BeFileName GetFileName() { return m_filename; }
        const bvector<DPoint3d>& GetGroundArea() { return m_groundArea; }
        uint64_t GetExpectedGroundPts() { return m_expectedGroundPts; }

        //ScalableMeshGTestUtil::SMMeshType GetType() { return ScalableMeshGTestUtil::GetFileType(m_filename); }


    };
