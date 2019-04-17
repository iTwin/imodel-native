/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_POINTCLOUD_NAMESPACE

#define DRAW_OUTPUTCAPACITY     (1048576)
#define DRAW_QUERYCAPACITY      (1048576)

//========================================================================================
// @bsiclass                                                        Eric.Paquet     2/2015
//========================================================================================
struct PointCloudRenderer 
    {
    private:
        PointCloudRenderer(); // disabled
        PointCloudRenderer(PointCloudRenderer const&); // disabled

#if defined (NEEDS_WORK_POINT_CLOUD)
        void            DrawPointBuffer(ViewContextR context, PointCloudDrawParams& buffer) const;
#endif
        void            ApplyClassification(BePointCloud::PointCloudQueryBuffers& channels, PointCloudClassificationSettings const* pClassifInfo, Dgn::ViewContextR context) const;

        uint32_t        m_outputCapacity;
    public:
        PointCloudRenderer (uint32_t outputCapacity);
        virtual ~PointCloudRenderer();

#if defined (NEEDS_WORK_POINT_CLOUD)
        Dgn::ProgressiveTask::Completion DrawPointCloud(Dgn::ViewContextR context, PointCloudClassificationSettings const* pClassifInfo, BePointCloud::PointCloudSceneCR pointCloudScene);
#endif
    };  //  PointCloudRenderer

END_BENTLEY_POINTCLOUD_NAMESPACE
