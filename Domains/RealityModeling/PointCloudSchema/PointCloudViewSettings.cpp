/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudViewSettings.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ViewSettingsData::ViewSettingsData ()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ViewSettingsData::~ViewSettingsData() 
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ViewSettingsData& ViewSettingsData::operator=(ViewSettingsData const& rhs)
    {
    m_flags = rhs.m_flags;
    m_contrast = rhs.m_contrast;
    m_brightness = rhs.m_brightness;
    m_dist = rhs.m_dist;
    m_off = rhs.m_off;
    m_adaptivePointSize = rhs.m_adaptivePointSize;
    m_intensityRampIdx = rhs.m_intensityRampIdx;
    m_planeRampIdx = rhs.m_planeRampIdx;
    m_planeAxis = rhs.m_planeAxis;
    m_displayStyle = rhs.m_displayStyle;
    m_planeRamp = rhs.m_planeRamp;
    m_intensityRamp = rhs.m_intensityRamp;
    m_useACSAsPlaneAxis =  rhs.m_useACSAsPlaneAxis;

    //Advanced Settings for SS4
    m_clampIntensity = rhs.m_clampIntensity;
    m_NeedClassifBuffer = rhs.m_NeedClassifBuffer;
    m_DisplayStyleName = rhs.m_DisplayStyleName;

    m_dsIdx = rhs.m_dsIdx;

    return *this;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewSettingsData::InitDefault()
    {
    // init with default values
    m_flags = (VIEWSETTINGS_FRONTBIAS_MASK);
    m_contrast = GetDefaultContrast();
    m_brightness = GetDefaultBrightness();
    m_dist = 10.0f;
    m_off = 0;
    m_adaptivePointSize = 0;
    m_intensityRampIdx = GetDefaultIntensityRampIndex();
    m_planeRampIdx = GetDefaultPlaneRampIndex();
    m_planeAxis = 2;
    m_useACSAsPlaneAxis = false;
    m_displayStyle = IPointCloudViewSettings::DisplayStyle_None;
    m_planeRamp     = PointCloudRamps::GetInstance().GetPlaneRampName(m_planeRampIdx);
    m_intensityRamp = PointCloudRamps::GetInstance().GetIntensityRampName(m_intensityRampIdx);

    //Advanced Settings for SS4
    m_clampIntensity = false;
    m_NeedClassifBuffer = false;
    m_DisplayStyleName = L"";
    m_dsIdx = -1; //Set it to -1 it means display style none
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ViewSettingsData::GetIntensityRampIndex() const   
    {
    return PointCloudRamps::GetInstance().GetIntensityRampIndex(GetIntensityRampName()); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ViewSettingsData::GetPlaneRampIndex() const       
    { 
    return PointCloudRamps::GetInstance().GetPlaneRampIndex(GetPlaneRampName()); 
    }    


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Daniel.McKenzie      12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudClassificationViewSettings::PointCloudClassificationViewSettings(ViewContextCR context)
    {
/* POINTCLOUD_WIP_GR06 - PointCloudClassificationViewSettings
    DynamicViewSettingsCP viewSettings = context.GetDynamicViewSettings(modelRefP);
 
    if (viewSettings->GetPointCloudDisplayStyleIndex() >= 0) //Style from the list
        {
        if (InitFromDisplayStyle(viewSettings->GetPointCloudDisplayStyleIndex(), modelRefP) == false) //could not find the display style, load it from the view
            CreateFromView(m_viewIndex);
        }
    else if (viewSettings->GetPointCloudDisplayStyleIndex() == -1) //This is the style none
        { 
        m_data.InitDefault();
        }
    else
        {
        CreateFromView(m_viewIndex);
        }
*/

    // POINTCLOUD_WIP_GR06 - For now, initialize with defaults. When we know how the app will store classification view settings, we'll be able to implement this method.
    m_data.InitDefault();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudClassificationViewSettings::PointCloudClassificationViewSettings (int viewIndex)
: m_viewIndex (viewIndex)
    {
    CreateFromView(m_viewIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Daniel.McKenzie      12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClassificationViewSettings::CreateFromView(int viewIndex)
    {
/* POINTCLOUD_WIP_GR06 - PointCloudClassificationViewSettings

    XAttributeHandlerId handlerId(XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_ClassificationViewSettings);

    ViewportP pViewport = IViewManager::GetActiveViewSet().GetViewport(viewIndex);
    if (NULL == pViewport)
        {
        m_data.InitDefault();
        return;
        }

    ViewInfoP pViewInfo(pViewport->GetViewInfoP());
    if (NULL != pViewInfo)
        {
        int dataSize;
        LasClassificationInfo const* savedData = reinterpret_cast <LasClassificationInfo const*> (pViewInfo->GetDynamicViewSettingsR().GetXAttributesHolderCR().GetXAttribute(&dataSize, handlerId, 0));

        if (NULL != savedData)
            {
            if (dataSize == sizeof (LasClassificationInfoSS3))
                {
                ColorDef     tempColor[CLASSIFCATION_COUNT];
                unsigned char   tempState[32];
                memset(tempColor, 255, CLASSIFCATION_COUNT * sizeof(ColorDef)); // all white
                memset(&tempState[0], 0xFFFFFFFF, 32 * sizeof(unsigned char)); // all to visible
                m_data.InitDefault(); //Make sure we don't have weird behavior

                UInt32 temp = savedData->GetClassificationStates(); //Get the states from SS3 and copy them into the settings in SS4
                memcpy(tempColor, savedData->GetClassificationColors(), 32 * sizeof(ColorDef));
                memcpy(tempState, &temp, sizeof (UInt32));

                m_data.SetClassificationColors(&tempColor[0]);
                m_data.SetClassificationVisibleStatesAdv(&tempState[0]);

                return;
                }
            else if (dataSize < sizeof (LasClassificationInfo))
                m_data.InitDefault();

            memcpy(&m_data, savedData, __min(dataSize, sizeof (LasClassificationInfo)));

            return;
            }

        }
*/
    m_data.InitDefault();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudClassificationViewSettings::~PointCloudClassificationViewSettings()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudClassificationViewSettings::_GetState(unsigned char idx) const
    {
    return m_data.GetClassificationState(idx);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClassificationViewSettings::_SetState(unsigned char idx, bool state)
    {
    m_data.SetClassificationState(idx, state);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudColorDefCR PointCloudClassificationViewSettings::_GetColor(unsigned char idx) const
    {
    return m_data.GetClassificationColor(idx);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClassificationViewSettings::_SetColor(unsigned char idx, PointCloudColorDefR color)
    {
    m_data.SetClassificationColor(idx, color);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudClassificationViewSettings::_GetBlendColor () const                     
    {
    return m_data.GetBlendColor();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClassificationViewSettings::_SetBlendColor (bool use)                   
    { 
    m_data.SetBlendColor(use);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudClassificationViewSettings::_GetUseBaseColor () const                   
    { 
    return m_data.GetUseBaseColor();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClassificationViewSettings::_SetUseBaseColor (bool use)
    { 
    m_data.SetUseBaseColor(use);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BePointCloud::PointCloudColorDef PointCloudClassificationViewSettings::_GetUnclassColor() const
    {
    return m_data.GetUnclassColor();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClassificationViewSettings::_SetUnclassColor(PointCloudColorDefR color)
    { 
    m_data.SetUnclassColor(color); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudClassificationViewSettings::_GetUnclassState()
    { 
    return m_data.GetUnclassState(); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClassificationViewSettings::_SetUnclassState(bool state)
    { 
    m_data.SetUnclassState(state);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudClassificationViewSettings::_GetClassActiveState(unsigned char idx)
    {
    return m_data.GetClassActiveState(idx); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudClassificationViewSettings::_SetClassActiveState(unsigned char idx, bool state)
    { 
    m_data.SetClassActiveState(idx,state); 
    }

/*---------------------------------------------------------------------------------**//**
* Published methods that wrap around the virtual methods.
* @bsimethod                                          
+---------------+---------------+---------------+---------------+---------------+------*/
bool                IPointCloudClassificationViewSettings::GetState (unsigned char idx) const                 {return _GetState (idx);}
bool                IPointCloudClassificationViewSettings::GetClassificationState (unsigned char idx) const   {return _GetState (idx);}
void                IPointCloudClassificationViewSettings::SetState (unsigned char idx, bool state)           {_SetState (idx, state);}
PointCloudColorDefCR IPointCloudClassificationViewSettings::GetColor (unsigned char idx) const                 {return _GetColor (idx);}
void                IPointCloudClassificationViewSettings::SetColor(unsigned char idx, PointCloudColorDefR color)       {_SetColor (idx, color);}
unsigned char       IPointCloudClassificationViewSettings::GetLastIndex() const                       {return _GetLastIndex ();}

bool                IPointCloudClassificationViewSettings::GetBlendColor () const                     { return _GetBlendColor ();}
void                IPointCloudClassificationViewSettings::SetBlendColor (bool use)                   { _SetBlendColor (use);}
bool                IPointCloudClassificationViewSettings::GetUseBaseColor () const                   { return _GetUseBaseColor ();}
void                IPointCloudClassificationViewSettings::SetUseBaseColor (bool use)                 { _SetUseBaseColor (use);}


BePointCloud::PointCloudColorDef IPointCloudClassificationViewSettings::GetUnclassColor() const                     { return _GetUnclassColor(); };
void               IPointCloudClassificationViewSettings::SetUnclassColor(PointCloudColorDefR color)  { _SetUnclassColor(color); };
bool               IPointCloudClassificationViewSettings::GetUnclassState()                           { return _GetUnclassState(); };
void               IPointCloudClassificationViewSettings::SetUnclassState(bool state)                 { _SetUnclassState(state); };
bool               IPointCloudClassificationViewSettings::GetClassActiveState(unsigned char idx)              { return _GetClassActiveState(idx); };
void               IPointCloudClassificationViewSettings::SetClassActiveState(unsigned char idx, bool state)  { _SetClassActiveState(idx,state); };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudViewSettings::PointCloudViewSettings (ViewContextCR context)
    {
/* POINTCLOUD_WIP_GR06 - PointCloudViewSettings
    m_viewIndex = -1;
    XAttributeHandlerId handlerId  (XATTRIBUTEID_PointCloudHandler, PointCloudMinorId_ViewSettings);

    int dataSize;
    void const* savedData = context.GetDynamicViewSettings(context.GetCurrentModel())->GetXAttributesHolderCR().GetXAttribute (&dataSize, handlerId, 0);

    m_hasAdvancedInfo = false;

    if (NULL != savedData)
        {
        DataInternalizer dataInternalizer ((byte*)savedData, dataSize);
        m_data.Init(dataInternalizer);
        }
    else if (context.GetViewport ())
        {
        InitFromViewNumber (context.GetViewport ()->GetViewNumber ());
        }
    else
        {
        m_data.InitDefault();
        }
*/

    // POINTCLOUD_WIP_GR06 - For now, initialize with defaults. When we know how the app will store view settings, we'll be able to implement this method.
    m_data.InitDefault();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudViewSettingsPtr PointCloudViewSettings::Create(ViewContextCR context)
    {
    return new PointCloudViewSettings(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudViewSettingsPtr PointCloudViewSettings::GetPointCloudViewSettings(ViewContextCR context)
{
    return PointCloudViewSettings::Create(context);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<WString> const& IPointCloudViewSettings::GetIntensityRamps ()
    {
    return PointCloudRamps::GetInstance().GetIntensityRamps();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<WString> const& IPointCloudViewSettings::GetPlaneRamps ()
    {
    return PointCloudRamps::GetInstance().GetPlaneRamps();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudClassificationViewSettingsPtr IPointCloudClassificationViewSettings::GetPointCloudClassificationViewSettings(int viewIndex)
    {
    return PointCloudClassificationViewSettings::Create (viewIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Daniel.McKenzie    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IPointCloudClassificationViewSettingsPtr IPointCloudClassificationViewSettings::GetPointCloudClassificationViewSettings(ViewContextCR context)
    {
    return PointCloudClassificationViewSettings::Create(context);
    }


/*---------------------------------------------------------------------------------**//**
* Published methods that wrap around the virtual methods.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IPointCloudViewSettings::GetUseRGB () const                         {return _GetUseRGB ();}
void    IPointCloudViewSettings::SetUseRGB (bool use)                       {_SetUseRGB (use);}
bool    IPointCloudViewSettings::GetUseIntensity () const                   {return _GetUseIntensity ();}
void    IPointCloudViewSettings::SetUseIntensity (bool use)                 {_SetUseIntensity (use);}
bool    IPointCloudViewSettings::GetUsePlane () const                       {return _GetUsePlane ();}
void    IPointCloudViewSettings::SetUsePlane (bool use)                     {_SetUsePlane (use);}
bool    IPointCloudViewSettings::GetUseFrontBias () const                   {return _GetUseFrontBias ();}
void    IPointCloudViewSettings::SetUseFrontBias (bool use)                 {_SetUseFrontBias (use);}
float   IPointCloudViewSettings::GetContrast () const                       {return _GetContrast ();}
void    IPointCloudViewSettings::SetContrast (float val)                    {_SetContrast (val);}
float   IPointCloudViewSettings::GetBrightness () const                     {return _GetBrightness ();}
void    IPointCloudViewSettings::SetBrightness (float val)                  {_SetBrightness (val);}
float   IPointCloudViewSettings::GetDistance () const                       {return _GetDistance ();}
void    IPointCloudViewSettings::SetDistance (float val)                    {_SetDistance (val);}
float   IPointCloudViewSettings::GetOffset () const                         {return _GetOffset ();}
void    IPointCloudViewSettings::SetOffset (float val)                      {_SetOffset (val);}
bool    IPointCloudViewSettings::GetACSAsPlaneAxis () const                 {return _GetACSAsPlaneAxis ();}
void    IPointCloudViewSettings::SetACSAsPlaneAxis (bool val)               {_SetACSAsPlaneAxis (val);}
uint16_t IPointCloudViewSettings::GetPlaneAxis () const                     {return _GetPlaneAxis ();}
void    IPointCloudViewSettings::SetPlaneAxis (uint16_t axis)               {_SetPlaneAxis (axis);}
uint32_t IPointCloudViewSettings::GetIntensityRampIndex () const            {return _GetIntensityRampIndex ();}
uint32_t IPointCloudViewSettings::GetPlaneRampIndex () const                {return _GetPlaneRampIndex ();}
WStringCR IPointCloudViewSettings::GetIntensityRamp () const                { return _GetIntensityRamp ();}
void      IPointCloudViewSettings::SetIntensityRamp(WCharCP rampName)       { _SetIntensityRamp(rampName); }
WStringCR IPointCloudViewSettings::GetPlaneRamp () const                    {return _GetPlaneRamp ();}
void      IPointCloudViewSettings::SetPlaneRamp(WCharCP rampName)           {_SetPlaneRamp(rampName); }

IPointCloudViewSettings::PointCloudDisplayStyles IPointCloudViewSettings::GetDisplayStyle () const {return _GetDisplayStyle ();}
void                                             IPointCloudViewSettings::SetDisplayStyle (PointCloudDisplayStyles style) {_SetDisplayStyle (style);}

//Advanced Information for new kind of display styles  
bool    IPointCloudViewSettings::GetClampIntensity () const { return _GetClampIntensity(); }
void    IPointCloudViewSettings::SetClampIntensity (bool use) { _SetClampIntensity(use); }
bool    IPointCloudViewSettings::GetNeedClassifBuffer() const { return _GetNeedClassifBuffer(); }      
void    IPointCloudViewSettings::SetNeedClassifBuffer(bool use) { _SetNeedClassifBuffer(use); }
void    IPointCloudViewSettings::SetHasAdvancedInfo(bool use) { _SetHasAdvancedInfo(use); }
WString IPointCloudViewSettings::GetDisplayStyleName() const { return _GetDisplayStyleName(); }      
void    IPointCloudViewSettings::SetDisplayStyleName(WCharCP use) { _SetDisplayStyleName(use); }
int     IPointCloudViewSettings::GetDisplayStyleIdx() const { return _GetDisplayStyleIdx(); }      
void    IPointCloudViewSettings::SetDisplayStyleIdx(int idx) { _SetDisplayStyleIdx(idx); }

