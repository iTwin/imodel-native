/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudProgressiveDisplay.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnViewport.h>

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

struct PtViewport;

//========================================================================================
// @bsiclass                                                        Eric.Paquet     5/2015
//========================================================================================
struct PointCloudProgressiveDisplay : Dgn::ProgressiveTask
    {
    friend struct PointCloudModel;

private:
    RefCountedPtr<PtViewport>   m_ptViewport;        
    DRange3d    m_sceneRangeWorld;      // point could scene range in World unit.
    uint64_t    m_nextRetryTime;                             //!< When to re-try to query points. unix millis UTC
    uint64_t    m_waitTime;                                  //!< How long to wait before re-trying to query points. millis 
    uint32_t    m_tentativeId;
    bool        m_doLowDensity;
    bool        m_lastTentativeStopped;
    uint64_t    m_firstPassPts;         // low density quick draw
    uint64_t    m_progressivePts;       // the last progressive display iteration. Usually the one that completed
    uint64_t    m_totalProgressivePts;  // Total number of points drawn 

    static bool ShouldDrawInContext(Dgn::RenderContextR context);

    void SetupPtViewport(Dgn::RenderContextR context);

    bool DrawPointCloud(Render::GraphicPtr* pGraphicsPtr, int64_t& pointToLoad, uint64_t& pointsDrawn, Dgn::RenderContextR context, BePointCloud::PtQueryDensity densityType, float density, bool doCheckStop);

protected:
    PointCloudModel const& m_model;

    //! Displays point cloud and schedules downloads. 
    virtual Completion _DoProgressive(Dgn::ProgressiveContext& context, WantShow&) override;

    void DrawView(Dgn::RenderContextR);

    PointCloudProgressiveDisplay(PointCloudModel const& model, PtViewport&);
    virtual ~PointCloudProgressiveDisplay();
    };

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
