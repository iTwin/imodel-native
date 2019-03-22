/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudViewport.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_POINTCLOUD_NAMESPACE

#define PTVORTEX_MAX_VIEWPORTS      256

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
struct PtViewport : RefCountedBase
{
private:
    int32_t m_id;

    PtViewport(int32_t id);
    ~PtViewport();

public:

    //! Create a new viewport may return null if no more viewport id are available.
    static RefCountedPtr<PtViewport> Create();

    int32_t GetId() const { return m_id; }
};

END_BENTLEY_POINTCLOUD_NAMESPACE
