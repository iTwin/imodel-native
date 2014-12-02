/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LineStyleLinkage.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/LsLocal.h>
#include <DgnPlatform/DgnHandlers/MultilineStyle.h>

struct  delParamsArg
    {
    UInt32  lineMask;
    UInt32  mlineFlags;
    bool    ignore_mlineFlags;
    bool    is3d;
    };

/*=================================================================================**//**
* @bsiclass                                                     John.Gooding    08/09
+===============+===============+===============+===============+===============+======*/
struct LocalPropertyCollector :      public IQueryProperties
    {
private:
    bool        m_anySet;
    bool        m_paramsSet;
    Int32       m_styleNumber;
    LineStyleParams m_params;

public: 
    bool        GetAnySet () { return m_anySet; }
    bool        GetParamsSet () { return m_paramsSet; }
    Int32       GetStyleNumber () { return m_styleNumber; }
    LineStyleParamsCP GetLineStyleParams () { return &m_params; }

    virtual ElementProperties _GetQueryPropertiesMask () override
        {
        return ELEMENT_PROPERTY_Linestyle;
        }

    virtual void _EachLineStyleCallback (EachLineStyleArg& arg) override
        {
        m_anySet = true;
        m_styleNumber = arg.GetStoredValue ();
        LineStyleParams const*params = arg.GetParams ();
        if (NULL != params)
            {
            m_paramsSet = true;
            m_params = *params;
            }
        }
        
    LocalPropertyCollector () : m_anySet (false), m_paramsSet (false), m_styleNumber (0) {}
    }; // LocalPropertyCollector

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/93
+---------------+---------------+---------------+---------------+---------------+------*/
int LineStyleLinkageUtil::AppendModifiers
(
byte*               pBuf,          /* <= Buffer to append to              */
LineStyleParamsCP   pParams,       /* => Modifiers to append              */
bool                is3d
)
    {
    byte        *pModData;

    if (NULL == pParams || pParams->modifiers == 0)
        return 0;

    pModData = pBuf;

    ByteStreamHelper::AppendUInt32 (pModData, pParams->modifiers);

    if (pParams->modifiers & STYLEMOD_SCALE)
        ByteStreamHelper::AppendDouble (pModData, pParams->scale);

    if (pParams->modifiers & STYLEMOD_DSCALE)
        ByteStreamHelper::AppendDouble (pModData, pParams->dashScale);

    if (pParams->modifiers & STYLEMOD_GSCALE)
        ByteStreamHelper::AppendDouble (pModData, pParams->gapScale);

    if (pParams->modifiers & STYLEMOD_SWIDTH)
        ByteStreamHelper::AppendDouble (pModData, pParams->startWidth);

    if (pParams->modifiers & STYLEMOD_EWIDTH)
        ByteStreamHelper::AppendDouble (pModData, pParams->endWidth);

    if (pParams->modifiers & STYLEMOD_DISTPHASE)
        ByteStreamHelper::AppendDouble (pModData, pParams->distPhase);

    if (pParams->modifiers & STYLEMOD_FRACTPHASE)
        ByteStreamHelper::AppendDouble (pModData, pParams->fractPhase);

    if (pParams->modifiers & STYLEMOD_NORMAL)
        ByteStreamHelper::AppendDPoint3d (pModData, pParams->normal);

    if (pParams->modifiers & STYLEMOD_RMATRIX)
        ByteStreamHelper::AppendRotMatrix (pModData, pParams->rMatrix, TO_BOOL (is3d));

    if (pParams->modifiers & STYLEMOD_LINEMASK)
        ByteStreamHelper::AppendUInt32 (pModData, pParams->lineMask);

    if (pParams->modifiers & STYLEMOD_MLINEFLAGS)
        ByteStreamHelper::AppendUInt32 (pModData, pParams->mlineFlags);

    return static_cast<int>(pModData - pBuf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/93
+---------------+---------------+---------------+---------------+---------------+------*/
int LineStyleLinkageUtil::ExtractModifiers (LineStyleParamsP pParams, byte* pBufIn, bool is3d)
    {
    byte*           pStart  = pBufIn;
    byte const *    pBuf    = (byte const *)pBufIn;

    ByteStreamHelper::ExtractUInt32 (pParams->modifiers, pBuf);

    if (pParams->modifiers & STYLEMOD_SCALE)
        ByteStreamHelper::ExtractDouble (pParams->scale, pBuf);

    if (pParams->modifiers & STYLEMOD_DSCALE)
        ByteStreamHelper::ExtractDouble (pParams->dashScale, pBuf);

    if (pParams->modifiers & STYLEMOD_GSCALE)
        ByteStreamHelper::ExtractDouble (pParams->gapScale, pBuf);

    if (pParams->modifiers & STYLEMOD_SWIDTH)
        ByteStreamHelper::ExtractDouble (pParams->startWidth, pBuf);

    if (pParams->modifiers & STYLEMOD_EWIDTH)
        ByteStreamHelper::ExtractDouble (pParams->endWidth, pBuf);

    if (pParams->modifiers & STYLEMOD_DISTPHASE)
        ByteStreamHelper::ExtractDouble (pParams->distPhase, pBuf);

    if (pParams->modifiers & STYLEMOD_FRACTPHASE)
        ByteStreamHelper::ExtractDouble (pParams->fractPhase, pBuf);

    if (pParams->modifiers & STYLEMOD_NORMAL)
        ByteStreamHelper::ExtractDPoint3d (pParams->normal, pBuf);

    if (pParams->modifiers & STYLEMOD_RMATRIX)
        {
        ByteStreamHelper::ExtractRotMatrix (pParams->rMatrix, pBuf, TO_BOOL (is3d));

        pParams->rMatrix.GetRow(*((DVec3d*)&pParams->normal),  2);
        }

    if (pParams->modifiers & STYLEMOD_LINEMASK)
        ByteStreamHelper::ExtractUInt32 (pParams->lineMask, pBuf);

    if (pParams->modifiers & STYLEMOD_MLINEFLAGS)
        ByteStreamHelper::ExtractUInt32 (pParams->mlineFlags, pBuf);

    return static_cast<int>(pBuf - pStart);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/93
+---------------+---------------+---------------+---------------+---------------+------*/
int LineStyleLinkageUtil::CreateRawLinkage (void *pStyleLink, LineStyleParamsCP pParams, bool is3d)
    {
    StyleLink   *styleLink = (StyleLink*)pStyleLink;
    int         linkBytes=0, modBytes;

    memset (styleLink, 0, sizeof(*styleLink));

    styleLink->linkHeader.user          = 1;
    styleLink->linkHeader.primaryID     = STYLELINK_ID;

    modBytes  = AppendModifiers ((byte*)&styleLink->modifiers, pParams, is3d);
    if (modBytes > 0)
        {
        linkBytes = (( (modBytes + 8 /* Header size */) + 7) & ~7);
        LinkageUtil::SetWords (&styleLink->linkHeader, linkBytes/2);
        }

    return linkBytes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/92
+---------------+---------------+---------------+---------------+---------------+------*/
int LineStyleLinkageUtil::ExtractRawLinkage (LineStyleParamsP pParams, void const* pLink, bool is3d)
    {
    StyleLink* pStyleLink = (StyleLink*) pLink;

    if (pParams)
        {
        pParams->Init();

        if (LinkageUtil::GetWords (&pStyleLink->linkHeader) == 4)
            return SUCCESS;

        ExtractModifiers (pParams, (byte*) &pStyleLink->modifiers, is3d);
        }

    return SUCCESS;
    }

#if defined (NEEDS_WORK_DGNITEM)

/*----------------------------------------------------------------------------------*//**
* Merge parameter sets.  Currently the first set of parameters wins.
* @bsimethod                                                    ChuckKirschman  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public void      LineStyleUtil::MergeParams
(
LineStyleParamsP    outParams,
LineStyleParamsP    masterParams,
LineStyleParamsP    paramsToAdd
)
    {
    LineStyleParams  params = *masterParams;

    if (paramsToAdd->modifiers & STYLEMOD_SCALE && !(params.modifiers & STYLEMOD_SCALE))
        {
        params.scale = paramsToAdd->scale;
        params.modifiers |= STYLEMOD_SCALE;
        }

    if (paramsToAdd->modifiers & STYLEMOD_DSCALE && !(params.modifiers & STYLEMOD_DSCALE))
        {
        params.dashScale = paramsToAdd->dashScale;
        params.modifiers |= STYLEMOD_DSCALE;
        }

    if (paramsToAdd->modifiers & STYLEMOD_GSCALE && !(params.modifiers & STYLEMOD_GSCALE))
        {
        params.gapScale = paramsToAdd->gapScale;
        params.modifiers |= STYLEMOD_GSCALE;
        }

    if (paramsToAdd->modifiers & STYLEMOD_SWIDTH && !(params.modifiers & STYLEMOD_SWIDTH))
        {
        params.startWidth = paramsToAdd->startWidth;
        params.modifiers |= STYLEMOD_SWIDTH;
        }

    if (paramsToAdd->modifiers & STYLEMOD_EWIDTH && !(params.modifiers & STYLEMOD_EWIDTH))
        {
        params.endWidth = paramsToAdd->endWidth;
        params.modifiers |= STYLEMOD_EWIDTH;
        }

    if (paramsToAdd->modifiers & STYLEMOD_DISTPHASE && !(params.modifiers & STYLEMOD_DISTPHASE))
        {
        params.distPhase = paramsToAdd->distPhase;
        params.modifiers |= STYLEMOD_DISTPHASE;
        }

    if (paramsToAdd->modifiers & STYLEMOD_FRACTPHASE && !(params.modifiers & STYLEMOD_FRACTPHASE))
        {
        params.scale = paramsToAdd->fractPhase;
        params.modifiers |= STYLEMOD_FRACTPHASE;
        }

    if (paramsToAdd->modifiers & STYLEMOD_NORMAL && !(params.modifiers & STYLEMOD_NORMAL))
        {
        params.normal = paramsToAdd->normal;
        params.modifiers |= STYLEMOD_NORMAL;
        }

    if (paramsToAdd->modifiers & STYLEMOD_RMATRIX && !(params.modifiers & STYLEMOD_RMATRIX))
        {
        params.rMatrix = paramsToAdd->rMatrix;
        params.modifiers |= STYLEMOD_RMATRIX;
        }

    if (paramsToAdd->modifiers & STYLEMOD_LINEMASK && !(params.modifiers & STYLEMOD_LINEMASK))
        {
        params.lineMask = paramsToAdd->lineMask;
        params.modifiers |= STYLEMOD_LINEMASK;
        }

    if (paramsToAdd->modifiers & STYLEMOD_MLINEFLAGS && !(params.modifiers & STYLEMOD_MLINEFLAGS))
        {
        params.mlineFlags = paramsToAdd->mlineFlags;
        params.modifiers |= STYLEMOD_MLINEFLAGS;
        }

    /* Flags only */
    if (paramsToAdd->modifiers & STYLEMOD_CENTERPHASE && !(params.modifiers & STYLEMOD_CENTERPHASE))
        params.modifiers |= STYLEMOD_CENTERPHASE;

    if (paramsToAdd->modifiers & STYLEMOD_TRUE_WIDTH && !(params.modifiers & STYLEMOD_TRUE_WIDTH))
        params.modifiers |= STYLEMOD_TRUE_WIDTH;

    if (paramsToAdd->modifiers & STYLEMOD_SEGMODE && !(params.modifiers & STYLEMOD_SEGMODE))
        params.modifiers |= STYLEMOD_SEGMODE;

    if (paramsToAdd->modifiers & STYLEMOD_NOSEGMODE && !(params.modifiers & STYLEMOD_NOSEGMODE))
        params.modifiers |= STYLEMOD_NOSEGMODE;

    *outParams = params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int      deleteLStyleLinkages
(
LinkageHeader*          outLinkP,       /* <=  replacement linkage */
void*                   vArg,           /*  => user args           */
LinkageHeader*          inLinkP,        /* <=> linkage to process  */
DgnElementP              elemP           /* <=> owner element       */
)
    {
    struct  delParamsArg* dpArg = (struct delParamsArg*) vArg;
    LineStyleParams  params;

    if (!inLinkP->user)
        return PROCESS_ATTRIB_STATUS_NOCHANGE;

    if (STYLELINK_ID == inLinkP->primaryID)
        {
        LineStyleLinkageUtil::ExtractRawLinkage (&params, inLinkP, dpArg->is3d != 0);

        /* Strip the element custom linkage if requested */
        if (0 == params.lineMask && 0 == dpArg->lineMask)
            {
            return (PROCESS_ATTRIB_STATUS_DELETE);
            }
        else if ((params.lineMask & dpArg->lineMask) &&
            (dpArg->ignore_mlineFlags || (params.mlineFlags & dpArg->mlineFlags)) )
            {
            UInt32  newLineMask = params.lineMask & ~(dpArg->lineMask);

            // Check for other bits set - either modify or delete
            if (0 == newLineMask)
                {
                return (PROCESS_ATTRIB_STATUS_DELETE);
                }
            else
                {
                params.lineMask = newLineMask;
                LineStyleLinkageUtil::CreateRawLinkage (inLinkP, &params, dpArg->is3d != 0);
                return (PROCESS_ATTRIB_STATUS_REPLACE);
                }
            }
        }
    return (PROCESS_ATTRIB_STATUS_NOCHANGE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  01/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void     lineStyle_clearMlineStyle
(
MlineSymbology* orgCap,
MlineSymbology* endCap,
MlineSymbology* midCap,
Int32           nLines,
MlineProfile*   profiles,
UInt32          lineMask,
UInt32          mlineFlags
)
    {
    /* Also clear the customStyle bit.  Today it just means that it has modifiers. */
    if (mlineFlags & MLSFLAG_LINE)
        {
        for (int iProfile=0; iProfile < nLines; iProfile++)
            {
            if (mlineFlags & (1<<iProfile))
                profiles[iProfile].symb.customStyle = false;
            }
        }
    if (mlineFlags & MLSFLAG_CAP)
        {
        if (MULTILINE_ORG_CAP_INDEX & lineMask)
            orgCap->customStyle = false;
        if (MULTILINE_END_CAP_INDEX & lineMask)
            endCap->customStyle = false;
        if (MULTILINE_MID_CAP_INDEX & lineMask)
            midCap->customStyle = false;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  01/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void lineStyle_clearStyle (DgnElementP pElem, UInt32 lineMask, UInt32 mlineFlags)
    {
    struct  delParamsArg    dpArg;

    memset (&dpArg, 0, sizeof(dpArg));
    dpArg.lineMask      = lineMask;
    dpArg.mlineFlags    = mlineFlags;
    dpArg.is3d = pElem->Is3d();

    elemUtil_deleteLinkage (pElem, STYLELINK_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/92
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleLinkageUtil::ClearElementStyle (DgnElementP elm, bool clearElementStyle, UInt32 lineMask, UInt32 mlineFlags)
    {
    if (clearElementStyle)
        lineStyle_clearStyle (elm, 0, 0);

    if (0 != lineMask)
        lineStyle_clearStyle (elm, lineMask, mlineFlags);
    }

#if defined (NEEDS_WORK_DGNITEM)
//  No longer needed by LsDefinition::SetStyleParams but still needed by 
//  ElemDisplayParams::ApplyByLevelOverrides
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 07/08
+---------------+---------------+---------------+---------------+---------------+------*/
Public void      LineStyleUtil::AdjustParamUorScale
(
LineStyleParamsP    paramsP,
Int32               styleNo,
DgnModelP        modelRef,
LevelId             level
)
    {
    DgnProjectP      dgnFile = &modelRef->GetDgnProject();

#if defined (LEVEL_FACET_REORG)
    // Someplace we need to modify width units to match the line style.  We try to use
    // UORs for widths because everything is file dependent, but for old RSC stuff there's
    // a lot of stuff in master units.
    if (level.IsValid() && STYLE_BYLEVEL == styleNo)
        {
        DgnLevels::Level const& level = DGN_TABLE_LEVEL_FOR_MODEL(modelRef).QueryLevelById (level);
        if (level.IsValid ())
            styleNo = levelH.GetAppearance().GetStyle();
        else
            styleNo = 0;
        }
#endif

    LsDefinitionP   nameRec = LsMap::GetMapPtr (*dgnFile, true)->Find (styleNo);

    if ((NULL != nameRec) && nameRec->IsUnitsUOR ())
        {
        double      uorScale = modelRef->GetMillimetersPerMaster();

        paramsP->distPhase *= uorScale;

        if (! (paramsP->modifiers & STYLEMOD_TRUE_WIDTH))
            {
            if (paramsP->modifiers & STYLEMOD_SWIDTH)
                paramsP->startWidth *= uorScale;
            if (paramsP->modifiers & STYLEMOD_EWIDTH)
                paramsP->endWidth   *= uorScale;
            }
        }
    }
#endif

/*----------------------------------------------------------------------------------*//**
* Set the style parameters without changing the style.  Necessary for multilines.
* @bsimethod                                                    ChuckKirschman  05/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      LineStyleLinkageUtil::SetStyleParams
(
DgnElementP          pElem,
LineStyleParamsCP   pParams
)
    {
    StyleLink   linkBuf;

    lineStyle_clearStyle (pElem, pParams->lineMask, pParams->mlineFlags);

    BeAssert (NULL != pParams);
    if (NULL == pParams)
        return BSIERROR;

    if (LineStyleLinkageUtil::CreateRawLinkage (&linkBuf, pParams, pElem->Is3d()) > 0)
        elemUtil_appendLinkage (pElem, &linkBuf.linkHeader);

    return BSISUCCESS;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus LineStyleUtil::MapToLineStyleStatus(int status)
    {
    switch (status)
        {
        case LINESTYLE_STATUS_Success:
        case LINESTYLE_STATUS_Error:
        case LINESTYLE_STATUS_BadArgument:
        case LINESTYLE_STATUS_UnknownResourceError:
        case LINESTYLE_STATUS_FileNotFound:
        case LINESTYLE_STATUS_NotSameFile:
        case LINESTYLE_STATUS_InvalidForV7Symbol:
        case LINESTYLE_STATUS_InvalidForV8Symbol:
        case LINESTYLE_STATUS_FileReadOnly:
        case LINESTYLE_STATUS_AlreadyExists:
        case LINESTYLE_STATUS_BadFormat:
        case LINESTYLE_STATUS_StyleNotFound:
            return static_cast<LineStyleStatus> (status);
        }

    return LINESTYLE_STATUS_Error;
    }


/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  08/08
+---------------+---------------+---------------+---------------+---------------+------*/
static bool scaleStyleParamsTransform (LineStyleParamsP pParams, double scaleFactor, bool    canChangeSize)
    {
    if (scaleFactor == 1.0)
        return false;

    bool changed = false;

    if (0 != (pParams->modifiers & STYLEMOD_SCALE))
        {
        pParams->scale *= scaleFactor;
        changed = true;
        }
    else if (canChangeSize)
        {
        pParams->modifiers |= STYLEMOD_SCALE;
        pParams->scale = scaleFactor;
        changed = true;
        }

    if (0 != (pParams->modifiers & STYLEMOD_TRUE_WIDTH))
        {
        if (0 != (pParams->modifiers & STYLEMOD_SWIDTH))
            {
            pParams->startWidth *= scaleFactor;
            changed = true;
            }

        if (0 != (pParams->modifiers & STYLEMOD_EWIDTH))
            {
            pParams->endWidth *= scaleFactor;
            changed = true;
            }
        }

    return changed;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  08/08
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    scaleStyleParamsUnits (LineStyleParamsP pParams, Int32 styleNo, DgnModelP sourceDgnModel, DgnModelP destDgnModel)
    {
    return false;
#ifdef DGNV10FORMAT_CHANGES_WIP_LINESTYLES
    double      uorRatio=1.0;
    mdlDgnModel_getUorScaleBetweenModels (&uorRatio, sourceDgnModel, destDgnModel);

    double muRatio = dgnModel_getUorPerMaster (sourceDgnModel) / dgnModel_getUorPerMaster (destDgnModel);

    LsDefinitionP   nameRec = LsMap::FindInRef (*destDgnModel->GetDgnProjectP (), styleNo);
    bool styleinMU = (NULL != nameRec &&  nameRec->IsUnitsMaster());
    bool styleinUOR = (NULL != nameRec &&  nameRec->IsUnitsUOR());

    double scaleFactor = styleinMU ? muRatio : 1.0/uorRatio;

    bool changed = false;

    if (scaleFactor != 1.0)
        {
        if (0 != (pParams->modifiers & STYLEMOD_SCALE))
            {
            pParams->scale *= scaleFactor;
            changed = true;
            }
        else
            {
            pParams->modifiers |= STYLEMOD_SCALE;
            pParams->scale = scaleFactor;
            changed = true;
            }
        }

    // Back out UOR scale if not True Width and style is in UORs.  These are affected by scale, so we have
    // to unscale to match change in scale.
    if (0 == (pParams->modifiers & STYLEMOD_TRUE_WIDTH) && styleinUOR)
        {
        if (0 != (pParams->modifiers & STYLEMOD_SWIDTH))
            {
            pParams->startWidth *= uorRatio;
            changed = true;
            }

        if (0 != (pParams->modifiers & STYLEMOD_EWIDTH))
            {
            pParams->endWidth *= uorRatio;
            changed = true;
            }
        }

    return changed;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool LineStyleParams::ApplyTransform (TransformCR transform, bool allowSizeChange)
    {
    bool        changed = false;

    // When copying between models account for scaling
    double      scaleFactor = 1.0;
    DPoint3d    scaleVector;

    LegacyMath::TMatrix::FromNormalizedRowsOfTMatrix (NULL, &scaleVector, &transform);

    // 3d->2d passes flatten transform, so dividing scaleVector by 3 gives wrong scaleFactor
    if (scaleVector.x != 0.0 && scaleVector.y != 0.0 && scaleVector.z != 0.0)
        scaleFactor = (scaleVector.x + scaleVector.y + scaleVector.z) / 3.0;
    else
        scaleFactor = (scaleVector.x + scaleVector.y + scaleVector.z) / 2.0;

    changed = scaleStyleParamsTransform (this, scaleFactor, allowSizeChange);

#if defined (NEED_TO_SAVE_FULL_RMATRIX)
    /* You might think that you could add a rotation to account for mirror transforms, but in 
       fact since the rotation is stored as a packed2x2 or quaternion, a left-handed matrix will be
       converted to right handed.  Mirror of elm w/linestyle appears to work in 3d...but in fact, 
       it's really a 180 degree rotation being applied...i.e. wrong z direction -BB 08/01 */
    if (0 == (modifiers & STYLEMOD_RMATRIX))
        {
        double      determ = 1.0;
        RotMatrix   rMatrix;

        rMatrix.InitFrom (transform);
        determ = rMatrix.Determinant ();

        if (determ < 0.0)
            {
            modifiers |= STYLEMOD_RMATRIX;

            changed = true;
            }
        }
#endif

    /*-------------------------------------------------------------------
    Most parameters in a line style linkage should NOT be transformed.
    Scaling a line should not change the length of dashes in the line
    but if the linkage contains a surface normal or rotation matrix
    they should be adjusted.
    -------------------------------------------------------------------*/
    if (modifiers & STYLEMOD_NORMAL)
        {
        transform.MultiplyMatrixOnly (normal);
        normal.Normalize ();

        changed = true;
        }

    if (modifiers & STYLEMOD_RMATRIX)
        {
        RotMatrix   rTmp;

        /*---------------------------------------------------------------
        The rotation matrix is stored (through some cosmic anomaly) in
        row format like shared cells and views so it needs to be
        transposed for the multiplication.
        ---------------------------------------------------------------*/
        rTmp.InverseOf (rMatrix);
        rTmp.InitProduct (transform, rTmp);
        rTmp.SquareAndNormalizeColumns (rTmp, 0, 1);
        
        rMatrix.InverseOf (rTmp);

        changed = true;
        }

    return changed;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    transformStyleParams
(
LineStyleParamsP pParams,
long            styleNo,
const Transform *pTrans,
DgnModelP    sourceDgnModel,
DgnModelP    destDgnModel,
bool            canChangeSize
)
    {
    // This is called 2 ways.  For transform, the model refs are the same and the pTrans is not NULL.
    // For change of units, it is called with a NULL pTrans and different model refs.

    // This is painful because the reference transform comes in with the UOR scale built
    // into it.  That makes doing the correct thing here impossible.  Because the line
    // styles are stored in Master Units when coming from a resource file, we don't want
    // to scale by the UOR scale, but all the rest of the ref transform applies.  The same
    // result is desired for UOR line styles because the UOR scale is covered when the
    // style definition is copied into the file.  So we actually unscale the scale in the
    // change of units case.

    // For scaling widths, True Width means that it is stored in UORs, and that it is not
    // affected by width.  In that case we need to apply the full scale, including the UOR scale.
    // For non-True Width, the width is in Master Units, so we again want the reference scale except
    // the UOR scale.

    // This is the units change case.  Do what we need to and return.
#if defined (NEEDS_WORK_DGNITEM)
    if (NULL == pTrans)
        return scaleStyleParamsUnits (pParams, styleNo, sourceDgnModel, destDgnModel);
#endif

    // The rest of this function is the transform case.
    bool    changed = false;

    // When copying between models account for scaling
    double      scaleFactor = 1.0;
    DPoint3d    scaleVector;

    LegacyMath::TMatrix::FromNormalizedRowsOfTMatrix (NULL, &scaleVector, pTrans);

    /* 3d->2d passes flatten transform, so dividing scaleVector by 3 gives wrong scaleFactor */
    if (scaleVector.x != 0.0 && scaleVector.y != 0.0 && scaleVector.z != 0.0)
        scaleFactor = (scaleVector.x + scaleVector.y + scaleVector.z) / 3.0;
    else
        scaleFactor = (scaleVector.x + scaleVector.y + scaleVector.z) / 2.0;

#if defined (NEEDS_WORK_DGNITEM)
    changed = scaleStyleParamsTransform (pParams, scaleFactor, canChangeSize);
#endif

#if defined (NEED_TO_SAVE_FULL_RMATRIX)
    /* You might think that you could add a rotation to account for
       mirror transforms, but in fact since the rotation is stored
       as a packed2x2 or quaternion, a left-handed matrix will be
       converted to right handed.  Mirror of elm w/linestyle appears
       to work in 3d...but in fact, it's really a 180 degree rotation
       being applied...i.e. wrong z direction -BB 08/01 */
    if (0 == (pParams->modifiers & STYLEMOD_RMATRIX))
        {
        double      determ = 1.0;
        RotMatrix   rMatrix;

        rMatrix.InitFrom(*pTrans);
        determ = rMatrix.Determinant ();

        if (determ < 0.0)
            {
            pParams->modifiers |= STYLEMOD_RMATRIX;

            changed = true;
            }
        }
#endif

    /*-------------------------------------------------------------------
    Most parameters in a line style linkage should NOT be transformed.
    Scaling a line should not change the length of dashes in the line
    but if the linkage contains a surface normal or rotation matrix
    they should be adjusted.
    -------------------------------------------------------------------*/
    if (pParams->modifiers & STYLEMOD_NORMAL)
        {
        pTrans->MultiplyMatrixOnly (*((DVec3d*)&pParams->normal));

        ((DVec3d*)&pParams->normal)->Normalize ();

        changed = true;
        }

    if (pParams->modifiers & STYLEMOD_RMATRIX)
        {
        RotMatrix   rTmp;

        /*---------------------------------------------------------------
        The rotation matrix is stored (through some cosmic anomaly) in
        row format like shared cells and views so it needs to be
        transposed for the multiplication.
        ---------------------------------------------------------------*/
        rTmp.InverseOf(pParams->rMatrix);
        rTmp.InitProduct(*pTrans, rTmp);
        rTmp.SquareAndNormalizeColumns (rTmp, 0, 1);
        pParams->rMatrix.InverseOf(rTmp);

        changed = true;
        }

    return changed;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Chuck.Kirschman                 05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct  LinestyleParamsTransformer : IEditProperties 
    {
protected:
    TransformCP     m_transform;
    DgnModelP    m_sourceModel, m_destModel;
    bool            m_canChangeSize;

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 05/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    LinestyleParamsTransformer (TransformCP pTrans, DgnModelP sourceDgnModel, DgnModelP destDgnModel, bool canChangeSize)
        :
        m_transform (pTrans),
        m_sourceModel (sourceDgnModel),
        m_destModel (destDgnModel),
        m_canChangeSize (canChangeSize)
        {
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 05/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementProperties    _GetEditPropertiesMask () override
        {
        return ELEMENT_PROPERTY_Linestyle;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Chuck.Kirschman                 05/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    void    _EachLineStyleCallback (EachLineStyleArg& arg) override
        {
        if (0 != (arg.GetPropertyFlags () & PROPSCALLBACK_FLAGS_ElementIgnoresID))
            return;
        
        Int32 effectiveStyleNo = arg.GetEffectiveValue();    
        LineStyleParams params =  *arg.GetParams();

#if defined (NEEDS_WORK_DGNITEM)
        ElementHandleCP element = arg.GetPropertyContext().GetCurrentElemHandleP();
        switch (element->GetLegacyType())
            {
            case CURVE_ELM:
            case ARC_ELM:
            case BSPLINE_CURVE_ELM:
            case ELLIPSE_ELM:
                params.modifiers |= STYLEMOD_NOSEGMODE;
            }
#endif

        if (transformStyleParams (&params, effectiveStyleNo, m_transform, m_sourceModel, m_destModel, m_canChangeSize))
            {
            arg.SetParams (&params);
            arg.SetParamsChanged ();
            }
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/95
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleUtil::TransformParams (EditElementHandleR elem, TransformCP pTrans, DgnModelP sourceDgnModel, DgnModelP destDgnModel, bool canChangeSize)
    {
    LinestyleParamsTransformer updater (pTrans, sourceDgnModel, destDgnModel, canChangeSize);
    PropertyContext::EditElementProperties (elem, &updater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LineStyleLinkageUtil::GetParamsFromElement (LineStyleParamsP pParams, DgnElementCP elm)
    {
    if (IS_LINECODE (elm->GetSymbology().style))
        return  ERROR;

#if defined (NEEDS_WORK_DGNITEM)
    if (MULTILINE_ELM == elm->GetLegacyType())
        return LineStyleLinkageUtil::ExtractHeaderStyleLinkageFromMultiLine (*pParams, elm);  // Get the params for the element, not a line within
#endif

    StyleLink   styleLink;
    if (NULL == elemUtil_extractLinkage (&styleLink, NULL, elm, STYLELINK_ID))
        return ERROR;

    if (SUCCESS != LineStyleLinkageUtil::ExtractRawLinkage (pParams, &styleLink, elm->Is3d()))
        return ERROR;

#if defined (NEEDS_WORK_DGNITEM)
    switch (elm->GetLegacyType())
        {
        case CURVE_ELM:
        case ARC_ELM:
        case BSPLINE_CURVE_ELM:
        case ELLIPSE_ELM:
            pParams->modifiers |= STYLEMOD_NOSEGMODE;
        }
#endif

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LineStyleUtil::GetParamsFromElement (LineStyleParamsP pParams, ElementHandleR eh)
    {
    LocalPropertyCollector  lsCollector;
    
    PropertyContext::QueryElementProperties (eh, &lsCollector);
    if (!lsCollector.GetParamsSet ())
        return BSIERROR;

    *pParams = *lsCollector.GetLineStyleParams ();

    return  SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/91
+---------------+---------------+---------------+---------------+---------------+------*/
int LineStyleLinkageUtil::ExtractParams (LineStyleParamsP params, DgnElementCP elm)
    {
    if (SUCCESS != LineStyleLinkageUtil::GetParamsFromElement (params, elm))
        params->Init();

    return  SUCCESS;
    }
#if defined (NEEDS_WORK_DGNITEM)

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    ChuckKirschman                  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
struct extractStyleInfo
    {
    bool                rootStyle;
    bool                found;
    UInt32              lineMask;
    UInt32              mlineFlags;
    LineStyleParams     params;
    };

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    ChuckKirschman                  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     findLinkage
(
LinkageHeader               *outLinkP,      /* <=  replacement linkage */
void                        *arg,          /*  => user args        */
LinkageHeader               *inLinkP,       /* <=> linkage to process  */
DgnElementP                   elemP          /* <=> owner element       */
)
    {
    struct extractStyleInfo     *info = (struct extractStyleInfo *)arg;
    LineStyleParams                  params;

    if (!inLinkP->user)
        return PROCESS_ATTRIB_STATUS_NOCHANGE;

    switch (inLinkP->primaryID)
        {
        case STYLELINK_ID:
            {
            LineStyleLinkageUtil::ExtractRawLinkage (&params, inLinkP, elemP->IsGraphic() && elemP->Is3d());

            if (true == info->rootStyle && 0 == params.mlineFlags)
                {
                memcpy (&info->params, &params, sizeof(info->params));
                info->found = true;
                return (PROCESS_ATTRIB_STATUS_ABORT);
                }
            else if ((params.mlineFlags & info->mlineFlags) &&
                      params.lineMask & info->lineMask)
                {
                memcpy (&info->params, &params, sizeof(info->params));
                info->found = true;
                return (PROCESS_ATTRIB_STATUS_ABORT);
                }
            break;
            }
        }
    return (PROCESS_ATTRIB_STATUS_NOCHANGE);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    ChuckKirschman                  12/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt      LineStyle_extractStyleLinkage
(
LineStyleParamsP    pParams,
DgnElementCP         pEl,
UInt32              mlineFlags,
UInt32              lineMask
)
    {
    struct extractStyleInfo     info;

    if (NULL != pParams)
        memset (pParams, 0, sizeof *pParams);

    memset (&info, 0, sizeof(info));
    if (lineMask > 0)
        {
        info.mlineFlags = mlineFlags;
        info.lineMask = lineMask;
        }
    else
        {
        info.rootStyle = true;
        }

    mdlElement_processLinkages (findLinkage, &info, (DgnElement *)pEl); /* Okay to cast away const; I'm not changing it */

    if (info.found)
        {
        if (NULL != pParams)
            *pParams = info.params;
        return (SUCCESS);
        }
    else
        {
        return (ERROR);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    ChuckKirschman                  12/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt      LineStyleLinkageUtil::ExtractStyleLinkageFromMultiLine
(
LineStyleParamsR        params,
DgnElementCP             pEl,
bool                    isCap,
int                     lsIndex  //  To get header linkage, call LineStyle_extractStyleLinkage with 0,0
)
    {
    params.Init();
    
    UInt32 mlineFlags = (isCap ? MLSFLAG_CAP : MLSFLAG_LINE); 
    UInt32 lineMask  = 1 << (lsIndex);

    return LineStyle_extractStyleLinkage (&params, pEl, mlineFlags, lineMask);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    ChuckKirschman                  12/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt      LineStyleLinkageUtil::ExtractHeaderStyleLinkageFromMultiLine (LineStyleParamsR params, DgnElementCP pEl)
    {
    params.Init();
    return LineStyle_extractStyleLinkage (&params, pEl, 0, 0);
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LineStyleParams::operator==(LineStyleParamsCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.modifiers != modifiers)
        return false;

    if (0 == rhs.modifiers && 0 == modifiers)
        return true; // No need to compare further if both inactive...

    if (rhs.reserved   != reserved   ||
        rhs.scale      != scale      ||
        rhs.dashScale  != dashScale  ||
        rhs.gapScale   != gapScale   ||
        rhs.startWidth != startWidth ||
        rhs.endWidth   != endWidth   ||
        rhs.distPhase  != distPhase  ||
        rhs.fractPhase != fractPhase ||
        rhs.lineMask   != lineMask   ||
        rhs.mlineFlags != mlineFlags)
        return false;

    if (!rhs.normal.IsEqual (normal))
        return false;

    if (!rhs.rMatrix.IsEqual (rMatrix))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     04/91
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BentleyApi::LineStyle_setElementStyle(EditElementHandleR eeh, Int32 styleNo, LineStyleParamsP pParams)
    {
    if (!elemUtil_supportsLineStyle (eeh))
        return  BSIERROR;

    ElementPropertiesSetter   setter;
    setter.SetLinestyle (styleNo, const_cast <LineStyleParamsP> (pParams));
    return setter.Apply (eeh) ? BSISUCCESS : BSIERROR;
    }

