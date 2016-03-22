/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudDrawBuffer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE


#if 0 //&&MM remove me
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudDrawBuffer : public Dgn::PointCloudDraw
    {
    private:
        virtual uint32_t                                _GetCapacity() = 0;                     // Number of points
        virtual BePointCloud::PointCloudRgbChannel*     _GetRgbChannel() = 0;
        virtual BePointCloud::PointCloudXyzChannel*     _GetPointChannel() = 0;
        virtual void                                    _SetNumPoints(uint32_t numPoints) = 0;
        virtual void                                    _InitFrom(uint32_t begin, uint32_t end, Transform const* pTransform, PointCloudDrawBuffer* source) = 0;
        virtual void                                    _ChangeCapacity(uint32_t capacity) = 0;   // Number of points
        virtual void                                    _SetIgnoreColor(bool ignoreColor) = 0;     // Use the RGB color in the PointCloud or Not

    protected:
                                                        PointCloudDrawBuffer();
        virtual                                         ~PointCloudDrawBuffer();

    public:
        void                                                            Deallocate();
        POINTCLOUDSCHEMA_EXPORT uint32_t                               GetCapacity();
        POINTCLOUDSCHEMA_EXPORT BePointCloud::PointCloudRgbChannel*    GetRgbChannel();
        POINTCLOUDSCHEMA_EXPORT BePointCloud::PointCloudXyzChannel*    GetPointChannel();
        POINTCLOUDSCHEMA_EXPORT void                                   SetNumPoints(uint32_t numPoints);
        POINTCLOUDSCHEMA_EXPORT void                                   InitFrom(uint32_t begin, uint32_t end, Transform const* pTransform, PointCloudDrawBuffer* source);
        POINTCLOUDSCHEMA_EXPORT void                                   ChangeCapacity(uint32_t capacity);
        POINTCLOUDSCHEMA_EXPORT void                                   SetIgnoreColor(bool ignoreColor);
    };
#endif

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
        //&&MM not_now void                                    InitFrom(uint32_t begin, uint32_t end, Transform const* pTransform, PointCloudDrawBuffer* source);
        void                                    ChangeCapacity(uint32_t capacity);
        void                                    SetIgnoreColor(bool ignoreColor) { m_ignoreColor = ignoreColor; }

        static RefCountedPtr<PointCloudDrawParams> Create(BePointCloud::PointCloudXyzChannel* pXyzChannel, BePointCloud::PointCloudRgbChannel* pRgbChannel);

    }; // PointCloudDrawParams


END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
