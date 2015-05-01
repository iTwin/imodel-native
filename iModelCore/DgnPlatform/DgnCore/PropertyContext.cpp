/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/PropertyContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define DGN_TABLE_LEVEL_FOR_MODEL(m)  (m)->GetDgnDb().Categories()

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
* EachCategoryArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachCategoryArg::EachCategoryArg (DgnCategoryId stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId EachCategoryArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachCategoryArg::SetStoredValue (DgnCategoryId newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId         EachCategoryArg::GetEffectiveValue ()
    {
    DgnCategoryId effective = m_storedValue;
    return effective;
    }

/*=================================================================================**//**
* EachColorArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachColorArg::EachColorArg (uint32_t stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        EachColorArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachColorArg::SetStoredValue (uint32_t newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t EachColorArg::GetEffectiveValue ()
    {
    uint32_t                    effective = m_storedValue;

#if defined (NEEDS_WORK_DGNITEM)
    if (COLOR_BYCELL == effective)
        {
        effective = 0; // If BYCELL color and not in a cell...color is ALWAYS color 0 (white w/ACAD color table)
        }
    else if (COLOR_BYLEVEL == effective)
        {
        DgnCategoryId currentCategory = m_processor.GetCurrentCategoryID();
        DgnCategories::SubCategory subCategory = DGN_TABLE_LEVEL_FOR_MODEL(m_processor.GetSourceDgnModel()).QuerySubCategoryByKey (SubCategoryKey(currentCategory));
        if (subCategory.IsValid ())
            effective = subCategory.GetAppearance().GetColor();
        }
#endif

    return effective;
    }

/*=================================================================================**//**
* EachLineStyleArg
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
EachLineStyleArg::EachLineStyleArg (int32_t stored, LineStyleParams const* params, PropsCallbackFlags flags, PropertyContext& processor)
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
int32_t         EachLineStyleArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t         EachLineStyleArg::GetEffectiveValue ()
    {
    int32_t                     effective = m_storedValue;
#if defined (NEEDS_WORK_DGNITEM)
    if (STYLE_BYCELL == effective)
        {
        // Use BYLEVEL style from default category (should be continuous for DWG...)
        DgnCategories::SubCategory subCategory = DGN_TABLE_LEVEL_FOR_MODEL(m_processor.GetSourceDgnModel()).QuerySubCategoryByKey (SubCategoryKey(LEVEL_DEFAULT_LEVEL_ID));
        if (subCategory.IsValid ())
            effective = subCategory.GetAppearance().GetStyle();
        else
            effective = 0;
        }
    else if (STYLE_BYLEVEL == effective)
        {
        DgnCategoryId currentCategory = m_processor.GetCurrentCategoryID ();

        DgnCategories::SubCategory subCategory = DGN_TABLE_LEVEL_FOR_MODEL(m_processor.GetSourceDgnModel()).QuerySubCategoryByKey (SubCategoryKey(currentCategory));
        if (subCategory.IsValid ())
            effective = subCategory.GetAppearance().GetStyle();
        }
#endif

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
StatusInt       EachLineStyleArg::SetStoredValue (int32_t newVal)
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
EachFontArg::EachFontArg (DgnFontId stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnFontId        EachFontArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachFontArg::SetStoredValue (DgnFontId newVal)
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
EachTextStyleArg::EachTextStyleArg (uint32_t stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue     = stored;
    m_paramsRemapping = (NULL == m_processor.GetIEditPropertiesP () ? StyleParamsRemapping::Invalid : StyleParamsRemapping::NoChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        EachTextStyleArg::GetStoredValue ()
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
StatusInt       EachTextStyleArg::SetStoredValue (uint32_t newVal)
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
EachDimStyleArg::EachDimStyleArg (uint64_t stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue     = stored;
    m_paramsRemapping = (NULL == m_processor.GetIEditPropertiesP () ? StyleParamsRemapping::Invalid : StyleParamsRemapping::NoChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t     EachDimStyleArg::GetStoredValue ()
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
StatusInt       EachDimStyleArg::SetStoredValue (uint64_t newVal)
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
EachMLineStyleArg::EachMLineStyleArg (uint64_t stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue     = stored;
    m_paramsRemapping = (NULL == m_processor.GetIEditPropertiesP () ? StyleParamsRemapping::Invalid : StyleParamsRemapping::NoChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t EachMLineStyleArg::GetStoredValue ()
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
StatusInt       EachMLineStyleArg::SetStoredValue (uint64_t newVal)
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
EachWeightArg::EachWeightArg (uint32_t stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        EachWeightArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachWeightArg::SetStoredValue (uint32_t newVal)
    {
    m_storedValue = newVal;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        EachWeightArg::GetEffectiveValue ()
    {
    uint32_t                    effective = m_storedValue;
#if defined (NEEDS_WORK_DGNITEM)
    if (WEIGHT_BYCELL == effective)
        {
        effective = 0; // If BYCELL weight and not in a cell...weigbt is ALWAYS weight 0...
        }
    else if (WEIGHT_BYLEVEL == effective)
        {
        DgnCategoryId currentCategory = m_processor.GetCurrentCategoryID ();

        DgnCategories::SubCategory subCategory = DGN_TABLE_LEVEL_FOR_MODEL(m_processor.GetSourceDgnModel()).QuerySubCategoryByKey (SubCategoryKey(currentCategory));
        if (subCategory.IsValid ())
            effective = subCategory.GetAppearance().GetWeight ();
        }
#endif

    return effective;
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
EachDisplayPriorityArg::EachDisplayPriorityArg (int32_t stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue = stored;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t         EachDisplayPriorityArg::GetStoredValue ()
    {
    return m_storedValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t         EachDisplayPriorityArg::GetEffectiveValue ()
    {
    int32_t     effective = m_storedValue;

    return effective;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EachDisplayPriorityArg::SetStoredValue (int32_t newVal)
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
EachElementTemplateArg::EachElementTemplateArg (DgnElementId stored, PropsCallbackFlags flags, PropertyContext& processor)
    :
    EachPropertyBaseArg (flags, processor)
    {
    m_storedValue       = stored;
    m_applyDefaultSymb  = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    EachElementTemplateArg::GetStoredValue ()
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
StatusInt       EachElementTemplateArg::SetStoredValue (DgnElementId newVal)
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
void PropertyContext::SetCurrentElemHandleP (ElementHandleCP eh)
    {
    m_elmHandle = eh;

    /* NOTE: Category needed for effective value. Should always be true that
             GetCategory is base category id. Mlines will need to account for
             profile categories; multi-category elements are an aberration. */

    SetCurrentCategoryID (eh ? eh->GetDgnElement()->GetCategoryId() : DgnCategoryId());
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
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoColorCallback (uint32_t* pNewColor, EachColorArg& arg)
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
        uint32_t color = arg.GetStoredValue();

        m_editObj->_EachColorCallback (arg);

        uint32_t newColor = arg.GetStoredValue();

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
bool            PropertyContext::DoCategoryCallback (DgnCategoryId* pNewCategoryID, EachCategoryArg& arg)
    {
    if (0 == (ELEMENT_PROPERTY_Category & GetElementPropertiesMask ()))
        return false;

    if (NULL != m_queryObj)
        {
        m_queryObj->_EachCategoryCallback (arg);
        return false;
        }

    if (NULL != m_editObj)
        {
        DgnCategoryId categoryID = arg.GetStoredValue();

        m_editObj->_EachCategoryCallback (arg);

        DgnCategoryId newCategoryID = arg.GetStoredValue();

        if (NULL != pNewCategoryID && newCategoryID != categoryID)
            {
            *pNewCategoryID = newCategoryID;

            SetElementChanged();
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyContext::DoLineStyleCallback (int32_t* pNewStyleID, EachLineStyleArg& arg)
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
        int32_t         styleID = arg.GetStoredValue();
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

        int32_t newStyleID = arg.GetStoredValue();

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
bool            PropertyContext::DoFontCallback (DgnFontId* pNewFontNo, EachFontArg& arg)
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
        DgnFontId fontNo = arg.GetStoredValue();

        m_editObj->_EachFontCallback (arg);

        DgnFontId newFontNo = arg.GetStoredValue();

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
bool            PropertyContext::DoDimStyleCallback (uint64_t* pNewStyleID, EachDimStyleArg& arg)
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
        uint64_t styleID = arg.GetStoredValue();

        m_editObj->_EachDimStyleCallback (arg);

        uint64_t newStyleID = arg.GetStoredValue();

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
bool            PropertyContext::DoMLineStyleCallback (DgnElementId* pNewStyleID, EachMLineStyleArg& arg)
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
        uint64_t styleID = arg.GetStoredValue();

        m_editObj->_EachMLineStyleCallback (arg);

        uint64_t newStyleID = arg.GetStoredValue();

        if (NULL != pNewStyleID && newStyleID != styleID)
            {
            *pNewStyleID = DgnElementId ((int64_t) newStyleID);

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
bool            PropertyContext::DoWeightCallback (uint32_t* pNewVal, EachWeightArg& arg)
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
        uint32_t    oldVal = arg.GetStoredValue();

        m_editObj->_EachWeightCallback (arg);

        uint32_t    newVal = arg.GetStoredValue();

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
bool            PropertyContext::DoDisplayPriorityCallback (int32_t* pNewVal, EachDisplayPriorityArg& arg)
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
        int32_t     oldVal = arg.GetStoredValue();

        m_editObj->_EachDisplayPriorityCallback (arg);

        int32_t     newVal = arg.GetStoredValue();

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
bool            PropertyContext::DoElementTemplateCallback (DgnElementId* pNewVal, EachElementTemplateArg& arg)
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
        DgnElementId   oldVal = arg.GetStoredValue();

        m_editObj->_EachElementTemplateCallback (arg);

        DgnElementId   newVal = arg.GetStoredValue();

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
        childElm = ChildElemIter (ElementHandle (scQuery->GetDefinition(eh, *eh.GetDgnDb())), ExposeChildrenReason::Query);
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
#if defined (NEEDS_WORK_DGNITEM)
    PropertyContext context (obj);
    eh.GetElementHandler().QueryProperties (eh, context);
#endif
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
#if defined (NEEDS_WORK_DGNITEM)
    PropertyContext context (obj);

    eeh.GetElementHandler().EditProperties (eeh, context);

    return context.GetElementChanged ();
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            PropertyContext::QueryPathProperties (HitPathCP path, IQueryProperties* obj)
    {
#if defined (NEEDS_WORK_DGNITEM)
    if (!path)
        return;

    PropertyContext context (obj);

    PropertyContext::ContextMark mark (context); // restored in destructor!

    for (int iPath=0; iPath <= path->GetCount (); iPath++)
        {
        ElementHandle  eh (path->GetPathElem (iPath));

        if (!eh.IsValid ())
            break;

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

        ElemHeaderOverrides         ovr;
        context.PushOverrides (&ovr);
        }
#endif
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     PropertyContext::ContextMark::SetNow ()
    {
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
