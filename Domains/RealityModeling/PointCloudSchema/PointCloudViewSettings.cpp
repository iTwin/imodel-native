/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudViewSettings.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Daniel.McKenzie      12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudClassificationViewSettings::PointCloudClassificationViewSettings(Dgn::ViewContextCR context)
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

