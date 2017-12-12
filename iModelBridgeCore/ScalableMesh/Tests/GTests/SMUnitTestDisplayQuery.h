/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/GTests/SMUnitTestDisplayQuery.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshProgressiveQuery.h>

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

    };
