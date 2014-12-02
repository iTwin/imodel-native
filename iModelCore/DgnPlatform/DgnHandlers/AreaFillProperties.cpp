/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/AreaFillProperties.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAreaFillPropertiesQuery::_GetAreaType (ElementHandleCR eh, bool* isHoleP) const
    {
#if defined (NEEDS_WORK_DGNITEM)
    if (isHoleP)
        *isHoleP = eh.GetElementCP()->IsHole();
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAreaFillPropertiesQuery::_GetSolidFill (ElementHandleCR eh, UInt32* fillColorP, bool* alwaysFilledP) const
    {
    DgnElementCP elmCP = eh.GetElementCP ();

    Display_attribute   attribute;

    if (!mdlElement_displayAttributePresent (elmCP, FILL_ATTRIBUTE, &attribute))
        return false;

    if (fillColorP)
        *fillColorP = attribute.attr_data.fill.color;

    if (alwaysFilledP)
        *alwaysFilledP = attribute.attr_data.fill.alwaysFilled;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAreaFillPropertiesQuery::_GetGradientFill (ElementHandleCR eh, GradientSymbPtr& symb) const
    {
    DgnElementCP elmCP = eh.GetElementCP ();

    Display_attribute   attribute;

    if (!mdlElement_displayAttributePresent (elmCP, GRADIENT_ATTRIBUTE, &attribute))
        return false;

    symb = GradientSymb::Create ();    

    return (SUCCESS == symb->FromDisplayAttribute (&attribute.attr_data.gradient) ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAreaFillPropertiesQuery::_GetPattern (ElementHandleCR eh, PatternParamsPtr& params, bvector<DwgHatchDefLine>* hatchDefLinesP, DPoint3dP originP, int index) const
    {
    params = PatternParams::Create ();

    if (hatchDefLinesP)
        {
        hatchDefLinesP->clear ();
        hatchDefLinesP->reserve (MAX_DWG_EXPANDEDHATCH_LINES);
        }

#if defined (NEEDS_WORK_DGNITEM)
    if (SUCCESS != PatternLinkageUtil::ExtractFromElement (NULL, *params, hatchDefLinesP ? &hatchDefLinesP->front () : NULL, hatchDefLinesP ? (int) hatchDefLinesP->capacity () : 0, originP, *eh.GetElementCP (), eh.GetDgnModelP (), index))
        return false;
#endif

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAreaFillPropertiesEdit::_RemoveAreaFill (EditElementHandleR eeh)
    {
    bool        changed = false;

    if (mdlElement_displayAttributeRemove (eeh.GetElementP (), FILL_ATTRIBUTE))
        changed = true;

    if (mdlElement_displayAttributeRemove (eeh.GetElementP (), GRADIENT_ATTRIBUTE))
        changed = true;

    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAreaFillPropertiesEdit::_RemovePattern (EditElementHandleR eeh, int index)
    {
#if defined (NEEDS_WORK_DGNITEM)
    return (0 != PatternLinkageUtil::DeleteFromElement (*eeh.GetElementP (), index));
#endif
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAreaFillPropertiesEdit::_AddSolidFill (EditElementHandleR eeh, UInt32* fillColorP, bool* alwaysFilledP)
    {
    DgnElementP  elmP = eeh.GetElementP ();
    DgnElementP  tmpElmP = (DgnElementP) alloca (elmP->Size () + (2*sizeof (Display_attribute)));

    elmP->CopyTo (*tmpElmP);

    mdlElement_displayAttributeRemove (tmpElmP, FILL_ATTRIBUTE); // Make sure fill attribute is singleton!
    mdlElement_displayAttributeRemove (tmpElmP, GRADIENT_ATTRIBUTE);

    bool        alwaysFilled = (alwaysFilledP ? *alwaysFilledP : false);

    if (SUCCESS != mdlElement_addFillDisplayAttribute (tmpElmP, fillColorP, &alwaysFilled))
        return false;

    return (SUCCESS == eeh.ReplaceElement (tmpElmP) ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAreaFillPropertiesEdit::_AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb)
    {
    DgnElementP  elmP = eeh.GetElementP ();
    DgnElementP  tmpElmP = (DgnElementP) alloca (elmP->Size () + (2*sizeof (Display_attribute)));

    elmP->CopyTo (*tmpElmP);

    mdlElement_displayAttributeRemove (tmpElmP, FILL_ATTRIBUTE); // Make sure fill attribute is singleton!
    mdlElement_displayAttributeRemove (tmpElmP, GRADIENT_ATTRIBUTE);

    if (SUCCESS != mdlElement_addGradientDisplayAttribute (tmpElmP, symb))
        return false;

    return (SUCCESS == eeh.ReplaceElement (tmpElmP) ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAreaFillPropertiesEdit::_AddPattern (EditElementHandleR eeh, PatternParamsR params, DwgHatchDefLineP hatchDefLinesP, int index)
    {
#if defined (NEEDS_WORK_DGNITEM)
    if (0 != index && NULL == dynamic_cast <IMultilineQuery*> (&eeh.GetHandler ()))
        index = 0; // Since index is now supplied by caller we can't trust it...

    if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Cell)) // area pattern
        {
        DgnProjectP        dgnFile = eeh.GetDgnProject ();

        if (0 == params.cellId && NULL != dgnFile)
            {
            PersistentElementRefPtr sharedCellDef;

            // add the shared cell def to file, if its not already there...
            sharedCellDef = ISharedCellQuery::FindDefinitionByName  (params.cellName, *dgnFile);

            if (sharedCellDef.IsValid())
                params.cellId = sharedCellDef->GetElementId().GetValue();
            }

        if (0 == params.cellId)
            return false;
         }

    DgnV8ElementBlank   tmpElm;
    eeh.GetElementP ()->CopyTo (tmpElm);

    if (SUCCESS != PatternLinkageUtil::AddToElement (tmpElm, params, hatchDefLinesP, index))
        return false;
    return (SUCCESS == eeh.ReplaceElement (&tmpElm) ? true : false);
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAreaFillPropertiesEdit::_SetAreaType (EditElementHandleR eeh, bool isHole)
    {
#if defined (NEEDS_WORK_DGNITEM)
    eeh.GetElementP()->SetIsHole(isHole);
#endif
    return true;
    }


