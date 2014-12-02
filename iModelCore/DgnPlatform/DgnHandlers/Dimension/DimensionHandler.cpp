/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Dimension/DimensionHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    "DimStyleInternal.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#define     fc_epsilon                 0.00001

struct  StrokeDimension : IAnnotationStrokeForCache
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void    StrokeScaledForCache (ElementHandleCR thisElm, ViewContextR context, AnnotationDisplayParameters const& parms) override
    {
    // Scale the dimension's attributes that are dependent on textsize.
    // The idea is to draw the dimension in the scaled state without
    // recomputing its range (because updateRange calls Draw again and
    // will cause an infinite loop)
    EditElementHandle   scaledElement (thisElm, true);

    //  Now apply the (uniform) annotation scale
    mdlDim_scale2 (scaledElement, parms.GetRescaleFactor(), true, false, false);

    //  Account for aspectRatioSkew -- must be done last!
    ViewContext::ContextMark    mark (context, thisElm);
    if (parms.HasAspectRatioSkew ())
        {
        TransformInfo tinfo (parms.GetAspectRatioSkew());
        tinfo.SetOptions (TRANSFORM_OPTIONS_DimValueMatchSource|TRANSFORM_OPTIONS_AnnotationSizeMatchSource|TRANSFORM_OPTIONS_DimSizeMatchSource);
        thisElm.GetHandler().ApplyTransform (scaledElement, tinfo);
        scaledElement.GetElementDescrP ()->Validate ();

#ifdef DGNV10FORMAT_CHANGES_WIP
        context.RemoveAspectRatioSkew (parms.GetAspectRatioSkew());
#endif
        }

    CachedDrawHandle dh(&scaledElement);
    _StrokeForCache (dh, context, 0.0);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Mukesh.Pant                     08/2011
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool            IsSameReference (DgnAttachmentP ref1, DgnAttachmentP ref2)
//    {
//    if (ref1 == NULL || ref2 == NULL)
//        return false;
//
//    return (ref1->GetElementId () == ref2->GetElementId () &&
//            mdlDgnModel_modelsAreSame (ref1, ref2) &&
//            mdlDgnModel_modelsAreSame (ref1->GetParentDgnModelP(), ref2->GetParentDgnModelP()));
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mukesh.Pant                     08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP    DoesMatchWithAnyModelInAssocToSheetPath (DgnModelR assocRefPathToSheetDgnModel, DgnModelR modelToMatch)
    {
    return (&assocRefPathToSheetDgnModel == &modelToMatch)? &modelToMatch: nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mukesh.Pant                     08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsValidPoint (DPoint3dR assocPoint, DPoint3dCP pointOnSheet, Transform assocRootToActiveTrans, Transform activeToSheetTrans)
    {
    // 1) Transform assocPoint from assoc root model to active model
    assocRootToActiveTrans.multiply (&assocPoint);

    // 2) Transform assocPoint from active model to sheet via refTrans
    activeToSheetTrans.multiply (&assocPoint);

    // 3) Check if assocPoint and pointOnSheet are perpendicular to sheet
    DVec3d zVec, pointsVec;
    zVec.init (0,0,1);
    pointsVec.normalizedDifference (&assocPoint, pointOnSheet);
    return pointsVec.smallerUnorientedAngleTo (&zVec) < fc_epsilon;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mukesh.Pant                     08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       GetValidAssocPointOnSheet (DPoint3dR assocPointOut, DgnModelR assocRefPathToSheetDgnModel, DgnModelR rootParentDgnModel, DPoint3dCP pointOnSheet, Transform activeToSheetTrans)
    {
    //DgnModelP    commonDgnModel = nullptr;
    //if (nullptr != (commonDgnModel = DoesMatchWithAnyModelInAssocToSheetPath (assocRefPathToSheetDgnModel, rootParentDgnModel)))
    //    {
    //    // Complete transform -- Assoc root model to active model (combine of step 1 and step 2)
    //    Transform   transFromAssocRootToActiveModel;
    //    transFromAssocRootToActiveModel.initIdentity();
    //
    //    // Step 1 : Transform from assoc root model to common model
    //    Transform   transFromAssocPathRootToCommonModel;
    //    mdlRefFile_getTransformToParent (&transFromAssocPathRootToCommonModel, assocRefPathToSheetDgnModel.AsDgnAttachmentP(), commonDgnModel->AsDgnAttachmentP());
    //    transFromAssocRootToActiveModel.productOf (&transFromAssocPathRootToCommonModel, &transFromAssocRootToActiveModel);
    //
    //    // Step 2 : Transform from common model to active model
    //    Transform   transFromCommonModelToActiveModel;
    //    mdlRefFile_getTransformToParent (&transFromCommonModelToActiveModel, rootParentDgnModel.AsDgnAttachmentP(), NULL);
    //    transFromAssocRootToActiveModel.productOf (&transFromCommonModelToActiveModel, &transFromAssocRootToActiveModel);
    //
    //    DPoint3d    copyOfAssocPoint = assocPointOut;
    //    if (IsValidPoint (copyOfAssocPoint, pointOnSheet, transFromAssocRootToActiveModel, activeToSheetTrans))
    //        {
    //        // if it is valid point, copy it into original point. Above function transformed the assoc point to sheet model and we need
    //        // transformed point to draw assoc line.
    //        assocPointOut = copyOfAssocPoint;
    //        return SUCCESS;
    //        }
    //    }
    //
    //DgnModelIterator  iterator (&rootParentDgnModel, MRITERATE_PrimaryChildRefs, 0);
    //DgnModelP      childDgnModel;
    //
    //while (NULL != (childDgnModel = iterator.GetNext ()))
    //    {
    //    DgnAttachmentP  refFile = childDgnModel->AsDgnAttachmentP();
    //    if (refFile->IsAnnotationAttachment())
    //        continue; // skip annotation attachment
    //
    //    if (SUCCESS == GetValidAssocPointOnSheet (assocPointOut, assocRefPathToSheetDgnModel, *childDgnModel, pointOnSheet, activeToSheetTrans))
    //        return SUCCESS;
    //    }
    //
    // return ERROR;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            StrokeAssocLines (ElementHandleCR dimEH, AdimProcess& ap)
    {
    // Don't draw assoc lines for plotting, EVE and range calculations
    if (DrawPurpose::Plot == ap.context->GetDrawPurpose () || DrawPurpose::ExportVisibleEdges == ap.context->GetDrawPurpose() || DrawPurpose::RangeCalculation == ap.context->GetDrawPurpose ())
        return;

    // // Draw only for dimensions in annotation attachments
    // DgnAttachmentP refFile = ap.GetDgnModelP()->AsDgnAttachmentP();
    // if (!refFile || !refFile->IsAnnotationAttachment ())
    //     return;
    // 
    // ViewportP      viewport    = ap.context->GetViewport ();
    // if (!viewport)
    //     return;
    // 
    // DimensionElm const* dim    = (DimensionElm const*) dimEH.GetElementCP();
    // 
    // // Activate dim line symbology
    // Symbology symb;
    // adim_getEffectiveSymbology (&symb, dimEH, DIM_MATERIAL_Extension, ADTYPE_EXT_LEFT, NULL);
    // 
    // ElemMatSymbP    currElemSymb  = ap.context->GetElemMatSymb ();
    // 
    // ap.context->SetIndexedLineWidth (*currElemSymb, symb.weight+1);
    // ap.context->SetIndexedLinePattern (*currElemSymb, 4 /* Dash-Dot */);
    // ap.context->GetIDrawGeom().ActivateMatSymb (currElemSymb);
    // 
    // // Get ref transform of design model attachment in sheet model
    // Transform activeToSheetTrans, sheetToActiveTrans;
    // mdlRefFile_getTransformFromParent (&activeToSheetTrans, ap.GetDgnModelP()->AsDgnAttachmentP(), NULL);
    // 
    // sheetToActiveTrans.inverseOf (&activeToSheetTrans);
    // 
    // bool boresite = (NULL != ap.context->GetIPickGeom ()) ? ap.context->GetIPickGeom ()->IsBoresite () : false;
    // 
    // // Find points
    // int                 ipt                         = 0;
    // DPoint3d            dimPoint;
    // DPoint3d            origin                      = dim->GetPoint(0);
    // DPoint3d            points[2], dimPtToTest;
    // AssocPoint          assocPt;
    // DgnModelP        assocRefPathToSheetDgnModel = INVALID_MODELREF;
    // 
    // for (ipt = 0, dimPoint = dim->GetPoint(ipt); ipt < dim->nPoints; ipt++)
    //     {
    //     if (!dim->GetDimTextCP(ipt)->flags.b.associative)
    //         continue;
    // 
    //     dimPtToTest = dimPoint;
    //     sheetToActiveTrans.multiply (&dimPtToTest);
    // 
    //     // Get assoc point in sheet coordinates. It should be transformed to assoc element's model > active model > back to sheet model.
    //     if (!ap.context->IsFrustumPointVisible (dimPtToTest, boresite) ||
    //         SUCCESS != AssociativePoint::GetPointFromElement (&points[0], dimEH, ipt, dim->nPoints, ap.GetDgnModelP()) ||
    //         SUCCESS != AssociativePoint::ExtractPoint (assocPt, dimEH, ipt, dim->nPoints) ||
    //         SUCCESS != AssociativePoint::GetRoot (NULL, &assocRefPathToSheetDgnModel, NULL, NULL, assocPt, ap.GetDgnModelP (), 0) ||
    //         INVALID_MODELREF == assocRefPathToSheetDgnModel)
    //         continue;
    // 
    //     // Step 1: Transform assoc point from sheet coordinates back to its root model
    //     Transform transFromSheetToAssocPathRoot;
    //     mdlRefFile_getTransformFromParent (&transFromSheetToAssocPathRoot, assocRefPathToSheetDgnModel->AsDgnAttachmentP(), ap.GetDgnModelP()->AsDgnAttachmentP());
    //     transFromSheetToAssocPathRoot.multiply (&points[0]);
    // 
    //     // Get stored point
    //     points[1] = dimPoint;
    //     if (ipt > 0 && dim->GetDimTextCP(ipt)->flags.b.relative)
    //         points[1].add (&origin);
    // 
    //     // Step 2: This function does two steps:
    //     // Finds out the correct reference path from assoc root to active model. Transform assoc point into active model
    //     // Transform assoc point from active model to sheet model. Check if computed assoc point is valid. (i.e. Prependicular to sheet)
    //     if (SUCCESS != GetValidAssocPointOnSheet (points[0], *assocRefPathToSheetDgnModel, *mdlDgnModel_getRootParent (ap.GetDgnModelP()), &points[1], activeToSheetTrans))
    //         continue;
    // 
    //     if (points[1].isEqual (&points[0], fc_epsilon))
    //         continue;
    // 
    //     // Draw a line between them
    //     ap.context->GetIDrawGeom().DrawLineString3d (2, points, NULL);
    //     }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    ElementHandleCR thisElm = *dh.GetElementHandleCP();
    AdimProcess     ap (thisElm, &context);

    // Need to stroke dimension for locates to properly setup hit detail, so we can't use the proxy...
    if (DrawPurpose::Pick == context.GetDrawPurpose () && NULL != context.GetIPickGeom())
        ap.proxyOverride = DIMPROXY_OVERRIDE_NoProxy;

    // For range we want the union of both proxy and dimension, so don't draw the proxy by itself...
    if (DrawPurpose::RangeCalculation == context.GetDrawPurpose ())
        ap.proxyOverride = DIMPROXY_OVERRIDE_NotOnlyProxy;

    bool    enablePatterns = (context.GetViewFlags() && !context.GetViewFlags()->patterns);

    // patterns in terminator cells should always display (cached won't be affected by viewflag change)...
    ViewFlags   flags;
    if (enablePatterns)
        {
        flags = *context.GetViewFlags ();
        flags.patterns = true;
        context.SetViewFlags (&flags);
        }

    adim_strokeDimension (ap);

    // Draw association lines
    StrokeAssocLines (thisElm, ap);

    // reset pattern flag...
    if (enablePatterns)
        {
        flags.patterns = false;
        context.SetViewFlags (&flags);
        }
    }
}; // StrokeDimension

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
bool DimensionHandler::_GetAnnotationScale (double* annotationScale, ElementHandleCR element) const
    {
    return (true == mdlDim_getEffectiveAnnotationScale (annotationScale, element));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::_GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials)
    {
    T_Super::_GetElemDisplayParams (thisElm, params, wantMaterials);

    // only dimensions think they're above the usual level symbology rules
    params.SetLevelSymbIgnored (thisElm.GetElementCP()->ToDimensionElm().flag.noLevelSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::_DrawFiltered
(
ElementHandleCR     elIter,
ViewContextR        context,
DPoint3dCP          pts,
double              size
)
    {
    if (context.GetViewFlags() && !context.GetViewFlags()->dimens)
        return;

    DisplayHandler::_DrawFiltered (elIter, context, pts, size);
    }

/*=================================================================================**//**
* @bsiclass                                                     Josh.Schifter   12/11
+===============+===============+===============+===============+===============+======*/
struct  UseWireframeRenderMode
{
bool            m_pushed;
ViewContextR    m_context;

UseWireframeRenderMode (ViewContextR in) : m_context (in), m_pushed (false)
    {
    if (NULL == m_context.GetViewFlags ())
        return;

    ViewFlags   flags = *m_context.GetViewFlags ();

    flags.SetRenderMode (MSRenderMode::Wireframe);
    m_context.GetIViewDraw ().PushRenderOverrides (flags, m_context.GetCurrentCookedDisplayStyle ()); // Preserve current display style (monochrome, etc.)
    m_pushed = true;
    }

~UseWireframeRenderMode ()
    {
    if (m_pushed)
        m_context.GetIViewDraw ().PopRenderOverrides ();
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    if (context.GetViewFlags() && !context.GetViewFlags()->dimens)
        return;

    UInt32      info = context.GetDisplayInfo (IsRenderable (thisElm));

    if (0 == (info & DISPLAY_INFO_Edge))
        return;

    // restored in destructor!
    ViewContext::ContextMark ovrMark (context, thisElm);

    // Always have QVis treat as wireframe (don't hidden line filled terminators, etc.)...restored in destructor!
    UseWireframeRenderMode wireframe (context);

    /* NOTE: Dimension components behave like cell components w/regard to BY_CELL symbology (ACAD).
             Want effective symbology for header overrides (current display params already resolved BYLEVEL values). */
    ElemDisplayParamsP  elParams = context.GetCurrentDisplayParams();
    Symbology           elSymb;
    ElemHeaderOverrides ovr;

    elSymb.style  = elParams->GetLineStyle ();
    elSymb.weight = elParams->GetWeight ();
    elSymb.color  = elParams->GetLineColor ();

    ovr.Init (NULL, elParams->GetLevelSubLevelId().GetLevel(), 0, thisElm.GetElementCP()->GetDisplayPriority(), elParams->GetElementClass (), &elSymb, NULL);
    context.PushOverrides (&ovr);

    StrokeDimension stroker;

    Transform       newScTrans, newTopTrans;

    if ((context.GetApplyRotationToDimView () || context.GetIgnoreScaleForDimensions ()) &&
        SUCCESS == ElementUtil::GetIgnoreScaleDisplayTransforms (&newTopTrans, &newScTrans, context))
        {
        EditElementHandle  workElm (thisElm, true);

        // Can't apply 3d transform to 2d element...
        if (!workElm.GetElementCP()->Is3d() && context.Is3dView())
            ConvertTo3d (workElm, 0.0);

        // Redefine top of stack to just reference transforms...
        ViewContext::ContextMark    mark (context, workElm);

        context.PushTransform (newTopTrans);

        // Apply non-WYSIWYG transform from shared cells to element
        TransformInfo   newScTransInfo (newScTrans);

        // Decide whether to apply default behavior to dimensions. Treat notes as special since
        // they are not regular dimensions (distance between endpoints don't measure a value).
        if (context.GetIgnoreScaleForDimensions () && !mdlDim_isNoteDimension (thisElm))
            newScTransInfo.SetOptions (newScTransInfo.GetOptions () | TRANSFORM_OPTIONS_DimSizeMatchSource);
        else
            newScTransInfo.SetOptions (newScTransInfo.GetOptions () | TRANSFORM_OPTIONS_DimValueMatchSource);

        if (context.GetApplyRotationToDimView ())
            newScTransInfo.SetOptions (newScTransInfo.GetOptions () | TRANSFORM_OPTIONS_RotateDimView);

        workElm.GetHandler().ApplyTransform (workElm, newScTransInfo);
        AnnotationDisplayParameters ad;
        if (ComputeAnnotationDisplayParameters (ad, workElm, context))
            stroker.StrokeScaledForCache (workElm, context, ad);
        else
            stroker._StrokeForCache (CachedDrawHandle(&workElm), context, 0.0); // Don't use cached representation...
        return;
        }

    StrokeAnnotationElm annotationStroker (this, thisElm, context, &stroker);
    annotationStroker.DrawUsingContext ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionHandler::_IsPlanar (ElementHandleCR thisElm, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal)
    {
    RotMatrix       tmpTrans;
    GetRotationMatrix (thisElm, tmpTrans);

    if (normal)
        tmpTrans.GetColumn(*normal,  2);

    if (point)
        BentleyApi::mdlDim_extractPointsD (point, thisElm, 0, 1);

    return true;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_transformProxyCell                                 |
|                                                                       |
| author        RayBentley                              05/02           |
|                                                                       |
+----------------------------------------------------------------------*/
static void     adim_transformProxyCell
(
EditElementHandleR     dimElement,
const Transform*    pTransform
)
    {
    DimProxyCellBlock       *pCellBlock;

    if (NULL != (pCellBlock = (DimProxyCellBlock*) mdlDim_getEditOptionBlock (dimElement, ADBLK_PROXYCELL, NULL)))
        {
        pTransform->Multiply(pCellBlock->origin);
        pCellBlock->rotScale.InitProduct(*pTransform, pCellBlock->rotScale);

        pCellBlock->checkSumType = CURRENT_DIM_CHECKSUM_TYPE;
        pCellBlock->checkSum = dimElement.GetElementCP()->ToDimensionElm().ComputeAdditiveCheckSum (pCellBlock->checkSumType);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     mdlDim_hasProxyCell (ElementHandleCR dimElement)
    {
    IDimensionQuery* hdlr = dynamic_cast<IDimensionQuery*> (&dimElement.GetHandler());
    if (NULL == hdlr)
        return false;

    ElementId proxyId;
    return (SUCCESS == hdlr->GetProxyCell (dimElement, proxyId, NULL, NULL)) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
int             DimensionHandler::TransformDimensionEx
(
EditElementHandleR  elemHandle,     /* <=> Dimension element to be transformed    */
Transform const*    ct,             /*  => composite transform                    */
DgnModelP        sourceDgnModel,
DgnModelP        modelRef,
bool                dontScaleSize,
bool                scaleValue,
bool                rotateDimView
)
    {
    RotMatrix   rMatrix, rMatrixOriginal;
    DVec3d      xCol, yCol, zCol;
    DVec3d      tangent;
    bool        hasProxyCell;

    hasProxyCell = mdlDim_hasProxyCell (elemHandle);      // Test first - before the checksum is invalidated.

    GetRotationMatrix (elemHandle, rMatrix);
    rMatrixOriginal = rMatrix;

    /* Apply the transform */
    rMatrix.InitProduct(*ct, rMatrix);

    if (scaleValue)
        {
        double  valueScale = 1.0;

        rMatrix.GetColumn(xCol,  0);
        valueScale = bsiDVec3d_magnitude (&xCol);

        BentleyApi::adimUtil_scaleDimValue (elemHandle, valueScale, true);
        }

    /*-------------------------------------------------------------------
    If its not an arbitrary axis dimension, make sure the matrix
    is still orthogonal.
    -------------------------------------------------------------------*/
    if (elemHandle.GetElementCP()->ToDimensionElm().flag.alignment != 3)
        {
        rMatrix.GetColumn(xCol,  0);
        rMatrix.GetColumn(yCol,  1);
        bsiDVec3d_crossProduct (&zCol, &xCol, &yCol);
        bsiDVec3d_crossProduct (&yCol, &zCol, &xCol);
        rMatrix.InitFromColumnVectors(xCol, yCol, zCol);
        }

    {

    DVec3d scaleVector;

    rMatrix.NormalizeColumnsOf (rMatrix, scaleVector);

    }
    BentleyApi::mdlDim_setDimRotMatrix (elemHandle, &rMatrix);

    // Transform the points. The distance factors stored in the points will be
    // transformed by adim_transformInternalParameters below.
    adim_transformDimPoints (elemHandle, &rMatrixOriginal, ct, true, false);

    for (int iPoint=0; iPoint<elemHandle.GetElementCP()->ToDimensionElm().nPoints-1; iPoint++)
        {
        if (mdlDim_segmentGetCurveStartTangent ((DPoint3d *) &tangent, elemHandle, iPoint))
            {
            ct->MultiplyMatrixOnly (tangent);
            mdlDim_segmentSetCurveStartTangent (elemHandle, iPoint, (DPoint3d *) &tangent);
            }

        if (mdlDim_segmentGetCurveEndTangent ((DPoint3d *) &tangent, elemHandle, iPoint))
            {
            ct->MultiplyMatrixOnly (tangent);
            mdlDim_segmentSetCurveEndTangent (elemHandle, iPoint, (DPoint3d *) &tangent);
            }
        }

    /* scale dimension from source model */
    if ( ! dontScaleSize)
        adim_transformInternalParameters (elemHandle, sourceDgnModel, modelRef, ct, true);

    /* rotate dim view rot matrix so that the dim text orientation remains constant relative to the dimension  */
    if (rotateDimView)
        {
        RotMatrix viewRotMatrix;
        DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&elemHandle.GetHandler());
        if (NULL == hdlr)
            return ERROR;
        hdlr->GetViewRotation (elemHandle, viewRotMatrix);
        viewRotMatrix.TransposeOf(viewRotMatrix);
        viewRotMatrix.InitProduct(*ct, viewRotMatrix);
        viewRotMatrix.TransposeOf(viewRotMatrix);
        {
        DVec3d scaleVector;
        viewRotMatrix.NormalizeColumnsOf (viewRotMatrix, scaleVector);
        }
        hdlr->SetViewRotation (elemHandle, viewRotMatrix);
        }

    if (hasProxyCell)
        adim_transformProxyCell (elemHandle, ct);

    // Now that we've modified the dimension element, make absolutely sure this ElementHandle is no longer treated as persistent...
    elemHandle.SetNonPersistent ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/94
 * @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_SetViewRotation  (EditElementHandleR dimElement, RotMatrixCR rMatrix)
    {
    DimViewBlock    *dvbP;

    if (NULL != (dvbP = (DimViewBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_VIEWROT, NULL)))
        {
        adim_packToFourDoubles (dvbP->viewRot, &rMatrix, dimElement.GetElementCP()->Is3d());
        return SUCCESS;
        }

    DimViewBlock    dimView;
    memset (&dimView, 0, sizeof(dimView));
    dimView.nWords = sizeof (dimView) / 2;
    dimView.type   = ADBLK_VIEWROT;
    adim_packToFourDoubles (dimView.viewRot, &rMatrix, dimElement.GetElementCP()->Is3d());
    return (BentleyStatus) mdlDim_insertOptionBlock (dimElement, (DimOptionBlockHeader *) &dimView, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/94
* @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_GetViewRotation  (ElementHandleCR dimElement, RotMatrixR rMatrix) const
    {
    // some callers do not check for return status and uses uninitialized matrix
    rMatrix.InitIdentity ();

    DimViewBlock const *dvbP;
    if (NULL == (dvbP = (DimViewBlock const*) mdlDim_getOptionBlock (dimElement, ADBLK_VIEWROT, NULL)))
        return ERROR;

    adim_unpackFromFourDoubles (&rMatrix, dvbP->viewRot, dimElement.GetElementCP()->Is3d());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_DIMENSION_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionHandler::_IsTransformGraphics (ElementHandleCR elemHandle, TransformInfoCR)
    {
    // *** TBD: analyze transform, looking for ???
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 09/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus          DimensionHandler::UpdateModelAnnotationScale (EditElementHandleR  element, ChangeAnnotationScale& changeContextIn)
    {
    if (DIMENSION_ELM != element.GetLegacyType())
        return ERROR;

    DimensionStylePtr style = GetDimensionStyle (element);
    if (style.IsNull())
        return ERROR;

    bool uNotUseModelAnnotationScale;
    style->GetBooleanProp (uNotUseModelAnnotationScale, DIMSTYLE_PROP_Placement_NotUseModelAnnotationScale_BOOLINT);
    if (uNotUseModelAnnotationScale)
        return ERROR;

    double                      currAnnotationScale   = 1.0;
    bool                        currAnnotationScaleFlag = false;
    double                      newAnnotationScale = 1.0;
    bool                        newAnnotationScaleFlag;
    double                      annotationScaleChange = 1.0;

    currAnnotationScaleFlag = mdlDim_overallGetModelAnnotationScale (&currAnnotationScale, element);

    if (fabs (currAnnotationScale - 0.0) < mgds_fc_epsilon)
        currAnnotationScale = 1.0;

    if (SUCCESS != mdlChangeAnnotationScale_getEffectiveValues (&changeContextIn, &newAnnotationScale, &newAnnotationScaleFlag, &annotationScaleChange, currAnnotationScale, currAnnotationScaleFlag))
        return ERROR;

    DimensionHandler::ScaleElement (element, annotationScaleChange, true, false, true);

    if (newAnnotationScaleFlag)
        mdlDim_overallSetModelAnnotationScale (element, &newAnnotationScale);
    else
        mdlDim_overallSetModelAnnotationScale (element, NULL);

    return DisplayHandler::ValidateElementRange (element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::_OnTransform
(
EditElementHandleR      elemHandle,
TransformInfoCR         trans
)
    {
    StatusInt   status;

    if ((status = T_Super::_OnTransform (elemHandle, trans)) != SUCCESS)
        return status;

    /*-------------------------------------------------------------------
        Step 1 : Propagate annotation scale
    -------------------------------------------------------------------*/
    UInt32              options         = trans.GetOptions ();
    DgnModelP           modelRef        = elemHandle.GetDgnModelP();

    if (options & TRANSFORM_OPTIONS_ApplyAnnotationScale)
        {
        ChangeAnnotationScaleP  changeContext = mdlChangeAnnotationScale_new (modelRef);
        mdlChangeAnnotationScale_setAction (changeContext, trans.GetAnnotationScaleAction (), trans.GetAnnotationScale ());
        UpdateModelAnnotationScale (elemHandle, *changeContext);
        mdlChangeAnnotationScale_free (&changeContext);
        }

    /*-------------------------------------------------------------------
        Step 2 : Propagate transform
    -------------------------------------------------------------------*/
    bool            dontScaleSize   = false;
    bool            scaleValue      = (0 != (options & TRANSFORM_OPTIONS_DimValueMatchSource));
    bool            rotateDimView   = (0 != (options & TRANSFORM_OPTIONS_RotateDimView));

    if (mdlDim_getEffectiveAnnotationScale (NULL, elemHandle))
        {
        dontScaleSize = (0 != (options & TRANSFORM_OPTIONS_AnnotationSizeMatchSource));
        }
    else
        {
        dontScaleSize = (0 != (options & TRANSFORM_OPTIONS_DimSizeMatchSource));
        if (dontScaleSize && mdlDim_isNoteDimension (elemHandle))
            dontScaleSize = !(0 != (options & TRANSFORM_OPTIONS_NoteScaleSize));
        }

    return TransformDimensionEx (elemHandle, trans.GetTransform (), modelRef, modelRef, dontScaleSize, scaleValue, rotateDimView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::_OnFenceStretch
(
EditElementHandleR      eeh,
TransformInfoCR         transform,
FenceParamsP            fp,
FenceStretchFlags       options
)
    {
    DimensionElm& el = eeh.GetElementP()->ToDimensionElmR();

    for (int i=0; i < el.nPoints; i++)
        {
        DPoint3d    point = el.GetPoint (i);

        if (fp->PointInside (point))
            {
            transform.GetTransform ()->Multiply (point);
            el.SetPoint (point, i);
            }
        }
    ValidateElementRange(eeh);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR eh,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    ElementAgenda agenda;

    if (SUCCESS != Drop (eh, agenda, DropGeometry (DropGeometry::OPTION_Dimensions)))
        return ERROR;

    for (EditElementHandleP curr = agenda.GetFirstP (), end = curr + agenda.GetCount (); curr < end ; curr++)
        curr->GetHandler().FenceClip (inside, outside, *curr, fp, options);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Dimensions & geometry.GetOptions ()))
        return ERROR;

    if (DropGeometry::DIMENSION_Segments == geometry.GetDimensionOptions ())
        return DropDimensionToSegments (dropGeom, eh);

    return DropToElementDrawGeom::DoDrop (eh, dropGeom, geometry, DropGraphics ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus     DimensionHandler::_OnGeoCoordinateReprojection
(
EditElementHandleR                     source,
IGeoCoordinateReprojectionHelper&       reprojectionHelper,
bool                                    inChain
)
    {
    // only go through the complicated transform for large dimensions
    if (!reprojectionHelper.ShouldStroke (source, ReprojectionOptionIfLarge))
        return T_Super::_OnGeoCoordinateReprojection (source, reprojectionHelper, inChain);

    // never try to do the complicated transform on note dimension.
    if (mdlDim_isNoteDimension (source))
        return T_Super::_OnGeoCoordinateReprojection (source, reprojectionHelper, inChain);

    // get the local transform and apply it to the rotMatrix.
    Transform   localTransform;
    DPoint3d    origin;
    _GetTransformOrigin (source, origin);
    reprojectionHelper.GetLocalTransform (&localTransform, origin, NULL, true, true);

    RotMatrix       rMatrix, rMatrixOriginal;
    GetRotationMatrix (source, rMatrix);
    rMatrixOriginal = rMatrix;

    /* Apply the transform */
    rMatrix.InitProduct(localTransform, rMatrix);

    /*-------------------------------------------------------------------
    If its not an arbitrary axis dimension, make sure the matrix
    is still orthogonal.
    -------------------------------------------------------------------*/
    DVec3d      xCol, yCol, zCol;
    if (source.GetElementCP()->ToDimensionElm().flag.alignment != 3)
        {
        rMatrix.GetColumn(xCol,  0);
        rMatrix.GetColumn(yCol,  1);
        bsiDVec3d_crossProduct (&zCol, &xCol, &yCol);
        bsiDVec3d_crossProduct (&yCol, &zCol, &xCol);
        rMatrix.InitFromColumnVectors(xCol, yCol, zCol);
        }

    {

    DVec3d scaleVector;

    rMatrix.NormalizeColumnsOf (rMatrix, scaleVector);

    }
    mdlDim_setDimRotMatrix (source, &rMatrix);

    ReprojectStatus status = REPROJECT_Success;
    for (int iPoint=0; iPoint<source.GetElementP()->ToDimensionElm().nPoints; iPoint++)
        {
        /* Figure we should do this always...because association is no longer going to be evaluated to display dimension */
        DPoint3d            point;
        mdlDim_extractPointsD (&point, source, iPoint, 1);
        ReprojectStatus     childStatus;
        if (REPROJECT_Success != (childStatus = reprojectionHelper.ReprojectPoints (&point, NULL, NULL, &point, 1)))
            status = childStatus;
        source.GetElementP()->ToDimensionElmR().SetPoint(point, iPoint);
        }

    /* scale dimension from source model */
    adim_transformInternalParameters (source, source.GetDgnModelP(), source.GetDgnModelP(), &localTransform, true);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DimensionHandler::_ApplyAnnotationScaleDifferential
(
EditElementHandleR thisElm,
double          scale
)
    {
    return mdlDim_scale2 (thisElm, scale, true, false, false);
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::_OnPreprocessCopy (EditElementHandleR thisElm, ElementCopyContextP ccP)
    {
    if (SUCCESS != T_Super::_OnPreprocessCopy (thisElm, ccP))
        return ERROR;

    double      scale;

    if (0 != (ccP->GetCloneOptions() & ElementCopyContext::CLONE_OPTIONS_ScaleReferenceDimensions) &&
        SUCCESS == ccP->CalculateReferenceScale (scale))
        BentleyApi::adimUtil_scaleDimValue (thisElm, scale, true);

    return _OnPreprocessCopyAnnotationScale (thisElm, ccP);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::_OnChangeOfUnits
(
EditElementHandleR elemHandle,
DgnModelP    sourceDgnModel,
DgnModelP    destDgnModel
)
    {
    StatusInt status;

    if ((status = T_Super::_OnChangeOfUnits (elemHandle, sourceDgnModel, destDgnModel)) != SUCCESS)
        return status;

    Transform trans;
    trans.initIdentity ();
    return TransformDimensionEx (elemHandle, &trans, sourceDgnModel, destDgnModel, false, false, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus    toTextPartId (DimensionTextPartType& textType, DimensionPartType partType)
    {
    switch (partType)
        {
        case ADTYPE_TEXT_SINGLE:    // fallthru
        case ADTYPE_TEXT_UPPER:     textType = DIMTEXTPART_Primary;     return SUCCESS;
        case ADTYPE_TEXT_LOWER:     textType = DIMTEXTPART_Secondary;   return SUCCESS;
        }

    BeAssert (false);
    textType = DIMTEXTPART_Primary;
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus    toTextSubPartId (DimensionTextPartSubType& textType, DimensionPartSubType subType)
    {
    switch (subType)
        {
        case ADSUB_NONE:        textType = DIMTEXTSUBPART_Main;             return SUCCESS;
        case ADSUB_TOL_UPPER:   textType = DIMTEXTSUBPART_Tolerance_Plus;   return SUCCESS;
        case ADSUB_TOL_SINGLE:  // fallthru
        case ADSUB_TOL_LOWER:   textType = DIMTEXTSUBPART_Tolerance_Minus;  return SUCCESS;
        case ADSUB_LIM_UPPER:   // fallthru
        case ADSUB_LIM_SINGLE:  textType = DIMTEXTSUBPART_Limit_Upper;      return SUCCESS;
        case ADSUB_LIM_LOWER:   textType = DIMTEXTSUBPART_Limit_Lower;      return SUCCESS;
        }

    BeAssert (false);
    textType = DIMTEXTSUBPART_Main;
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ITextPartIdPtr DimensionHandler::_GetTextPartId (ElementHandleCR eh, HitPathCR hitPath) const
    {
    UInt32                  partSegment;
    DimensionPartType       partType;
    DimensionPartSubType    partSubType;

    if (SUCCESS != hitPath.GetDimensionParameters (NULL, NULL, &partSegment, &partType, &partSubType))
        { BeAssert (false); return NULL; }

    EditElementHandle textElement;
    GetTextDescr (eh, textElement, partSegment, partType, partSubType);

    if ( ! textElement.IsValid())
        return NULL;

    return DimensionTextPartId::Create (partSegment, partType, partSubType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionHandler::_GetTextPartIds (ElementHandleCR eh, ITextQueryOptionsCR options, T_ITextPartIdPtrVectorR textPartIds) const
    {
    // Dimensions don't support fields.
    if (options.ShouldRequireFieldSupport ())
        return;

    UInt32  segmentCount    = GetNumSegments (eh);
    bool    hasDual         = TO_BOOL (eh.GetElementCP()->ToDimensionElm().flag.dual);

    // For note dimensions we need to bail out if there is no text inside them because we do not stroke the value.
    IDimElementHelperPtr helper = DimensionHelperFactory::CreateHelper (eh);
    if (!helper.IsValid())
        return;

    if (!helper->HasText())
        return;

    for (UInt32 segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex)
        {
        textPartIds.push_back (DimensionTextPartId::Create (segmentIndex, DIMTEXTPART_Primary, DIMTEXTSUBPART_Main));

        if (hasDual)
            textPartIds.push_back (DimensionTextPartId::Create (segmentIndex, DIMTEXTPART_Secondary, DIMTEXTSUBPART_Main));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TextBlockPtr DimensionHandler::_GetTextPart (ElementHandleCR eh, ITextPartIdCR textPartId) const
    {
    DimensionTextPartId const * dimTextPartId = dynamic_cast<DimensionTextPartId const *>(&textPartId);
    if (NULL == dimTextPartId)
        { BeAssert (false); return NULL; }

    bool            isPrimary   = (DIMTEXTPART_Secondary != dimTextPartId->GetPartType ());

    EditElementHandle  textElement;
    //We rely on mdlDim_getTextDescr to create a text element with * on it. It returns error for dimension with only dimText
    GetTextDescr (eh, textElement, dimTextPartId->GetPartSegment (), dimTextPartId->GetPartType (), dimTextPartId->GetPartSubType ());
    if (!textElement.IsValid())
        { BeAssert (false); return NULL; }

    DimDerivedDataP dimDerivedData = (DimDerivedDataP)malloc (sizeof(DimDerivedData));
    memset (dimDerivedData, 0, sizeof (*dimDerivedData));
    dimDerivedData->flags = DIMDERIVEDDATA_FLAGS_TEXTBOX;

    if (SUCCESS != mdlDim_getDerivedData (dimDerivedData, eh))
        {
        BeAssert (false);
        mdlDimDerivedData_free (&dimDerivedData);
        return NULL;
        }

    DPoint3d    origin;
    DVec3d      xVec;
    DVec3d      zVec;
    if (SUCCESS != mdlDimDerivedData_getTextBox (dimDerivedData, &origin, &xVec, &zVec, NULL, isPrimary ? true : false, dimTextPartId->GetPartSegment ()))
        {
        BeAssert (false);
        mdlDimDerivedData_free (&dimDerivedData);
        return NULL;
        }

    mdlDimDerivedData_free (&dimDerivedData);

    DVec3d yVec;
    bsiDVec3d_crossProduct (&yVec, &zVec, &xVec);

    RotMatrix rMatrix;
    rMatrix.InitFromColumnVectors(xVec, yVec, zVec);

    ITextQueryCP            childTextQuery  = textElement.GetITextQuery ();
    if (NULL == childTextQuery)
        { BeAssert (false); return NULL;}
    T_ITextPartIdPtrVector  textPartIds;

    childTextQuery->GetTextPartIds (textElement, *ITextQueryOptions::CreateDefault (), textPartIds);

    if (1 != textPartIds.size ())
        { BeAssert (false); return NULL; }

    TextBlockPtr textBlock = childTextQuery->GetTextPart (textElement, *textPartIds[0]);

    textBlock->SetOrientation (rMatrix);
    textBlock->SetTextElementOrigin (origin);

    return textBlock;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ITextEdit::ReplaceStatus DimensionHandler::_ReplaceTextPart (EditElementHandleR dimEeh, ITextPartIdCR textPartId, TextBlockCR textBlock)
    {
    DimensionTextPartId const * dimTextPartId = dynamic_cast<DimensionTextPartId const *>(&textPartId);
    if (NULL == dimTextPartId)
        { BeAssert (false); return ReplaceStatus_Error; }

    // NEEDS_THOUGHT: Should I make a copy?
    TextBlock processedTextBlock (textBlock);

    processedTextBlock.ReplaceFieldsWithNormalString ();

    EditElementHandle newText;
    if (TextBlock::TO_ELEMENT_RESULT_Success != processedTextBlock.ToElement (newText, dimEeh.GetDgnModelP (), NULL))
        return ReplaceStatus_Error;

    if (SUCCESS != SetTextFromDescr (dimEeh, newText, true, dimTextPartId->GetPartSegment (), dimTextPartId->GetPartType (), dimTextPartId->GetPartSubType ()))
        { BeAssert (false); return ReplaceStatus_Error; }

    DisplayHandler::ValidateElementRange (dimEeh);

    return ITextEdit::ReplaceStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    EditElementHandle org(eeh, true);
    eeh.GetElementP()->SetIs3d(true); // can't wait for super...need for BentleyApi::mdlDim_setDimRotMatrix...

    // Convert 2d packed 2x2 into 3d quaternion
    RotMatrix   rMatrix;

    GetRotationMatrix (org, rMatrix);
    BentleyApi::mdlDim_setDimRotMatrix (eeh, &rMatrix);

    if (org.GetElementCP()->ToDimensionElm().nOptions)
        {
        DimViewBlock const* inViewBlockPtr  = org.GetElementCP()->ToDimensionElm().GetDimViewBlockCP();
        DimViewBlock*       outViewBlockPtr = eeh.GetElementP()->ToDimensionElmR().GetDimViewBlockP();

        for (int iPoint=0; iPoint < org.GetElementCP()->ToDimensionElm().nOptions; iPoint++)
            {
            if (ADBLK_VIEWROT == inViewBlockPtr->type)
                {
                double      packed[4], quaternion[4];

                memcpy (packed, inViewBlockPtr->viewRot, sizeof (packed));
                rMatrix.InitFromRowValuesXY ( packed);
                rMatrix.GetQuaternion(quaternion, true);
                memcpy (outViewBlockPtr->viewRot, quaternion, sizeof (quaternion));
                }

            inViewBlockPtr = (DimViewBlock const*) ((const short *) inViewBlockPtr + inViewBlockPtr->nWords);
            outViewBlockPtr = (DimViewBlock *) ((short *) outViewBlockPtr + outViewBlockPtr->nWords);
            }
        }

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    RotMatrix   rMatrix;

    // Get the original rotation matrix before the 2D flattening
    GetRotationMatrix (eeh, rMatrix);

    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    EditElementHandle org(eeh, true);
    eeh.GetElementP()->SetIs3d(false); // can't wait for super...need for BentleyApi::mdlDim_setDimRotMatrix...

    // De-scale the matrix, only interested in the rotation
    double  det   = bsiRotMatrix_determinant (&rMatrix);
    double  scale = 1.0 / (pow (fabs (det), 1.0 / 3.0));

    bsiRotMatrix_scaleColumns (&rMatrix, &rMatrix, scale, scale, scale);

    // Convert 2d quaternion to 2d packed 2x2
    // A couple of special cases - if it is purely Z spin, then original func is fine.
    // If it falls to a line, then handle that case. Else use identity as a best guess.
    if (bsiRotMatrix_isXYRotation (&rMatrix, NULL))
        {
        // Ok...
        }
    else if (bsiRotMatrix_isOrthogonal (&rMatrix) && -1 == bsiRotMatrix_summaryZEffects (&rMatrix))
        {
        // Z has a mirror. XY alone is probably the best we can do ...
        }
    else if (fabs (rMatrix.form3d[2][2]) < mgds_fc_epsilon)
        {
        DRange3d range = org.GetElementCP()->GetRange();
        DSegment3d  segment;

        // if the [2][2] slot is 0, it is perpendicular to Z plane. Range is the best guess
        segment.point[0].x = range.low.x;
        segment.point[0].y = range.low.y;
        segment.point[1].x = range.high.x;
        segment.point[1].y = range.high.y;
        segment.point[0].z = segment.point[1].z = 0.0;

        EditElementHandle   tmpEeh;

        LineHandler::CreateLineElement (tmpEeh, &eeh, segment, false, *eeh.GetDgnModelP ());
        tmpEeh.GetElementP()->SetSizeWords(tmpEeh.GetElementP ()->GetAttributeOffset()); // nerf linkages...

        eeh.ReplaceElementDescr (tmpEeh.ExtractElementDescr().get());
        return;
        }
    else
        {
        // Don't know what to do - use identity
        rMatrix.InitIdentity ();
        }

    BentleyApi::mdlDim_setDimRotMatrix (eeh, &rMatrix);

    if (org.GetElementCP()->ToDimensionElm().nOptions)
        {
        DimViewBlock const* inViewBlockPtr = org.GetElementCP()->ToDimensionElm().GetDimViewBlockCP();
        DimViewBlock*       outViewBlockPtr = eeh.GetElementP()->ToDimensionElmR().GetDimViewBlockP();

        for (int iPoint=0; iPoint < org.GetElementCP()->ToDimensionElm().nOptions; iPoint++)
            {
            if (ADBLK_VIEWROT == inViewBlockPtr->type)
                {
                double      packed[4], quaternion[4];

                memcpy (quaternion, inViewBlockPtr->viewRot, sizeof(packed));
                rMatrix.InitTransposedFromQuaternionWXYZ ( quaternion);
                rMatrix.GetRowValuesXY(packed);
                memcpy (outViewBlockPtr->viewRot, packed, sizeof (quaternion));
                }

            inViewBlockPtr = (DimViewBlock const*) ((const short *) inViewBlockPtr + inViewBlockPtr->nWords);
            outViewBlockPtr = (DimViewBlock *) ((short *) outViewBlockPtr + outViewBlockPtr->nWords);
            }
        }

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DimensionHandler::CreateDimensionElement (EditElementHandleR eeh, IDimCreateDataCR createData, DimensionType dimType, bool is3d, DgnModelR modelRef)
    {
    DimensionHandler&   hdlr = DimensionHandler::GetInstance();

    static const int DIMELEM_BUFFER_PADDING = 100;
    size_t size = sizeof(DimensionElm) + DIMELEM_BUFFER_PADDING;
    DgnElementP      elemP  = (DgnElementP) _alloca (size);
    memset (elemP, 0, size);
    ElementUtil::SetRequiredFields (*elemP, NULL, DIMENSION_ELM, (int)size, false, false, false, (ElementUtil::ElemDim) is3d, &modelRef);

    MSElementDescrP descr = new MSElementDescr (*elemP, modelRef);

    eeh.SetElementDescr(descr, false);
    eeh.GetElementP()->SetElementClass(DgnElementClass::Dimension);
    eeh.GetElementP()->ToDimensionElmR().dimcmd                      = static_cast<byte>(dimType);
    eeh.GetElementP()->ToDimensionElmR().view                        = static_cast<byte>(createData._GetViewNumber());
    eeh.GetElementP()->ToDimensionElmR().version                     = DIM_VERSION;

    eeh.GetElementP()->SetSizeWordsNoAttributes(sizeof (DimensionElm)/2);

    BentleyApi::mdlDim_setDimRotMatrix (eeh, &createData._GetDimRMatrix());

    adim_insertViewRotBlock (eeh, createData._GetViewRMatrix());

    DimensionStyleCR    dimStyle = createData._GetDimStyle();

    hdlr.UpdateFromDimStyle (eeh, dimStyle, &createData, ADIM_PARAMS_CREATE);
    hdlr.SetShieldsFromStyle (eeh, dimStyle);
    hdlr.ValidateElementRange (eeh);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_SetPoint
(
EditElementHandleR  eeh,
DPoint3dCP          newPoint,
AssocPoint const*   assocPt,
int                 pointNo
)
    {
    if (0 > pointNo || GetNumPoints (eeh) <= pointNo)
        return ERROR;

    if (NULL != assocPt)
        {
        eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.associative = true;
        eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.relative    = false;

        if (SUCCESS != AssociativePoint::InsertPoint (eeh, *assocPt, pointNo, GetNumPoints (eeh)+1))
            return ERROR;

        DPoint3d point;
        AssociativePoint::GetPoint (&point, *assocPt, eeh.GetDgnModelP());
        eeh.GetElementP()->ToDimensionElmR().SetPoint(point, pointNo);
        }
    else if (eeh.GetElementP()->ToDimensionElm().GetDimTextCP(pointNo)->flags.b.associative &&
             mdlDim_isNoteDimension (eeh) && pointNo != GetNumPoints (eeh)-1)
        {
        AssociativePoint::RemovePoint (eeh, pointNo, GetNumPoints (eeh));
        eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.associative = false;
        eeh.GetElementP()->ToDimensionElmR().SetPoint(*newPoint, pointNo);
        }
    else if (!eeh.GetElementP()->ToDimensionElm().GetDimTextCP(pointNo)->flags.b.associative)
        {
        eeh.GetElementP()->ToDimensionElmR().SetPoint(*newPoint, pointNo);
        }

    if (mdlDim_isNoteDimension (eeh))
        {
        // Dwg leaders have a manadatory start tangent even though it is
        // free-flowing. In other words, it is not driven by a dimstyle setting
        // like the end tangent (specified by the note orientation).
        // When points change, remove that tangent so that it becomes
        // truly free-flowing. Otherwise the curve always honors the
        // start tangent and produces incorrect leaders.
        mdlDim_segmentSetCurveStartTangent (eeh, 0, NULL);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/07
+---------------+---------------+---------------+---------------+---------------+------*/
static     StatusInt   adim_testAllowableAssocAdjustment
(
ElementHandleCR element,
DPoint3dCP      evaluatedPoint,
DPoint3dCP      adjustedPoint
)
    {
    if (NULL == adjustedPoint)
        return SUCCESS;

    RotMatrix   rMatrix;
    DVec3d      planeNormal;

    DimensionHandler::GetInstance().GetRotationMatrix (element, rMatrix);
    rMatrix.getColumn (&planeNormal, 2);

    if (bsiTrig_pointNearlyOnRay (adjustedPoint, evaluatedPoint, &planeNormal))
        return SUCCESS;

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::InsertPointDirect
(
EditElementHandleR  eeh,
DPoint3dCP          newPoint,
AssocPoint const*   assocPt,
DimText const&      dimText,
int                 pointNo
)
    {
    if (DIMENSION_ELM != eeh.GetLegacyType())
        return ERROR;

    //Warning! It is the responsibility of the caller of this function to convert the point to
    //relative system if they wish to do so for radial dimensions.

    DgnElementCP     elemCP   = eeh.GetElementCP();
    size_t          dimSize  = elemCP->Size ();

    if (dimSize + sizeof (DimPoint) > MAX_V8_ELEMENT_SIZE ||
        elemCP->ToDimensionElm().nPoints >= MAX_ADIM_POINTS)
        return ERROR;

    if (pointNo == LAST_POINT)
        pointNo = elemCP->ToDimensionElm().nPoints;
    else if (pointNo < 0 || pointNo > (int) elemCP->ToDimensionElm().nPoints)
        return ERROR;

    {
    DgnV8ElementBlank tmpBuf;
    eeh.GetElementP()->CopyTo(tmpBuf);
    tmpBuf.ToDimensionElmR().InsertPoint (pointNo);
    /* shift assoc point dependency indices to account for insert that isn't at end */
    AssociativePoint::VertexAddedOrRemovedFromMSElement (tmpBuf, *eeh.GetDgnModelP (), pointNo, tmpBuf.ToDimensionElm().nPoints, true);
    eeh.ReplaceElement(&tmpBuf);
    }

    *eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo) = dimText;

    if (NULL == assocPt)
        {
        eeh.GetElementP()->ToDimensionElmR().SetPoint (*newPoint, pointNo);
        eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.associative = false;
        }
    else
        {
        DPoint3d    evaluatedPoint;

        // Evaluate association to test if assocPt is acceptable
        if (SUCCESS != AssociativePoint::GetPoint (&evaluatedPoint, *assocPt, eeh.GetDgnModelP()))
            return ERROR;

        if (! eeh.GetDgnModelP()->Is3d())
            evaluatedPoint.z = 0.0;

        if (SUCCESS != adim_testAllowableAssocAdjustment (eeh, &evaluatedPoint, newPoint))
            return ERROR;

        // Add the assoc point to the dependency linkage
        if (SUCCESS != AssociativePoint::InsertPoint (eeh, *assocPt, pointNo, eeh.GetElementP()->ToDimensionElm().nPoints+1))
            return ERROR;

        /*----------------------------------------------------------------------
          Tricky: newDimPointP is still valid since InsertPoint only changes
                  the linkage data which occurs after the point array.
        ----------------------------------------------------------------------*/

        // Set dim point as evaluated from the association
        eeh.GetElementP()->ToDimensionElmR().SetPoint (evaluatedPoint, pointNo);
        eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.associative = true;
        }

    DimOverrides    *pDimOverrides = NULL;

    mdlDim_overridesGet (&pDimOverrides, eeh);
    mdlDim_overridesPointInserted (pDimOverrides, pointNo);
    mdlDim_overridesSet (eeh, pDimOverrides);
    mdlDim_overridesFreeAll (&pDimOverrides);

    ValidateElementRange(eeh);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_InsertPoint
(
EditElementHandleR     eeh,
DPoint3dCP          newPoint,
AssocPoint const*   assocPt,
DimensionStyleCR    dimStyle,
int                 pointNo
)
    {
    DimText     dimText;

    dimStyle.InitDimText (dimText, eeh, pointNo);

    return DimensionHandler::InsertPointDirect (eeh, newPoint, assocPt, dimText, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
int             DimensionHandler::_GetNumPoints (ElementHandleCR eh) const
    {
    return eh.GetElementCP()->ToDimensionElm().nPoints;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int             DimensionHandler::_GetNumSegments (ElementHandleCR eh) const
    {
    IDimElementHelperPtr helper = DimensionHelperFactory::CreateHelper (eh);
    if (helper.IsNull())
        return -1;

    return helper->GetNumberofSegments();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_ExtractPoint (ElementHandleCR eh, DPoint3dR outPoint, int iPoint) const
    {
    DgnElementCP     elemCP = eh.GetElementCP();

    if (iPoint >= elemCP->ToDimensionElm().nPoints)
        return ERROR;

    //TODO test: The earlier code returned Raw Points which I beleive would be incorrect for
    //radial dimensions
    outPoint = elemCP->ToDimensionElm().GetPoint(iPoint);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_SetPointsForLabelLine (EditElementHandleR dimElem, DSegment3dCP segmentIn, HitPathP hitPath, double offset, RotMatrixCR viewRMatrix, DimensionStyleCR dimStyle)
    {
    if (NULL == segmentIn && NULL == hitPath)
        return ERROR;

    DSegment3d   segment;

    if (NULL != segmentIn)
        {
        segment  = *segmentIn;
        }
    else
        {
        if (SUCCESS != hitPath->GetLinearParameters (&segment, NULL, NULL))
            return ERROR;
        }

    DVec3d          direction, delta, viewZ, yDir;
    DPoint3d        dimPoint;
    RotMatrix       rMatrix;

    if (0.0 == direction.NormalizedDifference (segment.point[1], segment.point[0]))
        direction.Init (1.0, 0.0, 0.0);

    bsiDVec3d_scale (&delta, &direction, offset);
    bsiDPoint3d_addDPoint3dDPoint3d (&dimPoint, &delta, &segment.point[0]);

    viewRMatrix.GetRow(viewZ, 2);
    bsiDVec3d_crossProduct (&yDir, &viewZ, &direction);
    rMatrix.InitFromColumnVectors (direction, yDir, viewZ);

    StatusInt status = SetRotationMatrix (dimElem, rMatrix);
    BeAssert (SUCCESS == status);

    status = ERROR;

    if (NULL != hitPath)
        {
        hitPath->OnCreateAssociationToSnap (dimElem.GetDgnModelP());

        GeomDetailCR    hitDetail = hitPath->GetGeomDetail();
        int             vertexCount   = (int) hitDetail.GetPointCount ();
        int             segmentNumber = (int) hitDetail.GetSegmentNumber ();
        int             locatedLineNumber;
        AssocPoint      assoc;
        double          ratio;
        
        ((DSegment3d*) &segment)->LengthToFraction (ratio, 0, offset);

        if (SUCCESS == hitPath->GetMultilineParameters (NULL, NULL, NULL, &locatedLineNumber, NULL, NULL))
            AssociativePoint::InitMline (assoc, (UShort)segmentNumber, (UShort)vertexCount, (UShort)locatedLineNumber, ratio, false);
        else
            AssociativePoint::InitProjection (assoc, (UShort)segmentNumber, (UShort)vertexCount, ratio);

        if (SUCCESS == (status = AssociativePoint::SetRoot (assoc, hitPath, dimElem.GetDgnModelP(), true, 0)) &&
            SUCCESS == (status = AssociativePoint::IsValid (assoc, hitPath->GetEffectiveRoot(), dimElem.GetDgnModelP())))
            {
            // InsertPoint expects the assoc point to resolve to the same location as the DPoint3d.  In this case it
            // doesn't by design.  The assoc point is at the text display point, we want it to scale if the line
            // changes length.  The DPoint must be the segment origin for measuring the line and to be a backup if the
            // assoc point fails.  There is code in the dependency callback to account for the discrepancy.
            if (SUCCESS == (status = InsertPoint (dimElem, &dimPoint, &assoc, dimStyle, -1)))
                dimElem.GetElementP()->ToDimensionElmR().SetPoint (segment.point[0], 0);
            }
        }

    if (SUCCESS != status)
        {
        status = InsertPoint (dimElem, &segment.point[0], NULL, dimStyle, -1);
        SetJustification (dimElem, 0, DIMSTYLE_VALUE_Text_Justification_CenterLeft);
        }

    status = InsertPoint (dimElem, &segment.point[1], NULL, dimStyle, -1);
    assert (SUCCESS == status);

    DPoint2d textLoc;
    GetTextOffset (dimElem, 0, textLoc);
    textLoc.x = offset;
    SetTextOffset (dimElem, 0, textLoc);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_SetHeight (EditElementHandleR eeh, double height)
    {
    if(SUCCESS == mdlDim_setHeightDirect (eeh, height, 0))
        return ValidateElementRange (eeh);

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_GetHeight (ElementHandleCR eh, double& heightOut) const
    {
    return (SUCCESS == mdlDim_getHeightDirect (&heightOut, eh, 0)) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_GetWitnessVisibility (ElementHandleCR eh, int pointNo, bool& value) const
    {
    DimensionElm const* dim = &eh.GetElementCP()->ToDimensionElm();
    pointNo = (pointNo >= 0) ? pointNo : dim->nPoints - 1;
    if (pointNo >= dim->nPoints)
        return ERROR;

    value = !dim->GetDimTextCP(pointNo)->flags.b.noWitness;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    abeesh.basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_SetWitnessVisibility (EditElementHandleR eeh, int pointNo, bool value)
    {
    pointNo = (pointNo >= 0) ? pointNo : eeh.GetElementP()->ToDimensionElm().nPoints - 1;
    if (pointNo >= eeh.GetElementP()->ToDimensionElm().nPoints)
        return ERROR;

    eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.noWitness = !value;
    eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.witCtrlLocal = true;

    ValidateElementRange (eeh);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_SetJustification (EditElementHandleR eeh, int segmentNo, DimStyleProp_Text_Justification value)
    {
    int pointNo;
    if (SUCCESS != BentleyApi::mdlDim_getTextPointNo (&pointNo, eeh, segmentNo))
        return ERROR;

    if (pointNo >= eeh.GetElementP()->ToDimensionElm().nPoints)
        return ERROR;

    if (DIMSTYLE_VALUE_Text_Justification_CenterRight == value)
        {
        eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.just = DIMSTYLE_VALUE_Text_Justification_CenterLeft;
        eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.pushTextRight = true;
        }
    else
        {
        eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.just = value;
        eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.pushTextRight = false;
        }

    ValidateElementRange (eeh);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_GetJustification (ElementHandleCR dimElement, int segmentNo, DimStyleProp_Text_Justification& textJust) const
    {
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    int pointNo;
    if (SUCCESS != BentleyApi::mdlDim_getTextPointNo (&pointNo, dimElement, segmentNo))
        return ERROR;

    if (pointNo >= dim->nPoints)
        return ERROR;

    textJust = (DimStyleProp_Text_Justification)dim->GetDimTextCP(pointNo)->flags.b.just;
    if (DIMSTYLE_VALUE_Text_Justification_CenterLeft == textJust && dim->GetDimTextCP(pointNo)->flags.b.pushTextRight)
        textJust = DIMSTYLE_VALUE_Text_Justification_CenterRight;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionType   DimensionHandler::_GetDimensionType (ElementHandleCR dimElement) const
    {
    return (DimensionType)dimElement.GetElementCP()->ToDimensionElm().dimcmd;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_GetStackHeight (ElementHandleCR dimElement, int segNo, double& height) const
    {
    if (segNo == 0)
        {
        height = 0.0;
        return SUCCESS;
        }

    NullOutput  output;
    NullContext context (&output);

    context.SetDgnProject (*dimElement.GetDgnProject());

    AdimProcess ap(dimElement, &context);

    ap.proxyOverride = DIMPROXY_OVERRIDE_NoProxy;
    ap.stack.segNo   = (segNo == LAST_POINT) ? dimElement.GetElementCP()->ToDimensionElm().nPoints - 1 : segNo;

    if (SUCCESS != adim_strokeDimension (ap))
        return ERROR;

    height = ap.stack.height;
    return SUCCESS;
    }

#ifdef DGNV10FORMAT_CHANGES_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Abeesh.Basheer  10/07
+---------------+---------------+---------------+---------------+---------------+------*/
struct          DimensionVersionChangeListener : DgnFileEvents
{
private:

    static          DimensionVersionChangeListener* s_listener;
    static  const   int                             Beijing     = 103;
    static  const   int                             V8          = 8;
    void            UpgradeDgnModel(DgnModelR model);
    bool            UpgradeElement (EditElementHandleR element);
    void            RectifyTextParamFlags (EditElementHandleR dimension);
    void            UpdateDimVersion(EditElementHandleR dimElement);
    bool            UpdateDimViewBlock (EditElementHandleR dimElement);
    void            InsertDefaultViewBlock (EditElementHandleR dimension);
    bool            HasUpgradableChildren (ElementHandleCR dimension);
    static StatusInt RectifyTextParamFlagsInFormatter (DimFormattedText ** ppFmt,  void *pData, DgnModelP modelRef, DimMLText *pText, int currentRow);

public:

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Abeesh.Basheer
    +---------------+---------------+---------------+---------------+---------------+------*/
    static DimensionVersionChangeListener* GetInstance ()
        {
        if (NULL == s_listener)
            s_listener = new DimensionVersionChangeListener();
        return s_listener;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Abeesh.Basheer
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Register ()
        {
        DgnFile::AddListener (this);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Abeesh.Basheer  11/07
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _OnVersionChange (DgnModelR model, UInt32 oldMajorVer, UInt32 oldMinorVer) override
        {
        if ((oldMajorVer > V8) || ((V8 == oldMajorVer) && (oldMinorVer > Beijing)))
            return;

        UpgradeDgnModel (model);
        }

}; // DimensionVersionChangeListener

DimensionVersionChangeListener* DimensionVersionChangeListener::s_listener = NULL;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionVersionChangeListener::RectifyTextParamFlagsInFormatter
(
DimFormattedText ** ppFmt,
void *              pData,
DgnModelP        modelRef,
DimMLText *         pText,
int                 currentRow
)
    {
    if (NULL == ppFmt || NULL == *ppFmt)
        { BeAssert (0); return DGNHANDLERS_STATUS_BadArg; }

    TextParamWide   wideParams = (*ppFmt)->GetTextParamWide();

    int     currentSection  = wideParams.exFlags.stackedFractionSection;
    int *   previousSection = (int *) pData;
    bool    clearFlags      = false;

    switch (*previousSection)
        {
        case StackedFractionSection::None:
        case StackedFractionSection::Denominator:
            {
            switch (currentSection)
                {
                case StackedFractionSection::Denominator:
                    clearFlags = true;
                }
            break;
            }

        case StackedFractionSection::Numerator:
            {
            switch (currentSection)
                {
                case StackedFractionSection::Numerator:
                case StackedFractionSection::None:
                    clearFlags = true;
                }
            break;
            }
        }

    bool    modified = false;

    if (clearFlags)
        {
        wideParams.exFlags.stackedFractionSection = static_cast<UInt32>(StackedFractionSection::None);
        wideParams.exFlags.stackedFractionAlign   = StackedFractionAlignment::Bottom;
        wideParams.exFlags.stackedFractionType    = static_cast<UInt32>(StackedFractionType::None);
        modified = true;
        }

    if (wideParams.exFlags.isField)
        {
        wideParams.exFlags.isField = false;
        modified = true;
        }

    if (modified)
        (*ppFmt)->SetTextParamWide(wideParams);

    *previousSection = wideParams.exFlags.stackedFractionSection;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionVersionChangeListener::RectifyTextParamFlags (EditElementHandleR dimension)
    {
    StackedFractionSection  previousSection = StackedFractionSection::None;
    DimMLText *             pMLText         = NULL;

    int nSegments = DimensionHandler::GetInstance().GetNumSegments (dimension);

    for (int iSeg = 0; iSeg < nSegments; iSeg++)
        {
        mdlDimText_create (&pMLText);

        if (SUCCESS == mdlDim_getText (pMLText, dimension, iSeg))
            {
            previousSection = StackedFractionSection::None;
            mdlDimText_traverseFormatters (pMLText, RectifyTextParamFlagsInFormatter, &previousSection, NULL);
            mdlDim_setText (dimension, pMLText, iSeg);
            }

        mdlDimText_free (&pMLText);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionVersionChangeListener::InsertDefaultViewBlock (EditElementHandleR dimension)
    {
    DgnProjectP file = NULL;
    if (NULL == (file = dimension.GetDgnProject()))
        return;

    ViewGroupCollectionR viewGroups = file->GetViewGroupsR();
    ViewGroupPtr    activeViewGroup = viewGroups.FindByElementId (file->GetActiveViewGroupId());

    // the viewgroup stored in the tcb no longer exists. Use the most recently modified one instead.
    if (!activeViewGroup.IsValid())
        activeViewGroup = viewGroups.FindLastModifiedMatchingModel (INVALID_ELEMENTID, dimension.GetDgnModelP()->GetModelId(), false, -1);

    if (!activeViewGroup.IsValid())
        return;

    RotMatrix viewRot;
    if (dimension.GetElementCP()->ToDimensionElm().view >= 0 && dimension.GetElementCP()->ToDimensionElm().view < MAX_VIEWS)
        {
        ViewInfoCR viewInfo= activeViewGroup->GetViewInfo (dimension.GetElementCP()->ToDimensionElm().view);
        viewRot = viewInfo.GetRotation();
        }
    else
        {
        DimensionHandler::GetInstance().GetRotationMatrix (dimension, viewRot);
        }

    adim_insertViewRotBlock (dimension, viewRot);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionVersionChangeListener::UpdateDimViewBlock (EditElementHandleR dimElement)
    {
    bool modified = false;
    // some corrupt files have dimElement.GetElementP()->ToDimensionElm().scale == NAN. This will find that.
    if (!BeNumerical::BeFinite (dimElement.GetElementCP()->ToDimensionElm().GetScale()) || BeNumerical::BeIsnan (dimElement.GetElementCP()->ToDimensionElm().GetScale()))
        {
        dimElement.GetElementP()->ToDimensionElm().SetScale(1.0);
        modified = true;
        }

    if (NULL == mdlDim_getOptionBlock (dimElement, ADBLK_VIEWROT, NULL))
        {
        InsertDefaultViewBlock (dimElement);
        modified = true;
        }
    return modified;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionVersionChangeListener::UpdateDimVersion (EditElementHandleR dimElement)
   {
    RotMatrix  rMatrix;

    /* In V8.1, vertical text on linear dimensions can be placed anywhere
     * along the dim line as opposed to the previous versions where they
     * were always on top of the end extension line. Set up the pre-8.1
     * dim's justifications to end so we don't change their position.
     */
    //TODO:Test this

    if (dimElement.GetElementCP()->ToDimensionElm().version < 9)
        {
        //TODO: dim->tmpl.nofit_vertical && textSize.x + insideMinLeader > line_len)
        bool verticalText = dimElement.GetElementP()->ToDimensionElm().tmpl.vertical_text;
        if (verticalText)
            dimElement.GetElementP()->ToDimensionElm().GetDimTextP(1)->flags.b.just = DIMTEXT_END;
        }

    if (dimElement.GetElementCP()->ToDimensionElm().version == 1)
        {
        /*---------------------------------------------------------------
        Early 4.0 beta had only 16 templates
        ---------------------------------------------------------------*/
        switch (dimElement.GetElementCP()->ToDimensionElm().dimcmd)
            {
            case 22:  dimElement.GetElementP()->ToDimensionElm().dimcmd = DimensionType::RadiusExtended;     break;
            case 23:  dimElement.GetElementP()->ToDimensionElm().dimcmd = DimensionType::DiameterExtended;   break;
            case 17:  dimElement.GetElementP()->ToDimensionElm().dimcmd = DimensionType::Center;              break;
            case 18:  dimElement.GetElementP()->ToDimensionElm().dimcmd = DimensionType::AngleAxisX;        break;
            case 19:  dimElement.GetElementP()->ToDimensionElm().dimcmd = DimensionType::AngleAxisY;        break;
            case 20:  dimElement.GetElementP()->ToDimensionElm().dimcmd = DimensionType::LabelLine;          break;
            case 21:  dimElement.GetElementP()->ToDimensionElm().dimcmd = DimensionType::Note;                break;
            }
        dimElement.GetElementP()->ToDimensionElm().version = 2;
        }

    if (dimElement.GetElementCP()->ToDimensionElm().version < 3)
        {
        /*---------------------------------------------------------------
        4.0 beta - inverted rotation matrix to match arc/ellipse form
        - linear dimensions only -
        ---------------------------------------------------------------*/
        switch (dimElement.GetElementCP()->ToDimensionElm().dimcmd)
            {
            case DimensionType::SizeArrow:
            case DimensionType::SizeStroke:
            case DimensionType::LocateSingle:
            case DimensionType::LocateStacked:
            case DimensionType::CustomLinear:
            case DimensionType::Ordinate:
                DimensionHandler::GetInstance().GetRotationMatrix (dimElement, rMatrix);
                rMatrix.InverseOf(rMatrix);
                BentleyApi::mdlDim_setDimRotMatrix (dimElement, &rMatrix);
                break;
            }
        dimElement.GetElementP()->ToDimensionElm().version = 3;
        }

    /*-------------------------------------------------------------------
    4.0 beta - angular dimensions and label line were created with no
    rotation matrix.
    -------------------------------------------------------------------*/
    if (dimElement.GetElementCP()->ToDimensionElm().version < 4 &&
        dimElement.GetElementCP()->ToDimensionElm().quat[0] == 0L && dimElement.GetElementCP()->ToDimensionElm().quat[1] == 0L &&
        dimElement.GetElementCP()->ToDimensionElm().quat[2] == 0L && dimElement.GetElementCP()->ToDimensionElm().quat[3] == 0L)
        {
        rMatrix.InitIdentity ();
        BentleyApi::mdlDim_setDimRotMatrix (dimElement, &rMatrix);
        dimElement.GetElementP()->ToDimensionElm().version = 4;
        }

    if (dimElement.GetElementCP()->ToDimensionElm().version < 6)
        {
        /*---------------------------------------------------------------
        Starting with version 5.5 (dimElement.GetElementP()->ToDimensionElm().version=6) leading and trailing
        zeros are controlled separately for primary and secondary
        dimensions.
        ---------------------------------------------------------------*/
        dimElement.GetElementP()->ToDimensionElm().flag.leadingZero2   = dimElement.GetElementP()->ToDimensionElm().flag.leadingZero;
        dimElement.GetElementP()->ToDimensionElm().flag.trailingZeros2 = dimElement.GetElementP()->ToDimensionElm().flag.trailingZeros2;
        dimElement.GetElementP()->ToDimensionElm().version = 6;
        }

    /*-------------------------------------------------------------------
    In MSJ the dimension version is incremented to 7 which means:
    *   Angular dimensions can have tolerances. Previous versions actually
        put tolerance blocks on angular dimensions but they were not
        displayed. MSJ will not display tolerances for dimensions older
        than V7.
    --------------------------------------------------------------------*/

    if (dimElement.GetElementCP()->ToDimensionElm().version < 8)
        {
        /*---------------------------------------------------------------
        Version 8 introduced ball and chain into dimensions.
        ---------------------------------------------------------------*/
        int i;

        for (i = 0; i < dimElement.GetElementCP()->ToDimensionElm().nPoints; i++)
            {
            dimElement.GetElementP()->ToDimensionElm().GetDimTextP(i)->offsetY = 0;
            }

        dimElement.GetElementP()->ToDimensionElm().version = 8;
        }

    if (dimElement.GetElementCP()->ToDimensionElm().version < 9)
        {
        RectifyTextParamFlags (dimElement);

        dimElement.GetElementP()->ToDimensionElm().version = 9;
        }

    if (dimElement.GetElementCP()->ToDimensionElm().version < 10)
        {
        /*----------------------------------------------------------------------
           Previous to version 8.11, for diagonal stacked fractions we ignored
           the alignment setting and always used center alignment.
        ----------------------------------------------------------------------*/
        if (DIMSTYLE_VALUE_Text_StackedFractionType_Diagonal == dimElement.GetElementP()->ToDimensionElm().text.b.stackedFractionType)
            dimElement.GetElementP()->ToDimensionElm().text.b.stackedFractionAlign = DIMSTYLE_VALUE_Text_StackedFractionAlignment_Center;

        dimElement.GetElementP()->ToDimensionElm().version = 10;
        }

    if (dimElement.GetElementCP()->ToDimensionElm().version < 11)
        {
        DirectionFormatterPtr  formatter = DirectionFormatter::Create();
        formatter->SetDirectionModeFromLegacy (dimElement.GetElementP()->ToDimensionElm().frmt.angleMode_deprecated);

        DimStyleExtensions     styleExt;

        memset (&styleExt, 0, sizeof (styleExt));
        mdlDim_getStyleExtension (&styleExt, dimElement);

        adim_updateExtensionsFromDirectionFormat (styleExt, *formatter);

        mdlDim_setStyleExtension (dimElement, &styleExt);

        dimElement.GetElementP()->ToDimensionElm().version = 11;
        }
    UpdateDimViewBlock (dimElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool             DimensionVersionChangeListener::HasUpgradableChildren (ElementHandleCR element)
    {
    if (DIMENSION_ELM == element.GetLegacyType())
        return element.GetElementCP()->ToDimensionElm().version < DIM_VERSION;

    for (ChildElemIter child (element, ExposeChildrenReason::Count); child.IsValid(); child=child.ToNext())//Upgrade all children
        {
        if  (DIMENSION_ELM == child.GetLegacyType())
            return child.GetElementCP()->ToDimensionElm().version < DIM_VERSION;

        if (HasUpgradableChildren (child))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionVersionChangeListener::UpgradeElement (EditElementHandleR element)
    {
    if (DIMENSION_ELM != element.GetLegacyType()) //On upgrade we do not have any handler based dimension yet.
        {
        bool hasUpdatedChild = false;
        for (ChildEditElemIter child (element, ExposeChildrenReason::Count); child.IsValid(); child=child.ToNext())//Upgrade all children
             {
             if (UpgradeElement (child))
                 hasUpdatedChild = true;
             }

        return hasUpdatedChild;
        }

    DimensionElm const* cdim = &element.GetElementCP()->dim;
    /* Update the version to latest */
    if (cdim->version >= DIM_VERSION)
        return UpdateDimViewBlock (element);

    UpdateDimVersion (element);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionVersionChangeListener::UpgradeDgnModel (DgnModelR model)
    {
    DgnModel::ElementsCollection collection = model.GetElementsCollection ();
    for (PersistentElementRefP const& elemRef : collection)
       {
       EditElementHandle element(elemRef, &model);
       if (!HasUpgradableChildren (element))//Try to delay element descr allocation as long a possible.
           continue;

       element.GetElementDescrP();
       ElementRefP oldElement = element.GetElementRef ();
       if (UpgradeElement (element))
           element.ReplaceInModel(oldElement);
       }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            dimension_staticInitialize()
    {
    BeDebugLog ("dimension_staticInitialize");
    DimensionVersionChangeListener::GetInstance()->Register();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_SetTextOffset (EditElementHandleR eeh, int segmentNo, DPoint2dCR offset)
    {
    //We don't have a justification control local.
    int pointNo;
    if (SUCCESS != mdlDim_getTextPointNo (&pointNo, eeh, segmentNo))
        return ERROR;

    if (pointNo >= eeh.GetElementP()->ToDimensionElm().nPoints)
        return ERROR;

    eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->offset = offset.x;
    eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->offsetY = offset.y;
    ValidateElementRange (eeh);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_GetTextOffset (ElementHandleCR eh, int segmentNo, DPoint2dR offset) const
    {
    DimensionElm const* dim = &eh.GetElementCP()->ToDimensionElm();
    int pointNo;
    if (SUCCESS != mdlDim_getTextPointNo (&pointNo, eh, segmentNo))
        return ERROR;

    if (pointNo >= dim->nPoints)
        return ERROR;

    offset.x = dim->GetDimTextCP(pointNo)->offset;
    offset.y = dim->GetDimTextCP(pointNo)->offsetY;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::_SetAngularDimensionClockWiseSweep (EditElementHandleR eeh, bool value)
    {
    eeh.GetElementP()->ToDimensionElmR().extFlag.uAngClockwiseSweep = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionHandler::_GetAngularDimensionClockWiseSweep (ElementHandleCR dimElement) const
    {
    return dimElement.GetElementCP()->ToDimensionElm().extFlag.uAngClockwiseSweep;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionHandler::SetRadialDimensionCrossCenterFlag (EditElementHandleR eeh, bool value)
    {
    eeh.GetElementP()->ToDimensionElmR().flag.crossCenter = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_IsPointAssociative (ElementHandleCR eh, int pointNo, bool& flag) const
    {
    DimensionElm const* dim = &eh.GetElementCP()->ToDimensionElm();
    if (pointNo >= dim->nPoints)
        return ERROR;
    flag = dim->GetDimTextCP (pointNo)->flags.b.associative;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AngularDimensionHelper::InsertVertex (EditElementHandleR dim, DPoint3dCR point, HitPathCR hitPath, DimensionStyleCR dimStyle)
    {
    int     pointNo;
    if (SUCCESS != hitPath.GetDimensionParameters (NULL,  &pointNo, NULL, NULL, NULL))
        return ERROR;

    return m_hdlr.InsertPoint (dim, &point, NULL, dimStyle, pointNo +1);
    }

/*---------------------------------------------------------------------------------**//**
 name   calculateInsertionIndex
 author    JimBartlett 05/99|
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LinearDimensionHelper::InsertVertex (EditElementHandleR dim, DPoint3dCR point, HitPathCR hitPath, DimensionStyleCR dimStyle)
    {
    const int numPoints = m_hdlr.GetNumPoints (dim);
    if (numPoints < 1)
        return ERROR;

    /*-------------------------------------------------------------------
    Search the dimension point array and find the appropriate insertion
    index for the input point based on it's distance from the origin
    (vertex 0) projected along the dimension X axis
    -------------------------------------------------------------------*/
    RotMatrix rMatrix;
    if (SUCCESS != m_hdlr.GetRotationMatrix (dim, rMatrix))
        return ERROR;

    DVec3d xDir;
    rMatrix.GetColumn (xDir, 0);

    DPoint3d origin(dim.GetElementCP()->ToDimensionElm().GetPoint(0));
    DPoint3d test0 (point);
    test0.Subtract(origin);
    double inDist = xDir.DotProduct(test0);

    int insertionPoint = 0;
    for (; insertionPoint < numPoints +1; ++insertionPoint)//Not the +1
        {
        if (0 == insertionPoint && inDist < 0.0)
            break;

        if (insertionPoint == numPoints)
            break;

        DPoint3d test(dim.GetElementCP()->ToDimensionElm().GetPoint(insertionPoint));
        test.Subtract(origin);
        if (xDir.DotProduct(test) > inDist)
            break;
        }

    if (SUCCESS != m_hdlr.InsertPoint (dim, &point, NULL, dimStyle, insertionPoint))
        return ERROR;

    /*-------------------------------------------------------
        If the index is zero adjust the dimension height to
        keep the dimension line in the same location (height
        is stored in text.offset with point zero). Point 1
        represents the end of the new segment. Set it's text
        offset to zero and leave justification at default.
        ------------------------------------------------------*/
    if (0 == insertionPoint)
        {
        double deltaY = 0;
        if (inDist < 0.0)
            {
            /*---------------------------------------------------------------
            If the distance is negative then the index is zero. Calculate the
            Y offset so the caller can adhust the height withc is stored
            with the origin point.
            ---------------------------------------------------------------*/
            DVec3d      yDir;
            rMatrix.GetColumn (yDir, 1);
            deltaY = yDir.DotProduct (test0);
            }
        dim.GetElementP()->ToDimensionElmR().GetDimTextP(0)->offset = (dim.GetElementP()->ToDimensionElmR().GetDimTextP(1)->offset - deltaY);
        dim.GetElementP()->ToDimensionElmR().GetDimTextP(1)->offset = 0;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
name    insertOrdinateDimPoint
author    JVB 11/90
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   OrdinateDimensionHelper::InsertVertex (EditElementHandleR dim, DPoint3dCR point, HitPathCR hitPath, DimensionStyleCR dimStyle)
    {
    const int numPoints = m_hdlr.GetNumPoints (dim);
    if (numPoints < 1)
        return ERROR;

    UInt32     segmentNo;
    if (SUCCESS != hitPath.GetDimensionParameters (NULL,  NULL, &segmentNo, NULL, NULL))
        return ERROR;

    RotMatrix rMatrix;
    if (SUCCESS != m_hdlr.GetRotationMatrix (dim, rMatrix))
        return ERROR;

    DVec3d xDir;
    rMatrix.GetColumn (xDir, 0);
    DPoint3d origin(dim.GetElementCP()->ToDimensionElm().GetPoint(0));
    origin.Subtract(point);

    double offset = xDir.DotProduct(origin);
    if (SUCCESS != m_hdlr.InsertPoint (dim, &point, NULL, dimStyle, segmentNo))
        return ERROR;

    dim.GetElementP()->ToDimensionElmR().GetDimTextP(segmentNo)->offset = (dim.GetElementP()->ToDimensionElmR().GetDimTextP(0)->offset - offset);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
based on getRadialInsertionPoint     JoshSchifter        02/02
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus      getRadialOrNoteInsertionPoint (int& insertionPoint, HitPathCR hitPath, bool isNote)
    {
    DimensionPartType      partType;
    DimensionPartSubType   subType;
    UInt32                 segmentNumber;

    if (SUCCESS != hitPath.GetDimensionParameters (NULL, NULL, &segmentNumber, &partType, &subType))
        return ERROR;

    if (mdlDim_partTypeIsAnyDimLine(partType) && ADSUB_LEADER != subType)
        {
        int linSegNo;
        if (SUCCESS != hitPath.GetLinearParameters (NULL, NULL, &linSegNo))
            return ERROR;
        segmentNumber = linSegNo;
        }
    else if (mdlDim_partTypeIsAnyText (partType) || ADSUB_LEADER == subType)
        return SUCCESS;//the last point
    else if (mdlDim_partTypeIsAnyTerminator(partType))
        segmentNumber = -1;

    if (isNote)
        {
        insertionPoint = segmentNumber +1;
        return SUCCESS;
        }

    insertionPoint = segmentNumber +2;
    if (0 == insertionPoint || 1 == insertionPoint)
        insertionPoint = 2;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RadialDimensionHelper::InsertVertex (EditElementHandleR dim, DPoint3dCR point, HitPathCR hitPath, DimensionStyleCR dimStyle)
    {
    const int numPoints = m_hdlr.GetNumPoints (dim);
    if (numPoints < 1)
        return ERROR;

    int insertionPoint = numPoints;
    if (SUCCESS != getRadialOrNoteInsertionPoint (insertionPoint, hitPath, false))
        return ERROR;

    return m_hdlr.InsertPoint (dim, &point, NULL, dimStyle, insertionPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteDimensionHelper::InsertVertex (EditElementHandleR dim, DPoint3dCR point, HitPathCR hitPath, DimensionStyleCR dimStyle)
    {
    const int numPoints = m_hdlr.GetNumPoints (dim);
    if (numPoints < 1)
        return ERROR;

    int insertionPoint = numPoints;
    if (SUCCESS != getRadialOrNoteInsertionPoint (insertionPoint, hitPath, true))
        return ERROR;

    if (SUCCESS != m_hdlr.InsertPoint (dim, &point, NULL, dimStyle, insertionPoint))
        return ERROR;

    /*------------------------------------------------------------------
    Kludge: For notes, the text is stored on the last point, so when
    adding a new point on the end, need to move the text.
    Radial dimensions store the text on the first point. Much smarter.
    ------------------------------------------------------------------*/
    if (numPoints == insertionPoint)
        {
        int newNumPoints = numPoints +1;
        *dim.GetElementP()->ToDimensionElmR().GetDimTextP(newNumPoints -1) = *dim.GetElementP()->ToDimensionElmR().GetDimTextP(newNumPoints -2);
        memset (dim.GetElementP()->ToDimensionElmR().GetDimTextP(newNumPoints -2), 0, sizeof (*dim.GetElementP()->ToDimensionElmR().GetDimTextP(newNumPoints -2)));
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_InsertVertex (EditElementHandleR dim, HitPathCR hitPath, DPoint3dCR point)
    {
    DimensionStylePtr dimStyle = GetDimensionStyle(dim);
    if (dimStyle.IsNull())
        return ERROR;

    IDimElementHelperPtr helper = DimensionHelperFactory::CreateHelper (dim);
    if (SUCCESS != helper->InsertVertex (dim, point, hitPath, *dimStyle))
        return ERROR;

    return DisplayHandler::ValidateElementRange (dim);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_DeleteVertex (EditElementHandleR dim, HitPathCR hitPath)
    {
    int     pointNo;
    if (SUCCESS != hitPath.GetDimensionParameters (NULL,  &pointNo, NULL, NULL, NULL))
        return ERROR;

    IDimElementHelperPtr helper = DimensionHelperFactory::CreateHelper (dim);
    if (SUCCESS != helper->DeleteVertex (dim, pointNo))
        return ERROR;

    return DisplayHandler::ValidateElementRange (dim);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionHandler::_IsVertexInsertable (ElementHandleCR dim) const
    {
    if (DimensionType::LabelLine == GetDimensionType (dim))
        return false;

    return (GetNumPoints(dim) < MAX_ADIM_POINTS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RadialDimensionHelper::IsVertexDeletable (HitPathCP hitPath) const
    {
    int     pointNo;
    if (SUCCESS != hitPath->GetDimensionParameters (NULL,  &pointNo, NULL, NULL, NULL))
        return false;
    return (m_hdlr.GetNumPoints(m_dimension) > 3 && pointNo > 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionHandler::_IsVertexDeletable (ElementHandleCR dim, HitPathCR hitPath) const
    {
    IDimElementHelperPtr helper = DimensionHelperFactory::CreateHelper (dim);
    return helper->IsVertexDeletable (&hitPath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::DropAssociation (EditElementHandleR dimElement, int pointNo)
    {
    int numPoints = GetNumPoints (dimElement);
    if (pointNo < 0 || pointNo >= numPoints)
        return ERROR;

    if (!dimElement.GetElementCP()->ToDimensionElm().GetDimTextCP(pointNo)->flags.b.associative)
        return SUCCESS;//Nothing needs to be done

    if (GetDimensionType(dimElement) == DimensionType::LabelLine)
        return (BentleyStatus)AssociativePoint::RemoveAllAssociations (dimElement);

    BentleyStatus status = SUCCESS;
    if (SUCCESS == (status = (BentleyStatus)AssociativePoint::RemovePoint(dimElement, pointNo, numPoints)))
        dimElement.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.associative = false;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   IDimElementHelper::DeleteVertex (EditElementHandleR dimElement, int pointNo)
    {
    BentleyStatus status = SUCCESS;
    if (SUCCESS != (status = m_hdlr.DropAssociation (dimElement, pointNo)))
        return status;

    return m_hdlr.DeletePoint (dimElement, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AngularDimensionHelper::DeleteVertex (EditElementHandleR dimElement, int pointNo)
    {
    if (!IsVertexDeletable (NULL))
        return ERROR;

    if (1 == pointNo)
        {
        DimensionElmP dim = &dimElement.GetElementP()->ToDimensionElmR();
        double offset = dim->GetPoint(0).Distance(dim->GetPoint(2)) - dim->GetPoint(0).Distance(dim->GetPoint(1));
        dim->GetDimTextP(2)->offset = dim->GetDimTextCP(1)->offset - offset;
        }

    return T_Super::DeleteVertex (dimElement, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LinearDimensionHelper::DeleteVertex (EditElementHandleR dimElement, int pointNo)
    {
    if (!IsVertexDeletable (NULL))
        return ERROR;

    if (pointNo == 0)
        {
        RotMatrix rMatrix;
        if (SUCCESS != m_hdlr.GetRotationMatrix (dimElement, rMatrix))
            return ERROR;

        DVec3d yVec;
        rMatrix.GetColumn (yVec, 1);

        DimensionElmP dim = &dimElement.GetElementP()->ToDimensionElmR();

        DPoint3d distanceVec = dim->GetPoint (1);
        distanceVec.Subtract(dim->GetPoint (0));
        double offset = yVec.DotProduct (distanceVec);

        dim->GetDimTextP(1)->offset = dim->GetDimTextP(0)->offset - offset;
        }

    return T_Super::DeleteVertex (dimElement, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteDimensionHelper::DeleteVertex (EditElementHandleR dimElement, int pointNo)
    {
    if (!IsVertexDeletable (NULL))
        return ERROR;

     if (m_hdlr.GetNumPoints(dimElement)-1 == pointNo)
        {
        DimensionElmP dim = &dimElement.GetElementP()->ToDimensionElmR();
        *dim->GetDimTextP(pointNo-1) = *dim->GetDimTextP(pointNo);
        dim->GetDimTextP(pointNo)->address = -1;
        }

    return T_Super::DeleteVertex (dimElement, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ITextPartIdPtr  DimensionTextPartId::Create (UInt32 partSegment, DimensionPartType partType, DimensionPartSubType partSubType)
    {
    DimensionTextPartType       textType;
    DimensionTextPartSubType    textSubType;

    if (SUCCESS != toTextPartId (textType, partType))
        return NULL;

    if (SUCCESS != toTextSubPartId (textSubType, partSubType))
        return NULL;

    return new DimensionTextPartId(partSegment, textType, textSubType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
ITextPartIdPtr  DimensionTextPartId::Create (UInt32 partSegment, DimensionTextPartType partType, DimensionTextPartSubType partSubType)
    {
    return new DimensionTextPartId(partSegment, partType, partSubType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionTextPartId::DimensionTextPartId (UInt32 partSegment, DimensionTextPartType partType, DimensionTextPartSubType partSubType):
    m_partSegment   (partSegment),
    m_partType      (partType),
    m_partSubType   (partSubType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::GetTextPointNo(int& pointNo, ElementHandleCR dimElement, int segmentNo)
    {
    //There is a discrepancy with this function and the other one using hitpath. but we were not able to see any callers
    //who was actually using the point Number obtained from a hitpath. Also this function is used in stroke code which
    //confirms this as the version to use.
    pointNo = 0;
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    switch (static_cast<DimensionType>(dim->dimcmd))
        {
        case DimensionType::SizeArrow:        case DimensionType::SizeStroke:
        case DimensionType::LocateSingle:     case DimensionType::LocateStacked:
        case DimensionType::CustomLinear:
            pointNo = segmentNo + 1;
            break;

        case DimensionType::AngleSize:        case DimensionType::ArcSize:
        case DimensionType::AngleLocation:    case DimensionType::ArcLocation:
        case DimensionType::AngleLines:       case DimensionType::AngleAxisX:
        case DimensionType::AngleAxisY:
            pointNo = segmentNo + 2;
            break;

        case DimensionType::Radius:            case DimensionType::Diameter:
        case DimensionType::RadiusExtended:   case DimensionType::DiameterExtended:
        case DimensionType::Center:
            pointNo = 0;
            break;

        case DimensionType::Note:
            pointNo = dim->nPoints -1;
            break;

        case DimensionType::Ordinate:          case DimensionType::LabelLine:
            pointNo = segmentNo;
            break;

        case DimensionType::DiameterParallel:
        case DimensionType::DiameterPerpendicular:
            pointNo = segmentNo;
            break;
        }

    return ((pointNo >= 0 && pointNo < dim->nPoints) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_GetWitnessUseAltSymbology (ElementHandleCR eh, int pointNo, bool& status) const
    {
    DimensionElm const* dim = &eh.GetElementCP()->ToDimensionElm();
    pointNo = (pointNo >= 0) ? pointNo : dim->nPoints - 1;
    if (pointNo >= dim->nPoints)
        return ERROR;

    status =  dim->GetDimTextCP(pointNo)->flags.b.altSymb;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_SetWitnessUseAltSymbology (EditElementHandleR eeh, int pointNo, bool status)
    {
    pointNo = (pointNo >= 0) ? pointNo : eeh.GetElementP()->ToDimensionElm().nPoints - 1;
    if (pointNo >= eeh.GetElementP()->ToDimensionElm().nPoints)
        return ERROR;

    eeh.GetElementP()->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.altSymb = status;
    ValidateElementRange (eeh);
    return SUCCESS;
    }

