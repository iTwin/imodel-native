/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/GTests/SMUnitTestDisplayQuery.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define NOMINMAX

#include <Bentley/Bentley.h>
#include <Bentley/BeTest.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshProgressiveQuery.h>
#include "SMUnitTestUtil.h"

USING_NAMESPACE_BENTLEY_SCALABLEMESH

class DisplayQueryTester
    {
    private : 

        double           m_minScreenPixelsPerPoint = 800;
        double           m_maxPixelError = 1;
        double           m_rootToViewMatrix[4][4];
        ClipVectorPtr    m_clipVector;
        bool             m_displayTexture;
        IScalableMeshPtr m_smPtr;
        bool             m_waitQueryComplete = true;        
        bvector<double>  m_expectedResults;


        IScalableMeshDisplayCacheManagerPtr     m_displayCacheManager;
        IScalableMeshProgressiveQueryEnginePtr  m_progressiveQueryEngine;

        IScalableMeshProgressiveQueryEnginePtr GetProgressiveQueryEngine();

        void VerifyDisplayNodeFunctions(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes) const;
        void VerifyCurrentlyViewedNodesFunctions(bvector<IScalableMeshCachedDisplayNodePtr>& meshNodes);

    public : 

        DisplayQueryTester();

        virtual ~DisplayQueryTester();

        bool SetQueryParams(const BeFileName& smFileName, const DMatrix4d& rootToView, const bvector<DPoint4d>& clipPlanes, const bvector<double>& expectedResults);

        void DoQuery();

        void SetActiveClips(const bset<uint64_t>& clips);
        void GetActiveClips(bset<uint64_t>& clips);

    };

class ScalableMeshTestDisplayQuery : public ::testing::TestWithParam<std::tuple<BeFileName, DMatrix4d, bvector<DPoint4d>, bvector<double>>>
    {
    protected:
        BeFileName        m_filename;
        DMatrix4d         m_rootToViewMatrix;
        bvector<DPoint4d> m_clipPlanes;
        bvector<double>   m_expectedResults;

    public:

        virtual void SetUp() {
            auto paramList = GetParam();
            m_filename = std::get<0>(paramList);
            m_rootToViewMatrix = std::get<1>(paramList);
            m_clipPlanes = std::get<2>(paramList);
            m_expectedResults = std::get<3>(paramList);
            }

        virtual void TearDown() { }
        BeFileName GetFileName() { return m_filename; }
        const DMatrix4d& GetRootToViewMatrix() { return m_rootToViewMatrix; }
        const bvector<DPoint4d>& GetClipPlanes() { return m_clipPlanes; }
        bvector<double>& GetExpectedResults() { return m_expectedResults; }

        ScalableMeshGTestUtil::SMMeshType GetType() { return ScalableMeshGTestUtil::GetFileType(m_filename); }


    };
