/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudRenderer.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

        void            DrawPointBuffer(ViewContextR context, PointCloudDrawBuffer* buffer) const;
        void            ApplyClassification(BePointCloud::PointCloudQueryBuffers& channels, LasClassificationInfo const* pClassifInfo, ViewContextR context) const;

        uint32_t        m_outputCapacity;
    public:
        POINTCLOUDSCHEMA_EXPORT PointCloudRenderer (uint32_t outputCapacity);
        POINTCLOUDSCHEMA_EXPORT virtual ~PointCloudRenderer();

        POINTCLOUDSCHEMA_EXPORT BentleyApi::DgnPlatform::IProgressiveDisplay::Completion DrawPointCloud(ViewContextR context, LasClassificationInfo const* pClassifInfo, PointCloudSceneCR pointCloudScene);
    };  //  PointCloudRenderer

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
