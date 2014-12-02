/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementUtil.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/Linkage1.h>

#if defined (NEEDS_WORK_DGNITEM)
typedef struct
    {
    LinkageHeader   hdr;
    UInt32          scaleDimensions:1;      // true = when scInst is scaled, unscale the dim value (value should remain constant) AND scale dim text size [this is the opposite of regular dimension scaling]
    UInt32          scaleMultilines:1;      // true = when scInst is scaled, scale the multiline offset
    UInt32          isAnnotation:1;         // true = scDef is an annotation cell
    UInt32          rotateDimensions:1;     // true = when scInst is rotated, rotate the dim view matrix (so that text orientation remains constant)
    UInt32          reserved:28;

    } SharedCellFlagsLinkage;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/00
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isCellElement (DgnElementCR elm)
    {
    return (CELL_HEADER_ELM == elm.GetLegacyType() || SHAREDCELL_DEF_ELM == elm.GetLegacyType() || SHARED_CELL_ELM == elm.GetLegacyType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::GetCellName (WCharP cellNameP, int bufferSize, DgnElementCR elm)
    {
    cellNameP[0] = '\0';

    if (!isCellElement (elm))
        return ERROR;

    if (SUCCESS != LinkageUtil::ExtractNamedStringLinkageByIndex (cellNameP, bufferSize, STRING_LINKAGE_KEY_Name, 0, &elm))
        return ERROR;

    strutil_wstrpwspc (cellNameP);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::GetCellDescription (WCharP descrP, int bufferSize, DgnElementCR elm)
    {
    descrP[0] = '\0';

    if (!isCellElement (elm))
        return ERROR;

    if (SUCCESS != LinkageUtil::ExtractNamedStringLinkageByIndex (descrP, bufferSize, STRING_LINKAGE_KEY_Description, 0, &elm))
        return ERROR;

    strutil_wstrpwspc (descrP);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::ExtractName (WCharP cellName, int bufferSize, ElementHandleCR eh)
    {
    return CellUtil::GetCellName (cellName, bufferSize, *eh.GetElementCP ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::ExtractDescription (WCharP descr, int bufferSize, ElementHandleCR eh)
    {
    return CellUtil::GetCellDescription (descr, bufferSize, *eh.GetElementCP ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::SetCellName (DgnElementR elm, WCharCP cellNameP)
    {
    if (!isCellElement (elm))
        return ERROR;

    WChar     nameBuf[MAX_CELLNAME_LENGTH];

    wcsncpy (nameBuf, cellNameP, MAX_CELLNAME_LENGTH);
    nameBuf[MAX_CELLNAME_LENGTH-1] = 0;
    strutil_wstrpwspc (nameBuf);

    if (wcslen (nameBuf))
        return (BentleyStatus) LinkageUtil::SetStringLinkage (&elm, STRING_LINKAGE_KEY_Name, nameBuf);

    return (BentleyStatus) LinkageUtil::DeleteStringLinkage (&elm, STRING_LINKAGE_KEY_Name, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/00
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::SetCellDescription (DgnElementR elm, WCharCP descrP)
    {
    if (!isCellElement (elm))
        return ERROR;

    WChar     nameBuf[MAX_CELLDSCR_LENGTH];

    wcsncpy (nameBuf, descrP, MAX_CELLDSCR_LENGTH);
    nameBuf[MAX_CELLDSCR_LENGTH-1] = 0;
    strutil_wstrpwspc (nameBuf);

    if (wcslen (nameBuf))
        return (BentleyStatus) LinkageUtil::SetStringLinkage (&elm, STRING_LINKAGE_KEY_Description, nameBuf);

    return (BentleyStatus) LinkageUtil::DeleteStringLinkage (&elm, STRING_LINKAGE_KEY_Description, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CellUtil::IsPointCell (ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return false;

    switch (eh.GetLegacyType())
        {
        case CELL_HEADER_ELM:
        case SHARED_CELL_ELM:
        case SHAREDCELL_DEF_ELM:
            {
            DgnElementCR el = *eh.GetElementCP();
            return (!el.IsSnappable() && el.IsViewIndependent());
            }

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CellUtil::IsAnnotation (ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return false;

    switch (eh.GetLegacyType())
        {
        case CELL_HEADER_ELM:
            return ((Cell_2d const*)eh.GetElementCP())->flags.isAnnotation;

        case SHARED_CELL_ELM:
            return CellUtil::IsAnnotation (ElementHandle (eh.GetDgnProject()->Models().FindSharedCellDefForInstance(*eh.GetElementCP()).get()));

        case SHAREDCELL_DEF_ELM:
            {
            bool    isAnnotation;

            CellUtil::GetSharedCellDefFlags (NULL, NULL, NULL, &isAnnotation, eh);

            return isAnnotation;
            }

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CellUtil::IsAnonymous (ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return false;

    switch (eh.GetLegacyType())
        {
        case CELL_HEADER_ELM:
            return eh.GetElementCP()->IsHole();

        case SHARED_CELL_ELM:
            return CellUtil::IsAnonymous (ElementHandle (eh.GetDgnProject()->Models().FindSharedCellDefForInstance(*eh.GetElementCP()).get()));

        case SHAREDCELL_DEF_ELM:
            return TO_BOOL(((SharedCellDef const*)eh.GetElementCP())->anonymous);

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::ExtractOrigin (DPoint3dR origin, ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return ERROR;

    switch (eh.GetLegacyType())
        {
        case CELL_HEADER_ELM:
            {
            DgnElementCR el = *eh.GetElementCP ();

            if (el.Is3d())
                origin = ((Cell_3d const&)el).origin;
            else
                origin.init (&((Cell_2d const&)el).origin);

            return SUCCESS;
            }

        case SHARED_CELL_ELM:
            {
            origin = ((SharedCell const*)eh.GetElementCP())->origin;

            return SUCCESS;
            }

        case SHAREDCELL_DEF_ELM:
            {
            origin = ((SharedCellDef const*)eh.GetElementCP())->origin;

            return SUCCESS;
            }

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      RayBentley    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::ExtractRangeDiagonal (DRange3dR rDiag, ElementHandleCR eh)
    {
    DgnElementCP         el;

    if (CELL_HEADER_ELM != eh.GetLegacyType() ||
        NULL == (el = eh.GetElementCP()))
        return ERROR;
    
    if (el->Is3d())
        {
        Cell_3d const& cell_3d = (Cell_3d const&)*el;
        rDiag.low    = cell_3d.rnglow;
        rDiag.high   = cell_3d.rnghigh;
        }
    else
        {
        Cell_2d const& cell_2d = (Cell_2d const&)*el;
        rDiag.low.x  = cell_2d.rnglow.x;
        rDiag.low.y  = cell_2d.rnglow.y;
        rDiag.high.x = cell_2d.rnghigh.x;
        rDiag.high.y = cell_2d.rnghigh.y;
        rDiag.low.z  = rDiag.high.z = 0.0;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::ExtractRotation (RotMatrixR rMatrix, ElementHandleCR eh)
    {
    if (!eh.IsValid ())
        return ERROR;

    switch (eh.GetLegacyType())
        {
        case CELL_HEADER_ELM:
            {
            DgnElementCR el = *eh.GetElementCP ();

            if (el.Is3d())
                memcpy (rMatrix.form3d, el.ToCell_3d().transform, 9 * sizeof (double));
            else
                rMatrix.InitFromRowValuesXY ( &el.ToCell_2d().transform[0][0]);

            return SUCCESS;
            }

        case SHARED_CELL_ELM:
            {
            memcpy (&rMatrix, &eh.GetElementCP ()->ToSharedCell().rotScale, sizeof (RotMatrix));

            return SUCCESS;
            }

        case SHAREDCELL_DEF_ELM:
            {
            memcpy (&rMatrix, &eh.GetElementCP ()->ToSharedCellDef().rotScale, sizeof (RotMatrix));

            return SUCCESS;
            }

        default:
            return ERROR;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::GetSharedCellDefID (ElementHandleCR eh, ElementId& elemID)
    {
    DependencyLinkageAccessor depLinkP;

    if (SUCCESS != DependencyManagerLinkage::GetLinkage (&depLinkP, eh, DEPENDENCYAPPID_SharedCellDef, 0))
        return ERROR;

    if (depLinkP->u.f.invalid)
        return ERROR;

    elemID = ElementId(depLinkP->root.elemid[0]);

    return (0 != elemID.GetValue()) ? SUCCESS : ERROR;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::GetSharedCellDefFlags
(
bool*           pNondefaultScaleForDims,    // true = when scInst is scaled, unscale the dim value (value should remain constant) AND scale dim text size
bool*           pScaleMultilines,           // true = when scInst is scaled, scale the multiline offset
bool*           pRotateDimView,             // true = when scInst is rotated, rotate the dim view matrix (so that text orientation remains constant)
bool*           pIsAnnotation,              // true = scDef is an annotation cell
ElementHandleCR    eh
)
    {
    DgnElementCR el = *eh.GetElementCP ();

    if (NULL != pNondefaultScaleForDims)
        *pNondefaultScaleForDims = false;

    if (NULL != pScaleMultilines)
        *pScaleMultilines = false;

    if (NULL != pIsAnnotation)
        *pIsAnnotation = false;

    if (NULL != pRotateDimView)
        *pRotateDimView = false;

    if (SHAREDCELL_DEF_ELM != el.GetLegacyType())
        return ERROR;

    SharedCellFlagsLinkage  sharedCellFlagsLinkage;

    if (NULL == elemUtil_extractLinkage (&sharedCellFlagsLinkage, NULL, &el, LINKAGEID_SharedCellFlags))
        return ERROR;

    if (NULL != pNondefaultScaleForDims)
        *pNondefaultScaleForDims = sharedCellFlagsLinkage.scaleDimensions;

    if (NULL != pScaleMultilines)
        *pScaleMultilines  = sharedCellFlagsLinkage.scaleMultilines;

    if (NULL != pIsAnnotation)
        *pIsAnnotation  = sharedCellFlagsLinkage.isAnnotation;

    if (NULL != pRotateDimView)
        *pRotateDimView  = sharedCellFlagsLinkage.rotateDimensions;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::SetSharedCellDefFlags
(
EditElementHandleR eeh,
bool            nondefaultScaleForDims, // true = when scInst is scaled, unscale the dim value (value should remain constant) AND scale dim text size [this is the opposite of regular dimension scaling]
bool            scaleMultilines,        // true = when scInst is scaled, scale the multiline offset
bool            rotateDimView,          // true = when scInst is rotated, rotate the dim view matrix (so that text orientation remains constant)
bool            isAnnotation            // true = scDef is an annotation cell
)
    {
    DgnV8ElementBlank elm(*eeh.GetElementCP());

    if (nondefaultScaleForDims || scaleMultilines || isAnnotation || rotateDimView)
        {
        int     linkBytes;

        SharedCellFlagsLinkage    sharedCellFlagsLinkage;

        memset (&sharedCellFlagsLinkage, 0, sizeof (sharedCellFlagsLinkage));

        sharedCellFlagsLinkage.hdr.user       = 1;
        sharedCellFlagsLinkage.hdr.primaryID  = LINKAGEID_SharedCellFlags;

        linkBytes = (sizeof (sharedCellFlagsLinkage) + 7) & ~7;
        LinkageUtil::SetWords (&sharedCellFlagsLinkage.hdr, linkBytes/sizeof (short));

        // NOTE : sharedCellFlagsLinkage.scaleDimensions is a misnomer. It actually means
        // that the desired scaling behavior is opposite to the regular dim scaling behavior.

        sharedCellFlagsLinkage.scaleDimensions  = nondefaultScaleForDims;
        sharedCellFlagsLinkage.scaleMultilines  = scaleMultilines;
        sharedCellFlagsLinkage.rotateDimensions = rotateDimView;
        sharedCellFlagsLinkage.isAnnotation     = isAnnotation;

        elemUtil_deleteLinkage (&elm, LINKAGEID_SharedCellFlags);

        if (SUCCESS != elemUtil_appendLinkage (&elm, &sharedCellFlagsLinkage.hdr))
            return ERROR;
        }
    else
        {
        if (SUCCESS != elemUtil_deleteLinkage (&elm, LINKAGEID_SharedCellFlags))
            return SUCCESS;
        }

    eeh.ReplaceElement (&elm);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::ClipFromLinkage (ClipVectorPtr& clip, ElementHandleCR eh, TransformCP outputTransform, bool applyElementTransform)
    {
    // NOTE: For clip linkage on cells/shared cells from DWG...
    if (!mdlElement_attributePresent (eh.GetElementCP (), LINKAGEID_ClipBoundary, NULL))
        return ERROR;

    DPoint3d    originElem;
    RotMatrix   rMatrixElem;

    // NOTE: Still need to verify that it is a cell when not applying element transform...
    if (SUCCESS != CellUtil::ExtractOrigin (originElem, eh) || SUCCESS != CellUtil::ExtractRotation (rMatrixElem, eh))
        return ERROR;

    for (ConstElementLinkageIterator li = eh.BeginElementLinkages(); li != eh.EndElementLinkages(); ++li)
        {
        if (!li->user || LINKAGEID_ClipBoundary != li->primaryID)
            continue;

        DPoint3d    clipOrigin;
        RotMatrix   clipRMatrix;
        double      zLow, zHigh;
        bool        zLowValid = false, zHighValid = false;
        UInt32      flags;
        UInt32      nPoints;

        DataInternalizer reader ((const byte*) li.GetData (), MAX_V8_ELEMENT_SIZE);

        reader.get (&clipRMatrix.form3d[0][0], 9);
        reader.get (&clipOrigin.x, 3);
        reader.get (&zHigh);
        reader.get (&zLow);
        reader.get (&flags);
        reader.get (&nPoints);

        if (!nPoints)
            return ERROR;

        DPoint2dP   points = (DPoint2dP) alloca (nPoints * sizeof (DPoint2d));

        reader.get (&points[0].x, 2 * nPoints);

        zHighValid =  (0 != (flags & 0x0001));
        zLowValid  =  (0 != (flags & 0x0002));

        Transform   clipTransform;

        clipTransform.InitFrom (clipRMatrix, clipOrigin);

        if (applyElementTransform)
            {
            Transform   transform;

            transform.InitFrom (rMatrixElem, originElem);
            clipTransform.InitProduct (transform, clipTransform);
            }

        if (outputTransform)
            clipTransform.InitProduct (*outputTransform, clipTransform);

        clip = ClipVector::CreateFromPrimitive (ClipPrimitive::CreateFromShape (points, nPoints, false, zLowValid ? &zLow : NULL, zHighValid ? &zHigh : NULL, &clipTransform));

        return clip.IsValid () ? SUCCESS : ERROR;
        }

    return ERROR;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::AddClipLinkage
(
EditElementHandleR eeh,
RotMatrixCR     rMatrixIn, 
DPoint3dCR      originIn, 
double          zFront, 
double          zBack, 
bool            frontClipOn, 
bool            backClipOn, 
size_t          nPoints, 
DPoint2dCP      points,
bool            applyElementTransform
)
    {
    /*
    typedef struct  msClipBoundaryLinkageData
        {
        double      rotation[3][3];
        DPoint3d    origin;
        double      zFront;
        double      zBack;
        UInt32      flags;
        UInt32      nPoints;
        DPoint2d    points[0];

        } MSClipBoundaryLinkageData;
    */

    DPoint3d    originElem;
    RotMatrix   rMatrixElem;

    // NOTE: Still need to verify that it is a cell when not applying element transform...
    if (SUCCESS != CellUtil::ExtractOrigin (originElem, eeh) || SUCCESS != CellUtil::ExtractRotation (rMatrixElem, eeh))
        return ERROR;

    DPoint3d    origin = originIn;
    RotMatrix   rMatrix = rMatrixIn;

    if (applyElementTransform)
        {
        Transform   transformElem;

        transformElem.InitFrom (rMatrixElem, originElem);
        transformElem.InverseOf (transformElem);

        transformElem.Multiply (origin);
        rMatrix.InitProduct (rMatrix, transformElem);
        }

    CellUtil::DeleteClipLinkage (eeh);

    UInt32      flags = 0;

    if (frontClipOn)
        flags |= 0x0001;

    if (backClipOn)
        flags |= 0x0002;

    DataExternalizer   writer;

    writer.put (&rMatrix.form3d[0][0], 9);
    writer.put (&origin.x, 3);
    writer.put (zFront);
    writer.put (zBack);
    writer.put (flags);
    writer.put ((UInt32) nPoints);
    writer.put (&points[0].x, 2 * nPoints);

    return ElementLinkageUtil::AddLinkage (eeh, LINKAGEID_ClipBoundary, writer);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CellUtil::DeleteClipLinkage (EditElementHandleR eeh)
    {
    return (1 == linkage_deleteLinkageByIndex (eeh.GetElementP (), LINKAGEID_ClipBoundary, 0) ? SUCCESS : ERROR);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ICellQuery::_IsNormalCell (ElementHandleCR eh)
    {
    return (CELL_HEADER_ELM == eh.GetLegacyType());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ICellQuery::_IsSharedCell (ElementHandleCR eh)
    {
    return (SHARED_CELL_ELM == eh.GetLegacyType());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ICellQuery::_IsSharedCellDefinition (ElementHandleCR eh)
    {
    return (SHAREDCELL_DEF_ELM == eh.GetLegacyType());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ICellQuery::_IsPointCell (ElementHandleCR eh)
    {
    return CellUtil::IsPointCell (eh);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ICellQuery::_IsAnnotation (ElementHandleCR eh)
    {
    return CellUtil::IsAnnotation (eh);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ICellQuery::_IsAnonymous (ElementHandleCR eh)
    {
    return CellUtil::IsAnonymous (eh);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ICellQuery::_ExtractScale (DVec3dR scale, ElementHandleCR eh)
    {
    RotMatrix   rMatrix;

    if (SUCCESS != CellUtil::ExtractRotation (rMatrix, eh))
        return ERROR;

    rMatrix.normalizeColumnsOf (&rMatrix, &scale);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ICellQuery::_ExtractName (WCharP cellName, int bufferSize, ElementHandleCR eh)
    {
    return CellUtil::ExtractName (cellName, bufferSize, eh);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ICellQuery::_ExtractDescription (WCharP descr, int bufferSize, ElementHandleCR eh)
    {
    return CellUtil::ExtractDescription (descr, bufferSize, eh);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ISharedCellQuery::_GetMlineScaleOption (ElementHandleCR eh)
    {
    bool        option;

    // When supplied a shared cell instance get scDef...
    if (SHARED_CELL_ELM == eh.GetLegacyType())
        CellUtil::GetSharedCellDefFlags (NULL, &option, NULL, NULL, ElementHandle (eh.GetDgnProject()->Models().FindSharedCellDefForInstance(*eh.GetElementCP()).get()));
    else
        CellUtil::GetSharedCellDefFlags (NULL, &option, NULL, NULL, eh);

    return option;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ISharedCellQuery::_GetDimScaleOption (ElementHandleCR eh)
    {
    bool        option;

    // When supplied a shared cell instance get scDef...NOTE: Instance has an override for this...
    if (SHARED_CELL_ELM == eh.GetLegacyType())
        CellUtil::GetSharedCellDefFlags (&option, NULL, NULL, NULL, ElementHandle (eh.GetDgnProject()->Models().FindSharedCellDefForInstance(*eh.GetElementCP()).get()));
    else
        CellUtil::GetSharedCellDefFlags (&option, NULL, NULL, NULL, eh);

    return option;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ISharedCellQuery::_GetDimRotationOption (ElementHandleCR eh)
    {
    bool        option;

    // When supplied a shared cell instance get scDef...
    if (SHARED_CELL_ELM == eh.GetLegacyType())
        CellUtil::GetSharedCellDefFlags (NULL, NULL, &option, NULL, ElementHandle (eh.GetDgnProject()->Models().FindSharedCellDefForInstance(*eh.GetElementCP()).get()));
    else
        CellUtil::GetSharedCellDefFlags (NULL, NULL, &option, NULL, eh);

    return option;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP      ISharedCellQuery::_GetDefinition (ElementHandleCR eh, DgnProjectR project)
    {
    return project.Models().FindSharedCellDefForInstance(*eh.GetElementCP()).get();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ISharedCellQuery::_GetDefinitionID (ElementHandleCR eh, ElementId& elemID)
    {
    if (SHARED_CELL_ELM != eh.GetLegacyType())
        return ERROR;

    return CellUtil::GetSharedCellDefID (eh, elemID);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
SCOverride const*   ISharedCellQuery::_GetSharedCellOverrides (ElementHandleCR eh)
    {
    if (SHARED_CELL_ELM != eh.GetLegacyType())
        return NULL;

    return &eh.GetElementCP ()->ToSharedCell().m_override;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefPtr ISharedCellQuery::FindDefinitionByName (WCharCP name, DgnProjectR project)
    {
    if (NULL == name)
        { BeAssert (false); return NULL; }

    Utf8String defName (name);
    return project.Models().GetElementById (project.Models().GetSharedCellDefByName (defName.c_str()));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double  IBRepQuery::GetSolidKernelToUORScale (DgnModelP dgnCache)
    {
    return 1000.;
#if defined (NEEDS_WORK_DGNITEM)
    return dgnCache->GetDgnProject().Units().GetSolidExtent() / 1000.;
#endif
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    06/93
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::AppendRotMatrix (byte*& buffer, RotMatrixCR value, bool is3d)
    {
    double quat[4];

    if (is3d)
        value.GetQuaternion(quat, true);
    else
        value.GetRowValuesXY(quat);

    memcpy (buffer, quat, sizeof (quat));
    buffer += sizeof (quat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    06/93
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::ExtractRotMatrix (RotMatrixR value, byte const *& buffer, bool is3d)
    {
    double quat[4];

    memcpy (quat, buffer, sizeof (quat));
    buffer += sizeof (quat);

    if (is3d)
        value.InitTransposedFromQuaternionWXYZ (quat);
    else
        value.InitFromRowValuesXY (quat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    12/92
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::AppendDPoint3d (byte*& buffer, DPoint3dCR value)
    {
    double rBuf[3];

    rBuf[0] = value.x;
    rBuf[1] = value.y;
    rBuf[2] = value.z;

    memcpy (buffer, rBuf, sizeof (rBuf));
    buffer += sizeof (rBuf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jim.Bartlett    12/92
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::ExtractDPoint3d (DPoint3dR value, byte const *& buffer)
    {
    double rBuf[3];

    memcpy (rBuf, buffer, sizeof (rBuf));
    buffer += sizeof (rBuf);

    value.x = rBuf[0];
    value.y = rBuf[1];
    value.z = rBuf[2];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JVB             08/91
+---------------+---------------+---------------+---------------+---------------+------*/
void ByteStreamHelper::AppendDouble (byte*& buffer, double const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractDouble (double& value, byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendLong (byte*& buffer, long const & value)       { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractLong (long& value, byte const *& buffer)      { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendShort (byte*& buffer, short const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractShort (short& value, byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendInt64 (byte*& buffer, Int64 const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractInt64 (Int64& value, byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendUInt32 (byte*& buffer, UInt32 const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractUInt32 (UInt32& value, byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendInt (byte*& buffer, int const & value)         { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractInt (int& value, byte const *& buffer)        { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendUInt16 (byte*& buffer, UInt16 const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractUInt16 (UInt16& value, byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendInt32 (byte*& buffer, Int32 const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractInt32 (Int32& value, byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendUShort (byte*& buffer, UShort const & value)   { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractUShort (UShort& value, byte const *& buffer)  { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

void ByteStreamHelper::AppendULong (byte*& buffer, ULong const & value)     { memcpy (buffer, &value, sizeof (value)); buffer += sizeof (value); }
void ByteStreamHelper::ExtractULong (ULong& value, byte const *& buffer)    { memcpy (&value, buffer, sizeof (value)); buffer += sizeof (value); }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extractConvertDwgHatchDef
(
DwgHatchDef&        pDwgHatchDef,   /* <= DWG hatch definition  */
DwgHatchDefLine*    pHatchLines,
int                 nMaxLines,
byte const *&       buffer          /* => buffer        */
)
    {
    int         i, j;
    double      *pDash;

    memset (&pDwgHatchDef, 0, sizeof (pDwgHatchDef));

    ByteStreamHelper::ExtractShort (pDwgHatchDef.nDefLines, buffer);

    if (pDwgHatchDef.nDefLines < 0)
        pDwgHatchDef.nDefLines = 0;

    for (i=0; i<pDwgHatchDef.nDefLines; i++)
        {
        DwgHatchDefLine line;

        ByteStreamHelper::ExtractDouble (line.angle,        buffer);
        ByteStreamHelper::ExtractDouble (line.through.x,    buffer);
        ByteStreamHelper::ExtractDouble (line.through.y,    buffer);
        ByteStreamHelper::ExtractDouble (line.offset.x,     buffer);
        ByteStreamHelper::ExtractDouble (line.offset.y,     buffer);
        ByteStreamHelper::ExtractShort  (line.nDashes,      buffer);

        for (j = 0, pDash = line.dashes; j < line.nDashes; j++, pDash++)
            ByteStreamHelper::ExtractDouble (*pDash, buffer);

        if (i < nMaxLines)
            pHatchLines[i] = line;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    appendUnconvertDwgHatchDef
(
byte                **pBuf,             /* <= Append DWG Hatch Definition to buffer     */
DwgHatchDef         *pDwgHatchDef,      /* => DWG hatch definition to unconvert */
DwgHatchDefLine     *pHatchLines
)
    {
    int                 i, j;
    double              *pDash;
    DwgHatchDefLine     *pLine;

    ByteStreamHelper::AppendShort (*pBuf, pDwgHatchDef->nDefLines);

    for (i=0, pLine = pHatchLines; i<pDwgHatchDef->nDefLines; i++, pLine++)
        {
        ByteStreamHelper::AppendDouble  (*pBuf, pLine->angle);
        ByteStreamHelper::AppendDouble  (*pBuf, pLine->through.x);
        ByteStreamHelper::AppendDouble  (*pBuf, pLine->through.y);
        ByteStreamHelper::AppendDouble  (*pBuf, pLine->offset.x);
        ByteStreamHelper::AppendDouble  (*pBuf, pLine->offset.y);
        ByteStreamHelper::AppendShort   (*pBuf, pLine->nDashes);
        
        for (j=0, pDash = pLine->dashes; j<pLine->nDashes; j++)
            ByteStreamHelper::AppendDouble (*pBuf, *pDash++);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett   5/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    aspat_extractHatchData
(
PatternParams&      params,
DwgHatchDefLine*    pHatchLines,
int                 nMaxLines,
byte const *&       buffer,
bool                is3d
)
    {
    UInt32  modifiers;
    ByteStreamHelper::ExtractUInt32 (modifiers, buffer);
    params.modifiers = static_cast <PatternParamsModifierFlags>(modifiers);

    if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Space1))
        ByteStreamHelper::ExtractDouble (params.space1, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Angle1))
        {
        ByteStreamHelper::ExtractDouble (params.angle1, buffer);
//        mdlCnv_validateDoubleAndClampToRange ( & params.angle1, msGeomConst_2pi, -msGeomConst_2pi, fc_zero, fc_zero);
        LIMIT_RANGE (-msGeomConst_2pi, msGeomConst_2pi, params.angle1);
        }

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Space2))
        ByteStreamHelper::ExtractDouble (params.space2, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Angle2))
        {
        ByteStreamHelper::ExtractDouble (params.angle2, buffer);
//        mdlCnv_validateDoubleAndClampToRange ( &(params.angle2), msGeomConst_2pi, -msGeomConst_2pi, fc_zero, fc_zero);
        LIMIT_RANGE (-msGeomConst_2pi, msGeomConst_2pi, params.angle2);
        }
    else
        {
        params.angle2 = params.angle1 + msGeomConst_piOver2;
        }

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Scale))
        ByteStreamHelper::ExtractDouble (params.scale, buffer);
    else
        params.scale = 1.0;

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Tolerance))
        ByteStreamHelper::ExtractDouble (params.tolerance, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Style))
        ByteStreamHelper::ExtractUInt32 (params.style, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Weight))
        ByteStreamHelper::ExtractUInt32 (params.weight, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Color))
        ByteStreamHelper::ExtractUInt32 (params.color, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::RotMatrix))
        ByteStreamHelper::ExtractRotMatrix (params.rMatrix, buffer, TO_BOOL (is3d));

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Offset))
        ByteStreamHelper::ExtractDPoint3d (params.offset, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Multiline))
        {
        ByteStreamHelper::ExtractInt (params.minLine, buffer);
        ByteStreamHelper::ExtractInt (params.maxLine, buffer);
        }

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::HoleStyle))
        ByteStreamHelper::ExtractShort (params.holeStyle, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::DwgHatchDef))
        extractConvertDwgHatchDef (params.dwgHatchDef, pHatchLines, nMaxLines, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::PixelSize))
        ByteStreamHelper::ExtractDouble (params.dwgHatchDef.pixelSize, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::IslandStyle))
        ByteStreamHelper::ExtractShort (params.dwgHatchDef.islandStyle, buffer);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::AnnotationScale))
        ByteStreamHelper::ExtractDouble (params.annotationscale, buffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett   5/93
+---------------+---------------+---------------+---------------+---------------+------*/
static byte*   aspat_appendHatchData
(
byte*               pBuf,
PatternParams*      pParams,
DwgHatchDefLine*    pHatchLines,
bool                is3d
)
    {
    byte        *pMod = NULL;

    /*-------------------------------------------------------------------
    Skip over modifiers - we will write them out last.
    -------------------------------------------------------------------*/
    pMod = pBuf;
    pBuf += sizeof (long); // ***NEEDS WORK sizeof(long) varies. 

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Space1))
        ByteStreamHelper::AppendDouble (pBuf, pParams->space1);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Angle1))
        {
//        mdlCnv_validateDoubleAndClampToRange (&(pParams->angle1), msGeomConst_2pi, -msGeomConst_2pi, fc_zero, fc_zero);
        LIMIT_RANGE (-msGeomConst_2pi, msGeomConst_2pi, pParams->angle1);
        ByteStreamHelper::AppendDouble (pBuf, pParams->angle1);
        }

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Space2))
        ByteStreamHelper::AppendDouble (pBuf, pParams->space2);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Angle2))
        {
//        mdlCnv_validateDoubleAndClampToRange (&(pParams->angle2), msGeomConst_2pi, -msGeomConst_2pi, fc_zero, fc_zero);
        LIMIT_RANGE (-msGeomConst_2pi, msGeomConst_2pi, pParams->angle2);
        ByteStreamHelper::AppendDouble (pBuf, pParams->angle2);
        }

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Scale))
        ByteStreamHelper::AppendDouble (pBuf, 0.0 == pParams->scale ? 1.0 : pParams->scale);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Tolerance))
        ByteStreamHelper::AppendDouble (pBuf, pParams->tolerance);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Style))
        ByteStreamHelper::AppendUInt32 (pBuf, pParams->style);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Weight))
        ByteStreamHelper::AppendUInt32 (pBuf, pParams->weight);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Color))
        ByteStreamHelper::AppendUInt32 (pBuf, pParams->color);

    /* store rotation matrix as quaternions */
    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::RotMatrix))
        ByteStreamHelper::AppendRotMatrix (pBuf, pParams->rMatrix, TO_BOOL (is3d));

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Offset))
        ByteStreamHelper::AppendDPoint3d (pBuf, pParams->offset);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::Multiline))
        {
        ByteStreamHelper::AppendInt (pBuf, pParams->minLine);
        ByteStreamHelper::AppendInt (pBuf, pParams->maxLine);
        }

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::HoleStyle))
        ByteStreamHelper::AppendShort (pBuf, pParams->holeStyle);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::DwgHatchDef))
        appendUnconvertDwgHatchDef (&pBuf, &pParams->dwgHatchDef, pHatchLines);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::PixelSize))
        ByteStreamHelper::AppendDouble (pBuf, pParams->dwgHatchDef.pixelSize);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::IslandStyle))
        ByteStreamHelper::AppendShort (pBuf, pParams->dwgHatchDef.islandStyle);

    if (PatternParamsModifierFlags::None != (pParams->modifiers & PatternParamsModifierFlags::AnnotationScale))
        ByteStreamHelper::AppendDouble (pBuf, pParams->annotationscale);

    ByteStreamHelper::AppendUInt32 (pMod, static_cast<UInt32>(pParams->modifiers));

    return pBuf;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     02/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    transformPatternSpace
(
double*         pNewSpace,
double          oldSpace,
RotMatrixP      pPatRot,
double          angle,
TransformCP     pTrans
)
    {
    RotMatrix   tmpRot, angRot;
    DVec3d      yDir;

    if (angle != 0.0)
        {
        angRot.InitFromPrincipleAxisRotations(RotMatrix::FromIdentity (), 0.0, 0.0, angle);
        tmpRot.InitProduct(*pPatRot, angRot);
        }
    else
        {
        tmpRot = *pPatRot;
        }

    tmpRot.GetColumn(yDir, 1);
    yDir.Scale(yDir, oldSpace);
    pTrans->multiplyMatrixOnly (&yDir, &yDir);

    *pNewSpace = yDir.Magnitude();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    transformPatternAngle
(
double*         pNewAngle,
double          oldAngle,
RotMatrixP      pPatRot,
TransformCP     pTrans
)
    {
    RotMatrix   tmpRot, angRot;
    DVec3d      xDir;

    if (oldAngle != 0.0)
        {
        angRot.InitFromPrincipleAxisRotations(RotMatrix::FromIdentity (), 0.0, 0.0, oldAngle);
        tmpRot.InitProduct(*pPatRot, angRot);
        }
    else
        {
        tmpRot = *pPatRot;
        }

    tmpRot.GetColumn(xDir, 0);
    pTrans->multiplyMatrixOnly (&xDir, &xDir);

    *pNewAngle = atan2 (xDir.y, xDir.x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            PatternLinkageUtil::Transform (PatternParamsR params, DwgHatchDefLineP pHatchLines, TransformCR transform, bool flatten, bool allowSizeChange)
    {
    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Space1))
        transformPatternSpace (&params.space1, params.space1, &params.rMatrix, params.angle1, &transform);

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Space2))
        transformPatternSpace (&params.space2, params.space2, &params.rMatrix, params.angle2, &transform);

    if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Scale) || (allowSizeChange && (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Cell)) || (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::DwgHatchDef))) )
        {
        DVec3d      xDir;
        double      mag;

        transform.GetMatrixColumn (xDir,  0);
        mag = xDir.Magnitude ();

        if (mag > mgds_fc_epsilon)
            {
            params.scale *= mag;

            if (PatternParamsModifierFlags::None == (params.modifiers & PatternParamsModifierFlags::Scale) && (0.0 != params.scale && 1.0 != params.scale))
                params.modifiers = params.modifiers | PatternParamsModifierFlags::Scale;
            }
        }

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Offset))
        transform.multiplyMatrixOnly ((DVec3dP) &params.offset, (DVec3dP) &params.offset);

    // if we are flattening and have angles, correct the angles and set the rotmatrix to identity.
    if (flatten && (PatternParamsModifierFlags::None == (params.modifiers & PatternParamsModifierFlags::DwgHatchDef)))
        {
        // correct pattern angles to be the "apparent" angles as viewed from this orientation
        if (allowSizeChange)
            params.modifiers = params.modifiers | PatternParamsModifierFlags::Angle1; // Set modifier to adjust angle1 when not explicitly stored (0.0)...

        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Angle1))
            transformPatternAngle (&params.angle1, params.angle1, &params.rMatrix, &transform);

        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Angle2))
            transformPatternAngle (&params.angle2, params.angle2, &params.rMatrix, &transform);

        // reset rotation...accounted for in new angles
        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::RotMatrix))
            (params.rMatrix).InitIdentity ();
        }
    else
        {
        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::RotMatrix))
            {
            (params.rMatrix).InitProduct(transform, *( &params.rMatrix));
            (params.rMatrix).SquareAndNormalizeColumns (*( &params.rMatrix), 0, 1);
            }
        }

    if (pHatchLines && (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::DwgHatchDef)))
        {
        int         iDefLine;
        double      scale = 0.0;
        DVec3d      xDir;

        transform.GetMatrixColumn (xDir, 0);
        scale = xDir.Magnitude();

        if (scale > mgds_fc_epsilon)
            {
            for (iDefLine = 0; iDefLine < params.dwgHatchDef.nDefLines; iDefLine++)
                {
                int     iDash;

                pHatchLines[iDefLine].through.x *= scale;
                pHatchLines[iDefLine].through.y *= scale;

                pHatchLines[iDefLine].offset.x *= scale;
                pHatchLines[iDefLine].offset.y *= scale;

                for (iDash = 0; iDash < pHatchLines[iDefLine].nDashes; iDash++)
                    pHatchLines[iDefLine].dashes[iDash] *= scale;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/05
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   doTransformPattern
(
EditElementHandleR  eeh,
TransformCR         transform,
bool                allowSizeChange
)
    {
    DVec3d      zDir;
    DgnV8ElementBlank   tElm(*eeh.GetElementCP()); // full DgnElement on stack!!
    transform.GetMatrixRow (zDir,  2);

    bool            flatten = (0.0 == zDir.Magnitude());
    int             patIndex = 0;
    UShort*         pLinkage = NULL;
    PatternParams   params;
    DwgHatchDefLine hatchLines[MAX_DWG_EXPANDEDHATCH_LINES]; // huge local!

    while (NULL != (pLinkage = (UShort*) linkage_extractLinkageByIndex (NULL, &tElm, PATTERN_ID, NULL, patIndex)))
        {
        PatternLinkageUtil::Extract (params, hatchLines, MAX_DWG_EXPANDEDHATCH_LINES, (HatchLinkage*) pLinkage, tElm.Is3d());
        PatternLinkageUtil::Transform (params, hatchLines, transform, flatten, allowSizeChange);

        HatchLinkage    hLink;

        if (0 != PatternLinkageUtil::Create (hLink, params, hatchLines, tElm.Is3d()))
            elemUtil_replaceLinkage (&tElm, pLinkage, (UShort *) &hLink, allowSizeChange);

        patIndex++;
        }

    return eeh.ReplaceElement (&tElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PatternLinkageUtil::OnElementTransform (EditElementHandleR eeh, TransformCR transform, bool allowSizeChange)
    {
    if (0 == mdlLinkage_numLinkages (eeh.GetElementP (), PATTERN_ID))
        return SUCCESS;

    return doTransformPattern (eeh, transform, allowSizeChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett   5/93
+---------------+---------------+---------------+---------------+---------------+------*/
void            PatternLinkageUtil::GetHatchOrigin (DPoint3dR origin, DgnElementCR elm)
    {
    // NEEDSWORK: This should use the handler but we don't want to change old patterns... :(
    switch (elm.GetLegacyType())
        {
        case ELLIPSE_ELM:
            {
            if (elm.Is3d())
                origin = elm.ToEllipse_3d().origin;
            else
                origin.init (&elm.ToEllipse_2d().origin);
            break;
            }

        case SHAPE_ELM:
            {
            origin.x = elm.ToLine_String_3d().vertice->x;
            origin.y = elm.ToLine_String_3d().vertice->y;
            origin.z = elm.Is3d() ? elm.ToLine_String_3d().vertice->z : 0.0;
            break;
            }

        case MULTILINE_ELM:
            {
            origin = ((MlinePoint const*) (elm.ToMlineElm().profile + elm.ToMlineElm().nLines))->point;
            break;
            }

        default:
            {
            DRange3d     rangeVec = elm.GetRange();
            origin.x = rangeVec.low.x;
            origin.y = rangeVec.low.y;
            origin.z = elm.Is3d() ? rangeVec.low.z : 0.0;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     3/93
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PatternLinkageUtil::Extract
(
PatternParamsR      params,
DwgHatchDefLineP    pHatchLines,
int                 nMaxLines,
HatchLinkageP       pLink,
bool                is3d
)
    {
    if (!pLink)
        return ERROR;

    return PatternLinkageUtil::Extract (params, pHatchLines, nMaxLines, (byte const*) &pLink->modifiers, is3d);
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PatternLinkageUtil::Extract
(
PatternParamsR      params,
DwgHatchDefLineP    pHatchLines,
int                 nMaxLines,
byte const*         buffer,
bool                is3d
)
    {
    params.Init ();
    
    aspat_extractHatchData (params, pHatchLines, nMaxLines, buffer, is3d);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
int             PatternLinkageUtil::Create
(
HatchLinkageR       linkage,
PatternParamsR      params,
DwgHatchDefLineP    pHatchLines,
bool                is3d
)
    {
    linkage.linkHeader.user       = 1;
    linkage.linkHeader.primaryID  = PATTERN_ID;

    byte*       pEnd = aspat_appendHatchData ((byte *) &linkage.modifiers, &params, pHatchLines, is3d);
    int         linkBytes = ((pEnd - ((byte *) &linkage)) + 7) & ~7;

    LinkageUtil::SetWords (&linkage.linkHeader, linkBytes/2);

    return linkBytes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PatternLinkageUtil::AddToElement
(
DgnElementR          elm,
PatternParamsR      params,
DwgHatchDefLineP    pHatchLines,
int                 index
)
    {
    // If area pattern and we don't have cellId at this point, just return error, name lookup should have happened earlier!
    if ( (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Cell)) && !params.cellId)
        return ERROR;

    // -1 means to append new linkage to end
    if (index >= 0)
        PatternLinkageUtil::DeleteFromElement (elm, index); // returns num deleted
    else if (index < 0)
        index = mdlLinkage_numLinkages (&elm, PATTERN_ID);

    HatchLinkage    hatchLink;

    PatternLinkageUtil::Create (hatchLink, params, pHatchLines, elm.Is3d());

    if (SUCCESS != elemUtil_appendLinkage (&elm, &hatchLink.linkHeader))
        return ERROR;
    
    if (PatternParamsModifierFlags::None == (params.modifiers & PatternParamsModifierFlags::Cell))
        return SUCCESS;

    DependencyLinkage   depLinkage;

    DependencyManagerLinkage::InitLinkage (depLinkage, DEPENDENCYAPPID_PatternCell, DEPENDENCY_DATA_TYPE_ELEM_ID, DEPENDENCY_ON_COPY_DeepCopyRootsAcrossFiles);

    depLinkage.appValue         = (Int16)index;
    depLinkage.nRoots           = 1;
    depLinkage.root.elemid[0]   = params.cellId;

    return DependencyManagerLinkage::AppendLinkageToMSElement (&elm, depLinkage, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     8/93
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PatternLinkageUtil::ExtractFromElement
(
HatchLinkageP*      ppHatchLinkage,
PatternParamsR      params,
DwgHatchDefLineP    pHatchDefLines,
int                 maxHatchDefLines,
DPoint3dP           pOrigin,
DgnElementCR        elm,
DgnModelP           dgnCache,   // if NULL pParams->cellName just won't be set by looking up cellId...
int                 index
)
    {
    HatchLinkageP   hLink = (HatchLinkageP) linkage_extractLinkageByIndex (NULL, &elm, PATTERN_ID, NULL, index);

    if (NULL == hLink)
        return ERROR;

    PatternLinkageUtil::Extract (params, pHatchDefLines, maxHatchDefLines, hLink, elm.Is3d());

    if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Cell))
        {
        DependencyLinkageAccessor  depLinkageP;

        if (SUCCESS == DependencyManagerLinkage::GetLinkageFromMSElement (&depLinkageP, &elm, DEPENDENCYAPPID_PatternCell, (UShort) index))
            {
            params.cellId = depLinkageP->root.elemid[0];

            HighPriorityOperationBlock highPriorityOperationBlock;
            ElementRefP  elemRef;
            if (NULL != (elemRef = dgnCache->FindElementById(ElementId(params.cellId))))
                {
                ElementHandle  eh (elemRef);

                CellUtil::GetCellName (params.cellName, MAX_CELLNAME_LENGTH, *eh.GetElementCP ());
                }
            }

        if (!params.cellId)
            params.modifiers = params.modifiers& ~PatternParamsModifierFlags::Cell;
        }

    if (NULL != pOrigin)
        {
        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Origin))
            *pOrigin = params.origin;
        else
            PatternLinkageUtil::GetHatchOrigin (*pOrigin, elm);

        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Offset))
            bsiDPoint3d_addDPoint3dDPoint3d (pOrigin, pOrigin, (DVec3d *) &params.offset);
        }

    if (NULL != ppHatchLinkage)
        *ppHatchLinkage = hLink;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/09
+---------------+---------------+---------------+---------------+---------------+------*/
int             PatternLinkageUtil::DeleteFromElement (DgnElementR elm, int patIndex)
    {
    int         numPattern, numDeleted;
    
    // Get current linkage count and remove requested "normal" pattern linkage (-1 removes all)...
    if (0 == (numPattern = mdlLinkage_numLinkages (&elm, PATTERN_ID)) ||
        0 == (numDeleted = linkage_deleteLinkageByIndex (&elm, PATTERN_ID, patIndex)))
        return 0;

    // Remove all optional pattern linkages in reverse so string linkage loop index remains valid...
    if (patIndex < 0)
        {
        for (int index = numPattern-1; index >= 0; index--)
            {
            LinkageUtil::DeleteStringLinkage (&elm, STRING_LINKAGE_KEY_DWGPatternName, (UShort) index);
            DependencyManagerLinkage::DeleteLinkageFromMSElement (&elm, DEPENDENCYAPPID_PatternCell, (UShort) index);
            }

        return numDeleted;
        }

    // Remove the optional pattern linkages for the specified index...
    LinkageUtil::DeleteStringLinkage (&elm, STRING_LINKAGE_KEY_DWGPatternName, (UShort) patIndex);
    DependencyManagerLinkage::DeleteLinkageFromMSElement (&elm, DEPENDENCYAPPID_PatternCell, (UShort) patIndex);

    // NOTE: Must decrement dependency appValue indices greater than the deleted index...
    for (int index = patIndex+1; index < numPattern; index++)
        {
        DependencyLinkageAccessor depLink;

        if (SUCCESS == DependencyManagerLinkage::GetLinkageFromMSElement (&depLink, &elm, DEPENDENCYAPPID_PatternCell, (UShort) index))
            {
#if defined (WIP_NONPORT)
            ((DependencyLinkageP) depLink)->appValue--;
#else
            BeAssert (false && "You cannot write through a DependencyLinkageAccessor");
#endif
            }
        }

    return numDeleted;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PatternParams::Init ()
    {
    rMatrix.InitIdentity ();
    offset.Zero ();
    space1 = 0.0;
    angle1 = 0.0;
    space2 = 0.0;
    angle2 = 0.0;
    scale = 1.0;
    tolerance = 0.0;
    annotationscale = 1.0;
    memset (cellName, 0, sizeof (cellName));
    cellId = 0;
    modifiers = PatternParamsModifierFlags::None;
    minLine = -1;
    maxLine = -1;
    color = 0;
    weight = 0;
    style = 0;
    holeStyle = static_cast<Int16>(PatternParamsHoleStyleType::Normal);
    memset (&dwgHatchDef, 0, sizeof (dwgHatchDef));
    origin.Zero ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
PatternParams::PatternParams () {Init ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
PatternParamsPtr PatternParams::Create () {return new PatternParams ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PatternParamsPtr PatternParams::CreateFromExisting (PatternParamsCR params) 
    {
    PatternParamsP newParams = new PatternParams ();
    newParams->Copy (params);
    return  newParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PatternParams::Copy (PatternParamsCR params) 
    {
    rMatrix.copy (&params.rMatrix);
    offset = params.offset;
    space1 = params.space1;
    angle1 = params.angle1;
    space2 = params.space2;
    angle2 = params.angle2;
    scale = params.scale;
    tolerance = params.tolerance;
    annotationscale = params.annotationscale;
    memcpy (cellName, params.cellName, sizeof (cellName));
    cellId = params.cellId;
    modifiers = params.modifiers;
    minLine = params.minLine;
    maxLine = params.maxLine;
    color = params.color;
    weight = params.weight;
    style = params.style;
    holeStyle = params.holeStyle;
    memcpy (&dwgHatchDef, &params.dwgHatchDef, sizeof (dwgHatchDef));
    origin = params.origin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PatternParams::IsEqual (PatternParamsCR params, PatternParamsCompareFlags compareFlags) 
    {
    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_RMatrix)
        {
        if ((params.modifiers & PatternParamsModifierFlags::RotMatrix) != (modifiers & PatternParamsModifierFlags::RotMatrix))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::RotMatrix))
            {
            if (!rMatrix.isEqual (&params.rMatrix))
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Offset)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Offset) != (modifiers & PatternParamsModifierFlags::Offset))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Offset))
            {
            if (!offset.IsEqual(params.offset))
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Default)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Space1) != (modifiers & PatternParamsModifierFlags::Space1))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Space1))
            {
            if (space1 != params.space1)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Space2) != (modifiers & PatternParamsModifierFlags::Space2))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Space2))
            {
            if (space2 != params.space2)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Angle1) != (modifiers & PatternParamsModifierFlags::Angle1))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Angle1))
            {
            if (angle1 != params.angle1)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Angle2) != (modifiers & PatternParamsModifierFlags::Angle2))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Angle2))
            {
            if (angle2 != params.angle2)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Scale) != (modifiers & PatternParamsModifierFlags::Scale))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Scale))
            {
            if (scale != params.scale)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Cell) != (modifiers & PatternParamsModifierFlags::Cell))
            return false;
        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Cell))
            {
            if (0 != wcscmp (cellName, params.cellName))
                return false;

            if (cellId != params.cellId)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Symbology)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Color) != (modifiers & PatternParamsModifierFlags::Color))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Color))
            {
            if (color != params.color)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Weight) != (modifiers & PatternParamsModifierFlags::Weight))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Weight))
            {
            if (weight != params.weight)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Style) != (modifiers & PatternParamsModifierFlags::Style))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Style))
            {
            if (style != params.style)
                return false;
            }
        }
  
    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Mline)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Multiline) != (modifiers & PatternParamsModifierFlags::Multiline))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Multiline))
            {
            if (minLine != params.minLine)
                return false;

            if (maxLine != params.maxLine)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Tolerance)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Tolerance) != (modifiers & PatternParamsModifierFlags::Tolerance))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Tolerance))
            {
            if (tolerance != params.tolerance)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_AnnotationScale)
        {
        if ((params.modifiers & PatternParamsModifierFlags::AnnotationScale) != (modifiers & PatternParamsModifierFlags::AnnotationScale))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::AnnotationScale))
            {
            if (annotationscale != params.annotationscale)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_HoleStyle)
        {
        if ((params.modifiers & PatternParamsModifierFlags::HoleStyle) != (modifiers & PatternParamsModifierFlags::HoleStyle))
            return false;

        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::HoleStyle))
            {
            if (holeStyle != params.holeStyle)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_DwgHatch)
        {
        if ((params.modifiers & PatternParamsModifierFlags::DwgHatchDef) != (modifiers & PatternParamsModifierFlags::DwgHatchDef))
            return false;

        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::DwgHatchDef))
            {
            if (0 != memcmp (&dwgHatchDef, &params.dwgHatchDef, sizeof (dwgHatchDef)))
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Origin)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Origin) != (modifiers & PatternParamsModifierFlags::Origin))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Origin))
            {
            if (!origin.IsEqual(params.origin))
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPropertiesGetter::ElementPropertiesGetter (ElementHandleCR eh)
    {
    DisplayHandlerP dHandler = eh.GetDisplayHandler ();

    if (NULL == dHandler)
        m_displayParams.Init ();
    else
        dHandler->GetElemDisplayParams (eh, m_displayParams);

    // NOTE: Callers are expecting a normal element color id. In the off chance that a handler chooses
    //       to set it's default display params by RGB (not an extended color id), attempt to resolve
    //       to an existing extended color id. If current element color id isn't invalid, assume it's ok.
    if (INVALID_COLOR != m_displayParams.GetLineColor ())
        return;

    // Get existing element color for true color if it exists...returns INVALID_COLOR if not found...
    m_displayParams.SetLineColor (eh.GetDgnProject()->Colors().FindElementColor (IntColorDef (m_displayParams.GetLineColorTBGR ())));

    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPropertiesGetterPtr ElementPropertiesGetter::Create (ElementHandleCR eh)
    {
    return new ElementPropertiesGetter (eh);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
Int32 ElementPropertiesGetter::GetLineStyle (LineStyleParamsP lsParams) const
    {
    if (lsParams)
        {
        LineStyleParamsCP  params = m_displayParams.GetLineStyleParams ();

        if (params)
            *lsParams = *params;
        else
            lsParams->Init ();
        }
    
    return m_displayParams.GetLineStyle ();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ElementPropertiesGetter::GetColor () const {return m_displayParams.GetLineColor ();}
UInt32 ElementPropertiesGetter::GetWeight () const {return m_displayParams.GetWeight ();}
LevelId ElementPropertiesGetter::GetLevel () const {return m_displayParams.GetSubLevelId().GetLevel();}
DgnElementClass ElementPropertiesGetter::GetElementClass () const {return m_displayParams.GetElementClass ();}
Int32 ElementPropertiesGetter::GetDisplayPriority () const {return m_displayParams.GetElementDisplayPriority ();}
double ElementPropertiesGetter::GetTransparency () const {return m_displayParams.GetTransparency ();}
DVec3dCP ElementPropertiesGetter::GetThickness (bool& isCapped) const {return m_displayParams.GetThickness (isCapped);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPropertiesSetter::ElementPropertiesSetter ()
    {
    m_propMask          = ELEMENT_PROPERTY_None;
    m_changeAll         = false;

    m_setElemColor      = false;
    m_setFillColor      = false;

    m_color             = 0;
    m_fillColor         = 0;
    m_style             = 0;
    m_weight            = 0;
    m_elmClass          = DgnElementClass::Primary;
    m_priority          = 0;
    m_transparency      = 0.0;

    m_lsParams          = NULL;

    m_thickness         = 0.0;
    m_direction         = NULL;
    m_isCapped          = false;
    m_alwaysUseDir      = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetColor (UInt32 color)
    {
    m_propMask      = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Color);
    m_color         = color;
    m_setElemColor  = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetFillColor (UInt32 fillColor)
    {
    m_propMask      = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Color);
    m_fillColor     = fillColor;
    m_setFillColor  = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetLinestyle (Int32 style, LineStyleParamsCP lsParams)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Linestyle);
    m_style    = style;
    m_lsParams = lsParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetWeight (UInt32 weight)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Weight);
    m_weight = weight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetLevel (LevelId level)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Level);
    m_level = level;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetElementClass (DgnElementClass elmClass)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_ElementClass);
    m_elmClass = elmClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetDisplayPriority (Int32 priority)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_DisplayPriority);
    m_priority = priority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetTransparency (double transparency)
    {
    m_propMask = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Transparency);
    m_transparency = transparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::SetThickness (double thickness, DVec3dCP direction, bool isCapped, bool alwaysUseDirection)
    {
    m_propMask      = (ElementProperties) (m_propMask | ELEMENT_PROPERTY_Thickness);
    m_thickness     = thickness;
    m_direction     = direction;
    m_isCapped      = isCapped;
    m_alwaysUseDir  = alwaysUseDirection;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
void ElementPropertiesSetter::SetFont (DgnFontCR font)
    {
    m_propMask  = (ElementProperties)(m_propMask | ELEMENT_PROPERTY_Font);
    m_font      = &font;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::SetChangeEntireElement (bool changeAll)
    {
    m_changeAll = changeAll;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementPropertiesSetter::IsValidBaseID (EachPropertyBaseArg& arg)
    {
    if (!m_changeAll && 0 == (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBaseID))
        return false;

    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return false;

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::_EachColorCallback (EachColorArg& arg)
    {
    if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
        return;

    if (m_setElemColor && (m_changeAll || 0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBaseID)))
        {
        arg.SetStoredValue (m_color);

        if (!m_setFillColor)
            arg.SetPropertyCallerFlags ((PropsCallerFlags) (PROPSCALLER_FLAGS_PreserveOpaqueFill | PROPSCALLER_FLAGS_PreserveMatchingDecorationColor));
        else
            arg.SetPropertyCallerFlags (PROPSCALLER_FLAGS_PreserveMatchingDecorationColor);
        }

    if (m_setFillColor && (m_changeAll || 0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_IsBackgroundID)))
        arg.SetStoredValue (m_fillColor);
    if (m_changeAll) // Seems reasonable/desirable to add shared cell instance overrides when change all is set...
        arg.SetPropertyCallerFlags ((PropsCallerFlags) (arg.GetPropertyCallerFlags () | PROPSCALLER_FLAGS_SharedChildOvrSet));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::_EachLineStyleCallback (EachLineStyleArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_style);

    if (m_lsParams)
        arg.SetParams (m_lsParams);
    if (m_changeAll) // Seems reasonable/desirable to add shared cell instance overrides when change all is set...
        arg.SetPropertyCallerFlags ((PropsCallerFlags) (arg.GetPropertyCallerFlags () | PROPSCALLER_FLAGS_SharedChildOvrSet));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementPropertiesSetter::_EachWeightCallback (EachWeightArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_weight);
    if (m_changeAll) // Seems reasonable/desirable to add shared cell instance overrides when change all is set...
        arg.SetPropertyCallerFlags ((PropsCallerFlags) (arg.GetPropertyCallerFlags () | PROPSCALLER_FLAGS_SharedChildOvrSet));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::_EachLevelCallback (EachLevelArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_level);
    if (m_changeAll) // Seems reasonable/desirable to add shared cell instance overrides when change all is set...
        arg.SetPropertyCallerFlags ((PropsCallerFlags) (arg.GetPropertyCallerFlags () | PROPSCALLER_FLAGS_SharedChildOvrSet));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::_EachElementClassCallback (EachElementClassArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_elmClass);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::_EachDisplayPriorityCallback (EachDisplayPriorityArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_priority);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::_EachTransparencyCallback (EachTransparencyArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_transparency);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementPropertiesSetter::_EachThicknessCallback (EachThicknessArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;

    arg.SetStoredValue (m_thickness);
    arg.SetDirection (m_direction);
    arg.SetCapped (m_isCapped);
    arg.SetAlwaysUseDirection (m_alwaysUseDir);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     09/2011
//---------------------------------------------------------------------------------------
void ElementPropertiesSetter::_EachFontCallback (EachFontArg& arg)
    {
    if (!IsValidBaseID (arg))
        return;
    
    UInt32 fontNumber;
    if (SUCCESS != arg.GetPropertyContext().GetDestinationDgnModel()->GetDgnProject().Fonts().AcquireFontNumber (fontNumber, *m_font))
        { BeAssert (false); return; }
    
    arg.SetStoredValue (fontNumber);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ElementPropertiesSetter::Apply (EditElementHandleR eeh)
    {
    if (ELEMENT_PROPERTY_None == m_propMask)
        return false; // Nothing to do, no properties set...
    return PropertyContext::EditElementProperties (eeh, this);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPropertiesSetterPtr ElementPropertiesSetter::Create ()
    {
    return new ElementPropertiesSetter ();
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             09/89
+---------------+---------------+---------------+---------------+---------------+------*/
void DataConvert::DPointToUPoint (Upoint3d& uPnt, DPoint3dCR dPnt)
    {
    uPnt.x = DataConvert::RoundDoubleToULong (dPnt.x);
    uPnt.y = DataConvert::RoundDoubleToULong (dPnt.y);
    uPnt.z = DataConvert::RoundDoubleToULong (dPnt.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             09/89
+---------------+---------------+---------------+---------------+---------------+------*/
void DataConvert::UPointToDPoint (DPoint3dR dPnt, Upoint3d const& uPnt)
    {
    dPnt.x = unslong_to_double (uPnt.x);
    dPnt.y = unslong_to_double (uPnt.y);
    dPnt.z = unslong_to_double (uPnt.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             06/86
+---------------+---------------+---------------+---------------+---------------+------*/
void     DataConvert::IPointToDPoint (DPoint3dR rpnt, Point3dCR ipnt)
    {
    rpnt.x = (double) ipnt.x;
    rpnt.y = (double) ipnt.y;
    rpnt.z = (double) ipnt.z;
    }

/*---------------------------------------------------------------------------------**//**
* number to the nearest unsigned long integer.
* author RBB
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned long DataConvert::RoundDoubleToULong (double double_num)
    {
    if (double_num < 0.0)
        double_num = 0.0;

    double_num += 0.5;
    if (double_num > RMAXUI4 - 1.0)
        double_num = RMAXUI4 - 1.0;

    return (double_to_unslong (double_num));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/08
+---------------+---------------+---------------+---------------+---------------+------*/
long DataConvert::RoundDoubleToLong (double double_num)
    {
    if (double_num > 0.0)
        {
        if ((double_num += 0.5) > RMAXI4)
            return LMAXI4;
        }
    else
        {
        if ((double_num -= 0.5) < RMINI4)
            return LMINI4;
        }

    return (long)double_num;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DML             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     DataConvert::Points3dTo2d (DPoint2dP outP, DPoint3dCP inP, int numPts)
    {
    for (;numPts > 0; numPts--, inP++, outP++)
        {
        outP->x = inP->x;
        outP->y = inP->y;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             09/86
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     DataConvert::Points2dTo3d (DPoint3dP outP, DPoint2dCP inP, int numPts, double zElev)
    {
    for (;numPts > 0; numPts--, inP++, outP++)
        {
        outP->x = inP->x;
        outP->y = inP->y;
        outP->z = zElev;
        }
    }


#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     DataConvert::RotationToQuaternion (double* quaternion, double rotationAngle)
    {
    double      cosine, sine, rtemp;

    sine   = sin (-rotationAngle);
    cosine = cos (-rotationAngle);

    *quaternion = sqrt ((1.0 + cosine) / 2.0);

    *(quaternion+1) = 0.0;
    *(quaternion+2) = 0.0;

    rtemp = sqrt ((1.0 - cosine) / 2.0);
    if (sine < 0.0)
        rtemp = -1.0 * rtemp;

    *(quaternion+3) = rtemp;
    }

typedef union slbUnion
    {
    unsigned long ul;
    long          l;
    short         s[2];
    byte          b[4];
    } SLBUnion;

/*----------------------------------------------------------------------+
|                                                                       |
| name         DataConvert::swapWord                                    |
|                                                                       |
|                   Convert from PDP format longs to native longs.      |
|                   PDP:        3 4 1 2                                 |
|                   INTEL ORDER 1 2 3 4                                 |
|                   BIG_ENDIAN: 4 3 2 1                                 |
|                                                                       |
| author        BJB                                     11/89           |
|                                                                       |
+----------------------------------------------------------------------*/
void    DataConvert::SwapWord
(
UInt32         *input
)
    {
    SLBUnion    u;
    short       temp;

    u.l     = *input;
    temp    = u.s[0];
    u.s[0]  = u.s[1];
    u.s[1]  = temp;
    *input  = u.l;
    }

#if !defined (__APPLE__)
void     DataConvert::SwapWord (ULong32 *input) {return DataConvert::SwapWord((UInt32*)input);}
#endif
void     DataConvert::SwapWord ( Long32 *input) {return DataConvert::SwapWord((UInt32*)input);}
void     DataConvert::SwapWord (  Int32 *input) {return DataConvert::SwapWord((UInt32*)input);}

/*----------------------------------------------------------------------+
;
;  name         _mdlCnv_toScanFormat - changes input signed long into scan format
;               (i.e. unsigned long with words swapped)
;
+----------------------------------------------------------------------*/
unsigned long DataConvert::ToScanFormat(long sinput)
    {
    SLBUnion retVal;
    retVal.l = sinput ^ 0x80000000; // convert to unsigned
    
    short lowHalf = retVal.s[0];
    retVal.s[0]   = retVal.s[1];
    retVal.s[1]   = lowHalf;

    return retVal.ul;
    }

/*----------------------------------------------------------------------+
;
;  name         _mdlCnv_fromScanFormat - reverses action of _mdlCnv_toScanFmt
;
;  synopsis     output = _mdlCnv_fromScanFormat (input)
;               unsigned long input;
;               long output;
;
+----------------------------------------------------------------------*/
long DataConvert::FromScanFormat(unsigned long usInput)
    {
    SLBUnion retVal;
    retVal.ul = usInput;

    short lowHalf = retVal.s[0];
    retVal.s[0]   = retVal.s[1];
    retVal.s[1]   = lowHalf;
    
    retVal.l = retVal.l ^ 0x80000000; // convert to signed

    return retVal.l;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlCnv_doubleFromFileFormatArray                        |
|                                                                       |
| author        kab                                     6/86            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void DataConvert::DoubleFromFileFormatArray
(
double  *pntr,
size_t   numpoints
)
    {
    size_t i;
    for (i=0; i<numpoints; i++, pntr++)
        RscDataConvert::DoubleFromFileFormat (pntr);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlCnv_doubleToFileFormatArray                          |
|                                                                       |
| author        RBB                                     2/88            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     DataConvert::DoubleToFileFormatArray
(
double          *pntr,
size_t           numpoints
)
    {
    size_t  i;

    for (i=0; i<numpoints; i++, pntr++)
        RscDataConvert::DoubleToFileFormat (pntr);
    }

#endif

/*----------------------------------------------------------------------+
|                                                                       |
| name          DataConvert::ReverseUInt64                                    |
|                                                                       |
| author        RayBentley                             04/02k           |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     DataConvert::ReverseUInt64
(
UInt64&        output,
UInt64         input
)
    {
    UInt64      tmp = input;
    byte        *pTmpBytes = (byte *) &tmp, *pOutputBytes = (byte *) &output;

    pOutputBytes[0] = pTmpBytes[7];
    pOutputBytes[1] = pTmpBytes[6];
    pOutputBytes[2] = pTmpBytes[5];
    pOutputBytes[3] = pTmpBytes[4];
    pOutputBytes[4] = pTmpBytes[3];
    pOutputBytes[5] = pTmpBytes[2];
    pOutputBytes[6] = pTmpBytes[1];
    pOutputBytes[7] = pTmpBytes[0];
    }

#if defined (NEEDS_WORK_DGNITEM)

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlCnv_swapWordArray - swap an array of 4 byte integers |
|                                                                       |
| author        kab                                     7/86            |
|                                                                       |
+----------------------------------------------------------------------*/
Public void     DataConvert::SwapWordArray
(
void           *vpntr,
size_t          numpoints
)
    {
    size_t  i;
    UInt16  temp;
    UInt16* pntr = static_cast<UInt16*>(vpntr);

    for (i=0; i<numpoints; i++, pntr+=2)
        {
        temp = *pntr;
        *pntr = *(pntr+1);
        *(pntr+1) = temp;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             07/86
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     in_span (double theta, double start, double sweep)
    {
    theta -= start;

    if (fabs (theta) > 62.8) 
        return 0;

    while (theta < 0.0) 
        theta += msGeomConst_2pi;
    while (theta > msGeomConst_2pi) 
        theta -= msGeomConst_2pi;

    if (sweep > 0.0)
        {
        if (sweep >= theta)
            return true;

        return false;
        }

    if (theta >= (msGeomConst_2pi + sweep))
        return true;

    return false;
    }


#define XATTRIBUTEID_ElementBasis       22843       // Assigned by Mark Andersen   9/20.

enum  ElementBasis_XAttributeMinorId  
    {
    ElementBasis_XAttributeMinorId_Transform        = 0,
    ElementBasis_XAttributeMinorId_Range            = 1,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      RayBentley    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  BasisXAttributesUtil::GetRange (DRange3dP range, ElementHandleCR eh)
    {
    ElementHandle::XAttributeIter   iter (eh, XAttributeHandlerId (XATTRIBUTEID_ElementBasis, ElementBasis_XAttributeMinorId_Range));

    if (!iter.IsValid() || sizeof (DRange3d) != iter.GetSize())
        return ERROR;
    
    if (NULL != range)
        memcpy (range, iter.PeekData(), sizeof (*range));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      RayBentley    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  BasisXAttributesUtil::GetTransform (TransformP transform, ElementHandleCR eh)
    {
    ElementHandle::XAttributeIter   iter (eh, XAttributeHandlerId (XATTRIBUTEID_ElementBasis, ElementBasis_XAttributeMinorId_Transform));

    if (!iter.IsValid() || sizeof (Transform) != iter.GetSize())
        return ERROR;
    
    if (NULL != transform)
        memcpy (transform, iter.PeekData(), sizeof (*transform));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      RayBentley    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  BasisXAttributesUtil::SetRange (DRange3dCR range, EditElementHandleR eeh)
    {
    return SUCCESS == eeh.ScheduleWriteXAttribute (XAttributeHandlerId (XATTRIBUTEID_ElementBasis, ElementBasis_XAttributeMinorId_Range), 0, sizeof (range), &range) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                      RayBentley    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  BasisXAttributesUtil::SetTransform (TransformCR transform, EditElementHandleR eeh)
    {
    return SUCCESS == eeh.ScheduleWriteXAttribute (XAttributeHandlerId (XATTRIBUTEID_ElementBasis, ElementBasis_XAttributeMinorId_Transform), 0, sizeof (transform), &transform) ? SUCCESS : ERROR;
    }
#endif
