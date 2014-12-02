/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Dimension/DimGenerate.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define     SMALL_ELMBUF_SIZE       500
#define     SMALL_ELMBUF_PADDING    50


/*------------------------------------------------------------------------------
    adim_strokeDimension modifies the input element which is stupid, but
    difficult to fix currently.  One way that the element is changed is by the
    function adim_updateDimVersion which can actually change the size of the
    input element.

    Thus, all callers to adim_strokeDimension are required to pass the element
    inside a buffer that is large enough to include whatever growth that might
    occur.  Putting a full size buffer (128K) on the stack would be simple,
    but it is best to avoid such a huge stack variable.  The compromise is
    to use _alloca to create a stack buffer with sufficient padding.

    As of 2/7/2008, the only expansion is the directionFormatting settings
    which require approximately 25 bytes.  To be safe, use a 100 byte expansion.
------------------------------------------------------------------------------*/
#define     DIMELEM_BUFFER_PADDING  100

//Add this in to draw small circles at each dimension point.
//#define     DEBUG_DRAWPOINTS

USING_NAMESPACE_BENTLEY_DGNPLATFORM

static   DimProxyOption     s_dimensionProxyOption =  DIMPROXY_OPTION_Invalid;

#ifdef WIP_CFGVAR
DEFINE_CFGVAR_CHECKER(MS_DRAWDIMENSIONMASKING)
#endif

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  11/04
+===============+===============+===============+===============+===============+======*/
struct          DimensionTopology : IElemTopology
{
private:
    UInt32      m_partName;

public:
    DimensionTopology () {m_partName = 0;}
    explicit DimensionTopology (DimensionTopology const& from) {m_partName = from.m_partName;}
    virtual DimensionTopology* _Clone () const override {return new DimensionTopology (*this);}
    UInt32 GetPartName () const {return m_partName;}
    void SetPartName (UInt32 name) {m_partName = name;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/04
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::adim_setHitDetail
(
AdimProcess const  *ep     // => Function used to process element
)
    {
    DimensionTopology*  topoP = dynamic_cast <DimensionTopology*> (ep->context->GetElemTopology ());

    if (NULL != topoP)
        topoP->SetPartName (ep->partName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     adjustDimPointNumber
(
int*            pPointNo,      // <=> Closest point in dimension element
ElementHandleCR dimElement,    //  =>
int             partType,      //  =>
int             closeVertex,   //  =>
HitPathCP       path           //  => HitPath to examine
)
    {
    if (!mdlDim_partTypeIsAnyDimLine (partType))
        return;

    DgnElementCP pElm = dimElement.GetElementCP();

    if (mdlDim_isLinearDimension (dimElement) && ! pElm->ToDimensionElm().tmpl.stacked)
        {
        double      param1, param2;
        DSegment3d  hitSeg;
        DPoint3d    dimSeg[2];

        path->GetLinearParameters (&hitSeg, NULL, NULL);
        BentleyApi::mdlDim_extractPointsD (dimSeg, dimElement, *pPointNo - 1, 2);

        bsiGeom_projectPointToLine (NULL, &param1, &dimSeg[0], &hitSeg.point[0], &hitSeg.point[1]);
        bsiGeom_projectPointToLine (NULL, &param2, &dimSeg[1], &hitSeg.point[0], &hitSeg.point[1]);

        if (fabs (param1 - closeVertex) < fabs (param2 - closeVertex))
            (*pPointNo)--;
        }
    else if (mdlDim_isAngularDimension (dimElement) && ! pElm->ToDimensionElm().tmpl.stacked)
        {
        GeomDetailCR    detail = path->GetGeomDetail ();
        DEllipse3d      ellipse;

        if (detail.GetArc (ellipse) && ellipse.IsFullEllipse ())
            {
            DPoint3d    ePts[2], hitPt = detail.GetClosestPointLocal ();

            ellipse.EvaluateEndPoints (ePts[0], ePts[1]);

            // NEEDSWORK: I don't know what the heck the correct test here is...Josh?!?
            if (hitPt.Distance (ePts[0]) > hitPt.Distance (ePts[1]))
                (*pPointNo)--;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Get the parameters from a hitpath to a dimension
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       HitPath::GetDimensionParameters
(
UInt32*               pPartName,     // <= partname
int*                  pPointNo,      // <= Closest point in dimension element
UInt32*               pSegment,      // <= Selected dimension segment
DimensionPartType*    pPartType,     // <= Type of selected part of dim, one of ADTYPE_xxx
DimensionPartSubType* pPartSubType   // <= Sub Type of selected part of dim, one of ADSUB_xxx
) const
    {
    UInt32      partName = 0;

    DimensionTopology const *topoP = dynamic_cast <DimensionTopology const *> (GetElemTopology ());

    if (NULL != topoP)
        partName = topoP->GetPartName ();

    DimensionPartType      partType;
    DimensionPartSubType   partSubType;
    UInt32 segment;

    partType    = ADIM_GETTYPE (partName);
    partSubType = ADIM_GETSUB (partName);
    segment     = ADIM_GETSEG (partName);

    if (pPartName)
        *pPartName = partName;

    if (pPointNo)
        {
        ElementHandle elHandle (GetCursorElem());
        GeomDetail const&   detail = GetGeomDetail ();

        *pPointNo = mdlDim_getPointNumber (elHandle, segment, partType, partSubType, (int) detail.GetCloseVertex ());

        adjustDimPointNumber (pPointNo, elHandle, partType, (int) detail.GetCloseVertex (), this);
        }

    if (pSegment)
        *pSegment = segment;

    if (pPartType)
        *pPartType = partType;

    if (pPartSubType)
        *pPartSubType = partSubType;

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| ** Primitive element generation functions                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
void          BentleyApi::adim_getEffectiveSymbology
(
Symbology              *pSymbologyOut,  /* <= */
ElementHandleCR         dimElement,     /* => */
DimMaterialIndex        material,       /* => */
UInt32                  partName,       /* => */
DimOverrides           *pOverrides      /* => */
)
    {
    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    switch (material)
        {
        case DIM_MATERIAL_Text:
            {
            pSymbologyOut->style  = 0;
            pSymbologyOut->weight = pDim->text.b.useWeight ? pDim->text.weight : pDim->GetSymbology().weight;
            pSymbologyOut->color  = pDim->text.b.useColor ? pDim->text.color : pDim->GetSymbology().color;
            break;
            }

        case DIM_MATERIAL_Extension:
            {
            pSymbologyOut->style  = pDim->altSymb.style;
            pSymbologyOut->weight = pDim->altSymb.weight;
            pSymbologyOut->color  = pDim->altSymb.color;
            break;
            }

        case DIM_MATERIAL_Terminator:
            {
            DimTermSymbBlock    *pTermSymb;

            *pSymbologyOut        = pDim->GetSymbology();

            if ((pTermSymb = (DimTermSymbBlock*) mdlDim_getOptionBlock (dimElement, ADBLK_TERMSYMB, NULL)) != NULL)
                {
                if (pTermSymb->termSymb.useStyle)
                    pSymbologyOut->style  = pTermSymb->termSymb.style;

                if (pTermSymb->termSymb.useWeight)
                    pSymbologyOut->weight = pTermSymb->termSymb.weight;

                if (pTermSymb->termSymb.useColor)
                    pSymbologyOut->color  = pTermSymb->termSymb.color;
                }
            break;
            }

        case DIM_MATERIAL_MLUserText:
            break;

        case DIM_MATERIAL_DimLine:
        default:
            {
            *pSymbologyOut        = pDim->GetSymbology();
            break;
            }
        }

    if (pOverrides)
        {
        bool        gotOverrides = false;
        UInt32      color=0, weight=0;
        long        style=0;

        // apply overriding symbology
        switch (ADIM_GETTYPE (partName))
            {
            case ADTYPE_EXT_LEFT:       // (1st)
            case ADTYPE_EXT_RIGHT:      // (2nd..nth)
                {
                int pointNo = mdlDim_getPointNumber (dimElement, ADIM_GETSEG (partName), ADIM_GETTYPE (partName), ADIM_GETSUB (partName), 0);

                mdlDim_overridesGetPointWitnessColor  (&color,  pOverrides, pointNo, pSymbologyOut->color);
                mdlDim_overridesGetPointWitnessWeight (&weight, pOverrides, pointNo, pSymbologyOut->weight);
                mdlDim_overridesGetPointWitnessStyle  (&style,  pOverrides, pointNo, pSymbologyOut->style);
                gotOverrides = true;

                break;
                }

            default:
                break;
            }

        if (gotOverrides)
            {
            pSymbologyOut->color  = color;
            pSymbologyOut->weight = weight;
            pSymbologyOut->style  = style;
            }
        }

    if (STYLE_BYCELL == pSymbologyOut->style)
        pSymbologyOut->style = pDim->GetSymbology().style;

    if (WEIGHT_BYCELL == pSymbologyOut->weight)
        pSymbologyOut->weight = pDim->GetSymbology().weight;

    if (COLOR_BYCELL == pSymbologyOut->color)
        pSymbologyOut->color = pDim->GetSymbology().color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void    adim_prepareToDrawComponent
(
AdimProcess const*        ep,
DimMaterialIndex    material
)
    {
    // Step 1: setup the hit detail
    adim_setHitDetail (ep);

    // Step 2: setup the symbology
    Symbology       symb = ep->GetDimElementCP()->GetSymbology();

    adim_getEffectiveSymbology (&symb, ep->GetElemHandleCR(), material, ep->partName, ep->pOverrides);

    if (ep->context)
        {
        ElemDisplayParamsP  elParams = ep->context->GetCurrentDisplayParams();

        elParams->SetElementClass ((DgnElementClass) DgnElementClass::Dimension);
        elParams->SetLineColor (symb.color);
        elParams->SetWeight (symb.weight);
        elParams->SetLineStyle (symb.style, elParams->GetLineStyleParams ());

        ep->context->CookDisplayParams ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 02/04
+---------------+---------------+---------------+---------------+---------------+------*/
static int     getDimSegmentNumberFromPoint
(
ElementHandleCR    dimElement,
int             pointNo
)
    {
    // This is a temporary function written only for mdlDim_overridesPointDeleted
    if (mdlDim_isOrdinateDimension (dimElement))
        return pointNo;
    else
        return pointNo - 1;
    }

/*---------------------------------------------------------------------------------**//**
* Remove segment and point linkages involved with deleted point and above (if any).
* Components requiring removal will be marked as deleted.
*
* @param        pOverridesIn    <=> override collection
* @param        pointNo         =>
* @see          mdlDim_overridesGet,
*               mdlDim_overridesSet,
*               mdlDim_overridesPointInserted
*               mdlDim_overridesFreeAll
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void     mdlDim_overridesPointDeleted
(
EditElementHandleR dimElement,
DimOverrides    *pOverridesIn,
int             pointNo
)
    {
    int         index;
    
    if (pOverridesIn->nPoints)
        {
        for (index = 0; index < pOverridesIn->nPoints; index++)
            {
            PointOverride   *pPoint = pOverridesIn->pPoint + index;

            if (pPoint->pointNo == pointNo)
                pPoint->hdr.deleted = true;

            if (pPoint->pointNo > pointNo)
                pPoint->pointNo--;
            }
        }

    if (pOverridesIn->nSegments)
        {
        for (index = 0; index < pOverridesIn->nSegments; index++)
            {
            SegmentOverride     *pSegment = pOverridesIn->pSegment + index;

            if (pSegment->segmentNo == getDimSegmentNumberFromPoint (dimElement, pointNo))
                pSegment->hdr.deleted = true;

            if (pSegment->segmentNo > getDimSegmentNumberFromPoint (dimElement, pointNo))
                pSegment->segmentNo--;
            }
        }

    if (pOverridesIn->nSgmtFlgs)
        {
        for (index = 0; index < pOverridesIn->nSgmtFlgs; index++)
            {
            SegmentFlagOverride     *pSgmtFlag = pOverridesIn->pSgmtFlag + index;

            if (pSgmtFlag->segmentNo == getDimSegmentNumberFromPoint (dimElement, pointNo))
                pSgmtFlag->hdr.deleted = true;

            if (pSgmtFlag->segmentNo > getDimSegmentNumberFromPoint (dimElement, pointNo))
                pSgmtFlag->segmentNo--;
            }
        }

    if (pOverridesIn->nMLTexts)
        {
        for (index = 0; index < pOverridesIn->nMLTexts; index++)
            {
            DimMLTextOverride  *pTxt = pOverridesIn->pMLText + index;

            if (pTxt->segmentNo == getDimSegmentNumberFromPoint (dimElement, pointNo))
                pTxt->hdr.deleted = true;

            if (pTxt->segmentNo > getDimSegmentNumberFromPoint (dimElement, pointNo))
                pTxt->segmentNo--;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DimensionHandler::_DeletePoint(EditElementHandleR eeh, int pointNo)
    {
    DimOverrides    *pDimOverrides = NULL;

    if (pointNo == LAST_POINT)
        pointNo = eeh.GetElementP()->ToDimensionElm().nPoints - 1;
    else if (pointNo >= eeh.GetElementP()->ToDimensionElm().nPoints || pointNo < 0)
        return (ERROR);

    mdlDim_overridesGet (&pDimOverrides, eeh);
    mdlDim_overridesPointDeleted (eeh, pDimOverrides, pointNo);
    mdlDim_overridesSet (eeh, pDimOverrides);
    mdlDim_overridesFreeAll (&pDimOverrides);

    mdlDim_deleteTextCluster (eeh, pointNo);

    if (eeh.GetElementP()->ToDimensionElm().nPoints == 1)
        {
        eeh.GetElementP()->ToDimensionElmR().nPoints = 0;
        return (SUCCESS);
        }

    /* shift assoc point dependency indices to account for delete */
    AssociativePoint::VertexAddedOrRemovedFromMSElement (*eeh.GetElementP(), *eeh.GetDgnModelP (), pointNo, eeh.GetElementP()->ToDimensionElm().nPoints, false);
    eeh.GetElementP()->ToDimensionElmR().DeletePoint (pointNo);
    return ValidateElementRange (eeh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct
    {
    AdimProcess        *pAdimProcess;
    DimMaterialIndex    material;

    } ModifyDescrArg;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/06
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        drawDimensionMasking (AdimProcess* ep)
    {
#ifdef WIP_CFGVAR
    if (!CHECK_CFGVAR_IS_DEFINED(MS_DRAWDIMENSIONMASKING))
        return false;
#endif
    
    if (!ep->context)
        return false;
    
    DrawPurpose purpose = ep->context->GetDrawPurpose();
    switch (purpose)
        {
        case DrawPurpose::Update:
        case DrawPurpose::UpdateDynamic:
        case DrawPurpose::ChangedPre:
        case DrawPurpose::ChangedPost:
        case DrawPurpose::RestoredPre:
        case DrawPurpose::RestoredPost:
        case DrawPurpose::RangeCalculation:
        case DrawPurpose::FitView:
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
int          BentleyApi::adim_generateLine
(
AdimProcess        *ep,            /* => Function used to process element    */
DPoint3d           *start,         /* => Start point of line                 */
DPoint3d           *end,           /* => End point of line                   */
DimMaterialIndex    material       /* => Dimension material index            */
)
    {
    if (NULL ==  ep->context)
        return ERROR;

    if (DrawPurpose::RegionFlood == ep->context->GetDrawPurpose ())
        return SUCCESS;

    adim_prepareToDrawComponent (ep, material);

    DPoint3d    line[2];

    line[0] = *start;
    line[1] = *end;

    ViewContextP context = ep->context;

    if (DisplayHandler::Is3dElem ((DgnElementCP) ep->GetDimElementCP()))
        {
        context->DrawStyledLineString3d (2, line, NULL, false);
        }
    else
        {
        DPoint2d pts2d[2];

        pts2d[0].init (&line[0]);
        pts2d[1].init (&line[1]);

        context->DrawStyledLineString2d (2, pts2d, context->GetDisplayPriority(), NULL, false);
        }

    if (drawDimensionMasking (ep))
        {
        double      gap = 0.5 * (0.5 * ep->GetDimElementCP()->text.height);
        DPoint3d    shape[5];
        DVec3d      xVec, yVec, zVec;

        ep->rMatrix.GetColumn(zVec,  2);

        xVec.NormalizedDifference (line[1], line[0]);
        bsiDVec3d_crossProduct (&yVec, &zVec, &xVec);

        bsiDPoint3d_addScaledDVec3d (&shape[0], &line[0], &yVec, -gap);
        bsiDPoint3d_addScaledDVec3d (&shape[1], &line[0], &yVec,  gap);

        bsiDPoint3d_addScaledDVec3d (&shape[2], &line[1], &yVec,  gap);
        bsiDPoint3d_addScaledDVec3d (&shape[3], &line[1], &yVec, -gap);

        shape[4] = shape[0];

        if (ep->context)
            {
            ElemDisplayParamsP  elParams = ep->context->GetCurrentDisplayParams ();

            elParams->SetLineColor (0);
            elParams->SetWeight (0);
            elParams->SetLineStyle (0);
            elParams->SetFillColor (255);
            elParams->SetFillDisplay (FillDisplay::Blanking);

            ep->context->CookDisplayParams();

            if ( ! ep->GetDgnModelP()->Is3d())
                {
                double  priority = ep->context->GetDisplayPriority ();

                for (int iPt = 0; iPt < 5; iPt++)
                    shape[iPt].z = priority - 1;
                }

            ep->context->GetIDrawGeom().DrawShape3d (5, shape, true, NULL);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
int  BentleyApi::adim_generateLineString
(
AdimProcess            *ep,             /* => Function to process element       */
DPoint3d               *points,         /* => Vertices of line string           */
int                     nPoints,        /* => Number of vertices in line string */
int                     shape,          /* => 1 create shape, 2 add fill        */
DimMaterialIndex        material        /* => material                          */
)
    {
    if (NULL == ep->context)
        return ERROR;

    if (DrawPurpose::RegionFlood == ep->context->GetDrawPurpose ())
        return SUCCESS;

    if (nPoints == 2)
        return (adim_generateLine (ep, points, points+1, material));

    adim_prepareToDrawComponent (ep, material);

    bool            createLineString = (0 == shape);
    bool            isFilled = (2 == shape);
    ViewContextP    context = ep->context;
    IDrawGeomR      output = context->GetIDrawGeom();

    if (DisplayHandler::Is3dElem ((DgnElementCP) ep->GetDimElementCP()))
        {
        if (createLineString)
            context->DrawStyledLineString3d (nPoints, points, NULL);
        else
            output.DrawShape3d (nPoints, points, isFilled, NULL);
        }
    else
        {
        double                  priority = context->GetDisplayPriority();
        std::valarray<DPoint2d> pts2d (nPoints);

        for (int iPoint = 0; iPoint < nPoints; iPoint++)
            pts2d[iPoint].init (&points[iPoint]);

        if (createLineString)
            context->DrawStyledLineString2d (nPoints, &pts2d[0], priority, NULL);
        else
            output.DrawShape2d (nPoints, &pts2d[0], isFilled, priority, NULL);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
int          BentleyApi::adim_generateCircle
(
AdimProcess        *ep,
DPoint3d           *origin,
double              radius,
RotMatrix          *rMatrix,
bool                filled,
DimMaterialIndex    material
)
    {
    if (NULL == ep->context)
        return ERROR;

    if (DrawPurpose::RegionFlood == ep->context->GetDrawPurpose ())
        return SUCCESS;

    adim_prepareToDrawComponent (ep, material);

    ViewContextP    context = ep->context;
    IDrawGeomR      output  = ep->context->GetIDrawGeom();

    if (DisplayHandler::Is3dElem ((DgnElementCP) ep->GetDimElementCP()))
        {
        double      quaternion[4];
        DEllipse3d  ellipse;

        rMatrix->getQuaternion (quaternion, true);

        ellipse.InitFromDGNFields3d (*origin, &quaternion[0], radius, radius, 0.0, msGeomConst_2pi);
        output.DrawArc3d (ellipse, true, filled, NULL);
        }
    else
        {
        //Note We cannot use rmatrix->getRotationAngleAndVector because it tries to keep it under 2pi and flips the axis;
        //This will cause difference in the generated GPA.
        double      angle = rMatrix->ColumnXAngleXY ();
        DEllipse3d  ellipse;

        ellipse.InitFromDGNFields2d ((DPoint2dCR) *origin, angle, radius, radius, 0.0, msGeomConst_2pi, 0.0);
        output.DrawArc2d (ellipse, true, filled, context->GetDisplayPriority(), NULL);
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
    Merge the second textbox into the first, making it a union of the two

    NB: we can assume that all parts of the text for a given dimension
    are parallel to each and in the same plane, so we only need to consider
    the origin and extent in the union, as though it were unrotated and in 2D.
* @bsimethod                    SamWilson                       09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    unionTextBoxes
(
AdimRotatedTextBox          *pBox,      // <=>
const AdimRotatedTextBox    *pBox2      //  =>
)
    {
    DRange3d    rng, rng2;
    DVec3d      upDir;
    RotMatrix   rText;
    Transform   toPlane, toWorld;

    //  How to map into and out of text LCS
    bsiDVec3d_crossProduct (&upDir, &pBox->zvec, &pBox->baseDir);
    rText.InitFromColumnVectors(pBox->baseDir, upDir, pBox->zvec);
    LegacyMath::TMatrix::ComposeLocalOriginOperations (&toPlane,  NULL, NULL,  &rText, &pBox->baseFirstChar);
    LegacyMath::TMatrix::ComposeLocalOriginOperations (&toWorld, &pBox->baseFirstChar, &rText, NULL, NULL);

    //  Get current and proposed baseFirstChar ... in text LCS coordinates
    rng.low = pBox->baseFirstChar;
    rng2.low = pBox2->baseFirstChar;
    toPlane.Multiply (&rng.low,  1);
    toPlane.Multiply (&rng2.low,  1);

    //  In text LCS, the text range is an unrotated width X height box
    //  Get the two text boxes.
    rng.high.x = rng.low.x + pBox->width;
    rng.high.y = rng.low.y + pBox->height;
    rng.high.z = rng.low.z;

    rng2.high.x = rng2.low.x + pBox2->width;
    rng2.high.y = rng2.low.y + pBox2->height;
    rng2.high.z = rng2.low.z;

    //  Union the two text boxes
    bsiDRange3d_combineRange (&rng, &rng, &rng2);

    //  The width and height are just the dimensions of the (unrotated) union
    pBox->width  = (rng.high.x - rng.low.x);
    pBox->height = (rng.high.y - rng.low.y);

    //  The new baseFirstChar is the low corner of the union, projected back into world CS
    toWorld.Multiply (&rng.low,  1);
    pBox->baseFirstChar = rng.low;
    }

/*----------------------------------------------------------------------------------*//**
*    Record the actual location, orientation, and extent of the text as drawn.
*    The dimension line-drawing logic will use this rotated range in a later phase.
* @bsimethod                    SamWilson                       10/01
+---------------+---------------+---------------+---------------+---------------+------*/
SemiPrivate void BentleyApi::adim_updateTextBox
(
AdimProcess*    ep,            // <=>
DPoint3dCP      org,
DVec3dCP        baseDir,
RotMatrixCP     rMatrix,
DPoint2dCP      adSize
)
    {
    int                 boxIndex;
    AdimRotatedTextBox  textBox;

    textBox.baseFirstChar = *org;
    textBox.baseDir       = *baseDir;
    rMatrix->GetColumn(*(&textBox.zvec),  2);
    textBox.height   = adSize->y;
    textBox.width    = adSize->x;

    // for the current segment, keep track of two textboxes (upper and lower)
    if (ADTYPE_TEXT_LOWER == ADIM_GETTYPE (ep->partName))
        boxIndex = 1;
    else
        boxIndex = 0;

    //  If this is the first (or only) text generated for this dimension
    if (0.0 == ep->textBox[boxIndex].height)
        ep->textBox[boxIndex] = textBox;
    else // more than one piece of text for this dimension => compute union of their ranges
        unionTextBoxes (&ep->textBox[boxIndex], &textBox);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    04/09
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     updateTextBoxFromTextBlock
(
AdimProcess*    ep,            // <=>
TextBlockCR     textBlock
)
    {
    DRange3d range = textBlock.GetExactRange ();

    if (bsiDRange3d_isEmpty (&range))
        return;

    DPoint2d            textLen;
    RotMatrix           rMatrix = textBlock.GetOrientation ();
    DPoint3d            rotatedRangeY;
    DPoint3d            textLL;
    DVec3d              dir;
   
    rMatrix.getColumn (&dir, 0);

    // TextBlock's origin is upper left, we need the lower left
    textLen.x = range.high.x - range.low.x;
    textLen.y = range.high.y - range.low.y;

    rotatedRangeY.x = rotatedRangeY.z = 0.0;
    rotatedRangeY.y = (range.low.y - range.high.y);

    rMatrix.Multiply(rotatedRangeY);
    textLL.SumOf (rotatedRangeY, textBlock.GetOrigin());

    adim_updateTextBox (ep, &textLL, &dir, &rMatrix, &textLen);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::adim_generateText
(
AdimProcess                 *ep,            /* => Function used to process element    */
WCharCP                     text,
DPoint3d                    *rtxtorg,
DVec3d                      *dir,
TextElementJustification    just,
DPoint2dP                   tileSize
)
    {
    RotMatrix       rMatrix;
    DPoint2d        adTile;
    TextParamWide   textParams;

    DgnModelP modelRef = ep->GetDgnModelP();
    if (NULL != ep->pTextStyle)
        {
        textParams = DgnTextStylePersistence::Legacy::ToTextParamWide(*ep->pTextStyle, modelRef->GetDgnProject(), NULL, NULL);
        }
    else
        {
        DgnTextStylePtr dgnTextStyle = DgnTextStyle::Create(ep->GetDgnModelP()->GetDgnProject());
        textParams = DgnTextStylePersistence::Legacy::ToTextParamWide(*dgnTextStyle, NULL, NULL, ep->pTextStyle->fontNo, ep->pTextStyle->shxBigFont);
        }

    adim_prepareToDrawComponent (ep, DIM_MATERIAL_Text);

    // set the text color to active symbology color if there is no color property specified
    if (!textParams.exFlags.color)
        textParams.color = ep->context->GetCurrentDisplayParams()->GetLineColor ();

    if (NULL == tileSize)
        {
        adim_getTileSize (&adTile, NULL, ep->GetElemHandleCR());
        tileSize = &adTile;
        }

    BentleyApi::adim_getRMatrixFromDir (&rMatrix, dir, &ep->rMatrix, &ep->vuMatrix);

    if (dimTextBlock_stringContainsMarkup (text))
        {
        TextBlock textBlock (*ep->GetDgnModelP());

        if (SUCCESS ==  adim_loadTextBlockFromMarkedUpString (textBlock, text, &textParams, tileSize, ep))
            {
            textBlock.PerformLayout ();

            DPoint3d    textOrigin = *rtxtorg;

            // textOrigin is left center, textBlock expects upper left
            DPoint3d    offset = { 0.0, textBlock.GetExactHeight() / 2.0, 0.0 };
            rMatrix.Multiply(offset);
            textOrigin.add (&offset);

            textBlock.SetTextOrigin (textOrigin);
            textBlock.SetOrientation (rMatrix);

            ep->context->DrawTextBlock (textBlock);

            updateTextBoxFromTextBlock (ep, textBlock); 
            }
        }
    else
        {
        DPoint3d    textOrigin = *rtxtorg;
        DPoint2d    charSize = { tileSize->x, tileSize->y };
        UInt32      textStringWeight = 0;
        if (ep->context)
             textStringWeight = ep->context->GetCurrentDisplayParams()->GetWeight ();

        TextString  textString;

        textParams.just = static_cast<int>(just);
        textString.Init (text, textOrigin, rMatrix, TextStringProperties (textParams, charSize, ep->GetDgnModelP ()->Is3d (), ep->GetDgnModelP ()->GetDgnProject ()), 0, NULL, textStringWeight);
        textString.SetLineWeight (textStringWeight);

        textString.SetOriginFromUserOrigin (textOrigin);

        ep->context->DrawTextString (textString);

        DPoint2d    textSize  = { textString.GetWidth(), textString.GetHeight() };
        
        // Keep an 8.11 logic: store the left-bottom, instead of the user, origin in textBox:
        DgnFontCR   font = *DgnFontManager::ResolveFont (textParams.font, ep->GetDgnModelP ()->GetDgnProject (), DGNFONTVARIANT_DontCare);
        DPoint3d    offset = DPoint3d::From(0.0, 0.0, 0.0);
        TextString::ComputeUserOriginOffset (offset, textSize, 0.0, just, false, DIMENSION_ELM, &font);
        rMatrix.Multiply (offset);
        textOrigin.Subtract (offset);

        adim_updateTextBox (ep, &textOrigin, dir, &rMatrix, &textSize);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @return       elemproc status SUCCESS/ERROR or -1 not processed -> not enhanced text
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::adim_generateTextUsingDescr
(
AdimProcess     *ep,
DPoint3d        *origin,
DVec3d          *direction
)
    {
    DimMLText       *pText      = NULL;

    if (false == dimTextBlock_getRequiresTextBlock (ep, &pText))
        return -1;

    // Populate textblock
    if (! dimTextBlock_isPopulated (ep))
        dimTextBlock_populateWithValues (ep, pText, ep->strDat.GetPrimaryStrings());

    adim_prepareToDrawComponent (ep, DIM_MATERIAL_Text);
    
    TextBlockPtr    textBlock   = ep->m_textBlock;
    RotMatrix       rotMatrix;
    BentleyApi::adim_getRMatrixFromDir (&rotMatrix, direction, &ep->rMatrix, &ep->vuMatrix);

    textBlock->SetUserOrigin (*origin);
    textBlock->SetOrientation (rotMatrix);

    ep->context->DrawTextBlock (*textBlock);

    updateTextBoxFromTextBlock (ep, *textBlock);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::adim_generateArc
(
AdimProcess const*    ep,
DPoint3dCP      origin,
double          r0,
double          r1,
RotMatrixCP     rMatrix,
double          start,
double          sweep,
DimMaterialIndex material
)
    {
    if (NULL == ep->context)
        return ERROR;

    if (DrawPurpose::RegionFlood == ep->context->GetDrawPurpose ())
        return SUCCESS;

    adim_prepareToDrawComponent (ep, material);

    ViewContextP    context = ep->context;

    if (DisplayHandler::Is3dElem ((DgnElementCP) ep->GetDimElementCP()))
        {
        double      quaternion[4];
        DEllipse3d  ellipse;

        rMatrix->getQuaternion (quaternion, true);

        ellipse.InitFromDGNFields3d (*origin, &quaternion[0], r0, r1, start, sweep);
        context->DrawStyledArc3d (ellipse, false, NULL);
        }
    else
        {
        DVec3d      xVec, yVec;

        rMatrix->GetColumn (xVec, 0);
        rMatrix->GetColumn (yVec, 1);

        double      angle = Angle::Atan2 (xVec.y, xVec.x);
        DEllipse3d  ellipse;
        
        //rMatrix.InitFromAxisAndRotationAngle(2,  angle);
        //DVec3d  vec;
        //double  angle = rMatrix->getRotationAngleAndVector (&vec);

        if (xVec.CrossProductXY(yVec) < 0.0)
           {
           start *= -1.0;
           sweep *= -1.0;
           }

        ellipse.InitFromDGNFields2d ((DPoint2dCR) *origin, angle, r0, r1, start, sweep, 0.0);
        context->DrawStyledArc2d (ellipse, false, context->GetDisplayPriority(), NULL);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen     05/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::adim_generateArcByPoints
(
AdimProcess     *ep,
DPoint3d        *pPnts,
DimMaterialIndex material
)
    {
    DEllipse3d  ellipse;

    if (!ellipse.initFromPointsOnArc (&pPnts[0], &pPnts[1], &pPnts[2]))
        return SUCCESS;

    DPoint3d    origin;
    RotMatrix   rMatrix;
    double      r0, r1, start, sweep;

    ellipse.getScaledRotMatrix (&origin, &rMatrix, &r0, &r1, &start, &sweep);

    return adim_generateArc (ep, &origin, r0, r1, &rMatrix, start, sweep, material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
int          BentleyApi::adim_generateSymbol
(
AdimProcess*                ep,
int                         fontNo,
UInt16                      symbol,
DPoint3dP                   point,
RotMatrixP                  rMatrix,
DPoint2dP                   tile,
TextElementJustification    just,
DimMaterialIndex            material
)
    {
    if (NULL == ep->context)
        return ERROR;

    if (DrawPurpose::RegionFlood == ep->context->GetDrawPurpose ())
        return SUCCESS;

    adim_prepareToDrawComponent (ep, material);

    DPoint2d        charSize = { tile->x, tile->y };
    TextParamWide   textParams;

    memset (&textParams, 0, sizeof(textParams));

    textParams.font             = fontNo;
    textParams.just             = static_cast<int>(just);
    textParams.annotationScale  = 1.0;

//TODO TEST Dimesnion with symbol font
// 1) Test this piece of code with a symbol font and see whether it is acceptable.
// 2) How to create a unicode string from a 'int symbol'?

    DgnFontCR   font = *DgnFontManager::ResolveFont (fontNo, ep->GetDgnModelP ()->GetDgnProject (), DGNFONTVARIANT_DontCare);
    WChar uniString[16] = { font.RemapFontCharToUnicodeChar (symbol), '\0' };

    if (!textParams.exFlags.color)
        textParams.color = ep->context->GetCurrentDisplayParams()->GetLineColor ();
    TextString  textString (uniString, NULL, rMatrix, TextStringProperties (textParams, charSize, ep->GetDgnModelP()->Is3d (), ep->GetDgnModelP()->GetDgnProject ()));
    textString.SetOriginFromUserOrigin (*point);

    ep->context->DrawTextString (textString);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
int  BentleyApi::adim_findSharedCellDef
(
double*             widthP,        /* <= */
double*             heightP,       /* <= */
ElementRefPtr*         elemRefP,      /* <= */
UInt64           cellId,        /* => */
DgnModelP        modelRef       /* => */
)
    {
    ElementRefP  elemRef = modelRef->GetDgnProject().Models().GetElementById(ElementId(cellId)).get();

    if (NULL == elemRef)
        return ERROR;

    DRange3d range = elemRef->GetUnstableMSElementCP()->GetRange();

    if (widthP)
        {
        *widthP = range.high.x - range.low.x;

        if (*widthP <= 0.0)
            *widthP = 1.0;
        }

    if (heightP)
        {
        *heightP = range.high.y - range.low.y;

        if (*heightP <= 0.0)
            *heightP = 1.0;
        }

    if (elemRefP)
        *elemRefP = elemRef;

    return  SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass                                                     BrienBastings   02/13
+===============+===============+===============+===============+===============+======*/
struct DimSymbolCell : IDisplaySymbol // NOTE: IDisplaySymbol/DrawSymbol allows a context to continue to treat this as a "compound" element...
{
private:

EditElementHandle   m_eeh;

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetRange (DRange3dR range) const override
    {
    return m_eeh.GetDisplayHandler ()->CalcElementRange (m_eeh, range, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Draw (ViewContextR context) override
    {
    context.VisitElemHandle (m_eeh, false, false);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
DimSymbolCell (ElementRefP elemRef, DgnModelP modelRef, ElemHeaderOverridesCP ovr)
    {
    m_eeh.SetElementRef (elemRef);

    if (NULL == ovr)
        return;

    ElementPropertiesSetter remapper;

    if (ovr->GetFlags ().level)
        remapper.SetLevel (ovr->GetLevel ());

    if (ovr->GetFlags ().color)
        {
        remapper.SetColor (ovr->GetColor ());
        remapper.SetFillColor (ovr->GetColor ()); // Header color override also applies to fill...
        }

    if (ovr->GetFlags ().style)
        remapper.SetLinestyle (ovr->GetLineStyle (), ovr->GetLineStyleParams ());

    if (ovr->GetFlags ().weight)
        remapper.SetWeight (ovr->GetWeight ());

    remapper.SetChangeEntireElement (true); // Apply header overrides to everything and add overrides to nested shared cell instances...
    remapper.Apply (m_eeh); // NOTE: This is a no-op if there were no header overrides...
    }

}; // DimSymbolCell

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int generateCell
(
AdimProcess        *ep,
UInt64           cellId,
DPoint3d           *origin,
RotMatrix          *rMatrix,
double             *pScale,      /* => is scale provided, width and height are not used */
double             *pWidth,
double             *pHeight,
bool                symbOverride,
DimMaterialIndex    material
)
    {
    if (NULL == ep->context)
        return ERROR;

    if (DrawPurpose::RegionFlood == ep->context->GetDrawPurpose ())
        return SUCCESS;

    ElementRefPtr scDefElem;
    double      cellWidth, cellHeight;

    if (SUCCESS != adim_findSharedCellDef (&cellWidth, &cellHeight, &scDefElem, cellId, ep->GetDgnModelP()))
        return SUCCESS;

    double      xScale, yScale, zScale;

    if (NULL != pScale)
        {
        xScale = *pScale;
        yScale = *pScale;
        zScale = *pScale;
        }
    else
        {
        if (NULL == pWidth || NULL == pHeight)
            {
            /* either a scale or a width & height must be provided */
            BeAssert (false);
            return ERROR;
            }

        xScale = *pWidth  / cellWidth;
        yScale = *pHeight / cellHeight;
        zScale = 1.0;
        }

    DVec3d xAxis, yAxis, zAxis;

    rMatrix->getColumn (&xAxis, 0);
    rMatrix->getColumn (&yAxis, 1);
    rMatrix->getColumn (&zAxis, 2);

    xAxis.scale (&xAxis, xScale);
    yAxis.scale (&yAxis, yScale);
    zAxis.scale (&zAxis, zScale);

    RotMatrix   scaledRMatrix;

    scaledRMatrix.initFromColumnVectors (&xAxis, &yAxis, &zAxis);

    SCOverride  flags;

    memset (&flags, 0, sizeof(flags));

    flags.level  = symbOverride;
    flags.color  = symbOverride;
    flags.weight = symbOverride;
    flags.style  = symbOverride;

    ViewContext::ContextMark  mark (*ep->context, ep->GetElemHandleCR()); // NOTE: This also makes sure scDef cmpns don't end up in DisplayPath...restored in destructor!

    adim_setHitDetail (ep); // Setup hit details for part being drawn...

    Symbology           symb = ep->GetDimElementCP()->GetSymbology();
    ElemHeaderOverrides ovr;

    adim_getEffectiveSymbology (&symb, ep->GetElemHandleCR(), material, ep->partName, ep->pOverrides);
    ovr.Init (&flags, LevelId(ep->GetDimElementCP()->GetLevel()), 0, 0, (DgnElementClass) DgnElementClass::Primary, &symb, NULL);

    Transform   transform;

    transform.initFrom (&scaledRMatrix, origin);
    ep->context->PushTransform (transform);

    DimSymbolCell symbolCell (scDefElem.get(), ep->GetDgnModelP (), &ovr);

    ep->context->DrawSymbol (&symbolCell, NULL, NULL, false, false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
int  BentleyApi::adim_generateCell
(
AdimProcess        *ep,
UInt64              cellId,
DPoint3d           *origin,
RotMatrix          *rMatrix,
double              width,
double              height,
DimMaterialIndex    material
)
    {
    return generateCell (ep, cellId, origin, rMatrix, NULL, &width, &height, true, material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DonFu           02/01
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::adim_generateCellScale
(
AdimProcess        *ep,
UInt64              cellId,
DPoint3d           *origin,
RotMatrix          *rMatrix,
double              scale,
DimMaterialIndex    material
)
    {
    return generateCell (ep, cellId, origin, rMatrix, &scale, NULL, NULL, false, material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::adim_checkNoLineFlag
(
AdimProcess const   *ap,
int                 termIndex
)
    {
    switch (termIndex)
        {
        case 1:              /* Arrowhead terminator                        */
            return ap->GetDimElementCP()->extFlag.noLineThruArrowTerm;

        case 2:              /* Oblique stroke - terminator                 */
            return ap->GetDimElementCP()->extFlag.noLineThruStrokeTerm;

        case 3:              /* Common origin terminator                    */
            return ap->GetDimElementCP()->extFlag.noLineThruOriginTerm;

        case 4:              /* Dot (joint/bowtie) terminator               */
            return ap->GetDimElementCP()->extFlag.noLineThruDotTerm;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        isThruCellTerm
(
AdimProcess const   *pAdimProcess,
DimTermBlock const  *pTermBlock,
int                 termIndex
)
    {
    return  NULL != pTermBlock &&
            3 == pTermBlock->flags.arrow &&
            !adim_checkNoLineFlag(pAdimProcess, termIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
double  BentleyApi::adim_getTrimDistance
(
AdimProcess    *ap,
int             termIndex,
DimTermBlock const *pTermBlock,
bool            sameSide
)
    {
    double  trimDist = 0.0;

    switch (termIndex)
        {
        case 1:              /* Arrowhead terminator */
            if (sameSide)
                {
                if (pTermBlock && 1 == pTermBlock->flags.arrow)
                    trimDist = 0.0; /* symbol term unsupported */
                else if (isThruCellTerm(ap, pTermBlock, termIndex))
                    trimDist = 0.0; /* term allowing line thru can have a line thru */
                else
                    trimDist = ap->GetDimElementCP()->geom.termWidth;
                }
            else
                {
                trimDist = 0.0;
                }
            break;

        case 2:              /* Oblique stroke - terminator */
            if (pTermBlock && 1 == pTermBlock->flags.stroke)
                trimDist = 0.0; /* symbol term unsupported */
            else if (isThruCellTerm(ap, pTermBlock, termIndex))
                trimDist = 0.0; /* term allowing line thru can have a line thru */
            else
                trimDist = ap->GetDimElementCP()->geom.termWidth / 2.0;
            break;

        case 3:              /* Common origin terminator */
            if (pTermBlock && 1 == pTermBlock->flags.origin)
                trimDist = 0.0; /* symbol term unsupported */
            else if (isThruCellTerm(ap, pTermBlock, termIndex))
                trimDist = 0.0; /* term allowing line thru can have a line thru */
            else
                trimDist = ap->GetDimElementCP()->geom.termHeight / 2.0;

            break;

        case 4:              /* Dot (joint/bowtie) terminator */
            if (pTermBlock && 1 == pTermBlock->flags.dot)
                trimDist = 0.0; /* symbol term unsupported */
            else
            if (pTermBlock && 2 == pTermBlock->flags.dot)
                trimDist = ap->GetDimElementCP()->geom.termHeight / 2.0;
            else
            if (isThruCellTerm(ap, pTermBlock, termIndex))
                trimDist = 0.0; /* term allowing line thru can have a line thru */
            else
                trimDist = ap->GetDimElementCP()->geom.termHeight / 8.0;
            break;
        }

    return trimDist;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BentleyApi::adim_generateDimLine
(
AdimProcess        *ap,
DPoint3d           *pLeftPt,
DPoint3d           *pRightPt,
DimMaterialIndex    material,
int                 leftTermIn,
int                 rightTermIn,
int                 trimCode,
bool                lineInside,
bool                termsInside,
bool                bLeftToRight
)
    {
    /* The noLineThru flags indicate that the DimLine should be trimmed so as not
       to pass through the terminator.  Callers to this func use trimCode to indicate
       which side of the line to trim */
    if (TRIM_NONE != trimCode &&
        (ap->GetDimElementCP()->extFlag.noLineThruArrowTerm  || ap->GetDimElementCP()->extFlag.noLineThruStrokeTerm ||
         ap->GetDimElementCP()->extFlag.noLineThruOriginTerm || ap->GetDimElementCP()->extFlag.noLineThruDotTerm))
        {
        int             leftTerm, rightTerm;
        double          trimDist;
        bool            sameSide;
        DPoint3d        leftPt, rightPt;
        DVec3d          direction;
        DimTermBlock const   *pTermBlock = reinterpret_cast <DimTermBlock const*> (mdlDim_getOptionBlock (ap->GetElemHandleCR(), ADBLK_TERMINATOR, NULL));

        sameSide = (lineInside == termsInside);

        /* reverse points for rightToLeft */
        leftPt  = bLeftToRight ? *pLeftPt : *pRightPt;
        rightPt = bLeftToRight ? *pRightPt : *pLeftPt;

        /* reverse terminators for outside */
        leftTerm  = lineInside ? leftTermIn : rightTermIn;
        rightTerm = lineInside ? rightTermIn : leftTermIn;

        direction.NormalizedDifference (leftPt, rightPt);

        /* Trim to avoid the left terminator */
        if (adim_checkNoLineFlag (ap, leftTerm) && trimCode & TRIM_LEFT)
            {
            trimDist = adim_getTrimDistance (ap, leftTerm, pTermBlock, sameSide);

            if (bsiDPoint3d_distance (&leftPt, &rightPt) < trimDist)
                return SUCCESS;

            bsiDPoint3d_addScaledDVec3d (&leftPt, &leftPt, &direction, -1.0 * trimDist);
            }

        /* Trim to avoid the right terminator */
        if (adim_checkNoLineFlag (ap, rightTerm) && trimCode & TRIM_RIGHT)
            {
            trimDist = adim_getTrimDistance (ap, rightTerm, pTermBlock, sameSide);

            if (bsiDPoint3d_distance (&leftPt, &rightPt) < trimDist)
                return SUCCESS;

            bsiDPoint3d_addScaledDVec3d (&rightPt, &rightPt, &direction, trimDist);
            }

        /* swap the points back, so we don't change the intended direction */
        pLeftPt  = bLeftToRight ? &leftPt : &rightPt;
        pRightPt = bLeftToRight ? &rightPt : &leftPt;
        }

    return adim_generateLine (ap, pLeftPt, pRightPt, material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::adim_generateExtensionLine
(
AdimProcess        *pAdimProcess,  /* => Adim Process                        */
DPoint3d           *start,         /* => Start point of line                 */
DPoint3d           *end,           /* => End point of line                   */
bool                useExtSymb     /* => Use extension symbology             */
)
    {
    DimMaterialIndex    material = useExtSymb ? DIM_MATERIAL_Extension : DIM_MATERIAL_DimLine;

    return adim_generateLine (pAdimProcess, start, end, material);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       generateElbow
(
DPoint3d*                   pElbowOut,
AdimProcess*                pAdimProcess,
DPoint3d       const* const pCursor,
DVec3d         const* const pDirection,
DimOffsetBlock const* const pOffsetBlk,
bool           const        bOnLeft,
int                         alignment
)
    {
    DSegment3d elbow;
    double              dCharWidth  = pAdimProcess->strDat.charWidth;
    double              dElbowLen  = pAdimProcess->GetDimElementCP()->geom.margin ? pAdimProcess->GetDimElementCP()->geom.margin : 3.0 * dCharWidth;
    static int const    LEADER_None = 0;

    if (NULL != pElbowOut && NULL != pCursor)
        *pElbowOut = *pCursor;

    // no point for further visit here
    if (NULL == pOffsetBlk || ! pOffsetBlk->flags.elbow || LEADER_None == pOffsetBlk->flags.chainType)
         return SUCCESS;

    // if the elbow length for ball & chain is specified, apply it:
    mdlDim_extensionsGetBncElbowLength (&dElbowLen, pAdimProcess->pOverrides, dElbowLen);

    // Using the input cursor point as the elbow start, determine the elbow end point by using the text on left side flag.
    // If the text is on the right, the elbow end point is on the left and vice-versa. The elbow length is equal to the
    // min leader length.
    elbow.point[0] = *pCursor;

    switch (alignment)
        {
        case 2:     // text is always on right hand side
            bsiDPoint3d_addScaledDVec3d (&elbow.point[1], &elbow.point[0], pDirection, -dElbowLen);
            break;

        case 1:     // text is always on left hand side
            bsiDPoint3d_addScaledDVec3d (&elbow.point[1], &elbow.point[0], pDirection, dElbowLen);
            break;

        case 0:     // auto, text swaps
        default:
            {
            if (bOnLeft)
                bsiDPoint3d_addScaledDVec3d (&elbow.point[1], &elbow.point[0], pDirection, dElbowLen);
            else
                bsiDPoint3d_addScaledDVec3d (&elbow.point[1], &elbow.point[0], pDirection, -dElbowLen);

            break;
            }
        }

    if (NULL != pElbowOut)
        *pElbowOut = elbow.point[1];

    ADIM_SETNAME (pAdimProcess->partName, ADTYPE_CHAIN, ADSUB_NONE);
    return  adim_generateLine (pAdimProcess, &elbow.point[0], &elbow.point[1], DIM_MATERIAL_DimLine);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  04/02
+---------------+---------------+---------------+---------------+---------------+------*/
static void   computeElbowStartPointLocationAndSide
(
DPoint3d*                   cursor,
bool*                    pTextOnLeftSide,
AdimProcess const* const    ep,
DSegment3d const* const    leader,
DVec3d     const* const    textDirection,
double             const    textWidth,
double             const    textMargin,
bool               const    multiline
)
    {
    // At this point, it is assumed that the text is on the right hand side of the elbow.
    // Once we locate the elbow start point based on this assumption, we will decide if
    // the text needs to flip to the left hand side. If needed, the calling function will
    // make the text flip later.
    switch  (adim_getDimTextJustificationHorizontal (ep))
        {
        case TextElementJustification::RightMiddle:
            if (ep->GetDimElementCP()->flag.horizontal)
                bsiDPoint3d_addScaledDVec3d (cursor, &leader->point[1], textDirection, -(textWidth + textMargin));
            else
                bsiDPoint3d_addScaledDVec3d (cursor, &leader->point[1], textDirection, textMargin);
            break;

        case TextElementJustification::CenterMiddle:
            if (ep->GetDimElementCP()->flag.horizontal)
                bsiDPoint3d_addScaledDVec3d (cursor, &leader->point[1], textDirection, -(textWidth / 2.0 + textMargin));
            else
                *cursor = leader->point[1];
            break;

        default: // Left
            if (ep->GetDimElementCP()->flag.horizontal)
                bsiDPoint3d_addScaledDVec3d (cursor, &leader->point[1], textDirection, -textMargin);
            else
                bsiDPoint3d_addScaledDVec3d (cursor, &leader->point[1], textDirection, -textMargin);
            break;
        }

    if (NULL != pTextOnLeftSide)
        {
        DVec3d    dVec;

        // negative dotProduct tells that we're on left side
        bsiDVec3d_subtractDPoint3dDPoint3d (&dVec, cursor, &leader->point[0]);
        *pTextOnLeftSide = bsiDVec3d_dotProduct (textDirection, &dVec) < 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double   combinedResolution
(
double res0,
double res1
)
    {
    // copied from mstn\msbspline\bsp\bspbltwrappers.cpp
    res0 = fabs (res0);
    res1 = fabs (res1);
    return res0 > res1 ? res0 : res1;
    }

#define PI                      3.1415926535897932384626433

void DrawSimpleCurvedLeader
(
DPoint3d                    *termDir,               /* <=  */
AdimProcess*                ep,                     /*  => */
int                         nPoints,                /*  => */
DPoint3d                    *dPoints,               /*  => */
DPoint3d       const* const startTangent,          /*  => can be (0,0,0) */
DPoint3d       const* const endTangent             /*  => can be (0,0,0) */
)
    {
    if (   NULL != ep->context
        && NULL == startTangent
        && endTangent != NULL
        && nPoints == 2
        )
        {
        static double s_tangentFraction = 0.30; // Bigger number makes the curve hug end tangent longer
        static double s_fraction2 = 0.5;
        double chordDistance = dPoints[0].Distance (dPoints[1]);
        DVec3d endUnit;
        endUnit.Normalize (*(DVec3dCP)endTangent);
        DPoint3d poles[4];
        poles[0] = dPoints[0];
        poles[3] = dPoints[1];
        poles[2].SumOf (poles[3], endUnit, s_tangentFraction * chordDistance);
        poles[1].Interpolate (poles[0], s_fraction2, poles[2]);

        ViewContextP    context = ep->context;
        MSBsplineCurve  curve;

        curve.CreateFromPointsAndOrder (poles, 4, 4);

        if (DisplayHandler::Is3dElem ((DgnElementCP) ep->GetDimElementCP()))
            context->DrawStyledBSplineCurve3d (curve);
        else
            context->DrawStyledBSplineCurve2d (curve, context->GetDisplayPriority());

        curve.ReleaseMem ();
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 04/03
* Note : Major portion copied from drftools\callout.mc
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt            BentleyApi::adim_generateBSpline
(
DPoint3d                    *termDir,               /* <=  */
AdimProcess*                ep,                     /*  => */
int                         nPoints,                /*  => */
DPoint3d                    *dPoints,               /*  => */
DPoint3d       const* const startTangent,          /*  => can be (0,0,0) */
DPoint3d       const* const endTangent             /*  => can be (0,0,0) */
)
    {
    if (NULL == ep->context)
        return ERROR;
    if (DrawPurpose::RegionFlood == ep->context->GetDrawPurpose ())
        return SUCCESS;

    adim_prepareToDrawComponent (ep, DIM_MATERIAL_DimLine);

    ViewContextP   context = ep->context;
    DPoint3d       endTangents[2];

    memset (endTangents, 0, sizeof(endTangents));
    if (NULL != startTangent)
        endTangents[0] = *startTangent;
    if (NULL != endTangent)
        endTangents[1] = *endTangent;

    MSBsplineCurve  curve;

    memset (&curve, 0, sizeof(curve));

    if (SUCCESS != bspcurv_c2CubicInterpolateCurve (&curve, dPoints, NULL, nPoints, true, mgds_fc_epsilon, endTangents, false))
        return SUCCESS;

    if (termDir)
        {
        int             numPts = 0, iIterate = 0;
        bool            bDone = false;
        DVec3d          normal;
        RotMatrix       dimMatrix, dimMatrixInv;

        DimensionHandler::GetInstance().GetRotationMatrix (ep->GetElemHandleCR(), dimMatrix);
        dimMatrix.GetColumn(normal,  2);
        dimMatrixInv.TransposeOf(dimMatrix);

        // TR-119497 : In the case of leader with two points, if the two points are close to each other,
        // the bspline has a sharp bend at the start terminator causing the arrow to look as if it is not
        // pointing properly. The idea here is to create an imaginary circle of the radius of terminatorsize
        // and intersect it with the bspline. The bvector joining the start point and the intersection
        // point defines a better tangent so regenerate the bspline with the new tangent. Use recursion
        // to optimize the tangent bvector, the goal being that the terminator direction and the bspline
        // tangent match each other.
        // The above rule does not apply if the start tangent is specified. We are forced to use that
        // tangent.
        if (startTangent && (startTangent->x > mgds_fc_epsilon || startTangent->y > mgds_fc_epsilon || startTangent->z > mgds_fc_epsilon))
            {
            bsiDVec3d_scale ((DVec3d *)termDir, (DVec3d *) startTangent, -1.0);
            }
        else
            {
            static int s_drawSimpleCurve = 0;
            if (s_drawSimpleCurve)
                DrawSimpleCurvedLeader (termDir, ep, nPoints, dPoints, startTangent, endTangent);

            DVec3d          termDirIn, newTermDirIn;
            DPoint3dP       intersectPt;
            DEllipse3d      circle;
            MSBsplineCurve  circleBspline;

            while (!bDone && iIterate++ < 10)
                {
                DPoint3d xyz;
                curve.FractionToPoint (xyz, termDirIn, 0.0);
                termDirIn.Normalize ();

                // NEEDSWORK: why is the circleBspline created in the loop, isn't it the same every time?
                DPoint3d circleOrigin = dPoints[0];
                if ( ! ep->GetDgnModelP()->Is3d())
                    circleOrigin.z = 0;

                memset (&circleBspline, 0, sizeof(circleBspline));

                bsiDEllipse3d_initFromCenterNormalRadius (&circle, &circleOrigin, &normal, ep->GetDimElementCP()->geom.termHeight);
                bspconv_convertDEllipse3dToCurve (&circleBspline, &circle);

                double      res = combinedResolution (curve.Resolution(), circleBspline.Resolution());

                if (SUCCESS != bspcci_allIntersectsBtwCurves (&intersectPt, NULL, NULL, &numPts, &curve, &circleBspline, &res, &dimMatrixInv, false) || !numPts)
                    {
                    bsiDVec3d_scale ((DVec3d *)termDir, &termDirIn, -1.0);
                    }
                else
                    {
                    newTermDirIn.NormalizedDifference (*intersectPt, circleOrigin);
                    memutil_free (intersectPt);

                    if (bsiDPoint3d_pointEqualTolerance (&newTermDirIn, &termDirIn, mgds_fc_epsilon))
                        {
                        bsiDVec3d_scale ((DVec3d *)termDir, &termDirIn, -1.0);
                        bDone = true;
                        }
                    else
                        {
                        bsiDVec3d_scale ((DVec3d *)termDir, &newTermDirIn, -1.0);
                        memcpy (&endTangents[0], &newTermDirIn, sizeof(endTangents)[0]);

                        curve.ReleaseMem ();
                        if (SUCCESS != bspcurv_c2CubicInterpolateCurve (&curve, dPoints, NULL, nPoints, true, mgds_fc_epsilon, endTangents, false))
                            return SUCCESS;
                        }
                    }

                circleBspline.ReleaseMem ();
                }
            }
        }

    if (DisplayHandler::Is3dElem ((DgnElementCP) ep->GetDimElementCP()))
        context->DrawStyledBSplineCurve3d (curve);
    else
        context->DrawStyledBSplineCurve2d (curve, context->GetDisplayPriority());

    curve.ReleaseMem ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  04/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       generateCurve
(
bool*                    pOnLeftSide,
AdimProcess*                ep,
DPoint3d       const* const leaderTo,
DPoint3d       const* const leaderFrom,
DVec3d         const* const textDirection,
double         const        textWidth,
DimOffsetBlock const* const offsetBlock,
bool           const        multiline,
bool           const        bspline,
int                         alignment
)
    {
    StatusInt       status;
    double          angle, rad1, rad2;
    DPoint3d        center;
    DVec3d          yVec, xVec, dVec, arrowDir;
    DPoint3d        cursor, pointArray[3];
    RotMatrix       rMatrix;
    DSegment3d leader;
    double const    textMargin = ep->GetDimElementCP()->geom.textMargin;

    BentleyApi::adim_getRMatrixFromDir (&rMatrix, textDirection, &ep->rMatrix, &ep->vuMatrix);
    rMatrix.GetColumn(xVec,  0);
    rMatrix.GetColumn(yVec,  1);

    leader.point[0] = *leaderFrom;
    leader.point[1] = *leaderTo;

    computeElbowStartPointLocationAndSide (&cursor, pOnLeftSide, ep, &leader, textDirection, textWidth, textMargin, multiline);

    generateElbow (&leader.point[1], ep, &cursor, textDirection, offsetBlock, *pOnLeftSide, alignment);

    // dVec tells arc radii
    bsiDVec3d_subtractDPoint3dDPoint3d (&dVec, &leader.point[1], &leader.point[0]);

    if (*pOnLeftSide)
        {
        angle =  0.0;
        rad1  = -bsiDVec3d_dotProduct (textDirection, &dVec);
        rad2  =  bsiDVec3d_dotProduct (&yVec, &dVec);

        bsiDPoint3d_addScaledDVec3d (&center, &leader.point[0], textDirection, -rad1);
        }
    else
        {
        angle = msGeomConst_piOver2;
        rad1  = bsiDVec3d_dotProduct (textDirection, &dVec);
        rad2  = bsiDVec3d_dotProduct (&yVec, &dVec);

        bsiDPoint3d_addScaledDVec3d (&center, &leader.point[0], textDirection, rad1);
        }

    if (bspline)
        {
        DPoint3d    pts[2];
        DVec3d      endTangent;

        pointArray[0] = leader.point[0];
        pointArray[1] = leader.point[1];

        // compute the tangent at elbow point for bspline ball-n-chains
        memcpy (&pts[0], &leader.point[0], sizeof (pts[0]));
        memcpy (&pts[1], &cursor, sizeof (pts[1]));
        if (true == ep->GetDimElementCP()->flag.horizontal)
            {
            ep->vuMatrix.Multiply(pts[0]);
            ep->vuMatrix.Multiply(pts[1]);
            if (pts[1].x < pts[0].x)
                endTangent = *textDirection;
            else
                bsiDVec3d_scale (&endTangent, textDirection, -1.0);
            }
        else
            {
            DVec3d    dir = *textDirection;

            DVec3d viewZ;
            ep->vuMatrix.GetRow(viewZ,  2);

            /* Compute bvector perpendicular to line and view Z */
            DVec3d perpendicular;
            perpendicular.crossProduct(&dir, &viewZ);

            DVec3d planeNormal;
            if (LegacyMath::DEqual (perpendicular.magnitude(), 0))
                planeNormal = dir;
            else
                planeNormal.crossProduct(&perpendicular, &viewZ);

            // line is parallel to plane if and only if lineNormal is the zero bvector 
            planeNormal.Normalize ();
            LegacyMath::Vec::LinePlaneIntersect (&pts[0], &pts[1], &dir, &pts[0], &planeNormal, false);

            if (false == LegacyMath::RpntEqual (&pts[0], &pts[1]))
                endTangent.NormalizedDifference (pts[0], pts[1]);
            else
                bsiDVec3d_scale (&endTangent, textDirection, -1.0);
            }

        ADIM_SETNAME (ep->partName, ADTYPE_CHAIN, ADSUB_NONE);
        adim_generateBSpline (&arrowDir, ep, 2, pointArray, NULL, &endTangent);
        }
    else
        {
        if (0.0 < bsiDVec3d_dotProduct (&yVec, &dVec))
            bsiDVec3d_scale (&yVec, &yVec, -1.0);

        ADIM_SETNAME (ep->partName, ADTYPE_CHAIN, ADSUB_NONE);
        if (SUCCESS != (status = adim_generateArc (ep, &center, rad1, rad2, &rMatrix, angle, msGeomConst_piOver2)))
            return      status;

        arrowDir = yVec;
        }

    status = adim_generateTerminator (ep, &leader.point[0], &arrowDir, offsetBlock->flags.terminator);
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   generateLeader
(
bool*                    pOnLeftSide,
AdimProcess*                ep,
DPoint3d       const* const leaderTo,
DPoint3d       const* const leaderFrom,
DVec3d         const* const textDirection,
double         const        textWidth,
DimOffsetBlock const* const offsetBlock,
bool           const        multiline
)
    {
    StatusInt       status;
    DSegment3d leader;
    DPoint3d        cursor;
    double const    textMargin = ep->GetDimElementCP()->geom.textMargin;
    

    leader.point[0] = *leaderFrom;
    leader.point[1] = *leaderTo;

    computeElbowStartPointLocationAndSide (&cursor, pOnLeftSide, ep, &leader, textDirection, textWidth, textMargin, multiline);

    generateElbow (&leader.point[1], ep, &cursor, textDirection, offsetBlock, *pOnLeftSide, offsetBlock->flags.alignment);

    ADIM_SETNAME (ep->partName, ADTYPE_CHAIN, ADSUB_NONE);
    if (SUCCESS != (status = adim_generateDimLine (ep, &leader.point[0], &leader.point[1], DIM_MATERIAL_DimLine, offsetBlock->flags.terminator, 0,
                                                   TRIM_LEFT, true, true, true)))
        {
        return  status;
        }

    /* don't revert terminator for ball & chain */
    
    status = adim_generateLineTerminator (ep, &leader.point[1], &leader.point[0], offsetBlock->flags.terminator, true);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @description  Generate leader portion of the dimension "note" aka ball-and-chain.
* @param        pChainedText    OUT     chained text location
* @param        ep              IN OUT  adim process information
* @param        pOriginPnt      IN      text location
* @param        pStartPnt       IN      leader start point on dimension line
* @param        direction       IN      direction of dimension text
* @param        textSize        IN      text size information
* @bsimethod                                                    petri.niiranen  04/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::adim_generateBallAndChain
(
DPoint3d*               pChainedText,
AdimProcess*            ep,
DPoint3d const* const   pOriginPnt,
DPoint3d const* const   pStartPnt,
DVec3d   const* const   direction,
DPoint2dCP              textSize,
double const            offsetY
)
    {
    UInt16          effectiveChainType = DIMSTYLE_VALUE_BallAndChain_ChainType_None;
    StatusInt       status      = ERROR;
    DimOffsetBlock const* offsetBlock = NULL;
    bool            multiline, onLeftSide, flipSide = false;

    *pChainedText = *pOriginPnt;

    if (NULL == pChainedText)
        return  ERROR;

    // these are ok
    if (NULL == pStartPnt)
        return  SUCCESS;

    if (!BentleyApi::adim_checkForLeader(&effectiveChainType, ep, offsetY, textSize))
        return  SUCCESS;

    if (NULL == (offsetBlock = (DimOffsetBlock const*) mdlDim_getOptionBlock (ep->GetElemHandleCR(), ADBLK_EXTOFFSET, NULL)))
        return  SUCCESS;

    /* multiline = dimTextBlock_isPopulated (ep); */
    multiline = ep->flags.textBlockPopulated;
    switch (effectiveChainType)
        {
        case 3:     // bspline
            {
            status   = generateCurve (&onLeftSide, ep, pOriginPnt, pStartPnt, direction, textSize->x, offsetBlock, multiline, true, 0);
            flipSide = onLeftSide;
            break;
            }

        case 2:     // arc
            {
            status   = generateCurve (&onLeftSide, ep, pOriginPnt, pStartPnt, direction, textSize->x, offsetBlock, multiline, false, 0);
            flipSide = onLeftSide;
            break;
            }

        case 1:     // line
            {
            static int const    ALIGN_Auto = 0;
            static int const    ALIGN_Left = 1;

            status = generateLeader (&onLeftSide, ep, pOriginPnt, pStartPnt, direction, textSize->x, offsetBlock, multiline);

            if ((ALIGN_Auto == offsetBlock->flags.alignment && onLeftSide) || ALIGN_Left == offsetBlock->flags.alignment)
                {
                flipSide = true;
                }

            break;
            }

        case 0:     // none
        default:
            break;
        }

    *pChainedText = *pOriginPnt;

    if (multiline && !ep->GetDimElementCP()->flag.horizontal)
        {
        /*-------------------------------------------------------------------------------
        Adjust text ORIGIN to meet cursor at edge for non-horizontal text. Normally
        non-horizontal text origin is at offset point, but when with leader we need
        to calculate the text egde to cursor.
        -------------------------------------------------------------------------------*/
        switch  (adim_getDimTextJustificationHorizontal (ep))
            {
            case TextElementJustification::RightMiddle:
                bsiDPoint3d_addScaledDVec3d (pChainedText, pOriginPnt, direction, textSize->x + 2.0 * ep->GetDimElementCP()->geom.textMargin);
                break;

            case TextElementJustification::CenterMiddle:
                bsiDPoint3d_addScaledDVec3d (pChainedText, pOriginPnt, direction, (textSize->x + 2.0 * ep->GetDimElementCP()->geom.textMargin) / 2.0);
                break;
            }
        }

    // Until this point, all the calculations were performed assuming the text was on the
    // right hand side of the elbow. If the text needs to be flipped to the left hand side
    // of the elbow, simply move the text origin by the text width + twice text margin.
    if (flipSide)
        {
        double const dTotalWidth = textSize->x + 2.0 * ep->GetDimElementCP()->geom.textMargin;

        bsiDPoint3d_addScaledDVec3d (pChainedText, pChainedText, direction, -dTotalWidth);
        }

    return  status;
    }

#if defined (DEBUG_DRAWPOINTS)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void adim_drawPoints
(
AdimProcess  *ap
)
    {
    int        iPt;
    DPoint3d   point;

    for (iPt = 0; iPt < ap->GetDimElementCP()->nPoints; iPt++)
        {
        BentleyApi::mdlDim_extractPointsD (&point, ap->GetDimElementCP(), ap->currModel, iPt, 1);
        adim_generateCircle (ap, &point, ap->GetDimElementCP()->text.height/2.0, NULL, false, 0);
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static DimProxyOption     getDimensionProxyOption (DimProxyOverride proxyOverride)
    {
    /*-------------------------------------------------------------------------
        The basic option defaults to JustProxy and can be set by the configVar
    -------------------------------------------------------------------------*/
    if (s_dimensionProxyOption == DIMPROXY_OPTION_Invalid)
        {
        s_dimensionProxyOption = DIMPROXY_OPTION_JustProxy;

#ifdef WIP_CFGVAR
        WString     definition;
        if (SUCCESS == ConfigurationManager::GetVariable (definition, L"MS_DIMENSIONPROXY"))
            {
            if (0 == definition.CompareToI (L"ON"))
                s_dimensionProxyOption = DIMPROXY_OPTION_JustProxy;
            else if (0 == definition.CompareToI (L"BOTH"))
                s_dimensionProxyOption = DIMPROXY_OPTION_Both;
            else
                s_dimensionProxyOption = DIMPROXY_OPTION_NoProxy;
            }
#endif
        }

    /*-------------------------------------------------------------------------
        The context can override for specific situations, ex. Pick, Range
    -------------------------------------------------------------------------*/
    if (DIMPROXY_OPTION_NoProxy == s_dimensionProxyOption)
        return s_dimensionProxyOption;

    if (DIMPROXY_OVERRIDE_NoProxy == proxyOverride)
        return DIMPROXY_OPTION_NoProxy;

    if (DIMPROXY_OVERRIDE_NotOnlyProxy == proxyOverride)
        return DIMPROXY_OPTION_Both;

    return s_dimensionProxyOption;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  01/02
+---------------+---------------+---------------+---------------+---------------+------*/
void  BentleyApi::mdlDim_setProxyOption
(
DimProxyOption  iOption
)
    {
    s_dimensionProxyOption = iOption;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          strokeProxy                                             |
|                                                                       |
| author        RayBentley                              09/01           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt     strokeProxy (AdimProcess* pAdimProcess)
    {
    if (!pAdimProcess->context)
        return ERROR;

    ElementId   proxyCellId;
    DPoint3d    origin;
    RotMatrix   rMatrix;
    
    IDimensionQuery* queryHdlr = dynamic_cast<IDimensionQuery*> (&pAdimProcess->GetElemHandleCR().GetHandler());

    if (SUCCESS != queryHdlr->GetProxyCell (pAdimProcess->GetElemHandleCR(), proxyCellId, &origin, &rMatrix))
        return ERROR;
    
    ElementRefP  scDefElem = pAdimProcess->GetDgnModelP ()->FindElementById (proxyCellId);

    if (NULL == scDefElem)
        return ERROR;

    ViewContext::ContextMark  mark (*pAdimProcess->context, pAdimProcess->GetElemHandleCR ()); // NOTE: This also makes sure scDef cmpns don't end up in DisplayPath...restored in destructor!

    adim_setHitDetail (pAdimProcess);

    // Symbology/level is probably covered by ElemDisplayParams/ElemHeaderOverrides established by DimensionHandler::_Draw?
    // SharedCellHandler::CreateSharedCellElement (eeh, 2 != s_dimensionProxyOption ? &pAdimProcess->GetElemHandleCR() : NULL, NULL, &origin, &rMatrix, NULL, pAdimProcess->GetDgnModelP()->Is3d(), *pAdimProcess->GetDgnModelP());
    // eeh.GetElementP ()->ToSharedCell().GetLevel = pAdimProcess->GetElemHandleCR().GetElementCP ()->GetLevel();

    Transform   transform;

    transform.initFrom (&rMatrix, &origin);
    pAdimProcess->context->PushTransform (transform);

    DimSymbolCell symbolCell (scDefElem, pAdimProcess->GetDgnModelP (), NULL);

    pAdimProcess->context->DrawSymbol (&symbolCell, NULL, NULL, false, false);
    pAdimProcess->flags.proxyStroked = true;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IDimElementHelper::CanReturnWithoutStroke () const
    {
    return m_dimension.GetElementCP()->ToDimensionElm().freezeGroup ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            AdimProcess::Init ()
    {
    m_textBlock = TextBlock::Create (*GetDgnModelP());

    if (context)
        {
        DimensionTopology*  topology = new DimensionTopology();

        context->SetElemTopology (topology);
        }

    // override information
    mdlDim_overridesGet (&pOverrides, m_dimensionElement);
    adim_allocateAndExtractTextStyleForStroke (&pTextStyle, m_dimensionElement);
    flags.embed = m_dimensionElement.GetElementCP()->ToDimensionElm().flag.embed;
    points = new DPoint3d [MAX_ADIM_POINTS];//TODO make this a bvector
    memset(points, 0, sizeof(*points) * MAX_ADIM_POINTS);

    InitRotMatrix();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
AdimProcess::AdimProcess (ElementHandleCR element, ViewContextP contextIn)
 :m_dimensionElement (element), context(contextIn), points(NULL), stackHeight(0), partName(0), pOverrides(NULL), pdDimLengths(NULL), pLeaderedMask(NULL),
        pTextBoxes(NULL), pTextStyle(NULL), pDerivedData(NULL), dProjectedTextWidth(0)
    {
    memset(&ep, 0, sizeof(ep));
    // memset(&strDat, 0, sizeof(strDat)); NO! strDat contains WStrings. You must let them construct themselves.
    memset(&vuMatrix, 0, sizeof(vuMatrix));
    memset(&stack, 0, sizeof(stack));
    memset(&textBox, 0, sizeof(textBox));
    memset (&flags, 0, sizeof(flags));

    proxyOverride = DIMPROXY_OVERRIDE_None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
AdimProcess::~AdimProcess ()
    {
    if (context)
        {
        DimensionTopology*  topology = dynamic_cast <DimensionTopology*> (context->GetElemTopology ());

        if (NULL != topology)
            delete topology;

        context->SetElemTopology (NULL);
        }

    // free allocated memory
    if (pOverrides)
        mdlDim_overridesFreeAll (&pOverrides);

    if (pTextStyle)
        memutil_free (pTextStyle);
    
    if (points)
        delete [] points;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
int BentleyApi::adim_strokeDimension (AdimProcess& ep) /* => Function used to process elements */
    {
    
#if defined (DEBUG_DRAWPOINTS)
    adim_drawPoints (ep);
#endif

    DimProxyOption proxyOption = getDimensionProxyOption (ep.proxyOverride);

    if (DIMPROXY_OPTION_NoProxy != proxyOption)
        {
        if (SUCCESS == strokeProxy (&ep) && DIMPROXY_OPTION_Both != proxyOption)
            return SUCCESS;
        }

    IDimElementHelperPtr helper = DimensionHelperFactory::CreateHelper (ep.GetElemHandleCR());
    if (!helper.IsValid())
        return ERROR;

    return helper->StrokeDimension (ep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
DimDerivedDataP BentleyApi::mdlDimDerivedData_create
(
UShort                 flags               //  =>
)
    {
    DimDerivedData *pDerivedData = (DimDerivedData*) calloc (1, sizeof (DimDerivedData));

    pDerivedData->flags = flags;

    return pDerivedData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDimDerivedData_free
(
DimDerivedDataP  *pDerivedData       //  =>
)
    {
    if (NULL != *pDerivedData)
        {
        if (NULL != (*pDerivedData)->pTermDirs)
            free ((*pDerivedData)->pTermDirs);

        if (NULL != (*pDerivedData)->pIsTextDocked)
            free ((*pDerivedData)->pIsTextDocked);

        if (NULL != (*pDerivedData)->pIsTextOutside)
            free ((*pDerivedData)->pIsTextOutside);

        if (NULL != (*pDerivedData)->pArcDefPoint)
            free ((*pDerivedData)->pArcDefPoint);

        if (NULL != (*pDerivedData)->pTextBoxes)
            free ((*pDerivedData)->pTextBoxes);

        if (NULL != (*pDerivedData)->pChainType)
            free ((*pDerivedData)->pChainType);

        if (NULL != (*pDerivedData)->pDimValues)
            free ((*pDerivedData)->pDimValues);

        if (NULL != (*pDerivedData)->pSuperscripted)
            free ((*pDerivedData)->pSuperscripted);

        if (NULL != (*pDerivedData)->pDelimiterOmitted)
            free ((*pDerivedData)->pDelimiterOmitted);

        free (*pDerivedData);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    adim_setDerivedDataBuffers
(
AdimProcess*        ap,                 // <=>
ElementHandleCR     element,            //  =>
DimDerivedData*     pDerivedData        //  =>
)
    {
    if (NULL == pDerivedData)
        return ERROR;

    pDerivedData->numSegments = DimensionHandler::GetInstance().GetNumSegments (element);

    if (0 >= pDerivedData->numSegments)
        {
        BeAssert (false);
        return ERROR;
        }

    /*--------------------------------------------------------------------------
    * Any memory allocated here must be freed in mdlDimDerivedData_free
    *-------------------------------------------------------------------------*/

    if (DIMDERIVEDDATA_FLAGS_TERMDIR & pDerivedData->flags)
        pDerivedData->pTermDirs =  (DimTermDirs *) calloc (pDerivedData->numSegments, sizeof (DimTermDirs));

    if (DIMDERIVEDDATA_FLAGS_TEXTDOCKED & pDerivedData->flags)
        {
        int iSegment;

        pDerivedData->pIsTextDocked = (bool*) calloc (pDerivedData->numSegments, sizeof (bool));

        for (iSegment = 0; iSegment < pDerivedData->numSegments; iSegment++)
            pDerivedData->pIsTextDocked[iSegment] = true;
        }

    if (DIMDERIVEDDATA_FLAGS_TEXTOUTSIDE & pDerivedData->flags)
        pDerivedData->pIsTextOutside = (bool*) calloc (pDerivedData->numSegments, sizeof (bool));

    if (DIMDERIVEDDATA_FLAGS_ARCDEFPOINT & pDerivedData->flags)
        pDerivedData->pArcDefPoint = (DPoint3d*) calloc (1, sizeof (DPoint3d));

    if (DIMDERIVEDDATA_FLAGS_TEXTBOX & pDerivedData->flags)
        pDerivedData->pTextBoxes = (AdimSegmentTextBoxes *) calloc (pDerivedData->numSegments, sizeof (AdimSegmentTextBoxes));

    if (DIMDERIVEDDATA_FLAGS_CHAINTYPE & pDerivedData->flags)
        pDerivedData->pChainType = (UInt16*) calloc (pDerivedData->numSegments, sizeof (UInt16));

    if (DIMDERIVEDDATA_FLAGS_DIMVALUES & pDerivedData->flags)
        {
        pDerivedData->pDimValues = (double*) calloc (pDerivedData->numSegments, sizeof (double));
        ap->pdDimLengths = pDerivedData->pDimValues;
        }

    if (DIMDERIVEDDATA_FLAGS_SUPERSCRIPTED & pDerivedData->flags)
        pDerivedData->pSuperscripted = (bool*) calloc (pDerivedData->numSegments, sizeof (bool));

    if (DIMDERIVEDDATA_FLAGS_DELIMITEROMITTED & pDerivedData->flags)
        pDerivedData->pDelimiterOmitted = (bool*) calloc (pDerivedData->numSegments, sizeof (bool));

    ap->pDerivedData = pDerivedData;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDimDerivedData_getTermDir
(
DPoint3d        *pDir,                  // <=
DimDerivedDataP pDerivedData,           //  =>
bool            left,                   //  => true = left, false = right
int             iSegment                //  =>
)
    {
    StatusInt       status = ERROR;

    if (NULL == pDir || NULL == pDerivedData || NULL == pDerivedData->pTermDirs ||
        0 > iSegment ||
        pDerivedData->numSegments <= iSegment)
        {
        return ERROR;
        }

    if (left)
        {
        if (pDerivedData->pTermDirs[iSegment].flags.hasLeftTerm)
            {
            *pDir = pDerivedData->pTermDirs[iSegment].leftDir;
            status = SUCCESS;
            }
        }
    else
        {
        if (pDerivedData->pTermDirs[iSegment].flags.hasRightTerm)
            {
            *pDir = pDerivedData->pTermDirs[iSegment].rightDir;
            status = SUCCESS;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDimDerivedData_getIsTextDocked
(
bool            *pIsTextDocked,         // <=
DimDerivedDataP pDerivedData,           //  =>
int             iSegment                //  =>
)
    {
    if (NULL == pIsTextDocked || NULL == pDerivedData || NULL == pDerivedData->pIsTextDocked ||
        0 > iSegment ||
        pDerivedData->numSegments <= iSegment)
        {
        return ERROR;
        }

    *pIsTextDocked = pDerivedData->pIsTextDocked[iSegment];

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlDimDerivedData_getArcDefPoint
(
DimDerivedDataP     pDerivedData,           // =>
DPoint3d           *pArcDefPoint            // <=
)
    {
    if (NULL == pArcDefPoint || NULL == pDerivedData || NULL == pDerivedData->pArcDefPoint)
        {
        return ERROR;
        }

    *pArcDefPoint = *(pDerivedData->pArcDefPoint);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlDimDerivedData_getTextBox
(
DimDerivedDataP     pDerivedData,           //  =>
DPoint3dP           pOrigin,                //  <=
DPoint3dP           pXVec,                  //  <=
DPoint3dP           pZVec,                  //  <=
DPoint2dP           pSize,                  //  <=
bool                primary,                //  =>
int                 iSegment                //  =>
)
    {
    AdimRotatedTextBox  *pTextBox = NULL;

    if (NULL == pDerivedData || NULL == pDerivedData->pTextBoxes ||
        0 > iSegment ||
        pDerivedData->numSegments <= iSegment)
        {
        return ERROR;
        }

    if (primary && pDerivedData->pTextBoxes[iSegment].flags.hasPrimary)
        pTextBox = &pDerivedData->pTextBoxes[iSegment].primary;
    else
    if ( ! primary && pDerivedData->pTextBoxes[iSegment].flags.hasSecondary)
        pTextBox = &pDerivedData->pTextBoxes[iSegment].secondary;

    if (NULL == pTextBox)
        return ERROR;

    if (pOrigin)
        *pOrigin = pTextBox->baseFirstChar;

    if (pXVec)
        *pXVec = pTextBox->baseDir;

    if (pZVec)
        *pZVec = pTextBox->zvec;

    if (pSize)
        {
        pSize->y = pTextBox->height;
        pSize->x = pTextBox->width;
        }

    return SUCCESS;
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JVB             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     adim_clearGraphicGroup
(
DgnElementP      pElement,
void           *pUserData,
int             op,
UInt32          filePos,
MSElementDescr *edP
)
    {
    pElement->hdr.dhdr.grphgrp  = 0;

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @return       what the return value means
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       adim_scaleMLText
(
EditElementHandleR dimElement,
const double    scale
)
    {
    int             iPnt;
    DimMLText*      pText = NULL;

    for (iPnt = 0; iPnt < dimElement.GetElementCP()->ToDimensionElm().nPoints - 1; iPnt++)
        {
        mdlDimText_create (&pText);
        mdlDim_getText (pText, dimElement, iPnt);

        mdlDimText_scale (pText, scale);

        mdlDim_deleteText (dimElement, iPnt);
        mdlDim_setText (dimElement, pText, iPnt);

        mdlDimText_free (&pText);
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   adim_scaleDimPointDistances
(
EditElementHandleR dimensionElement,
const double    scale
)
    {
    Transform trans;
    trans.InitIdentity ();
    trans.ScaleMatrixColumns (trans,  scale,  scale,  scale);

    DimensionElm * dim = &dimensionElement.GetElementP()->ToDimensionElmR();
    DPoint3d pt0 = dim->GetPoint(0);
    bsiTransform_setFixedPoint (&trans, &pt0);

    RotMatrix rMatrix;
    DimensionHandler::GetInstance().GetRotationMatrix (dimensionElement, rMatrix);
    //TODO TEST The scaling operation would fail for radial relative dimensions
    adim_transformDimPoints (dimensionElement, &rMatrix, &trans, false, true);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void     scaleDimProp (double& value, double scale, DimStyleProp prop, DimStylePropMaskR shields)
    {
    value = value * scale;
    shields.SetPropertyBit (prop, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      DimensionHandler::ScaleElement (EditElementHandleR element, double scale, bool modifyAllowed,
                                                bool setShields, bool updateRange)
    {
    if (scale == 1.0)
        return SUCCESS;

    LegacyTextStyle           textStyle;

    if (SUCCESS == GetTextStyle (element, &textStyle))
        {
        textStyle.Scale(scale);

        SetTextStyle (element, &textStyle, setShields, modifyAllowed);
        }

    DimStylePropMaskPtr shieldFlags = GetOverrideFlags (element);
    DimensionElmR  dim = element.GetElementP()->ToDimensionElmR();

    scaleDimProp (dim.geom.witOffset,   scale, DIMSTYLE_PROP_ExtensionLine_Offset_DOUBLE,   *shieldFlags);
    scaleDimProp (dim.geom.witExtend,   scale, DIMSTYLE_PROP_ExtensionLine_Extend_DOUBLE,   *shieldFlags);
    scaleDimProp (dim.geom.textLift,    scale, DIMSTYLE_PROP_Text_VerticalMargin_DOUBLE,    *shieldFlags);
    scaleDimProp (dim.geom.textMargin,  scale, DIMSTYLE_PROP_Text_HorizontalMargin_DOUBLE,  *shieldFlags);
    scaleDimProp (dim.geom.margin,      scale, DIMSTYLE_PROP_Terminator_MinLeader_DOUBLE,   *shieldFlags);
    scaleDimProp (dim.geom.termWidth,   scale, DIMSTYLE_PROP_Terminator_Width_DOUBLE,       *shieldFlags);
    scaleDimProp (dim.geom.termHeight,  scale, DIMSTYLE_PROP_Terminator_Height_DOUBLE,      *shieldFlags);
    scaleDimProp (dim.geom.stackOffset, scale, DIMSTYLE_PROP_General_StackOffset_DOUBLE,    *shieldFlags);
    scaleDimProp (dim.geom.centerSize,  scale, DIMSTYLE_PROP_General_CenterMarkSize_DOUBLE, *shieldFlags);

    DimTolrBlock*   tolBlock = (DimTolrBlock*)mdlDim_getOptionBlock (element, ADBLK_TOLERANCE, NULL);

    if (NULL != tolBlock)
        {
        scaleDimProp (tolBlock->tolWidth,    scale, DIMSTYLE_PROP_Tolerance_TextScale_DOUBLE,              *shieldFlags);
        scaleDimProp (tolBlock->tolHeight,   scale, DIMSTYLE_PROP_Tolerance_TextScale_DOUBLE,              *shieldFlags);
        scaleDimProp (tolBlock->tolHorizSep, scale, DIMSTYLE_PROP_Tolerance_TextHorizontalMargin_DOUBLE,   *shieldFlags);
        scaleDimProp (tolBlock->tolVertSep,  scale, DIMSTYLE_PROP_Tolerance_TextVerticalSeparation_DOUBLE, *shieldFlags);
        }

    SaveShieldsDirect (element, *shieldFlags);

    DimTermBlock*   termBlock = (DimTermBlock*)mdlDim_getOptionBlock (element, ADBLK_TERMINATOR, NULL);

    if (NULL != termBlock)
        {
        termBlock->scScale *= scale;
        }

    // Ball and chain elbow length is stored as a distance instead of a scale factor. Apply the scale to it.
    DimStyleExtensions     styleExt;
    memset (&styleExt, 0, sizeof (styleExt));
    mdlDim_getStyleExtension (&styleExt, element);
    styleExt.dBncElbowLength *= scale;
    mdlDim_setStyleExtension (element, &styleExt);

    if (updateRange && GetNumPoints(element)  > 1)
        ValidateElementRange (element);

    // scale mltexts too
    adim_scaleMLText (element, scale);

    // scale distances stored in dim points
    adim_scaleDimPointDistances (element, scale);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/94
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::mdlDim_scale2
(
EditElementHandleR dimElement,           /* <=> dimension element to scale */
double          scale,          /*  => scale factor */
bool         modifyAllowed,  /*  => allowed to change element size */
bool         setShields,     /*  => protect scaled sizes from dimstyle */
bool         updateRange     /*  => update the element range */
)
    {
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
    return hdlr->ScaleElement (dimElement, scale, modifyAllowed, setShields, updateRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/94
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::mdlDim_scale
(
EditElementHandleR dimElement,     /* <=> dimension element to scale */
double          scale,          /*  => scale factor */
bool         modifyAllowed   /*  => allowed to change element size */
)
    {
    return mdlDim_scale2 (dimElement, scale, modifyAllowed, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 02/07
+---------------+---------------+---------------+---------------+---------------+------*/
static double  getScaleFromTransform
(
TransformCP     pTrans,
bool            treatAs2d
)
    {
    DPoint3d    scaleVector;

    LegacyMath::TMatrix::FromNormalizedRowsOfTMatrix (NULL, &scaleVector, pTrans);

    if (treatAs2d)
        return ((scaleVector.x + scaleVector.y) / 2.0);

    /* 3d->2d passes flatten transform, so dividing scaleVector by 3 gives wrong scaleFactor */
    if (scaleVector.x != 0.0 && scaleVector.y != 0.0 && scaleVector.z != 0.0)
        return ((scaleVector.x + scaleVector.y + scaleVector.z) / 3.0);
    else
        return ((scaleVector.x + scaleVector.y + scaleVector.z) / 2.0);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          getUnitScale                                            |
|                                                                       |
| author        JoshSchifter                              11/01         |
|                                                                       |
+----------------------------------------------------------------------*/
double       BentleyApi::adim_getUnitScale
(
DgnModelP        srcDgnModel,
DgnModelP        dstDgnModel,
const Transform    *pTrans          /* Used when src or dest is a cell model ref */
)
    {
    double          scale = 1.0;

    /* The scale is kind of funny.  For reference files, want to scale by the inverse so that they
       will look correct when then scaled by the reference scale; but for cell libraries want to scale
       by the forward so that they will be the correct size */

    //if (dstDgnModel->IsDgnAttachment())
    //    {
    //    DgnAttachmentCP destRefP = dstDgnModel->AsDgnAttachmentCP();
    //    scale = 1.0 / destRefP->GetDisplayScale();
    //    }
    //else 
    if (NULL != pTrans)
        {
        scale = getScaleFromTransform (pTrans, false);
        }

    // Compute the scale. Do not attempt to incorporate ref display scale here.
    // It will be applied separately during cloning. This function is supposed
    // to handle only unit conversions.
    if (/*!srcDgnModel->IsDgnAttachment() && */ NULL != pTrans)
        scale *= getScaleFromTransform (pTrans, false);

    return scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/94
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::adim_transformInternalParameters
(
EditElementHandleR dimElement,              /* <=> element to transform */
DgnModelP    srcDgnModel,
DgnModelP    dstDgnModel,
const Transform *pTrans,            /*  => Used when src or dest is a cell model ref */
bool         modifyAllowed       /*  => allowed to change element size */
)
    {
    double      scale = 1.0;

    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
    if (NULL == hdlr)
        return ERROR;

    if (srcDgnModel == dstDgnModel)
        scale = getScaleFromTransform (pTrans, !dimElement.GetElementCP()->Is3d());
    else
        scale = adim_getUnitScale (srcDgnModel, dstDgnModel, pTrans);

    if (fabs (scale - 1.0) > mgds_fc_epsilon)
        mdlDim_scale (dimElement, scale, modifyAllowed);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
int             BentleyApi::mdlDim_fixRadialPoints (EditElementHandleR dimElement, DisplayPathCP pathP)
    {
    if (DIMENSION_ELM != dimElement.GetLegacyType() || !DIM_RADIAL (dimElement.GetElementCP()->ToDimensionElm().dimcmd))
        return SUCCESS;
    
    bool isAssociative = dimElement.GetElementCP()->ToDimensionElm().GetDimTextCP(0)->flags.b.associative;
    if ( ! isAssociative && NULL == pathP)
        return SUCCESS;

    DimArcInfo      dimArc;
    if (isAssociative)
        {
        AssocPoint      assoc;
        RotMatrix       rMatrix;
        
        if (SUCCESS != AssociativePoint::ExtractPoint (assoc, dimElement, 0, dimElement.GetElementCP()->ToDimensionElm().nPoints))
            return ERROR;

        if (SUCCESS != BentleyApi::adim_getArcInfo (&dimArc, &rMatrix, NULL, &assoc, dimElement))
            return ERROR;

        if (ARC_ASSOC != ((AssocGeom*) (&assoc))->type || dimArc.notCircular)
            return SUCCESS;
        }
    else if (pathP)
        {
        RotMatrix       rMatrix;
        
        if (SUCCESS != BentleyApi::adim_getArcInfo (&dimArc, &rMatrix, pathP, NULL, dimElement))
            return ERROR;

        if (!dimArc.notCircular)
            return SUCCESS;
        }
    
    /* set up center (point 0) */
    dimElement.GetElementP()->ToDimensionElmR().SetPoint(dimArc.center, 0);

    /* set up point on arc (point 1) */
    DVec3d    lineVec;
    lineVec.NormalizedDifference (dimElement.GetElementP()->ToDimensionElm().GetPoint(2), dimElement.GetElementP()->ToDimensionElm().GetPoint(0));
    
    DPoint3d scaledResult;
    scaledResult.SumOf (dimElement.GetElementP()->ToDimensionElm().GetPoint(0), lineVec, dimArc.radius);
    dimElement.GetElementP()->ToDimensionElmR().SetPoint (scaledResult, 1);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     05/90
+---------------+---------------+---------------+---------------+---------------+------*/
int             LabelLineHelper::ReEvaluateElement (EditElementHandleR element) const
    {
    if (DIMENSION_ELM != element.GetLegacyType() || DimensionType::LabelLine != static_cast<DimensionType>(element.GetElementCP()->ToDimensionElm().dimcmd))
        return SUCCESS;

    if (element.GetElementCP()->ToDimensionElm().nPoints < 2)
        return ERROR;

    if (element.GetElementCP()->ToDimensionElm().GetDimTextCP(0)->flags.b.associative)
        {
        DPoint3d    txtOrg;
        DVec3d      segDir, orgDir;
        DSegment3d  dPoints;
        double      offset;
        AssocPoint  assoc;

        BentleyApi::mdlDim_extractPointsD (&txtOrg, element, 0, 1);

        if (SUCCESS != AssociativePoint::ExtractPoint (assoc, element, 0, element.GetElementCP()->ToDimensionElm().nPoints) ||
            SUCCESS != AssociativePoint::GetAssocPointSegment (dPoints, assoc, element.GetDgnModelP()))
            return ERROR;

        segDir.NormalizedDifference (*( &dPoints.point[1]), *( &dPoints.point[0]));
        bsiDVec3d_subtractDPoint3dDPoint3d (&orgDir, &txtOrg, &dPoints.point[0]);
        offset = bsiDVec3d_dotProduct (&segDir, &orgDir);

        element.GetElementP()->ToDimensionElmR().SetPoint (dPoints.point[0], 0);
        element.GetElementP()->ToDimensionElmR().SetPoint (dPoints.point[1], 1);
        element.GetElementP()->ToDimensionElmR().GetDimTextP(0)->offset = offset;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      08/94
+---------------+---------------+---------------+---------------+---------------+------*/
bool  BentleyApi::mdlDim_isTextOutside (ElementHandleCR dimElement)
    {
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
     if (NULL == hdlr)
        {
        BeAssert (false);
        return false;
        }

    return hdlr->IsTextOutside (dimElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen     05/00
+---------------+---------------+---------------+---------------+---------------+------*/
double   BentleyApi::mdlDim_getStackHeight
(
ElementHandleCR    dimElement,
int             segNo
)
    {
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
    double stackHeight = 0.0;
    if (NULL == hdlr)
        {
        BeAssert (false);
        return stackHeight;
        }

    BentleyStatus status = hdlr->GetStackHeight (dimElement, segNo, stackHeight);
    BeAssert (SUCCESS == status);
    return stackHeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DimensionHandler::_IsTextOutside (ElementHandleCR eh) const
    {
    NullOutput  output;
    NullContext context (&output);

    context.SetDgnProject (*eh.GetDgnProject());

    AdimProcess ap(eh, &context);

    ap.proxyOverride = DIMPROXY_OVERRIDE_NoProxy;

    adim_strokeDimension (ap);

    return ap.flags.pushTextOutside;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDim_getDerivedData
(
DimDerivedData  *pDerivedData,      // <=
ElementHandleCR    element             // =>
)
    {
    if (NULL == pDerivedData)
        return DGNHANDLERS_STATUS_BadArg;

    NullOutput  output;
    NullContext context (&output);

    context.SetDgnProject (*element.GetDgnProject());

    AdimProcess ap(element, &context);
    ap.proxyOverride = DIMPROXY_OVERRIDE_NoProxy;
    adim_setDerivedDataBuffers (&ap, element, pDerivedData);

    return adim_strokeDimension (ap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_hasLeaderedText
(
BitMaskP        pBitMask,
ElementHandleCR    dimElement
)
    {
    bool            needToFreeMask = false, anyBitsSet = false;
    
    if (NULL == pBitMask)
        {
        pBitMask = BitMask::Create ( false);
        needToFreeMask = true;
        }

    NullOutput  output;
    NullContext context (&output);

    context.SetDgnProject (*dimElement.GetDgnProject());
    
    AdimProcess     ap(dimElement, &context);

    ap.proxyOverride = DIMPROXY_OVERRIDE_NoProxy;
    ap.pLeaderedMask = pBitMask;

    anyBitsSet = pBitMask->AnyBitSet ();

    if (needToFreeMask)
        pBitMask->Free ();

    return anyBitsSet;
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 03/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    transformHasScale
(
Transform const* trans
)
    {
    DVec3d      xcol, ycol, zcol;

    trans->GetMatrixColumn (xcol,  0);
    trans->GetMatrixColumn (ycol,  1);
    trans->GetMatrixColumn (zcol,  2);

    double xscale = bsiDVec3d_magnitude (&xcol);
    double yscale = bsiDVec3d_magnitude (&ycol);
    double zscale = bsiDVec3d_magnitude (&zcol);

    return !LegacyMath::DEqual (xscale, 1.0) || !LegacyMath::DEqual (yscale, 1.0) || !LegacyMath::DEqual (zscale, 1.0);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    transformDimScalar
(
double          *scalar,
DVec3d          *directionIn,
Transform const* trans,
bool            is3d
)
    {
    DVec3d      direction;

    if (*scalar == 0.0)
        return;

    direction = *directionIn;
    trans->MultiplyMatrixOnly (direction);

    *scalar = *scalar * bsiDVec3d_magnitude (&direction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     10/90
+---------------+---------------+---------------+---------------+---------------+------*/
void         BentleyApi::adim_transformDimPoints
(
EditElementHandleR     dimElement,
RotMatrix const*    directionMatrix,
Transform const*    trans,
bool                transformPoints,
bool                transformDistances
)
    {
    DPoint3d        point;
    DVec3d          xCol;
    DVec3d          yCol;
    Transform       modifiedTransform = *trans;

    if (!transformPoints && !transformDistances)
        return;

#ifdef NEEDSWORK

    // This approach breaks cloning of notes from scaled and rotated references.
    // For 8.9.4, do not treat the distance between leader endpoints as a length
    // that is managed by annotation scale.

    // For notes, the gaps between dim points are treated as a factor of text height
    if (mdlDim_isNoteDimension (dim))
        {
        // If distances are not supposed to be transformed (scaled), then remove the scale portion
        // from the transform
        if (!transformDistances && transformHasScale (trans))
            {
            DPoint3d origin;
            BentleyApi::mdlDim_extractPointsD (&origin, dim, modelRef, 0, 1);
            LegacyMath::TMatrix::Unscale (&modifiedTransform, &modifiedTransform, &origin);
            }
        }
#endif

    directionMatrix->GetColumn(xCol,  0);
    directionMatrix->GetColumn(yCol,  1);
    for (int iPoint=0; iPoint< dimElement.GetElementP()->ToDimensionElm().nPoints; iPoint++)
        {
        if (transformPoints)
            {
            /* Figure we should do this always...because association is no
            longer going to be evaluated to display dimension */
            BentleyApi::mdlDim_extractPointsD (&point, dimElement, iPoint, 1);
            modifiedTransform.Multiply(point);
            dimElement.GetElementP()->ToDimensionElmR().SetPoint (point, iPoint);

            // Labelline's text.offset is used to locate the text. Scaling it independent of its root element will cause
            // a mismatch between text origin and the corresponding assoc point, resulting in broken association
            // (look at depcallback.c\getDimensionComparePoint). Therefore, treat it as a point value and not as a distance.
            if (DIM_LABELLINE (static_cast<DimensionType>(dimElement.GetElementP()->ToDimensionElm().dimcmd)))
                transformDimScalar (&dimElement.GetElementP()->ToDimensionElmR().GetDimTextP(iPoint)->offset, &xCol, &modifiedTransform, dimElement.GetElementP()->ToDimensionElm().Is3d());
            }

        if (transformDistances)
            {
            DimensionType   dimensionType = static_cast<DimensionType>(dimElement.GetElementP()->ToDimensionElm().dimcmd);
            if (DIM_LINEAR (dimensionType) && iPoint == 0)
                transformDimScalar (&dimElement.GetElementP()->ToDimensionElmR().GetDimTextP(0)->offset, &yCol, &modifiedTransform, dimElement.GetElementP()->ToDimensionElm().Is3d());
            else if (!DIM_LABELLINE (dimensionType))
                transformDimScalar (&dimElement.GetElementP()->ToDimensionElmR().GetDimTextP(iPoint)->offset, &xCol, &modifiedTransform, dimElement.GetElementP()->ToDimensionElm().Is3d());

            transformDimScalar (&dimElement.GetElementP()->ToDimensionElmR().GetDimTextP(iPoint)->offsetY, &yCol, &modifiedTransform, dimElement.GetElementP()->ToDimensionElm().Is3d());
            }
        }
    }

