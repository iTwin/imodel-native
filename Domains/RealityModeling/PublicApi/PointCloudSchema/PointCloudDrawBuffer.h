/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudDrawBuffer.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

struct PointCloudDrawParams;
struct PointCloudDrawBuffer;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudDrawBuffer : public Dgn::IPointCloudDrawParams
    {
    private:
        virtual uint32_t                                _GetCapacity() = 0;                     // Number of points
        virtual BePointCloud::PointCloudRgbChannel*     _GetRgbChannel() = 0;
        virtual BePointCloud::PointCloudXyzChannel*     _GetPointChannel() = 0;
        virtual void                                    _SetNumPoints(uint32_t numPoints) = 0;
        virtual void                                    _InitFrom(uint32_t begin, uint32_t end, Transform const* pTransform, PointCloudDrawBuffer* source) = 0;
        virtual void                                    _ChangeCapacity(uint32_t capacity) = 0;   // Number of points
        virtual void                                    _SetIgnoreColor(bool ignoreColor) = 0;     // Use the RGB color in the PointCloud or Not
        virtual void                                    _Deallocate() = 0;

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

/*=================================================================================**//**
* @bsiclass                                                     John.Gooding    02/2009
+===============+===============+===============+===============+===============+======*/
struct PointCloudDrawParams : public PointCloudDrawBuffer
    {
    private:
        BeAtomic<uint32_t>                                  m_refCount;
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
        // IPointCloudDrawParams
        virtual uint32_t            AddRef () override;
        virtual uint32_t            Release () override;
        virtual bool                IsThreadBound () override               { return false; }
        virtual bool                GetRange (DPoint3d*range) override      { return false; }
        virtual bool                GetOrigin (DPoint3d*origin) override    { return false; }
        virtual Dgn::ColorDef const*     
                                    GetRgbColors () override                { return (m_ignoreColor || m_colors.IsNull()) ? NULL : reinterpret_cast<Dgn::ColorDef const*>(m_colors->GetChannelBuffer()); } 

        virtual uint32_t            GetNumPoints () override                { return m_points.IsValid() ? (uint32_t)m_points->GetSize() : 0; }
        virtual DPoint3dCP          GetDPoints () override                  { return m_points->GetChannelBuffer(); }
        virtual FPoint3dCP          GetFPoints () override                  { return NULL; } 

        // PointCloudDrawBuffer
        virtual void                            _Deallocate() override                      { delete this; }
        uint32_t                                _GetCapacity () override                    { return m_points.IsValid() ? (uint32_t)m_points->GetCapacity() : 0; }
        BePointCloud::PointCloudRgbChannel*     _GetRgbChannel () override                  { return m_colors.get(); }
        BePointCloud::PointCloudXyzChannel*     _GetPointChannel () override                { return m_points.get(); }
        void                                    _SetNumPoints (uint32_t numPoints) override;
        void                                    _InitFrom(uint32_t begin, uint32_t end, Transform const* pTransform, PointCloudDrawBuffer* source) override;
        void                                    _ChangeCapacity(uint32_t capacity) override;
        void                                    _SetIgnoreColor(bool ignoreColor) override     { m_ignoreColor = ignoreColor; }

        // PointCloudDrawParams
        POINTCLOUDSCHEMA_EXPORT static PointCloudDrawParams* Create(BePointCloud::PointCloudXyzChannel* pXyzChannel, BePointCloud::PointCloudRgbChannel* pRgbChannel);

    }; // PointCloudDrawParams

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
