/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudDrawBuffer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__
#if defined (NEEDS_WORK_POINT_CLOUD)

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     John.Gooding    02/2009
+===============+===============+===============+===============+===============+======*/
struct PointCloudDrawParams : public Dgn::Render::PointCloudDraw
    {
    private:
        RefCountedPtr<BePointCloud::PointCloudXyzChannel>   m_points;   // Mandatory, 24 bytes per points
        RefCountedPtr<BePointCloud::PointCloudRgbChannel>   m_colors;   // 3 bytes per points 
        bool                                                m_ignoreColor;

        PointCloudDrawParams(); // Undefined
        PointCloudDrawParams(PointCloudDrawParams const&); // disabled
        PointCloudDrawParams& operator=(PointCloudDrawParams const&); // Undefined

        void ValidateCapacity(uint32_t capacity);

   protected:
        PointCloudDrawParams (BePointCloud::PointCloudXyzChannel* pXyzChannel, BePointCloud::PointCloudRgbChannel* pRgbChannel);
        virtual ~PointCloudDrawParams ();

    public:
        // Dgn::PointCloudDraw
        virtual bool                _IsThreadBound () override               { return false; }
        virtual bool                _GetRange (DPoint3d*range) override      { return false; }
        virtual bool                _GetOrigin (DPoint3d*origin) override    { return false; }
        virtual Dgn::ColorDef const* _GetRgbColors () override               { return (m_ignoreColor || m_colors.IsNull()) ? NULL : reinterpret_cast<Dgn::ColorDef const*>(m_colors->GetChannelBuffer()); } 
        virtual uint32_t            _GetNumPoints() override                { return m_points.IsValid() ? (uint32_t)m_points->GetSize() : 0; }
        virtual DPoint3dCP          _GetDPoints() override                  { return m_points->GetChannelBuffer(); }
        virtual FPoint3dCP          _GetFPoints() override                  { return NULL; }


        uint32_t                                GetCapacity() { return m_points.IsValid() ? (uint32_t) m_points->GetCapacity() : 0; }
        BePointCloud::PointCloudRgbChannel*     GetRgbChannel() { return m_colors.get(); }
        BePointCloud::PointCloudXyzChannel*     GetPointChannel() { return m_points.get(); }
        void                                    SetNumPoints(uint32_t numPoints);
        void                                    ChangeCapacity(uint32_t capacity);
        void                                    SetIgnoreColor(bool ignoreColor) { m_ignoreColor = ignoreColor; }

        static RefCountedPtr<PointCloudDrawParams> Create(BePointCloud::PointCloudXyzChannel* pXyzChannel, BePointCloud::PointCloudRgbChannel* pRgbChannel);

    }; // PointCloudDrawParams


END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE


#endif
