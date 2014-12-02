/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/PropertyContext.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*=================================================================================**//**
* EachPropertyBaseArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
PropsCallbackFlags EachPropertyBaseArg::GetPropertyFlags ()
    {
    return m_flags;
    }

/*=================================================================================**//**
* EachLevelArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachLevelArg::EachLevelArg (LevelId stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
LevelId EachLevelArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachLevelArg::SetStoredValue (LevelId newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
LevelId         EachLevelArg::GetEffectiveValue ()
    {
    LevelId                     effective = m_storedValue;
    ElemHeaderOverrides const*  overrides = m_processor.GetHeaderOverrides ();

    if (NULL != overrides)
        effective = overrides->AdjustLevel (effective);

    return effective;
    }

/*=================================================================================**//**
* EachColorArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachColorArg::EachColorArg (UInt32 stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          EachColorArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachColorArg::SetStoredValue (UInt32 newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          EachColorArg::GetEffectiveValue ()
    {
    UInt32                      effective = m_storedValue;
    ElemHeaderOverrides const*  overrides = m_processor.GetHeaderOverrides ();

    if (overrides && overrides->GetFlags ().color)
        {
        effective = overrides->GetColor ();
        }
    else if (COLOR_BYCELL == effective)
        {
        if (NULL == overrides)
            effective = 0; // If BYCELL color and not in a cell...color is ALWAYS color 0 (white w/ACAD color table)
        else
            effective = overrides->GetColor ();
        }
    else if (COLOR_BYLEVEL == effective)
        {
        LevelId     currentLevel = (overrides ? overrides->AdjustLevel (m_processor.GetCurrentLevelID()) : m_processor.GetCurrentLevelID ());
        DgnLevels::SubLevel subLevel = DGN_TABLE_LEVEL_FOR_MODEL(m_processor.GetSourceDgnModel()).QuerySubLevelById (SubLevelId(currentLevel));
        if (subLevel.IsValid ())
            effective = subLevel.GetAppearance().GetColor();
        }

    return effective;
    }

/*=================================================================================**//**
* EachLineStyleArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachLineStyleArg::EachLineStyleArg (Int32 stored, LineStyleParams const* params, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    if (NULL != params)
        m_params = *params;
    else
        m_params.Init();

    m_paramsChanged = false;
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
Int32           EachLineStyleArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
Int32           EachLineStyleArg::GetEffectiveValue ()
    {
    Int32                       effective = m_storedValue;
    ElemHeaderOverrides const*  overrides = m_processor.GetHeaderOverrides ();

    if (overrides && overrides->GetFlags ().style)
        {
        effective = overrides->GetLineStyle ();
        }
    else if (STYLE_BYCELL == effective)
        {
        if (NULL == overrides)
            {
            // Use BYLEVEL style from default level (should be continuous for DWG...)
            DgnLevels::SubLevel subLevel = DGN_TABLE_LEVEL_FOR_MODEL(m_processor.GetSourceDgnModel()).QuerySubLevelById (SubLevelId(LEVEL_DEFAULT_LEVEL_ID));
            if (subLevel.IsValid ())
                effective = subLevel.GetAppearance().GetStyle();
            else
                effective = 0;
            }
        else
            {
            effective = overrides->GetLineStyle ();
            }
        }
    else if (STYLE_BYLEVEL == effective)
        {
        LevelId     currentLevel = (overrides ? (overrides->AdjustLevel (m_processor.GetCurrentLevelID ())) : m_processor.GetCurrentLevelID ());

        DgnLevels::SubLevel subLevel = DGN_TABLE_LEVEL_FOR_MODEL(m_processor.GetSourceDgnModel()).QuerySubLevelById (SubLevelId(currentLevel));
        if (subLevel.IsValid ())
            effective = subLevel.GetAppearance().GetStyle();
        }

    return effective;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            EachLineStyleArg::GetEffectiveParams (LineStyleParams& lsParams)
    {
    // NEEDSWORK...
    memcpy (&lsParams, &m_params, sizeof (lsParams));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleParams const*  EachLineStyleArg::GetParams ()
    {
    return &m_params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            EachLineStyleArg::GetParamsChanged ()
    {
    return m_paramsChanged;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachLineStyleArg::SetStoredValue (Int32 newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachLineStyleArg::SetParams (LineStyleParams const* newParams)
    {
    if (!newParams)
        {
        m_params.Init ();

        BeAssert (false && "Does caller want active params? Use LineStyleParams::GetActiveParams");

        return SUCCESS;
        }

    m_params = *newParams;

    // This option is only for linestrings/shapes, will be set based on tcb flag, clear for other other element types...
    if (0 == (m_params.modifiers & STYLEMOD_NOSEGMODE))
        return SUCCESS;

    ElementHandleCP ehCP = m_processor.GetCurrentElemHandleP ();

    if (!ehCP)
        return SUCCESS;

    m_params.modifiers &= ~STYLEMOD_NOSEGMODE;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachLineStyleArg::SetParamsChanged ()
    {
    m_paramsChanged = true;

    return SUCCESS;
    }

/*=================================================================================**//**
* EachFontArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachFontArg::EachFontArg (UInt32 stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          EachFontArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachFontArg::SetStoredValue (UInt32 newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*=================================================================================**//**
* EachTextStyleArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachTextStyleArg::EachTextStyleArg (UInt32 stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue     = stored;
    m_paramsRemapping = (NULL == m_processor.GetIEditPropertiesP () ? StyleParamsRemapping::Invalid : StyleParamsRemapping::NoChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          EachTextStyleArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StyleParamsRemapping    EachTextStyleArg::GetRemappingAction ()
    {
    return m_paramsRemapping;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachTextStyleArg::SetStoredValue (UInt32 newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachTextStyleArg::SetRemappingAction (StyleParamsRemapping action)
    {
    m_paramsRemapping = action;

    return SUCCESS;
    }

/*=================================================================================**//**
* EachDimStyleArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachDimStyleArg::EachDimStyleArg (UInt64 stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue     = stored;
    m_paramsRemapping = (NULL == m_processor.GetIEditPropertiesP () ? StyleParamsRemapping::Invalid : StyleParamsRemapping::NoChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
UInt64       EachDimStyleArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StyleParamsRemapping    EachDimStyleArg::GetRemappingAction ()
    {
    return m_paramsRemapping;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachDimStyleArg::SetStoredValue (UInt64 newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachDimStyleArg::SetRemappingAction (StyleParamsRemapping action)
    {
    m_paramsRemapping = action;

    return SUCCESS;
    }

/*=================================================================================**//**
* EachMLineStyleArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachMLineStyleArg::EachMLineStyleArg (UInt64 stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue     = stored;
    m_paramsRemapping = (NULL == m_processor.GetIEditPropertiesP () ? StyleParamsRemapping::Invalid : StyleParamsRemapping::NoChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
UInt64 EachMLineStyleArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StyleParamsRemapping    EachMLineStyleArg::GetRemappingAction ()
    {
    return m_paramsRemapping;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachMLineStyleArg::SetStoredValue (UInt64 newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachMLineStyleArg::SetRemappingAction (StyleParamsRemapping action)
    {
    m_paramsRemapping = action;

    return SUCCESS;
    }

/*=================================================================================**//**
* EachMaterialArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachMaterialArg::EachMaterialArg (DgnMaterialId stored, PropsCallbackFlags flags, PropertyContextR processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
EachMaterialArg::EachMaterialArg (DgnMaterialId stored, WCharCP subEntity, PropsCallbackFlags flags, PropertyContextR processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    m_subEntity = subEntity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/11
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         EachMaterialArg::GetSubEntity ()
    {
    return m_subEntity.c_str ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterialId EachMaterialArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EachMaterialArg::SetStoredValue (DgnMaterialId newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*=================================================================================**//**
* EachWeightArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachWeightArg::EachWeightArg (UInt32 stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          EachWeightArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachWeightArg::SetStoredValue (UInt32 newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          EachWeightArg::GetEffectiveValue ()
    {
    UInt32                      effective = m_storedValue;
    ElemHeaderOverrides const*  overrides = m_processor.GetHeaderOverrides ();

    if (overrides && overrides->GetFlags ().weight)
        {
        effective = overrides->GetWeight ();
        }
    else if (WEIGHT_BYCELL == effective)
        {
        if (NULL == overrides)
            effective = 0; // If BYCELL weight and not in a cell...weigbt is ALWAYS weight 0...
        else
            effective = overrides->GetWeight ();
        }
    else if (WEIGHT_BYLEVEL == effective)
        {
        LevelId     currentLevel = (overrides ? (overrides->AdjustLevel (m_processor.GetCurrentLevelID ())) : m_processor.GetCurrentLevelID ());

        DgnLevels::SubLevel subLevel = DGN_TABLE_LEVEL_FOR_MODEL(m_processor.GetSourceDgnModel()).QuerySubLevelById (SubLevelId(currentLevel));
        if (subLevel.IsValid ())
            effective = subLevel.GetAppearance().GetWeight ();
        }

    return effective;
    }

/*=================================================================================**//**
* EachElementClassArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*------------------------------------------------------dev---------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachElementClassArg::EachElementClassArg (DgnElementClass stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementClass    EachElementClassArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementClass    EachElementClassArg::GetEffectiveValue ()
    {
    DgnElementClass            effective = m_storedValue;
    ElemHeaderOverrides const*  overrides = m_processor.GetHeaderOverrides ();

    if (NULL != overrides && overrides->GetFlags().classValue)
        effective = (DgnElementClass) overrides->GetElementClass();

    return effective;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachElementClassArg::SetStoredValue (DgnElementClass newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*=================================================================================**//**
* EachTransparencyArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachTransparencyArg::EachTransparencyArg (double stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          EachTransparencyArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachTransparencyArg::SetStoredValue (double newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*=================================================================================**//**
* EachThicknessArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachThicknessArg::EachThicknessArg (double stored, DVec3dP direction, bool capped, bool alwaysUseDirection, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue   = stored;

    m_capped             = capped;
    m_alwaysUseDirection = alwaysUseDirection;
    m_haveDirection      = (NULL != direction);

    if (direction)
        m_direction = *direction;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          EachThicknessArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachThicknessArg::SetStoredValue (double newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            EachThicknessArg::GetCapped ()
    {
    return m_capped;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachThicknessArg::SetCapped (bool capped)
    {
    m_capped = capped;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            EachThicknessArg::GetAlwaysUseDirection ()
    {
    return m_alwaysUseDirection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachThicknessArg::SetAlwaysUseDirection (bool alwaysUseDirection)
    {
    m_alwaysUseDirection = alwaysUseDirection;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachThicknessArg::GetDirection (DVec3dR direction)
    {
    if (!m_haveDirection)
        return ERROR;

    direction = m_direction;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachThicknessArg::SetDirection (DVec3dCP direction)
    {
    m_haveDirection = (NULL != direction);

    if (direction)
        m_direction = *direction;

    return SUCCESS;
    }

/*=================================================================================**//**
* EachDisplayPriorityArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachDisplayPriorityArg::EachDisplayPriorityArg (Int32 stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
Int32           EachDisplayPriorityArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
Int32           EachDisplayPriorityArg::GetEffectiveValue ()
    {
    Int32       effective = m_storedValue;

    if (DISPLAYPRIORITY_BYCELL == effective)
        {
        ElemHeaderOverrides const*  overrides = m_processor.GetHeaderOverrides ();

        if (NULL == overrides)
            effective = 0; // Don't leave this with BYCELL value...
        else
            effective = overrides->GetDisplayPriority();
        }

    return effective;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachDisplayPriorityArg::SetStoredValue (Int32 newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*=================================================================================**//**
* EachElementTemplateArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachElementTemplateArg::EachElementTemplateArg (ElementId stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue       = stored;
    m_applyDefaultSymb  = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId       EachElementTemplateArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            EachElementTemplateArg::GetApplyDefaultSymbology ()
    {
    return m_applyDefaultSymb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachElementTemplateArg::SetStoredValue (ElementId newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachElementTemplateArg::SetApplyDefaultSymbology (bool apply)
    {
    m_applyDefaultSymb = apply;

    return SUCCESS;
    }

/*=================================================================================**//**
* PropertyContext
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyContext::PropertyContext (IQueryProperties* obj)
    {
    m_queryObj          = obj;
    m_editObj           = NULL;
    m_elementChanged    = false;
    m_elmHandle         = NULL;
    m_hitPath           = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyContext::PropertyContext (IEditProperties* obj)
    {
    m_editObj           = obj;
    m_queryObj          = NULL;
    m_elementChanged    = false;
    m_elmHandle         = NULL;
    m_hitPath           = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
ElementProperties   PropertyContext::GetElementPropertiesMask ()
    {
    if (NULL != m_queryObj)
        return m_queryObj->_GetQueryPropertiesMask ();

    if (NULL != m_editObj)
        return m_editObj->_GetEditPropertiesMask ();

    return ELEMENT_PROPERTY_None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertyContext::SetCurrentElemHandleP (ElementHandleCP eh)
    {
    m_elmHandle = eh;

    /* NOTE: Level needed for effective value. Should always be true that
             GetLevel is base level id. Mlines will need to account for
             profile levels; multi-level elements are an aberration. */
    DgnElementCP elmCP = (eh ? eh->GetElementCP () : NULL);

    SetCurrentLevelID (elmCP ? elmCP->GetLevel() : LevelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP    PropertyContext::GetSourceDgnModel ()
    {
    ElementHandleCP    eh = GetCurrentElemHandleP ();

    return (eh ? eh->GetDgnModelP () : NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP    PropertyContext::GetDestinationDgnModel ()
    {
    if (NULL != m_editObj)
        {
        DgnModelP    modelRef = m_editObj->_GetDestinationDgnModel ();

        if (NULL != modelRef)
            return modelRef;
        }

    return GetSourceDgnModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertyContext::PushOverrides (ElemHeaderOverrides* in)
    {
    size_t numEntries = m_headerOvr.size ();

    m_headerOvr.resize (numEntries+1); // NOTE: May realloc, can't save pointer from GetHeaderOvr!
    ElemHeaderOverrides* newEntry = &m_headerOvr.at (numEntries);

    newEntry->MergeFrom (in, numEntries > 0 ? &m_headerOvr.at (numEntries-1) : NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertyContext::PopOverrides ()
    {
    m_headerOvr.pop_back ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             PropertyContext::GetOverridesStackDepth ()
    {
    return (int) m_headerOvr.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ElemHeaderOverrides const*    PropertyContext::GetHeaderOverrides ()
    {
    return m_headerOvr.empty() ? NULL : &m_headerOvr.back ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoColorCallback (UInt32* pNewColor, EachColorArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_Color & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachColorCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        UInt32  color = arg.GetStoredValue();

        m_editObj->_EachColorCallback (arg);

        UInt32  newColor = arg.GetStoredValue();

        if (NULL != pNewColor && newColor != color)
            {
            *pNewColor = newColor;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoLevelCallback (LevelId* pNewLevelID, EachLevelArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_Level & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachLevelCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        LevelId     levelID = arg.GetStoredValue();

        m_editObj->_EachLevelCallback (arg);

        LevelId     newLevelID = arg.GetStoredValue();

        if (NULL != pNewLevelID && newLevelID != levelID)
            {
            *pNewLevelID = newLevelID;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoLineStyleCallback (Int32* pNewStyleID, EachLineStyleArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_Linestyle & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachLineStyleCallback (arg);
        return false;
        }

    bool    changed = false;

    if (NULL != m_editObj)
        {
        Int32           styleID = arg.GetStoredValue();
        LineStyleParams inParams = *(arg.GetParams());

        m_editObj->_EachLineStyleCallback (arg);

        // Structure may have been initialized using memset or lsutil_initializeParams...
        bool    paramsChanged = ((0 == inParams.modifiers && 0 == arg.GetParams()->modifiers) ? false : (0 != memcmp (&inParams, arg.GetParams(), sizeof (inParams))));

        if (paramsChanged)
            {
            arg.SetParamsChanged();
            SetElementChanged();
            changed = true;
            }

        Int32   newStyleID = arg.GetStoredValue();

        if (NULL != pNewStyleID && newStyleID != styleID)
            {
            *pNewStyleID = newStyleID;

            SetElementChanged();
            changed = true;
            }
        }

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoFontCallback (UInt32* pNewFontNo, EachFontArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_Font & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachFontCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        UInt32  fontNo = arg.GetStoredValue();

        m_editObj->_EachFontCallback (arg);

        UInt32  newFontNo = arg.GetStoredValue();

        if (NULL != pNewFontNo && newFontNo != fontNo)
            {
            *pNewFontNo = newFontNo;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoTextStyleCallback (UInt32* pNewStyleID, EachTextStyleArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_TextStyle & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachTextStyleCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        UInt32  styleID = arg.GetStoredValue();

        m_editObj->_EachTextStyleCallback (arg);

        UInt32  newStyleID = arg.GetStoredValue();

        if (NULL != pNewStyleID && newStyleID != styleID)
            {
            *pNewStyleID = newStyleID;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoDimStyleCallback (UInt64* pNewStyleID, EachDimStyleArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_DimStyle & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachDimStyleCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        UInt64   styleID = arg.GetStoredValue();

        m_editObj->_EachDimStyleCallback (arg);

        UInt64   newStyleID = arg.GetStoredValue();

        if (NULL != pNewStyleID && newStyleID != styleID)
            {
            *pNewStyleID = newStyleID;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoMLineStyleCallback (ElementId* pNewStyleID, EachMLineStyleArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_MLineStyle & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachMLineStyleCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        UInt64 styleID = arg.GetStoredValue();

        m_editObj->_EachMLineStyleCallback (arg);

        UInt64   newStyleID = arg.GetStoredValue();

        if (NULL != pNewStyleID && newStyleID != styleID)
            {
            *pNewStyleID = ElementId(newStyleID);

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoMaterialCallback (DgnMaterialId* pNewID, EachMaterialArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_Material & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachMaterialCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        DgnMaterialId oldID = arg.GetStoredValue();

        m_editObj->_EachMaterialCallback (arg);

        DgnMaterialId newID = arg.GetStoredValue();

        if (NULL != pNewID && newID == oldID)
            {
            *pNewID = newID;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoWeightCallback (UInt32* pNewVal, EachWeightArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_Weight & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachWeightCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        UInt32      oldVal = arg.GetStoredValue();

        m_editObj->_EachWeightCallback (arg);

        UInt32      newVal = arg.GetStoredValue();

        if (NULL != pNewVal && newVal != oldVal)
            {
            *pNewVal = newVal;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoElementClassCallback (DgnElementClass* pNewVal, EachElementClassArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_ElementClass & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachElementClassCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        DgnElementClass    oldVal = arg.GetStoredValue();

        m_editObj->_EachElementClassCallback (arg);

        DgnElementClass    newVal = arg.GetStoredValue();

        if (NULL != pNewVal && newVal != oldVal)
            {
            *pNewVal = newVal;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoTransparencyCallback (double* pNewVal, EachTransparencyArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_Transparency & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachTransparencyCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        double      oldVal = arg.GetStoredValue();

        m_editObj->_EachTransparencyCallback (arg);

        double      newVal = arg.GetStoredValue();

        if (NULL != pNewVal && newVal != oldVal)
            {
            *pNewVal = newVal;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoThicknessCallback (double* pNewVal, EachThicknessArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_Thickness & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachThicknessCallback (arg);
        return false;
        }

    bool    changed = false;

    if (NULL != m_editObj)
        {
        double      oldVal = arg.GetStoredValue();
        DVec3d      oldDir;
        bool        haveOldDir = (SUCCESS == arg.GetDirection (oldDir));
        bool        oldCapped = arg.GetCapped ();
        bool        oldAlwaysDir = arg.GetAlwaysUseDirection ();

        m_editObj->_EachThicknessCallback (arg);

        double      newVal = arg.GetStoredValue();

        if (NULL != pNewVal && newVal != oldVal)
            {
            *pNewVal = newVal;

            SetElementChanged();
            changed = true;
            }

        if (oldCapped != arg.GetCapped ())
            {
            SetElementChanged();
            changed = true;
            }

        if (oldAlwaysDir != arg.GetAlwaysUseDirection ())
            {
            SetElementChanged();
            changed = true;
            }

        DVec3d      newDir;
        bool        haveNewDir = (SUCCESS == arg.GetDirection (newDir));

        if (haveOldDir != haveNewDir || (haveOldDir && !oldDir.isEqual (&newDir)))
            {
            SetElementChanged();
            changed = true;
            }
        }

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoDisplayPriorityCallback (Int32* pNewVal, EachDisplayPriorityArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_DisplayPriority & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachDisplayPriorityCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        Int32       oldVal = arg.GetStoredValue();

        m_editObj->_EachDisplayPriorityCallback (arg);

        Int32       newVal = arg.GetStoredValue();

        if (NULL != pNewVal && newVal != oldVal)
            {
            *pNewVal = newVal;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoElementTemplateCallback (ElementId* pNewVal, EachElementTemplateArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_ElementTemplate & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachElementTemplateCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        ElementId   oldVal = arg.GetStoredValue();

        m_editObj->_EachElementTemplateCallback (arg);

        ElementId   newVal = arg.GetStoredValue();

        // if apply symbology set then force element change flag to true so template properties are reapplied.
        if (NULL != pNewVal && (newVal != oldVal || arg.GetApplyDefaultSymbology()) )
            {
            *pNewVal = newVal;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertyContext::QueryChildElementProperties (ElementHandleCR eh)
    {
    ChildElemIter     childElm;
    ISharedCellQuery* scQuery;

    if (m_queryObj->_WantSharedChildren () && NULL != (scQuery = dynamic_cast <ISharedCellQuery*> (&eh.GetHandler ())) && scQuery->IsSharedCell (eh))
        childElm = ChildElemIter (ElementHandle (scQuery->GetDefinition(eh, *eh.GetDgnProject())), ExposeChildrenReason::Query);
    else
        childElm = ChildElemIter (eh, ExposeChildrenReason::Query);

    if (!childElm.IsValid ())
        return;

    IDisplayHandlerPathEntryExtension* extension = IDisplayHandlerPathEntryExtension::Cast (eh.GetHandler ());

    if (extension)
        {
        ElemHeaderOverrides ovr;

        if (extension->_GetElemHeaderOverrides (eh, ovr))
            {
            PropertyContext::ContextMark mark (this); // restored in destructor!

            PushOverrides (&ovr);

            for (; childElm.IsValid (); childElm = childElm.ToNext ())
                {
                childElm.GetHandler().QueryProperties (childElm, *this);

                QueryChildElementProperties (childElm);
                }

            return;
            }
        }

    for (; childElm.IsValid (); childElm = childElm.ToNext ())
        {
        childElm.GetHandler().QueryProperties (childElm, *this);

        QueryChildElementProperties (childElm);
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertyContext::QueryElementProperties (ElementHandleCR eh, IQueryProperties* obj)
    {
    PropertyContext context (obj);
    eh.GetHandler().QueryProperties (eh, context);
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertyContext::EditChildElementProperties (EditElementHandleR eeh)
    {
    ChildEditElemIter childElm (eeh, ExposeChildrenReason::Edit);

    if (!childElm.IsValid ())
        return;

    IDisplayHandlerPathEntryExtension* extension = IDisplayHandlerPathEntryExtension::Cast (eeh.GetHandler ());

    if (extension)
        {
        ElemHeaderOverrides ovr;

        if (extension->_GetElemHeaderOverrides (eeh, ovr))
            {
            PropertyContext::ContextMark mark (this); // restored in destructor!

            PushOverrides (&ovr);

            for (; childElm.IsValid (); childElm = childElm.ToNext ())
                {
                childElm.GetHandler().EditProperties (childElm, *this);

                EditChildElementProperties (childElm);
                }

            return;
            }
        }

    for (; childElm.IsValid (); childElm = childElm.ToNext ())
        {
        childElm.GetHandler().EditProperties (childElm, *this);

        EditChildElementProperties (childElm);
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::EditElementProperties (EditElementHandleR eeh, IEditProperties* obj)
    {
    PropertyContext context (obj);

    eeh.GetHandler().EditProperties (eeh, context);

    return context.GetElementChanged ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertyContext::QueryPathProperties (HitPathCP path, IQueryProperties* obj)
    {
    if (!path)
        return;

    PropertyContext context (obj);

    PropertyContext::ContextMark mark (context); // restored in destructor!

    for (int iPath=0; iPath <= path->GetCount (); iPath++)
        {
        ElementHandle  eh (path->GetPathElem (iPath));

        if (!eh.IsValid ())
            break;

#if defined (NEEDS_WORK_DGNITEM)
        // Keep looking down path as long as we're allowed to query child properties...
        if (!eh.GetHandler().ExposeChildren (eh, ExposeChildrenReason::Query))
            {
            ISharedCellQuery* scQuery;

            // If shared cell def components are requested, keep going...
            if (!obj->_WantSharedChildren () || NULL == (scQuery = dynamic_cast <ISharedCellQuery*> (&eh.GetHandler ())) || !scQuery->IsSharedCell (eh))
                {
                context.SetQueryPath (path);
                eh.GetHandler().QueryProperties (eh, context);
                break;
                }
            }
#endif

        ElemHeaderOverrides         ovr;
        context.PushOverrides (&ovr);
        }
    }

/*=================================================================================**//**
* PropertyContext::ContextMark
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     PropertyContext::ContextMark::Pop ()
    {
    if (NULL == m_context)
        return;

    while (m_context->GetOverridesStackDepth () > m_hdrOvrMark)
        m_context->PopOverrides ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     PropertyContext::ContextMark::SetNow ()
    {
    m_hdrOvrMark = m_context->GetOverridesStackDepth ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyContext::ContextMark::ContextMark (PropertyContextP context)
    {
    if (NULL == (m_context = context))
        Init (context);
    else
        SetNow ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyContext::ContextMark::ContextMark (PropertyContextR context)
    {
    m_context = &context;
    SetNow ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyContext::ContextMark::~ContextMark ()
    {
    Pop ();
    }
