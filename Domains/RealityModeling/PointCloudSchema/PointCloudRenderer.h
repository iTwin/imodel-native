/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudRenderer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

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

        void            DrawPointBuffer(ViewContextR context, PointCloudDrawParams& buffer) const;
        void            ApplyClassification(BePointCloud::PointCloudQueryBuffers& channels, LasClassificationInfo const* pClassifInfo, Dgn::ViewContextR context) const;

        uint32_t        m_outputCapacity;
    public:
        PointCloudRenderer (uint32_t outputCapacity);
        virtual ~PointCloudRenderer();

        Dgn::ProgressiveTask::Completion DrawPointCloud(Dgn::ViewContextR context, LasClassificationInfo const* pClassifInfo, BePointCloud::PointCloudSceneCR pointCloudScene);
    };  //  PointCloudRenderer

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
