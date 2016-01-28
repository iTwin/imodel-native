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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        void            DrawPointBuffer(ViewContextR context, PointCloudDrawBuffer* buffer) const;
#endif
        void            ApplyClassification(BePointCloud::PointCloudQueryBuffers& channels, LasClassificationInfo const* pClassifInfo, Dgn::ViewContextR context) const;

        uint32_t        m_outputCapacity;
    public:
        POINTCLOUDSCHEMA_EXPORT PointCloudRenderer (uint32_t outputCapacity);
        POINTCLOUDSCHEMA_EXPORT virtual ~PointCloudRenderer();

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        POINTCLOUDSCHEMA_EXPORT IProgressiveDisplay::Completion DrawPointCloud(Dgn::ViewContextR context, LasClassificationInfo const* pClassifInfo, BePointCloud::PointCloudSceneCR pointCloudScene);
#endif
    };  //  PointCloudRenderer

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
