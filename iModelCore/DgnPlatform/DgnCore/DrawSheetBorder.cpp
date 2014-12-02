/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DrawSheetBorder.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnCore/SheetDef.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    12/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     translatePointArray (DPoint3dP pointArrayIn, int numPointsIn, double xDeltaIn, double yDeltaIn)
    {
    if (0 == xDeltaIn && 0 == yDeltaIn)
        return;

    DPoint3d    translation;
    Transform   pointTransform;

    translation.Init (xDeltaIn, yDeltaIn, 0.0);
    pointTransform.InitIdentity ();
    pointTransform.SetTranslation (translation);
    pointTransform.Multiply (pointArrayIn, numPointsIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    12/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     rotatePointArray (DPoint3dP pointArrayIn, int numPointsIn, double rotationIn)
    {
    if (0 == rotationIn)
        return;

    Transform  pointTransform;

    pointTransform.InitIdentity ();
    pointTransform.InitFromPrincipleAxisRotations (pointTransform, 0.0, 0.0, rotationIn);
    pointTransform.Multiply (pointArrayIn, numPointsIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    12/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     scalePointArray (DPoint3dP pointArrayIn, int numPointsIn, double scaleIn)
    {
    if (1.0 == scaleIn)
        return;

    Transform   pointTransform;

    pointTransform.InitIdentity ();
    pointTransform.ScaleMatrixColumns (pointTransform, scaleIn, scaleIn, 1.0);
    pointTransform.Multiply (pointArrayIn, numPointsIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    12/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     getRectanglePoints
(
DPoint3dP       points,
DPoint2dCR      baseOrigin,
DPoint2dCR      origin,
double          width,
double          height,
double          rotation,
double          scale
)
    {
    memset (points, 0, 5 * sizeof (DPoint3d));

    points[1].x = points[2].x = width;
    points[2].y = points[3].y = height;

    translatePointArray (points, 5, origin.x - baseOrigin.x, origin.y - baseOrigin.y);
    rotatePointArray (points, 5, rotation);
    scalePointArray (points, 5, scale);
    translatePointArray (points, 5, baseOrigin.x - origin.x, baseOrigin.y - origin.y);
    translatePointArray (points, 5, origin.x, origin.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void     getShadowPoints
(
DPoint3dP       points,
DPoint2dCR      baseOrigin,
DPoint2dCR      origin,
double          width,
double          height,
double          rotation,
double          scale,
double          shadowWidth
)
    {
    memset (points, 0, 7 * sizeof (DPoint3d));

    points[0].x = shadowWidth;
    points[1].x = width;

    points[2].x = width;
    points[2].y = height - shadowWidth;

    points[3].x = width + shadowWidth;
    points[3].y = height - shadowWidth;

    points[4].x = width + shadowWidth;
    points[4].y = -shadowWidth;

    points[5].x = shadowWidth;
    points[5].y = -shadowWidth;

    points[6].x = shadowWidth;

    translatePointArray (points, 7, origin.x - baseOrigin.x, origin.y - baseOrigin.y);
    rotatePointArray (points, 7, rotation);
    scalePointArray (points, 7, scale);
    translatePointArray (points, 7, baseOrigin.x - origin.x, baseOrigin.y - origin.y);
    translatePointArray (points, 7, origin.x, origin.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt getPaperRectangle
(
DPoint2dR       baseOrigin,
DPoint2dR       paperOrigin,
double&         paperWidth,
double&         paperHeight,
double&         paperRotation,
SheetDefCP      sheetDef
)
    {
    sheetDef->GetPaperOrigin (paperOrigin);
    baseOrigin.Zero ();
    sheetDef->GetPaperSize (paperWidth, paperHeight);
    paperRotation = sheetDef->GetPaperRotation ();

    if (paperWidth && paperHeight)
        return SUCCESS;

    sheetDef->GetOrigin (paperOrigin);
    baseOrigin = paperOrigin;
    sheetDef->GetSize (paperWidth, paperHeight);
    paperRotation = sheetDef->GetRotation (); // If sizes come from sheet, rotation also comes from sheet

    double      topMargin, leftMargin, bottomMargin, rightMargin;

    sheetDef->GetPaperMargins (topMargin, leftMargin, bottomMargin, rightMargin);

    paperOrigin.x -= leftMargin;
    paperOrigin.y -= bottomMargin;

    paperWidth  += leftMargin + rightMargin;
    paperHeight += bottomMargin + topMargin;

    return (paperWidth && paperHeight) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt getPrintableRectangle
(
DPoint2dR       printableOrigin,
double&         printableWidth,
double&         printableHeight,
DPoint2dCR      paperOrigin,
double          paperWidth,
double          paperHeight,
SheetDefCP      sheetDef
)
    {
    if (0 == paperWidth || 0 == paperHeight)
        return ERROR;

    double      topMargin, leftMargin, bottomMargin, rightMargin;

    sheetDef->GetPaperMargins (topMargin, leftMargin, bottomMargin, rightMargin);

    printableOrigin = paperOrigin;
    printableOrigin.x += leftMargin;
    printableOrigin.y += bottomMargin;

    printableWidth  = paperWidth - leftMargin - rightMargin;
    printableHeight = paperHeight - topMargin - bottomMargin;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DeepakMalkan    11/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt getSheetRectangle
(
DPoint2dR       sheetOrigin,
double&         sheetWidth,
double&         sheetHeight,
double&         sheetRotation,
SheetDefCP      sheetDef
)
    {
    sheetDef->GetOrigin (sheetOrigin);
    sheetDef->GetSize (sheetWidth, sheetHeight);
    sheetRotation = sheetDef->GetRotation ();

    return (sheetWidth && sheetHeight) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            SheetDef::Draw (ViewContextP context) const
    {
    if (!IsEnabled ())
        return;

    if (DrawPurpose::Plot == context->GetDrawPurpose ())
        return; // Sheet border never plots...

    ViewportP   vp = context->GetViewport ();

    if (NULL == vp)
        return;

    IViewDrawR  viewDraw = context->GetIViewDraw ();
    double      paperWidth, paperHeight, paperRotation;
    DPoint2d    baseOrigin, paperOrigin;

    if (SUCCESS != getPaperRectangle (baseOrigin, paperOrigin, paperWidth, paperHeight, paperRotation, this))
        return;

    double      annotationScale = 1.0;//context->GetCurrentModel ()->GetAnnotationScaleFactor ();
    double      plotScale = GetPlotScaleFactor ();
    double      paperScale = plotScale * annotationScale;
    DPoint3d    outlinePoints[5], shadowPoints[7];

    getRectanglePoints (outlinePoints, baseOrigin, paperOrigin, paperWidth, paperHeight, paperRotation, paperScale);
    getShadowPoints (shadowPoints, baseOrigin, paperOrigin, paperWidth, paperHeight, paperRotation, paperScale, (paperWidth < paperHeight ? paperWidth : paperHeight) / 75);

    bool        drawPrintRect = false;
    double      printableWidth, printableHeight;
    DPoint2d    printableOrigin;
    DPoint3d    printPoints[5];

    if (SUCCESS == getPrintableRectangle (printableOrigin, printableWidth, printableHeight, paperOrigin, paperWidth, paperHeight, this))
        {
        if (drawPrintRect = (printableOrigin.x != paperOrigin.x || printableOrigin.y != paperOrigin.y || printableWidth != paperWidth || printableHeight != paperHeight))
            getRectanglePoints (printPoints, baseOrigin, printableOrigin, printableWidth, printableHeight, paperRotation, paperScale);
        }

    bool        drawSheetRect = false;
    double      sheetWidth, sheetHeight, sheetRotation;
    DPoint2d    sheetOrigin;
    DPoint3d    sheetPoints[5];

    if (SUCCESS == getSheetRectangle (sheetOrigin, sheetWidth, sheetHeight, sheetRotation, this))
        {
        if (drawSheetRect = (sheetOrigin.x != paperOrigin.x || sheetOrigin.y != paperOrigin.y || sheetWidth != paperWidth || sheetHeight != paperHeight))
            getRectanglePoints (sheetPoints, sheetOrigin, sheetOrigin, sheetWidth, sheetHeight, sheetRotation, annotationScale);
        }

    // Don't draw sheet and print rects if they are the same...just draw sheet rect...
    if (drawSheetRect && drawPrintRect)
        drawPrintRect = (printableOrigin.x != sheetOrigin.x || printableOrigin.y != sheetOrigin.y || printableWidth != sheetWidth || printableHeight != sheetHeight);

    context->GetOverrideMatSymb ()->SetFlags (MATSYMB_OVERRIDE_None);
    context->ActivateOverrideMatSymb ();

#ifdef COMMENT_OUT
    IPickGeom*  pickGeom;

    if (NULL != (pickGeom = context->GetIPickGeom ()))
        {
        pickGeom->GetGeomDetail().Init ();

        DgnModelR   dgnModel = *context->GetCurrentModel()->GetDgnModelP();

        if (NULL == SheetBoundaryAppData::GetFakeElementRef (dgnModel))
            {
            WString  descr;

            GetFormName (descr);
            SheetBoundaryAppData::CreateAndStoreFakeElementRef (dgnModel, descr.c_str ());
            }

        context->PushPath (SheetBoundaryAppData::GetFakeElementRef(dgnModel));

        viewDraw.DrawLineString3d (5, outlinePoints, NULL);

        if (drawPrintRect)
            viewDraw.DrawLineString3d (5, printPoints, NULL);

        if (drawSheetRect)
            viewDraw.DrawLineString3d (5, sheetPoints, NULL);

        context->PopPath ();

        return;
        }
#endif

    ElemMatSymbP    matSymb = context->GetElemMatSymb ();
    UInt32          color = vp->GetContrastToBackgroundColor ();

    matSymb->Init ();
    matSymb->SetLineColorTBGR (color);
    matSymb->SetFillColorTBGR (color);
    matSymb->SetWidth (2);
    matSymb->SetRasterPattern (0);

    GradientSymbPtr gradient = GradientSymb::Create();
    double          keyValues[2];
    RgbColorDef     keyColors[2];

    keyValues[0] = 0.0;
    keyValues[1] = 0.5;

    keyColors[0].red = keyColors[0].green = keyColors[0].blue = 25;
    keyColors[1].red = keyColors[1].green = keyColors[1].blue = 150;

    gradient->SetMode (GradientMode::Linear);
    gradient->SetFlags (0);
    gradient->SetAngle (-45.0);
    gradient->SetTint (0.0);
    gradient->SetShift (0.0);
    gradient->SetKeys (2, keyColors, keyValues);
    matSymb->SetGradient (gradient.get());

    viewDraw.ActivateMatSymb (matSymb);

    viewDraw.DrawShape3d (7, shadowPoints, true, NULL);
    viewDraw.DrawLineString3d (5, outlinePoints, NULL);

    if (drawPrintRect)
        {
        matSymb->SetWidth (1);
        matSymb->SetIndexedRasterPattern (1, context->GetIndexedLinePattern (1));
        viewDraw.ActivateMatSymb (matSymb);

        viewDraw.DrawLineString3d (5, printPoints, NULL);
        }

    if (drawSheetRect)
        {
        matSymb->SetWidth (1);
        matSymb->SetIndexedRasterPattern (2, context->GetIndexedLinePattern (2));
        viewDraw.ActivateMatSymb (matSymb);

        viewDraw.DrawLineString3d (5, sheetPoints, NULL);

        matSymb->SetWidth (2);
        matSymb->SetRasterPattern (0);
        viewDraw.ActivateMatSymb (matSymb);

        for (size_t iTab = 0; iTab < 4; iTab++)
            {
            double      defaultTabSizeFactor = 0.02;
            DPoint3d    tabPoints[3];

            tabPoints[0].Interpolate (sheetPoints[iTab], defaultTabSizeFactor, sheetPoints[iTab+1]);
            tabPoints[1] = sheetPoints[iTab];
            tabPoints[2].Interpolate (sheetPoints[iTab], defaultTabSizeFactor, sheetPoints[iTab ? iTab-1 : 3]);

            viewDraw.DrawLineString3d (3, tabPoints, NULL);
            }
        }
    }


