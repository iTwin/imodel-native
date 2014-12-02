/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DisplayAttribute.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

struct FindDisplayAttribute
    {
    UInt32              attributeID;
    Display_attribute*  attributeP;
    bool                hasDisplayAttr;
    int                 desiredAction;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     findDisplayAttribute
(
LinkageHeader   *replaceLinkBufP,
void            *paramsP,
LinkageHeader   *linkBufP,
DgnElement       *elmP
)
    {
    FindDisplayAttribute    *findDisplayAttrP = (FindDisplayAttribute *) paramsP;
    Display_attribute       *daP = (Display_attribute *) linkBufP;

    if (!linkBufP->user)
        return PROCESS_ATTRIB_STATUS_NOCHANGE;

    if (daP->linkHdr.primaryID == DISPLAY_ATTRIBUTE_ID &&
        daP->attr_type == findDisplayAttrP->attributeID)
        {
        findDisplayAttrP->hasDisplayAttr = true;

        switch (findDisplayAttrP->desiredAction)
            {
            case PROCESS_ATTRIB_STATUS_ABORT:
                // Return linkage found
                if (findDisplayAttrP->attributeP)
                    memcpy (findDisplayAttrP->attributeP, daP, LinkageUtil::GetWords (&daP->linkHdr) * 2);
                break;

            case PROCESS_ATTRIB_STATUS_REPLACE:
                // Replace the linkage found
                if (findDisplayAttrP->attributeP)
                    memcpy (replaceLinkBufP, findDisplayAttrP->attributeP, LinkageUtil::GetWords (&findDisplayAttrP->attributeP->linkHdr) * 2);
                break;

            case PROCESS_ATTRIB_STATUS_DELETE:
                // Don't need to do anything
                break;
            }

        return findDisplayAttrP->desiredAction;
        }

    return PROCESS_ATTRIB_STATUS_NOCHANGE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   12/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   displayAttributeVisitFunc
(
LinkageHeader const*   linkage,
void*                  arg
)
    {
    FindDisplayAttribute*    fda = (FindDisplayAttribute*) arg;
    Display_attribute const* daP = (Display_attribute const*) linkage;

    if (linkage->user && daP->linkHdr.primaryID == DISPLAY_ATTRIBUTE_ID && daP->attr_type == fda->attributeID)
        {
        fda->hasDisplayAttr = true;
        
        if (fda->attributeP)
            {
            int linkageSize = LinkageUtil::GetWords (&daP->linkHdr) * 2;
            
            // TR#295423 - the test file had Fill attributes that appeared to be V7 format linkages, which were smaller than the V8 Fill linkage. Make sure the part beyond is cleared.
            if ((FILL_ATTRIBUTE == fda->attributeID) && (linkageSize < (offsetof (Display_attribute, attr_data) + sizeof (Display_attribute_fill))))
                memset (fda->attributeP, 0, (offsetof (Display_attribute, attr_data) + sizeof (Display_attribute_fill)));
            
            memcpy (fda->attributeP, daP, linkageSize);
            }
        // error means stop.
        return ERROR;
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   12/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   findAttributeVisitFunc
(
LinkageHeader const*   linkage,
void*                  arg
)
    {
    FindDisplayAttribute*    fda = (FindDisplayAttribute*) arg;

    if (linkage->user && (linkage->primaryID == fda->attributeID))
        {
        fda->hasDisplayAttr = true;

        if (fda->attributeP)
            memcpy (fda->attributeP, linkage, LinkageUtil::GetWords (linkage) * 2);

        // error means stop.
        return  ERROR;
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlElement_displayAttributePresent
(
DgnElementCP         pElm,           // => element pointer
int                 attributeID,    // => attr_type to look for
Display_attribute*  attributeP      // <= buffer large enough to hold type (or NULL)
)
    {
    FindDisplayAttribute    findDisplayAttr;

    findDisplayAttr.attributeID     = attributeID;
    findDisplayAttr.attributeP      = attributeP;
    findDisplayAttr.hasDisplayAttr  = false;

    mdlElement_visitLinkages (displayAttributeVisitFunc, &findDisplayAttr, pElm);

    return findDisplayAttr.hasDisplayAttr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlElement_attributePresent
(
DgnElement const*        pElm,           // => element pointer
int                     attributeID,    // => attr_type to look for
void*                   attributeP      // <= buffer large enough to hold type (or NULL)
)
    {
    FindDisplayAttribute    findDisplayAttr;

    findDisplayAttr.attributeID     = attributeID;
    findDisplayAttr.attributeP      = (Display_attribute *) attributeP;
    findDisplayAttr.hasDisplayAttr  = false;

    mdlElement_visitLinkages (findAttributeVisitFunc, &findDisplayAttr, pElm);

    return findDisplayAttr.hasDisplayAttr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlElement_displayAttributeRemove
(
DgnElement       *pElm,               /* => element pointer */
int             attributeID
)
    {
    FindDisplayAttribute    findDisplayAttr;

    memset (&findDisplayAttr, 0, sizeof (findDisplayAttr));
    findDisplayAttr.attributeID     = attributeID;
    findDisplayAttr.attributeP      = NULL;
    findDisplayAttr.desiredAction   = PROCESS_ATTRIB_STATUS_DELETE;

    mdlElement_processLinkages (findDisplayAttribute, &findDisplayAttr, pElm);

    return findDisplayAttr.hasDisplayAttr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlElement_displayAttributeReplace
(
DgnElement               *pElm,          /* => element pointer                   */
int                     attributeID,    /* => attr_type to look for             */
Display_attribute       *attributeP     /* => pointer to new display attr       */
)
    {
    FindDisplayAttribute    findDisplayAttr;

    memset (&findDisplayAttr, 0, sizeof (findDisplayAttr));
    findDisplayAttr.attributeID     = attributeID;
    findDisplayAttr.attributeP      = attributeP;
    findDisplayAttr.desiredAction   = PROCESS_ATTRIB_STATUS_REPLACE;

    mdlElement_processLinkages (findDisplayAttribute, &findDisplayAttr, pElm);

    return findDisplayAttr.hasDisplayAttr ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlElement_displayAttributeCreate
(
Display_attribute   *pDa,           /* Display_attribute buffer padded to account for databytes */
int                 attributeID,
int                 dataBytes,
UShort              *attributeDataP
)
    {
    int             linkBytes = offsetof (Display_attribute, attr_data) + dataBytes;

    memset (pDa, 0, sizeof (Display_attribute));

    pDa->linkHdr.primaryID  = DISPLAY_ATTRIBUTE_ID;
    pDa->linkHdr.user       = true;
    pDa->attr_type          = attributeID;

    memcpy (&pDa->attr_data, attributeDataP, dataBytes);

    linkBytes = ((linkBytes + 7) & ~7);
    LinkageUtil::SetWords ((LinkageHeader *) &pDa->linkHdr, linkBytes/2);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlElement_displayAttributeAdd
(
DgnElementP          pElm,       /* <=> element pointer          */
Display_attribute   *pDa        /*  => display_attribute to add */
)
    {
    return elemUtil_appendLinkage (pElm, (LinkageHeader *) pDa);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             1/88
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlElement_addFillDisplayAttribute
(
DgnElementP   elmP,
UInt32*      fillColor,
bool*        alwaysFilled
)
    {
    StatusInt               status;
    Display_attribute       dispAttr;
    Display_attribute_fill  fillAttr;

    memset (&fillAttr, 0, sizeof (fillAttr));
    fillAttr.color = fillColor ? *fillColor : elmP->GetSymbology().color;
    fillAttr.alwaysFilled = alwaysFilled ? *alwaysFilled : false;

    if (SUCCESS == (status = mdlElement_displayAttributeCreate (&dispAttr, FILL_ATTRIBUTE, sizeof (fillAttr), (UShort *) &fillAttr)))
        status = mdlElement_displayAttributeAdd (elmP, &dispAttr);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlElement_addGradientDisplayAttribute
(
DgnElementP      elmP,
GradientSymbCR  gradientSymb
)
    {
    StatusInt                   status;
    Display_attribute_gradient  gradient;
    Display_attribute           displayAttribute;

    if (SUCCESS != (status = gradientSymb.ToDisplayAttribute (gradient)))
        return status;

    if (SUCCESS != (status = mdlElement_displayAttributeCreate (&displayAttribute, GRADIENT_ATTRIBUTE, sizeof (gradient) - (MAX_GRADIENT_KEYS-gradient.nKeys) * sizeof (GradientKey), (UShort *) &gradient)))
        return status;

    return mdlElement_displayAttributeAdd (elmP, &displayAttribute);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/13
//---------------------------------------------------------------------------------------
bool BentleyApi::mdlElement_addTransparencyDisplayAttribute (EditElementHandleR eeh, double transparency)
    {
    DgnElementP          elmP = eeh.GetElementP();
    Display_attribute   dispAttr;
    if (mdlElement_displayAttributePresent (elmP, TRANSPARENCY_ATTRIBUTE, &dispAttr))
        {
        dispAttr.attr_data.transparency.transparency = transparency;
        mdlElement_displayAttributeReplace (elmP, TRANSPARENCY_ATTRIBUTE, &dispAttr);
        return true;
        }

    DgnElementP tmpElmP = (DgnElementP) alloca (elmP->Size() + (2*sizeof (Display_attribute)));
    elmP->CopyTo(*tmpElmP);

    Display_attribute_transparency transparencyData;
    transparencyData.transparency = transparency;

    if (SUCCESS != mdlElement_displayAttributeCreate (&dispAttr, TRANSPARENCY_ATTRIBUTE, sizeof (transparencyData), (UShort *) &transparencyData))
        return false;

    if (SUCCESS != mdlElement_displayAttributeAdd (tmpElmP, &dispAttr))
        return false;

    return (SUCCESS == eeh.ReplaceElement (tmpElmP) ? true : false);
    }
