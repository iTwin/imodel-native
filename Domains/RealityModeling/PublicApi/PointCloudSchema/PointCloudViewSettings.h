/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudViewSettings.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------+
|   NOTE: This file was moved from $(SrcRoot)MstnPlatform\PPModules\PointCloud\Component\PointCloudHandler\PointCloudViewSettings.h
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <BePointCloud/PointCloudColorDef.h>

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

#define CLASSIFCATION_COUNT    (32)
#define CLASSIFCATION_BIT_MASK (0x1F)
#define CLASSIFCATION_MAXDISPLAYCOUNT 13

#define POINTCLOUD_DEFAULT_VIEW_CONTRAST    (50.0f)
#define POINTCLOUD_DEFAULT_VIEW_BRIGHTNESS  (180.0f)

#define VIEWSETTINGS_RGB_MASK        (0x00000001)
#define VIEWSETTINGS_INTENSITY_MASK  (0x00000002)
#define VIEWSETTINGS_LIGHTNING_MASK  (0x00000004)
#define VIEWSETTINGS_PLANE_MASK      (0x00000008)
#define VIEWSETTINGS_FRONTBIAS_MASK  (0x00000010)


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct LasClassificationInfo
    {
    public:
        /*---------------------------------------------------------------------------------**//**
        * This ctor leaves the object unitialized.
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        LasClassificationInfo()
            {
            }

        /*---------------------------------------------------------------------------------**//**
        * visibleState contains the visibility state of the 32 classification categories.
        * classificationColor is a pointer to an array of 32 ColorDef
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        LasClassificationInfo(uint32_t visibleState, PointCloudColorDefP classificationColor)
            {
            m_visibleState = visibleState;
            m_blendColor = false;
            m_useBasecolor= false;
            memcpy(m_classificationColor, classificationColor, CLASSIFCATION_COUNT * sizeof(BePointCloud::PointCloudColorDef));
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        ~LasClassificationInfo()
            {
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        uint32_t GetClassificationStates() const 
            { 
            return m_visibleState; 
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        unsigned char* GetClassificationStatesAdv() const 
            { 
            return (unsigned char*)&m_visibleStateAdv[0]; 
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassificationStates(uint32_t states) 
            { 
            m_visibleState = states; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie  08/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool GetClassificationState(unsigned char idx) const    
            { 
            int arrayPosition = idx / 8;

            int positionBitField = idx % 8;

            return TO_BOOL(m_visibleStateAdv[arrayPosition] & 0x00000001 << positionBitField); 

            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie  08/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassificationState(unsigned char idx, bool state)    
            { 
            int arrayPosition = idx / 8;

            int positionBitField = idx % 8;



            if (state)

                m_visibleStateAdv[arrayPosition] |= (0x00000001 << positionBitField); 

            else

                m_visibleStateAdv[arrayPosition] &= ~(0x00000001 << positionBitField);

            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        PointCloudColorDefCP GetClassificationColors() const 
            {
            return m_classificationColor; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassificationColors(PointCloudColorDefCP classificationColors) 
            {
            memcpy(m_classificationColor, classificationColors, CLASSIFCATION_COUNT * sizeof(BePointCloud::PointCloudColorDef));
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassificationVisibleStatesAdv(unsigned char const* advState) 
            { 
            memcpy(m_visibleStateAdv, advState, 32 * sizeof(unsigned char));
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        PointCloudColorDefCR GetClassificationColor(unsigned char idx) const 
            {
            return m_classificationColor[idx]; 

            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassificationColor(unsigned char idx, BePointCloud::PointCloudColorDef& color)

            {

            m_classificationColor[idx] = color; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool const GetBlendColor() const 
            {
            return m_blendColor;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetBlendColor(bool use)
            {
            m_blendColor = use;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool const GetUseBaseColor() const 
            {
            return m_useBasecolor;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetUseBaseColor(bool use)
            {
            m_useBasecolor = use;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool GetClassActiveState(unsigned char idx) const    
            { 
            int arrayPosition = idx / 8;
            int positionBitField = idx % 8;
            return TO_BOOL(m_activeStateAdv[arrayPosition] & 0x00000001 << positionBitField); 
            }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetClassActiveState(unsigned char idx, bool state)    
            { 
            int arrayPosition = idx / 8;
            int positionBitField = idx % 8;

            if (state)
                m_activeStateAdv[arrayPosition] |= (0x00000001 << positionBitField); 
            else
                m_activeStateAdv[arrayPosition] &= ~(0x00000001 << positionBitField);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetUnclassState(bool use)
            {
            m_unclassVisible = use;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    Daniel.McKenzie 05/2012
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool GetUnclassState() const 
            {
            return m_unclassVisible;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        BePointCloud::PointCloudColorDef GetUnclassColor() const 
            {
            return m_unclassColor; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetUnclassColor(BePointCloud::PointCloudColorDef& color)
            {
            m_unclassColor = color; 
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void InitDefault()
            {
            m_blendColor = false;
            m_useBasecolor = false;
            m_visibleState = 0xFFFFFFFF;  // sets all to visible

            m_classificationColor[0].SetRed(255);   m_classificationColor[0].SetGreen(255); m_classificationColor[0].SetBlue(0);    //    eCreated = 0,
            m_classificationColor[1].SetRed(255);   m_classificationColor[1].SetGreen(255); m_classificationColor[1].SetBlue(255);  //    eUnclassified,
            m_classificationColor[2].SetRed(255);   m_classificationColor[2].SetGreen(0);   m_classificationColor[2].SetBlue(0);    //    eGround,
            m_classificationColor[3].SetRed(0);     m_classificationColor[3].SetGreen(200); m_classificationColor[3].SetBlue(50);   //    eLowVegetation,
            m_classificationColor[4].SetRed(0);     m_classificationColor[4].SetGreen(200); m_classificationColor[4].SetBlue(100);  //    eMediumVegetation,
            m_classificationColor[5].SetRed(0);     m_classificationColor[5].SetGreen(200); m_classificationColor[5].SetBlue(200);  //    eHighVegetation,
            m_classificationColor[6].SetRed(150);   m_classificationColor[6].SetGreen(150); m_classificationColor[6].SetBlue(150);  //    eBuilding,
            m_classificationColor[7].SetRed(0);     m_classificationColor[7].SetGreen(50);  m_classificationColor[7].SetBlue(100);  //    eLowPoint,
            m_classificationColor[8].SetRed(255);   m_classificationColor[8].SetGreen(255); m_classificationColor[8].SetBlue(255);  //    eModelKeyPoint,
            m_classificationColor[9].SetRed(0);     m_classificationColor[9].SetGreen(0);   m_classificationColor[9].SetBlue(255);  //    eWater = 9,
            m_classificationColor[10].SetRed(0);    m_classificationColor[10].SetGreen(0);  m_classificationColor[10].SetBlue(0);   // = 10 // reserved for ASPRS Definition
            m_classificationColor[11].SetRed(0);    m_classificationColor[11].SetGreen(0);  m_classificationColor[11].SetBlue(0);   // = 11 // reserved for ASPRS Definition
            m_classificationColor[12].SetRed(0);    m_classificationColor[12].SetGreen(0);  m_classificationColor[12].SetBlue(0);   //    eOverlapPoints = 12

            // init classification color vector
            memset (&m_classificationColor[13], 255, (CLASSIFCATION_COUNT - 13) * sizeof(BePointCloud::PointCloudColorDef)); // all white
            memset (&m_visibleStateAdv[0], 0xFFFFFFFF, 32 * sizeof(unsigned char)); // all to visible
            memset (&m_activeStateAdv[0], 0xFFFFFFFF, 32 * sizeof(unsigned char)); // all to visible

            m_unclassVisible = true;

            m_unclassColor.SetRed(255); m_unclassColor.SetGreen(0); m_unclassColor.SetBlue(255);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                                    StephanePoulin  12/2009
        +---------------+---------------+---------------+---------------+---------------+------*/
        void Reset()
            {
            m_visibleState = 0xFFFFFFFF;  // sets all to visible
            memset (m_classificationColor, 255, CLASSIFCATION_COUNT * sizeof(BePointCloud::PointCloudColorDef)); // all white

            m_blendColor = false;

            m_useBasecolor = false;

            memset (&m_visibleStateAdv[0], 0xFFFFFFFF, 32 * sizeof(unsigned char)); // all to visible

            memset (&m_activeStateAdv[0], 0xFFFFFFFF, 32 * sizeof(unsigned char)); // all to visible

            m_unclassVisible = false;

            }

        private:
            uint32_t            m_visibleState;

            BePointCloud::PointCloudColorDef  m_classificationColor[CLASSIFCATION_COUNT];

            bool                m_blendColor;

            bool                m_useBasecolor;

            unsigned char       m_visibleStateAdv[32];

            unsigned char       m_activeStateAdv[32];

            BePointCloud::PointCloudColorDef  m_unclassColor;

            bool                m_unclassVisible;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudClassificationViewSettings : public RefCounted<IPointCloudClassificationViewSettings>
    {
    public:
        static PointCloudClassificationViewSettingsPtr Create(int viewIndex) { return new PointCloudClassificationViewSettings(viewIndex); }
        static PointCloudClassificationViewSettingsPtr Create(ViewContextCR context) { return new PointCloudClassificationViewSettings(context); }

        // from IPointCloudClassificationViewSettings
        bool                        _GetState(unsigned char idx) const override;
        void                        _SetState(unsigned char idx, bool state) override;
        BePointCloud::PointCloudColorDef const&   _GetColor(unsigned char idx) const override;

        void                        _SetColor(unsigned char idx, BePointCloud::PointCloudColorDef& color) override;
        unsigned char               _GetLastIndex () const override {return CLASSIFCATION_MAXDISPLAYCOUNT-1;}

        LasClassificationInfo*      GetLasClassificationInfo() { return &m_data; }

        //Added for advanced display

        bool                        _GetBlendColor() const override;
        void                        _SetBlendColor(bool use) override;
        bool                        _GetUseBaseColor() const override;
        void                        _SetUseBaseColor(bool use) override;
        BePointCloud::PointCloudColorDef          _GetUnclassColor() const override;
        void                        _SetUnclassColor(BePointCloud::PointCloudColorDef& color) override;
        bool                        _GetUnclassState() override;
        void                        _SetUnclassState(bool state) override;

        bool                        _GetClassActiveState(unsigned char idx) override;

        void                        _SetClassActiveState(unsigned char idx, bool state) override;


    private:
        PointCloudClassificationViewSettings (int viewIndex);
        PointCloudClassificationViewSettings (ViewContextCR context);
        ~PointCloudClassificationViewSettings ();

        PointCloudClassificationViewSettings(); // disabled
        PointCloudClassificationViewSettings(IPointCloudClassificationViewSettings const&); // disabled
        IPointCloudClassificationViewSettings& operator=(IPointCloudClassificationViewSettings const&); // disabled
        
        void CreateFromView(int viewIndex);

        // private members
        LasClassificationInfo m_data;
        int                   m_viewIndex;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ViewSettingsData
    {
    public:
        ViewSettingsData::ViewSettingsData ();
        ~ViewSettingsData();
        ViewSettingsData& operator=(ViewSettingsData const& rhs);
        void InitDefault();
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool      GetUseRgb() const               { return TO_BOOL(m_flags & VIEWSETTINGS_RGB_MASK); }
        bool      GetUseIntensity() const         { return TO_BOOL(m_flags & VIEWSETTINGS_INTENSITY_MASK); }
        bool      GetUseLightning() const         { return TO_BOOL(m_flags & VIEWSETTINGS_LIGHTNING_MASK); }
        bool      GetUsePlane() const             { return TO_BOOL(m_flags & VIEWSETTINGS_PLANE_MASK); }
        bool      GetUseFrontBias() const         { return TO_BOOL(m_flags & VIEWSETTINGS_FRONTBIAS_MASK); }
        float     GetContrast() const             { return m_contrast; }
        float     GetBrightness() const           { return m_brightness; }
        float     GetDistance() const             { return m_dist; }
        float     GetOffset() const               { return m_off; }
        int       GetAdaptivePointSize() const    { return m_adaptivePointSize; }
        uint32_t  GetIntensityRampIndex() const;
        uint32_t  GetPlaneRampIndex() const;
        bool      GetACSAsPlaneAxis () const      { return m_useACSAsPlaneAxis; }
        uint16_t  GetPlaneAxis() const            { return m_planeAxis; }
        WStringCR GetPlaneRampName() const        { return m_planeRamp; }
        WStringCR GetIntensityRampName() const    { return m_intensityRamp; }
        IPointCloudViewSettings::PointCloudDisplayStyles GetDisplayStyle() const { return m_displayStyle; }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        void SetUseRgb(bool value)                  { value ? m_flags |= VIEWSETTINGS_RGB_MASK       : m_flags &= ~VIEWSETTINGS_RGB_MASK; }
        void SetUseIntensity(bool value)            { value ? m_flags |= VIEWSETTINGS_INTENSITY_MASK : m_flags &= ~VIEWSETTINGS_INTENSITY_MASK; }
        void SetUseLightning(bool value)            { value ? m_flags |= VIEWSETTINGS_LIGHTNING_MASK : m_flags &= ~VIEWSETTINGS_LIGHTNING_MASK; }
        void SetUsePlane(bool value)                { value ? m_flags |= VIEWSETTINGS_PLANE_MASK     : m_flags &= ~VIEWSETTINGS_PLANE_MASK; }
        void SetUseFrontBias(bool value)            { value ? m_flags |= VIEWSETTINGS_FRONTBIAS_MASK : m_flags &= ~VIEWSETTINGS_FRONTBIAS_MASK; }
        void SetContrast(float value)               { m_contrast = value; }
        void SetBrightness(float value)             { m_brightness = value; }
        void SetDistance(float value)               { m_dist = value; }
        void SetOffset(float value)                 { m_off = value; }
        void SetAdaptivePointSize(int value)        { m_adaptivePointSize = value; }
        void SetACSAsPlaneAxis(bool value)          { m_useACSAsPlaneAxis = value; }
        void SetPlaneAxis(uint16_t value)             { m_planeAxis = value; }
        void SetPlaneRampName(WStringCR value)      { m_planeRamp = value; }
        void SetIntensityRampName(WStringCR value)  { m_intensityRamp = value; }
        void SetPlaneRampIndex(uint16_t value)        { m_planeRampIdx = value; }
        void SetIntensityRampIndex(uint16_t value)    { m_intensityRampIdx = value; }
        void SetDisplayStyle(IPointCloudViewSettings::PointCloudDisplayStyles value) { m_displayStyle = value; }
        
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        static uint16_t GetDefaultIntensityRampIndex() { return 0; }
        static uint16_t GetDefaultPlaneRampIndex()     { return 1; }
        static float  GetDefaultContrast()           { return POINTCLOUD_DEFAULT_VIEW_CONTRAST; }
        static float  GetDefaultBrightness()         { return POINTCLOUD_DEFAULT_VIEW_BRIGHTNESS; }


        //Advanced Settings for SS4
        bool      GetClampIntentisy() const             { return m_clampIntensity; }
        void      SetClampIntentisy(bool value)         { m_clampIntensity = value; }
        
        bool      GetNeedClassifBuffer   () const       { return m_NeedClassifBuffer; }
        void      SetNeedClassifBuffer   (bool value)   { m_NeedClassifBuffer = value; }

        WString   GetDisplayStyleName   () const        { return m_DisplayStyleName; }
        void      SetDisplayStyleName(WCharCP value) { m_DisplayStyleName = value; }
        
        int       GetDisplayStyleIdx   () const         { return m_dsIdx; }
        void      SetDisplayStyleIdx   (int idx)        { m_dsIdx = idx; }

    private:
        uint32_t m_flags;
        float   m_contrast;
        float   m_brightness;
        float   m_dist;
        float   m_off;
        int     m_adaptivePointSize;
        uint16_t m_intensityRampIdx;
        uint16_t m_planeRampIdx;
        uint16_t m_planeAxis; //{x,y,z} index
        IPointCloudViewSettings::PointCloudDisplayStyles m_displayStyle;
        WString m_planeRamp;
        WString m_intensityRamp;
        bool    m_useACSAsPlaneAxis;

        //Advanced Settings from SS4
        bool    m_clampIntensity;
        bool    m_NeedClassifBuffer;
        WString m_DisplayStyleName;
        int     m_dsIdx;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudViewSettings : public RefCounted<IPointCloudViewSettings>
    {
    private:
        int                         m_viewIndex;
        ViewSettingsData            m_data;
        bool                        m_hasAdvancedInfo;

    public:
        PointCloudViewSettings (ViewContextCR viewContextCP);
        ~PointCloudViewSettings() { };


        // from IPointCloudViewSettings
        bool    _GetUseRGB () const override            { return m_data.GetUseRgb(); }
        void    _SetUseRGB (bool use) override          { m_data.SetUseRgb(use); }
        bool    _GetUsePlane () const override          { return m_data.GetUsePlane(); }
        void    _SetUsePlane (bool use) override        { m_data.SetUsePlane(use); }
        bool    _GetUseIntensity () const override      { return m_data.GetUseIntensity(); }
        void    _SetUseIntensity (bool use) override    { m_data.SetUseIntensity(use); }
        bool    _GetUseFrontBias () const override      { return m_data.GetUseFrontBias(); }
        void    _SetUseFrontBias (bool use) override    { m_data.SetUseFrontBias(use); }
        float   _GetContrast () const override          { return m_data.GetContrast(); }
        void    _SetContrast (float val) override       { m_data.SetContrast(val); }
        float   _GetBrightness () const override        { return m_data.GetBrightness(); }
        void    _SetBrightness (float val) override     { m_data.SetBrightness(val); }
        float   _GetDistance () const override          { return m_data.GetDistance(); }
        void    _SetDistance (float val) override       { m_data.SetDistance(val); }
        float   _GetOffset () const override            { return m_data.GetOffset(); }
        void    _SetOffset (float val) override         { m_data.SetOffset(val); }
        uint16_t _GetPlaneAxis () const override         { return m_data.GetPlaneAxis(); }
        void    _SetPlaneAxis (uint16_t axis) override       

            { 

            if (axis > 3 ||  axis < 0)

                assert (!"Plane axis should be 0 for X, 1 for Y or 2 for Z, 3 is for ACS");

            else if (axis == 3)

                {

                m_data.SetACSAsPlaneAxis(true);

                }

            else

                {

                m_data.SetPlaneAxis(axis);

                m_data.SetACSAsPlaneAxis(false);

                }

            }
        bool    _GetACSAsPlaneAxis () const override    {return m_data.GetACSAsPlaneAxis ();}
        void    _SetACSAsPlaneAxis (bool val)  override    {return m_data.SetACSAsPlaneAxis (val);}

        uint32_t    _GetIntensityRampIndex () const override        { return m_data.GetIntensityRampIndex(); }
        uint32_t    _GetPlaneRampIndex () const override            { return m_data.GetPlaneRampIndex(); }
        WStringCR   _GetIntensityRamp () const override             { return m_data.GetIntensityRampName(); }
        void        _SetIntensityRamp(WCharCP rampName) override { return m_data.SetIntensityRampName(rampName); }
        WStringCR   _GetPlaneRamp () const override                 { return m_data.GetPlaneRampName(); }
        void        _SetPlaneRamp(WCharCP rampName) override     { m_data.SetPlaneRampName(rampName); }
        PointCloudDisplayStyles _GetDisplayStyle () const override                          { return m_data.GetDisplayStyle(); }
        void                    _SetDisplayStyle (PointCloudDisplayStyles style) override   { m_data.SetDisplayStyle(style); }

        //Advanced Information for new kind of display styles

        virtual void    _SetHasAdvancedInfo        (bool use) { m_hasAdvancedInfo = use; }

        virtual bool    _GetClampIntensity         () const { return m_data.GetClampIntentisy();  }     

        virtual void    _SetClampIntensity         (bool use) { m_data.SetClampIntentisy(use); }

        virtual bool    _GetNeedClassifBuffer      () const { return m_data.GetNeedClassifBuffer(); }

        virtual void    _SetNeedClassifBuffer      (bool use) { m_data.SetNeedClassifBuffer(use); }

        virtual WString _GetDisplayStyleName       () const { return m_data.GetDisplayStyleName(); }

        virtual void    _SetDisplayStyleName(WCharCP use) { m_data.SetDisplayStyleName(use); }

        virtual void    _SetDisplayStyleIdx        (int idx) { m_data.SetDisplayStyleIdx(idx); }

        virtual int     _GetDisplayStyleIdx        () const { return m_data.GetDisplayStyleIdx(); }

        // static methods
        POINTCLOUDSCHEMA_EXPORT static IPointCloudViewSettingsPtr GetPointCloudViewSettings(ViewContextCR viewContext);
        static IPointCloudViewSettingsPtr Create(ViewContextCR viewContext);
    };

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
