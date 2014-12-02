/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Dimension/Note.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatformInternal/DgnHandlers/RelativeOffsetAssociation.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#define     fc_uorTol                  0.25

#define CLOSED_TOLERANCE               3        /* 3 uors */

#define TOLERANCE_MinPreservedAngle    (10.0*msGeomConst_radiansPerDegree)

// #define DEBUG_DUMP                  1        // Uncomment to produce debug output


struct  NoteProperties
    {
    double                      dElbowLength;
    double                      dLeftMargin;
    double                      dLowerMargin;
    TextElementJustification    iHorAttachmentJust;
    UInt16                      iVerAttachment;
    int                         iFrameType;
    int                         iHorJust;
    bool                        bHasLeader;
    } ;

struct RectifyProperties
    {
    struct
        {
        UShort          dElbowLength:1;
        UShort          dLeftMargin:1;
        UShort          dLowerMargin:1;
        UShort          iHorAttachmentJust:1;
        UShort          iVerAttachment:1;
        UShort          iFrameType:1;
        UShort          iHorJust:1;
        UShort          bHasLeader:1;
        UShort          reserved:8;
        } flags;

    NoteProperties      props;
    } ;

struct NoteInfo
    {
    int                 frameTypeFromGeom;
    bool                leader;
    ElementHandle       pFrameEdP;
    ElementHandle       pLeaderElm;
    ElementHandle       pCellElm;
    UInt64              nodeID;
    DPoint3d            oldOuterBox[5];
    DPoint3d            oldInnerBox[5];
    DPoint3d            oldLeaderPoints[2];
    DPoint3d            oldOriginOffset;
    };

static byte s_compiled_DataDefID_NoteData[] = 
{
0x70,0x6F,0x76,0x63,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x15,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
0x04,0x00,0x00,0x00,0x0D,0xF0,0xAD,0xBA,0x0D,0xF0,0xAD,0xBA,0x05,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
0x04,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
0x01,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x00,0x00,0x00,0x04,0x00,0x00,0x00,
0x04,0x00,0x00,0x00,0x0D,0xF0,0xAD,0xBA,0x0D,0xF0,0xAD,0xBA,
};

/*-------------------------------------------------------------------------------------*
* @bsimethod                                    SunandSandurkar                 06/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void            getActualTextRotMatrix
(
RotMatrix *             textRotMatrix,
ElementHandleCR            textNode
)
    {
    /*-------------------------------------------------------------------
       Note : The textRotMatrix returned by mdlTextNode_extract does not
       include the backwards and upsidedown aspects because it needs to
       remain right-handed. Get the actual rot matrix from the idocument.
    -------------------------------------------------------------------*/
    TextBlock       textBlock (textNode);

    Transform tMatrix = textBlock.GetTransform ();

    DVec3d          vec[3];
    for (int index = 0; index < 3; index++)
        tMatrix.GetMatrixColumn (vec[index],  index);

    textRotMatrix->InitFromColumnVectors(vec[0], vec[1], vec[2]);
    }

/*-------------------------------------------------------------------------------------*
* @bsimethod                                    SunandSandurkar                 06/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool                areMatricesEqual
(
RotMatrix *         rMatrix1,
RotMatrix *         rMatrix2
)
    {
    double      quat1[4], quat2[4];

    rMatrix1->GetQuaternion(quat1, true);
    rMatrix2->GetQuaternion(quat2, true);

    RotMatrix       rotMatrix1, rotMatrix2;
    rotMatrix1.InitTransposedFromQuaternionWXYZ ( quat1);
    rotMatrix2.InitTransposedFromQuaternionWXYZ ( quat2);

    return bsiRotMatrix_matrixEqualTolerance (&rotMatrix1, &rotMatrix2, mgds_fc_epsilon);
    }

#ifdef UNUSED_FUNCTION
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        dumpCellHeader
(
ElementHandleCR        cellElm,
char *              label
)
    {
#ifdef  DEBUG_DUMP

    if ( ! cellElm.IsValid() || cellElm.GetElementCP()->GetLegacyType() != CELL_HEADER_ELM)
        return;

    DPoint3d        point;
    RotMatrix       rMatrix;

    CellUtil::ExtractOrigin   (point,   cellElm);
    CellUtil::ExtractRotation (rMatrix, cellElm);

    printf ("%hs : Origin  : %f, %f, %f\n", label, point.x, point.y, point.z);

    for (int index = 0; index < 3; index++)
        {
        DVec3d  rowVec;

        rMatrix.GetRow(rowVec,  index);
        printf ("%hs : RMatrix Row %d : %f, %f, %f\n", label, index, point.x, point.y, point.z);
        }

    printf ("\n");

#endif
    }
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        dumpTextNode
(
ElementHandleCP        newTextElemHandle,
char const*              label
)
    {
#ifdef  DEBUG_DUMP

    if (!newTextElemHandle)
        return;

    MSElementDescrCP textNodeEdP = newTextElemHandle->GetElementDescrCP ();
    if (!textNodeEdP || textNodeEdP->el.GetLegacyType() != TEXT_NODE_ELM)
        return;

    DPoint3d    point;
    DPoint2d    size;
    RotMatrix   rMatrix;

    TextNodeHandler* hdlr = dynamic_cast<TextNodeHandler*> (&newTextElemHandle->GetHandler());
    hdlr->GetUserOrigin (*newTextElemHandle, point);
    hdlr->GetOrientation (*newTextElemHandle, rMatrix);
    hdlr->GetFontSize (*newTextElemHandle, size);
    
    printf ("%hs : Origin   : %f, %f, %f\n", label, point.x, point.y, point.z);
    printf ("%hs : TextSize : %f, %f\n", label, size.x, size.y);

    if (textNodeEdP->el.Is3d())
        {
        printf ("%hs : Quat Row 0 : %f, %f\n", label, textNodeEdP->el.text_node_3d.quat[0], textNodeEdP->el.text_node_3d.quat[1]);
        printf ("%hs : Quat Row 1 : %f, %f\n", label, textNodeEdP->el.text_node_3d.quat[2], textNodeEdP->el.text_node_3d.quat[3]);
        }
    else
        {
        double angle = textNodeEdP->el.text_node_2d.rotationAngle;
        printf ("%hs : RotationAngle : %f radians; %f degrees\n", label, angle, angle * (180 / msGeomConst_pi));
        }

    for (int index = 0; index < 3; index++)
        {
        rMatrix.GetRow(point,  index);
        printf ("%hs : Raw Matrix Row %d : %f, %f, %f\n", label, index, point.x, point.y, point.z);
        }

    getActualTextRotMatrix (&rMatrix, *newTextElemHandle);
    for (int index = 0; index < 3; index++)
        {
        rMatrix.GetRow(point,  index);
        printf ("%hs : Actual Matrix Row %d : %f, %f, %f\n", label, index, point.x, point.y, point.z);
        }

    printf ("\n");

#endif
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        dumpCell
(
ElementHandleCR        cellElm,
char const*              label
)
    {
#ifdef  DEBUG_DUMP

    if ( ! cellElm.IsValid() || cellElm.GetElementCP()->GetLegacyType() != CELL_HEADER_ELM)
        return;

    dumpCellHeader (cellElm, label);

    ElementHandle      textElemHandle;
    if (SUCCESS != mdlNote_findTextNodeElement (textElemHandle, cellElm))
        return;

    

    char        newLabel[128] = "";
    strncpy (newLabel, label, 128);
    strcat (newLabel, " : TextNode");
    dumpTextNode (&textElemHandle, newLabel);

#endif
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        dumpDimension
(
ElementHandleCR        dimElemHandle,
char const*              label
)
    {
#ifdef  DEBUG_DUMP

    DgnElementCP     dimensionElm = dimElemHandle.GetElementCP ();
    if (!dimensionElm || dimensionElm->GetLegacyType() != DIMENSION_ELM)
        return;

    DPoint3d        point = dimensionElm->dim.GetPoint(0);
    printf ("%hs : Origin   : %f, %f, %f\n", label, point.x, point.y, point.z);

    point = dimensionElm->dim.GetPoint (dimensionElm->dim.nPoints - 1);
    printf ("%hs : Last Point : %f, %f, %f\n", label, point.x, point.y, point.z);

    RotMatrix       rMatrix;
    DimensionHandler::GetInstance().GetRotationMatrix (dimElemHandle, rMatrix);

    for (int index = 0; index < 3; index++)
        {
        rMatrix.GetRow(point,  index);
        printf ("%hs : Matrix Row %d : %f, %f, %f\n", label, index, point.x, point.y, point.z);
        }

    printf ("\n");

#endif
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/03
+---------------+---------------+---------------+---------------+---------------+------*/
static double      getLineTextHeight
(
ElementHandleCR        textElement,
bool                bFirstLine      // true : First Line; false : Last Line
)
    {
    if (!textElement.IsValid())
        return 0.0;

    TextBlock       textBlock (textElement);

    DRange3d        range;
    bFirstLine ?
        textBlock.GetLineExactLocalRange (range, 0) :
        textBlock.GetLineExactLocalRange (range, textBlock.GetLineCount (textBlock.Begin (), textBlock.End ()) - 1);

    return range.isNull () ? 0.0 : fabs (range.high.y - range.low.y);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BradRushing     09/94
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    getLinePoints
(
DPoint3d *                  linePoints,
DPoint3d *                  shapePointsIn,
TextElementJustification    just
)
    {
    if (NULL == linePoints || NULL == shapePointsIn || DIMSTYLE_VALUE_MLNote_Justification_Center == HORJUSTMODE (just))
        return false;

    if (DIMSTYLE_VALUE_MLNote_Justification_Left == HORJUSTMODE (just))
        {
        linePoints[0] = shapePointsIn[4];
        linePoints[1] = shapePointsIn[3];
        }
    else
        {
        linePoints[0] = shapePointsIn[1];
        linePoints[1] = shapePointsIn[2];
        }

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/06
*
* Note : This function will directly return the pointer to the edP in the cell.
*
*   pTextNodeEd -> next(TextFrameShape) -> next(InlineLeaderLine)
*        |
*         -> firstElem(TextElement1) -> next(TextElement2) ....
*
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt BentleyApi::mdlNote_findCellComponent (ElementHandleR component, int elementType, ElementHandleCR cellEdP)
    {
    if (!cellEdP.IsValid())
        return ERROR;

    for (ChildElemIter child(cellEdP, ExposeChildrenReason::Count); child.IsValid(); child=child.ToNext())
        {
        if (elementType == child.GetLegacyType())
            {
            component = child;
            return SUCCESS;
            }
        }
    return  ERROR;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 01/03
*
* Note : This function will directly return the pointer to the textnode in the cell.
*
*   pTextNodeEd -> next(TextFrameShape) -> next(InlineLeaderLine)
*        |
*         -> firstElem(TextElement1) -> next(TextElement2) ....
*
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt   BentleyApi::mdlNote_findTextNodeElement
(
ElementHandleR  ppTextNodeEd,       // May be NULL
ElementHandleCR pCellEd
)
    {
    return mdlNote_findCellComponent (ppTextNodeEd, TEXT_NODE_ELM, pCellEd);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BradRushing     09/94
+---------------+---------------+---------------+---------------+---------------+------*/
static void    adjustShape
(
DPoint3d        points[],   /* shape to adjust */
double          xMarg,      /* x margin */
double          yMarg       /* y margin */
)
    {
    points[0].x = points[3].x = points[4].x -= xMarg;
    points[1].x = points[2].x += xMarg;
    points[0].y = points[1].y = points[4].y -= yMarg;
    points[2].y = points[3].y += yMarg;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        getTextNodeOrigin
(
DPoint3dR          nodeOrigin,
ElementHandleCR        element            // text node or cell containing text node
)
    {
    ElementHandle textElement;
    if (CELL_HEADER_ELM == element.GetLegacyType() && SUCCESS != mdlNote_findTextNodeElement (textElement, element))
        return;

    TextNodeHandler::GetUserOrigin (textElement, nodeOrigin);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BradRushing     02/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void        extractShape        // returns a 3D range in global coordinates
(
DPoint3d *          shapePointsOut,
ElementHandleCR        element                 // text node or cell containing text node
)
    {
    ElementHandle textElement(element);
    memset (shapePointsOut, 0, 5 * sizeof (DPoint3d));

    if (CELL_HEADER_ELM !=  element.GetLegacyType() && TEXT_NODE_ELM !=  element.GetLegacyType())
        return;

    if (CELL_HEADER_ELM ==  element.GetLegacyType() && SUCCESS != mdlNote_findTextNodeElement (textElement, element))
        return;
    
    TextBlock textBlock (textElement);
    textBlock.GetTextBlockBox (shapePointsOut);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           BentleyApi::mdlNote_getTextNodeJust
(
TextElementJustification&   just,
ElementHandleCR             element     // Node or cell containing node
)
    {
    if (!element.IsValid())
        return ERROR;

    ElementHandle textNode(element);
    if (CELL_HEADER_ELM == element.GetLegacyType() && SUCCESS != mdlNote_findTextNodeElement (textNode, element))
        return ERROR;

    TextParamWide params;
    TextNodeHandler::GetTextParams (textNode, params);
    
    just = static_cast<TextElementJustification>(params.just);
    return SUCCESS;
    }

#ifdef UNUSED_FUNCTION
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   getOriginForNewJustification
(
DPoint3d *          originNew,
DPoint3d *          originOld,
ElementHandleCR        textNode,
int                 newjust,
int                 oldjust
)
    {
    double          xRange = 0.0, yRange = 0.0;
    DPoint3d        shape[5];


    if (!textNode.IsValid() || TEXT_NODE_ELM != textNode.GetLegacyType())
        return ERROR;

    extractShape (shape, textNode);

    xRange = fabs (shape[1].x - shape[0].x);
    yRange = fabs (shape[3].y - shape[0].y);

    memcpy (originNew, originOld, sizeof *originNew);

    // Adjust along the horizontal
    switch (HORJUSTMODE (newjust))
        {
        case DIMSTYLE_VALUE_MLNote_Justification_Left:
            {
            switch (HORJUSTMODE (oldjust))
                {
                case DIMSTYLE_VALUE_MLNote_Justification_Center:
                    originNew->x -= xRange / 2.0;
                    break;

                case DIMSTYLE_VALUE_MLNote_Justification_Right:
                    originNew->x -= xRange;
                    break;
                }
            break;
            }

        case DIMSTYLE_VALUE_MLNote_Justification_Center:
            {
            switch (HORJUSTMODE (oldjust))
                {
                case DIMSTYLE_VALUE_MLNote_Justification_Left:
                    originNew->x += xRange / 2.0;
                    break;

                case DIMSTYLE_VALUE_MLNote_Justification_Right:
                    originNew->x -= xRange / 2.0;
                    break;
                }
            break;
            }

        case DIMSTYLE_VALUE_MLNote_Justification_Right:
            {
            switch (HORJUSTMODE (oldjust))
                {
                case DIMSTYLE_VALUE_MLNote_Justification_Left:
                    originNew->x += xRange;
                    break;

                case DIMSTYLE_VALUE_MLNote_Justification_Center:
                    originNew->x += xRange / 2.0;
                    break;
                }
            break;
            }
        }

    // Adjust along the vertical
    switch (VERJUSTMODE (newjust))
        {
        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Top:
            {
            switch (VERJUSTMODE (oldjust))
                {
                case DIMSTYLE_VALUE_MLNote_VerticalJustification_Center:
                    originNew->y += yRange / 2.0;
                    break;

                case DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom:
                    originNew->y += yRange;
                    break;
                }
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Center:
            {
            switch (VERJUSTMODE (oldjust))
                {
                case DIMSTYLE_VALUE_MLNote_VerticalJustification_Top:
                    originNew->y -= yRange / 2.0;
                    break;

                case DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom:
                    originNew->y += yRange / 2.0;
                    break;
                }
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom:
            {
            switch (VERJUSTMODE (oldjust))
                {
                case DIMSTYLE_VALUE_MLNote_VerticalJustification_Top:
                    originNew->y -= yRange;
                    break;

                case DIMSTYLE_VALUE_MLNote_VerticalJustification_Center:
                    originNew->y -= yRange / 2.0;
                    break;
                }
            break;
            }
        }

    return SUCCESS;
    }
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   setTextNodeJust
(
EditElementHandleR          textNodeElm,
TextElementJustification    just
)
    {
    TextBlock   textBlock (textNodeElm);

    ParagraphRange paragraphRange (textBlock);
    textBlock.SetJustification ((TextElementJustification) just, paragraphRange);
    textBlock.SetJustificationOverrideFlag (true, paragraphRange);

    DPoint3d documentOrigin = textBlock.GetTextOrigin ();
    textBlock.SetTextOrigin (documentOrigin);
    textBlock.SetForceTextNodeFlag (true);

    textBlock.Reprocess ();
    
    EditElementHandle newTextElement;
    if (TextBlock::TO_ELEMENT_RESULT_Success != textBlock.ToElement (newTextElement, textNodeElm.GetDgnModelP (), &textNodeElm))
        return ERROR;
    
    return textNodeElm.ReplaceElementDescr (newTextElement.ExtractElementDescr().get());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BradRushing     09/94
+---------------+---------------+---------------+---------------+---------------+------*/
static TextElementJustification getDynamicJustification
(
ElementHandleCR             dimElement,        // in lcs, may be NULL
bool                        isDimPersistent,
TextElementJustification    oldJust
)
    {
    bool                        horJustLeft, verJustTop;
    TextElementJustification    just;
    UInt16                      noteRotation = DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal;
    double                      xDelta, yDelta;
    DPoint3d                    points[2];

    if (!dimElement.IsValid())
        return oldJust;

    if (2 > dimElement.GetElementCP()->ToDimensionElm().nPoints)
        return TextElementJustification::LeftTop;

    BentleyApi::mdlDim_extractPointsD (points, dimElement, dimElement.GetElementCP()->ToDimensionElm().nPoints-2, 2);

    bool    useRotation = !isDimPersistent;

    if (useRotation)
        mdlDim_getNoteTextRotation (&noteRotation, dimElement);

    if (noteRotation == DIMSTYLE_VALUE_MLNote_TextRotation_Vertical)
        {
        // Find the Horizontal Justification
        xDelta = points[0].y - points[1].y;
        horJustLeft = (xDelta < -mgds_fc_epsilon);

        // Find the Vertical Justification
        yDelta = points[0].x - points[1].x;
        verJustTop = (yDelta < -mgds_fc_epsilon);

        // Put Hor and Ver together
        if (horJustLeft)
            just = (verJustTop) ? TextElementJustification::LeftTop : TextElementJustification::LeftBaseline;
        else
            just = (verJustTop) ? TextElementJustification::RightTop : TextElementJustification::RightBaseline;
        }
    else    // hor, inline
        {
        // Find the Horizontal Justification
        xDelta = points[0].x - points[1].x;
        horJustLeft = (xDelta < -mgds_fc_epsilon);

        // Find the Vertical Justification
        yDelta = points[0].y - points[1].y;
        verJustTop = (yDelta > mgds_fc_epsilon);

        // Put Hor and Ver together
        if (horJustLeft)
            just = (verJustTop) ? TextElementJustification::LeftTop : TextElementJustification::LeftBaseline;
        else
            just = (verJustTop) ? TextElementJustification::RightTop : TextElementJustification::RightBaseline;
        }

    return just;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void    resolveJustification
(
TextElementJustification*   just,
ElementHandleCR             dimElement,
TextElementJustification    oldJust,
TextElementJustification    dynamicJust,
int                         horizontalOverrideJust,
int                         verticalOverrideJust
)
    {
    /* Rules for text node justification
     * ---------------------------------
     * Use the horizontal and vertical justification from the toolsettings.
     * If either are dynamic, use the corresponding one from dynamicJust.
     */
    switch (horizontalOverrideJust)
        {
        case DIMSTYLE_VALUE_MLNote_Justification_Left:
            {
            if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Top == verticalOverrideJust)
                *just = TextElementJustification::LeftTop;
            else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Center == verticalOverrideJust)
                *just = TextElementJustification::LeftMiddle;
            else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom == verticalOverrideJust)
                *just = TextElementJustification::LeftBaseline;
            else
                {
                if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Top == VERJUSTMODE (dynamicJust))
                    *just = TextElementJustification::LeftTop;
                else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Center == VERJUSTMODE (dynamicJust))
                    *just = TextElementJustification::LeftMiddle;
                else
                    *just = TextElementJustification::LeftBaseline;
                }
            break;
            }

        case DIMSTYLE_VALUE_MLNote_Justification_Right:
            {
            if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Top == verticalOverrideJust)
                *just = TextElementJustification::RightTop;
            else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Center == verticalOverrideJust)
                *just = TextElementJustification::RightMiddle;
            else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom == verticalOverrideJust)
                *just = TextElementJustification::RightBaseline;
            else
                {
                if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Top == VERJUSTMODE (dynamicJust))
                    *just = TextElementJustification::RightTop;
                else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Center == VERJUSTMODE (dynamicJust))
                    *just = TextElementJustification::RightMiddle;
                else
                    *just = TextElementJustification::RightBaseline;
                }
            break;
            }

        case DIMSTYLE_VALUE_MLNote_Justification_Center:
            {
            if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Top == verticalOverrideJust)
                *just = TextElementJustification::CenterTop;
            else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Center == verticalOverrideJust)
                *just = TextElementJustification::CenterMiddle;
            else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom == verticalOverrideJust)
                *just = TextElementJustification::CenterBaseline;
            else
                {
                if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Top == VERJUSTMODE (dynamicJust))
                    *just = TextElementJustification::CenterTop;
                else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Center == VERJUSTMODE (dynamicJust))
                    *just = TextElementJustification::CenterMiddle;
                else
                    *just = TextElementJustification::CenterBaseline;
                }
            break;
            }

        case DIMSTYLE_VALUE_MLNote_Justification_Dynamic:
        default:
            {
            TextElementJustification justToUse = dynamicJust;

            if (!dimElement.IsValid() || !mdlDim_getNoteAllowAutoMode (dimElement))
                justToUse = oldJust;

            if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Top == verticalOverrideJust)
                {
                if (DIMSTYLE_VALUE_MLNote_Justification_Left == HORJUSTMODE (justToUse))
                    *just = TextElementJustification::LeftTop;
                else
                    *just = TextElementJustification::RightTop;
                }
            else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Center == verticalOverrideJust)
                {
                if (DIMSTYLE_VALUE_MLNote_Justification_Left == HORJUSTMODE (justToUse))
                    *just = TextElementJustification::LeftMiddle;
                else
                    *just = TextElementJustification::RightMiddle;
                }
            else if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom == verticalOverrideJust)
                {
                if (DIMSTYLE_VALUE_MLNote_Justification_Left == HORJUSTMODE (justToUse))
                    *just = TextElementJustification::LeftBaseline;
                else
                    *just = TextElementJustification::RightBaseline;
                }
            else
                {
                *just = justToUse;
                }
            break;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        computeHorizontalAttachmentLocation
(
double *            leaderPt0X,
double *            leaderPt1X,
DPoint3d *          shapePoints,
NoteProperties *    props
)
    {
    if (DIMSTYLE_VALUE_MLNote_Justification_Left == HORJUSTMODE (props->iHorAttachmentJust))
        {
        *leaderPt0X = *leaderPt1X = shapePoints[0].x;
        if (props->bHasLeader)
            {
            *leaderPt1X -= props->dElbowLength;
            if (DIMSTYLE_VALUE_MLNote_VerAttachment_Underline == props->iVerAttachment)
                *leaderPt0X += fabs (bsiDPoint3d_distance (&shapePoints[0], &shapePoints[1]));
            }
        }
    else
        {
        *leaderPt0X = *leaderPt1X = shapePoints[1].x;
        if (props->bHasLeader)
            {
            *leaderPt1X += props->dElbowLength;
            if (DIMSTYLE_VALUE_MLNote_VerAttachment_Underline == props->iVerAttachment)
                *leaderPt0X -= fabs (bsiDPoint3d_distance (&shapePoints[0], &shapePoints[1]));
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static double      computeVerticalAttachmentLocation
(
DPoint3d *                  tightShapePoints,
ElementHandleCR             textNode,
double                      dLowerMargin,
int                         verAttachmentLoc,
TextElementJustification    dynamicJust
)
    {
    switch (verAttachmentLoc)
        {
        case DIMSTYLE_VALUE_MLNote_VerAttachment_Top:
            return tightShapePoints[3].y;
            break;

        case DIMSTYLE_VALUE_MLNote_VerAttachment_Middle:
            return tightShapePoints[0].y + 0.5 * (tightShapePoints[3].y - tightShapePoints[0].y);
            break;

        case DIMSTYLE_VALUE_MLNote_VerAttachment_BottomLine:
            return tightShapePoints[0].y + 0.5 * getLineTextHeight (textNode, false);
            break;

        case DIMSTYLE_VALUE_MLNote_VerAttachment_Bottom:
            return tightShapePoints[0].y;
            break;

        case DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicLine:
            if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Top == VERJUSTMODE (dynamicJust))   // point to first line of text
                return tightShapePoints[3].y - 0.5 * getLineTextHeight (textNode, true);
            else                                                // point to last line of text
                return tightShapePoints[0].y + 0.5 * getLineTextHeight (textNode, false);
            break;

        case DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicCorner:
            if (DIMSTYLE_VALUE_MLNote_VerticalJustification_Top == VERJUSTMODE (dynamicJust))   // point to top corner
                return tightShapePoints[3].y;
            else                                                // point to bottom corner
                return tightShapePoints[0].y;
            break;

        case DIMSTYLE_VALUE_MLNote_VerAttachment_Underline:
            return tightShapePoints[0].y - dLowerMargin;
            break;

        case DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine:
        default:
            return tightShapePoints[3].y - 0.5 * getLineTextHeight (textNode, true);
            break;
        }
    }

/*-------------------------------------------------------------------------------------*
* @bsimethod                                    SunandSandurkar             01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        adjustLeaderLineForIntersection
(
DPoint3d *          leaderPoints,
ElementHandleCR        frame,                // Has been transformed to LCS
DPoint3d *          shapePoints,
NoteProperties *    noteProps
)
    {
    if (!frame.IsValid())
        return;

    DPoint3d            testLeaderPoints[2];

    // Stretch the leader long enough to intersect with the frame. Do not have the intersect function automatically
    // extend the geometry since it will also extend the frame and cause false intersections.
    testLeaderPoints[0].x = shapePoints[0].x; testLeaderPoints[0].y = leaderPoints[0].y; testLeaderPoints[0].z = shapePoints[0].z;
    testLeaderPoints[1].x = shapePoints[1].x; testLeaderPoints[1].y = leaderPoints[1].y; testLeaderPoints[1].z = shapePoints[1].z;
    testLeaderPoints[0].x -= 100.0 * (shapePoints[1].x - shapePoints[0].x);
    testLeaderPoints[1].x += 100.0 * (shapePoints[1].x - shapePoints[0].x);
    
    GPArraySmartP   lineGPA, frameGPA;
    if (SUCCESS != frameGPA->Add (frame))
        return;
    
    lineGPA->Add (testLeaderPoints, 2);

    GPArraySmartP   intersectionGPA, collector2;
    double tolerance = GPArray::GetTolerance (lineGPA, frameGPA);
    lineGPA->GetClosestApproachPoints (*intersectionGPA, *collector2, *frameGPA, tolerance, false, false, tolerance);
    int numIntersections =  intersectionGPA->GetCount();
    if (numIntersections < 1)
        return;
    
    DPoint3d intersect[2];
    intersectionGPA->GetPoint (intersect[0], 0);
    intersectionGPA->GetPoint (intersect[1], 1);

    leaderPoints[0] = intersect[0];
    if (DIMSTYLE_VALUE_MLNote_Justification_Left == HORJUSTMODE (noteProps->iHorAttachmentJust))
        {
        if (1 < numIntersections && ((intersect[1].x - intersect[0].x)< -mgds_fc_epsilon))
            leaderPoints[0] = intersect[1];

        leaderPoints[1].x = leaderPoints[0].x;
        if (noteProps->bHasLeader)
            leaderPoints[1].x -= noteProps->dElbowLength;
        }
    else
        {
        if (1 < numIntersections && (intersect[1].x - intersect[0].x > mgds_fc_epsilon))
            leaderPoints[0] = intersect[1];

        leaderPoints[1].x = leaderPoints[0].x;
        if (noteProps->bHasLeader)
            leaderPoints[1].x += noteProps->dElbowLength;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          generate_polygon                                        |
|                                                                       |
| author        RBB                                     2/88            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     generate_polygon
(
EditElementHandleR eeh,
ElementHandleCP    elmIn,
DPoint3d *      center,
double          radius,
double          angle,
int             numEdges,
DgnModelP    modelRef
)
    {
    int         i;
    DPoint3d *  tmp = (DPoint3d *) _alloca ((numEdges + 1) * sizeof (DPoint3d));
    double      delta, theta;

    delta = (2 * PI) / (double) numEdges;

    theta = delta/2.0;
    angle -= theta;
    radius /= cos(theta);

    for (i=0; i<numEdges; i++, angle+=delta)
        {
        tmp[i].x = radius*cos(angle);
        tmp[i].y = radius*sin(angle);
        tmp[i].z = 0.0;
        bsiDPoint3d_addDPoint3dDPoint3d (&tmp[i], &tmp[i], center);
        }

    tmp[numEdges] = tmp[0];

    ShapeHandler::CreateShapeElement (eeh, elmIn, tmp, numEdges + 1, modelRef->Is3d(), *modelRef);

    return  SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    JoshSchifter    03/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void    setSymbologyFromDimension (EditElementHandleR eeh, DimensionElm const& dimElem)
    {
    ElementPropertiesSetter propSetter;

    propSetter.SetColor     (dimElem.GetSymbology().color);
    propSetter.SetWeight    (dimElem.GetSymbology().weight);
    propSetter.SetLinestyle (dimElem.GetSymbology().style, NULL);
    propSetter.SetLevel     (LevelId(dimElem.GetLevel()));

    propSetter.Apply(eeh);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void addLineFrame (EditElementHandleR out, ElementHandleCR dimElement, NoteInfo * noteInfo, ElementHandle cellElement, TextElementJustification horAttachmentJust, double dLeftMargin, double dLowerMargin)
    {
    DPoint3d            linePoints[2];
    DPoint3d            shapePoints[5];

    // Determine the line extents
    bool                bScale = false;
    if (!mdlDim_getNoteScaleFrame (&bScale, dimElement) || !bScale)
        {
        extractShape (shapePoints, cellElement);
        adjustShape (shapePoints, dLeftMargin, dLowerMargin);
        }
    else
        {
        DPoint3d        nodeOrigin;
        getTextNodeOrigin (nodeOrigin, cellElement);

        double      frameSize = 1.0;
        mdlDim_getNoteFrameScale (&frameSize, dimElement);

        frameSize *= dimElement.GetElementCP()->ToDimensionElm().text.height;

        for (int index = 0; index < 4; index++)
            shapePoints[index] = nodeOrigin;

        shapePoints[0].x -= 0.5 * frameSize;
        shapePoints[0].y -= 0.5 * frameSize;
        shapePoints[4]    = shapePoints[0];

        shapePoints[1].x += 0.5 * frameSize;
        shapePoints[1].y -= 0.5 * frameSize;

        shapePoints[2].x += 0.5 * frameSize;
        shapePoints[2].y += 0.5 * frameSize;

        shapePoints[3].x -= 0.5 * frameSize;
        shapePoints[3].y += 0.5 * frameSize;
        }

    // Draw the line
    if (true == getLinePoints (linePoints, shapePoints, horAttachmentJust))
        {
        if ((noteInfo->pFrameEdP.IsValid() && noteInfo->frameTypeFromGeom == DIMSTYLE_VALUE_MLNote_FrameType_Line) && noteInfo->pFrameEdP.GetElementId().IsValid())
            {
            LineStringHandler::CreateLineStringElement (out, &noteInfo->pFrameEdP, linePoints, 2, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());
            out.GetElementP()->SetElementId(noteInfo->pFrameEdP.GetElementId());
            }
        else
            {
            LineStringHandler::CreateLineStringElement (out, NULL, linePoints, 2, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());
            }

        if (dimElement.IsValid())
            setSymbologyFromDimension (out, dimElement.GetElementCP()->ToDimensionElm());
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void addBoxFrame (EditElementHandleR out, ElementHandleCR dimElement, NoteInfo * noteInfo, ElementHandleCR cellElement, double dLeftMargin, double dLowerMargin, int frame)
    {
    DPoint3d            shapePoints[5];

    // Determine the box extents
    bool                bScale = false;
    if (!mdlDim_getNoteScaleFrame (&bScale, dimElement) || !bScale)
        {
        extractShape (shapePoints, cellElement);
        adjustShape (shapePoints, dLeftMargin, dLowerMargin);
        }
    else
        {
        DPoint3d        nodeOrigin;
        getTextNodeOrigin (nodeOrigin, cellElement);

        double      frameSize = 1.0;
        mdlDim_getNoteFrameScale (&frameSize, dimElement);

        frameSize *= dimElement.GetElementCP()->ToDimensionElm().text.height;

        for (int index = 0; index < 4; index++)
            shapePoints[index] = nodeOrigin;

        shapePoints[0].x -= 0.5 * frameSize;
        shapePoints[0].y -= 0.5 * frameSize;
        shapePoints[4]    = shapePoints[0];

        shapePoints[1].x += 0.5 * frameSize;
        shapePoints[1].y -= 0.5 * frameSize;

        shapePoints[2].x += 0.5 * frameSize;
        shapePoints[2].y += 0.5 * frameSize;

        shapePoints[3].x -= 0.5 * frameSize;
        shapePoints[3].y += 0.5 * frameSize;
        }

    if (DIMSTYLE_VALUE_MLNote_FrameType_RotatedBox == frame)
        {
        DPoint3d  center;
        center.x = (shapePoints[0].x + shapePoints[2].x) / 2.0;
        center.y = (shapePoints[0].y + shapePoints[2].y) / 2.0;
        center.z = (shapePoints[0].z + shapePoints[2].z) / 2.0;

        double rad1 = bsiDPoint3d_distance (&shapePoints[0], &shapePoints[1]);
        double rad2 = bsiDPoint3d_distance (&shapePoints[0], &shapePoints[3]);
        double rad  = rad1 > rad2 ? rad1 : rad2;
        rad *= 0.5 * 1.414;

        for (int index = 0; index < 5; index++)
            shapePoints[index] = center;

        shapePoints[0].y -= rad;
        shapePoints[1].x += rad;
        shapePoints[2].y += rad;
        shapePoints[3].x -= rad;
        shapePoints[4].y -= rad;
        }

    // Draw the box
    if ((noteInfo->pFrameEdP.IsValid() && noteInfo->frameTypeFromGeom == DIMSTYLE_VALUE_MLNote_FrameType_Box) && noteInfo->pFrameEdP.GetElementId().IsValid())
        {
        ShapeHandler::CreateShapeElement (out, &noteInfo->pFrameEdP, shapePoints, 5, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());
        out.GetElementP()->SetElementId(noteInfo->pFrameEdP.GetElementId());
        }
    else
        {
        ShapeHandler::CreateShapeElement (out, NULL, shapePoints, 5, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());
        }

    if (dimElement.IsValid())
        setSymbologyFromDimension (out, dimElement.GetElementCP()->ToDimensionElm());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void addCircleFrame (EditElementHandleR out, ElementHandleCR dimElement, NoteInfo * noteInfo, ElementHandleCR cellElement, double dLeftMargin, double dLowerMargin)
    {
    DPoint3d            shapePoints[5];

    // Determine center and radius
    DPoint3d            center;
    bool                bScale = false;
    double              radius = 0.0;
    if (!mdlDim_getNoteScaleFrame (&bScale, dimElement) || !bScale)
        {
        extractShape (shapePoints, cellElement);
        adjustShape (shapePoints, dLeftMargin, dLowerMargin);

        center.x = (shapePoints[0].x + shapePoints[2].x) / 2.0;
        center.y = (shapePoints[0].y + shapePoints[2].y) / 2.0;
        center.z = (shapePoints[0].z + shapePoints[2].z) / 2.0;

        radius = bsiDPoint3d_distance (&shapePoints[0], &center);
        }
    else
        {
        DPoint3d        nodeOrigin;
        getTextNodeOrigin (nodeOrigin, cellElement);

        double      frameSize = 1.0;
        mdlDim_getNoteFrameScale (&frameSize, dimElement);

        frameSize *= dimElement.GetElementCP()->ToDimensionElm().text.height;

        center = nodeOrigin;
        radius = 0.5 * frameSize;
        }

    // Draw the circle
    if ((noteInfo->pFrameEdP.IsValid() && noteInfo->frameTypeFromGeom == DIMSTYLE_VALUE_MLNote_FrameType_Circle) && noteInfo->pFrameEdP.GetElementId().IsValid())
        {
        EllipseHandler::CreateEllipseElement (out, &noteInfo->pFrameEdP, center, radius, radius, 0, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());
        out.GetElementP()->SetElementId(noteInfo->pFrameEdP.GetElementId());
        }
    else
        {
        EllipseHandler::CreateEllipseElement (out, NULL, center, radius, radius, 0, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());
        }

    if (dimElement.IsValid())
        setSymbologyFromDimension (out, dimElement.GetElementCP()->ToDimensionElm());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void addCapsuleFrame (EditElementHandleR capsuleElement, ElementHandleCR dimElement, NoteInfo * noteInfo, ElementHandleCR cellElement, double dLeftMargin, double dLowerMargin)
    {
    DPoint3d            shapePoints[5];
    // Determine the box extents
    bool                bScale = false;

    if (!mdlDim_getNoteScaleFrame (&bScale, dimElement) || !bScale)
        {
        extractShape (shapePoints, cellElement);
        adjustShape (shapePoints, dLeftMargin, dLowerMargin);
        }
    else
        {
        DPoint3d        nodeOrigin;
        getTextNodeOrigin (nodeOrigin, cellElement);

        double      frameSize = 1.0;
        mdlDim_getNoteFrameScale (&frameSize, dimElement);

        frameSize *= dimElement.GetElementCP()->ToDimensionElm().text.height;

        for (int index = 0; index < 4; index++)
            shapePoints[index] = nodeOrigin;

        shapePoints[0].x -= 0.5 * frameSize;
        shapePoints[0].y -= 0.5 * frameSize;
        shapePoints[4]    = shapePoints[0];

        shapePoints[1].x += 0.5 * frameSize;
        shapePoints[1].y -= 0.5 * frameSize;

        shapePoints[2].x += 0.5 * frameSize;
        shapePoints[2].y += 0.5 * frameSize;

        shapePoints[3].x -= 0.5 * frameSize;
        shapePoints[3].y += 0.5 * frameSize;
        }

    DPoint3d    midPts[2];
    midPts[0].x = (shapePoints[0].x + shapePoints[3].x) / 2.0;
    midPts[0].y = (shapePoints[0].y + shapePoints[3].y) / 2.0;
    midPts[0].z = (shapePoints[0].z + shapePoints[3].z) / 2.0;
    midPts[1].x = (shapePoints[1].x + shapePoints[2].x) / 2.0;
    midPts[1].y = (shapePoints[1].y + shapePoints[2].y) / 2.0;
    midPts[1].z = (shapePoints[1].z + shapePoints[2].z) / 2.0;

    double radius = bsiDPoint3d_distance (&midPts[0], &shapePoints[0]);
    
    ChainHeaderHandler::CreateChainHeaderElement (capsuleElement, NULL, true, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());

    //Left arc
    {
    double angle = 0.5 * PI;
    EditElementHandle  partElement;
    ArcHandler::CreateArcElement (partElement, noteInfo->pFrameEdP.IsValid() ? &noteInfo->pFrameEdP : NULL, midPts[0], radius, radius, angle, 0, PI, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());
    if (dimElement.IsValid())
        setSymbologyFromDimension (partElement, dimElement.GetElementCP()->ToDimensionElm());
    ChainHeaderHandler::AddComponentElement(capsuleElement, partElement);
    }
    //top line
    {
    EditElementHandle  partElement;
    DSegment3d      segment;
    segment.Init (shapePoints[0], shapePoints[1]);
    LineHandler::CreateLineElement (partElement, noteInfo->pFrameEdP.IsValid() ? &noteInfo->pFrameEdP : NULL, segment, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());

    if (dimElement.IsValid())
        setSymbologyFromDimension (partElement, dimElement.GetElementCP()->ToDimensionElm());
    ChainHeaderHandler::AddComponentElement(capsuleElement, partElement);
    }
    //Right arc
    {
    double angle = 1.5 * PI;
    EditElementHandle  partElement;
    ArcHandler::CreateArcElement (partElement, noteInfo->pFrameEdP.IsValid() ? &noteInfo->pFrameEdP : NULL, midPts[1], radius, radius, angle, 0, PI, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());

    if (dimElement.IsValid())
        setSymbologyFromDimension (partElement, dimElement.GetElementCP()->ToDimensionElm());
    ChainHeaderHandler::AddComponentElement(capsuleElement, partElement);
    }
    //bottom line    
    {
    EditElementHandle  partElement;
    DSegment3d      segment;
    segment.Init (shapePoints[2], shapePoints[3]);
    LineHandler::CreateLineElement (partElement, noteInfo->pFrameEdP.IsValid() ? &noteInfo->pFrameEdP : NULL, segment, cellElement.GetDgnModelP()->Is3d(), *cellElement.GetDgnModelP());
    if (dimElement.IsValid())
        setSymbologyFromDimension (partElement, dimElement.GetElementCP()->ToDimensionElm());
    ChainHeaderHandler::AddComponentElement(capsuleElement, partElement);
    }

    ChainHeaderHandler::AddComponentComplete(capsuleElement);

    if (noteInfo->pFrameEdP.IsValid())
        capsuleElement.GetElementP()->SetElementId(noteInfo->pFrameEdP.GetElementId());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void addPolygonFrame(EditElementHandleR out, ElementHandleCR dimElement, NoteInfo * noteInfo, int frameType, ElementHandleCR cellElement, double dLeftMargin, double dLowerMargin, int numEdges, double angle)
    {
    DPoint3d            shapePoints[5];

    // Determine the box extents
    bool                bScale = false;
    if (!mdlDim_getNoteScaleFrame (&bScale, dimElement) || !bScale)
        {
        extractShape (shapePoints, cellElement);
        adjustShape (shapePoints, dLeftMargin, dLowerMargin);
        }
    else
        {
        DPoint3d        nodeOrigin;
        getTextNodeOrigin (nodeOrigin, cellElement);

        double      frameSize = 1.0;
        mdlDim_getNoteFrameScale (&frameSize, dimElement);

        frameSize *= dimElement.GetElementCP()->ToDimensionElm().text.height;

        for (int index = 0; index < 4; index++)
            shapePoints[index] = nodeOrigin;

        shapePoints[0].x -= 0.5 * frameSize;
        shapePoints[0].y -= 0.5 * frameSize;
        shapePoints[4]    = shapePoints[0];

        shapePoints[1].x += 0.5 * frameSize;
        shapePoints[1].y -= 0.5 * frameSize;

        shapePoints[2].x += 0.5 * frameSize;
        shapePoints[2].y += 0.5 * frameSize;

        shapePoints[3].x -= 0.5 * frameSize;
        shapePoints[3].y += 0.5 * frameSize;
        }

    DPoint3d            center;
    center.x = (shapePoints[0].x + shapePoints[2].x) / 2.0;
    center.y = (shapePoints[0].y + shapePoints[2].y) / 2.0;
    center.z = (shapePoints[0].z + shapePoints[2].z) / 2.0;

    // Draw the box
    if ((noteInfo->pFrameEdP.IsValid() && noteInfo->frameTypeFromGeom == frameType) && noteInfo->pFrameEdP.GetElementId().IsValid())
        {
        generate_polygon (out, &noteInfo->pFrameEdP, &center, 0.5 * bsiDPoint3d_distance (&shapePoints[0], &shapePoints[2]), angle, numEdges, cellElement.GetDgnModelP());
        out.GetElementP()->SetElementId(noteInfo->pFrameEdP.GetElementId());
        }
    else
        {
        generate_polygon (out, NULL, &center, 0.5 * bsiDPoint3d_distance (&shapePoints[0], &shapePoints[2]), angle, numEdges, cellElement.GetDgnModelP());
        }

    if (dimElement.IsValid())
        setSymbologyFromDimension (out, dimElement.GetElementCP()->ToDimensionElm());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void addTextFrame(EditElementHandleR out, ElementHandleCR cellElement, ElementHandleCR dimElement, NoteInfo* noteInfo, NoteProperties* noteProps)
    {
    switch (noteProps->iFrameType)
        {
        case DIMSTYLE_VALUE_MLNote_FrameType_Line:
            return addLineFrame (out, dimElement, noteInfo, cellElement, noteProps->iHorAttachmentJust, noteProps->dLeftMargin, noteProps->dLowerMargin);

        case DIMSTYLE_VALUE_MLNote_FrameType_Box:
            return addBoxFrame (out, dimElement, noteInfo, cellElement, noteProps->dLeftMargin, noteProps->dLowerMargin, noteProps->iFrameType);

        case DIMSTYLE_VALUE_MLNote_FrameType_RotatedBox:
            return addPolygonFrame (out, dimElement, noteInfo, noteProps->iFrameType, cellElement, noteProps->dLeftMargin, noteProps->dLowerMargin, 4, 0.25 * PI);

        case DIMSTYLE_VALUE_MLNote_FrameType_Circle:
            return addCircleFrame (out, dimElement, noteInfo, cellElement, noteProps->dLeftMargin, noteProps->dLowerMargin);

        case DIMSTYLE_VALUE_MLNote_FrameType_Capsule:
            return addCapsuleFrame (out, dimElement, noteInfo, cellElement, noteProps->dLeftMargin, noteProps->dLowerMargin);

        case DIMSTYLE_VALUE_MLNote_FrameType_Triangle:
            return addPolygonFrame (out, dimElement, noteInfo, noteProps->iFrameType, cellElement, noteProps->dLeftMargin, noteProps->dLowerMargin, 3, -0.5 * PI);

        case DIMSTYLE_VALUE_MLNote_FrameType_Hexagon:
            return addPolygonFrame (out, dimElement, noteInfo, noteProps->iFrameType, cellElement, noteProps->dLeftMargin, noteProps->dLowerMargin, 6, 0.5 * PI);

        case DIMSTYLE_VALUE_MLNote_FrameType_RotatedHexagon:
            return addPolygonFrame (out, dimElement, noteInfo, noteProps->iFrameType, cellElement, noteProps->dLeftMargin, noteProps->dLowerMargin, 6, 0.0);

        case DIMSTYLE_VALUE_MLNote_FrameType_Pentagon:
            return addPolygonFrame (out, dimElement, noteInfo, noteProps->iFrameType, cellElement, noteProps->dLeftMargin, noteProps->dLowerMargin, 5, -0.5 * PI);

        case DIMSTYLE_VALUE_MLNote_FrameType_Octagon:
            return addPolygonFrame (out, dimElement, noteInfo, noteProps->iFrameType, cellElement, noteProps->dLeftMargin, noteProps->dLowerMargin, 8, 0.0);
        }

    out.Invalidate();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        setCellOrigin
(
MSElementDescr *    cellEdP,
DPoint3d *          origin
)
    {
    if (!cellEdP || !origin)
        return;

    DgnElementR el = cellEdP->ElementR();
    if (el.Is3d())
        {
        el.ToCell_3dR().origin = *origin;
        }
    else
        {
        el.ToCell_2dR().origin.x = origin->x;
        el.ToCell_2dR().origin.y = origin->y;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/04
+---------------+---------------+---------------+---------------+---------------+------*/
static TextElementJustification getHorizontalAttachmentFromCellGeometry
(
ElementHandleCR    cellElement
)
    {
    DPoint3d    cellOrigin;
    DPoint3d    nodeOrigin;

    if (!cellElement.IsValid() || CELL_HEADER_ELM != cellElement.GetLegacyType())
        return TextElementJustification::LeftTop;

    // Get the cell and textnode origins
    CellUtil::ExtractOrigin (cellOrigin, cellElement);
    getTextNodeOrigin (nodeOrigin, cellElement);

    // The origins are already rotated with respect to the note rotation. Simply
    // check the x-coordinate to decide the attachment side.
    if (nodeOrigin.x > cellOrigin.x)
        return TextElementJustification::LeftTop;
    else
        return TextElementJustification::RightTop;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/04
+---------------+---------------+---------------+---------------+---------------+------*/
static TextElementJustification getHorizontalAttachmentFromDimPoints (ElementHandleCR dimElement)
    {
    DPoint3d    points[2];

    if (!dimElement.IsValid() || 2 > dimElement.GetElementCP()->ToDimensionElm().nPoints)
        return TextElementJustification::LeftTop;

    RotMatrix   dimMatrixR, dimMatrixC;
    DimensionHandler::GetInstance().GetRotationMatrix (dimElement, dimMatrixC);

    dimMatrixR.TransposeOf(dimMatrixC);

    // Get the last two dim points and prepare them for comparing. During creation,
    // the dim is in lcs and can be compared directly. However, during a recreate
    // it needs to be untransformed by its current dimrotmatrix.
    BentleyApi::mdlDim_extractPointsD (points, dimElement, dimElement.GetElementCP()->ToDimensionElm().nPoints-2, 2);
    dimMatrixR.Multiply(points[0]);
    dimMatrixR.Multiply(points[1]);

    UInt16      rotation = DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal;
    mdlDim_getNoteTextRotation (&rotation,  dimElement);

    switch (rotation)
        {
        case DIMSTYLE_VALUE_MLNote_TextRotation_Vertical:
            {
            if ((points[1].y -points[0].y)>mgds_fc_epsilon)
                return TextElementJustification::LeftTop;
            else
                return TextElementJustification::RightTop;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal:
        default:
            {
            if ((points[1].x - points[0].x) >mgds_fc_epsilon)
                return TextElementJustification::LeftTop;
            else
                return TextElementJustification::RightTop;
            break;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/04
+---------------+---------------+---------------+---------------+---------------+------*/
static TextElementJustification getHorizontalAttachmentSide
(
int                 horAttachmentFlag,
ElementHandleCR     dimElement,
bool                bForOldNote,
ElementHandleCR     cellForReference
)
    {
    switch (horAttachmentFlag)
        {
        case DIMSTYLE_VALUE_MLNote_HorAttachment_Right:
            {
            return TextElementJustification::RightTop;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_HorAttachment_Left:
            {
            return TextElementJustification::LeftTop;
            break;
            }

        default:
        case DIMSTYLE_VALUE_MLNote_HorAttachment_Auto:
            {
            if ((bForOldNote || !mdlDim_getNoteAllowAutoMode (dimElement)) && cellForReference.IsValid())
                return getHorizontalAttachmentFromCellGeometry (cellForReference);
            else
                return getHorizontalAttachmentFromDimPoints (dimElement);
            break;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 04/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void     resolveFrameType
(
NoteInfo&       noteInfo,
ElementHandleCR frameElement,
int             defaultFrameType
)
    {
    // Distinguish between the rotated and non-rotated frame types when
    // the dimension leader is missing. The fix is to compare the x-bvector
    // of the cell and the first segment of the frame shape. If they are
    // parallel, then the frame is not a rotated type. This check cannot be
    // done on legacy notes whose cell rotation matrix used to always be
    // identity.

    RotMatrix cellRotMatrix;
    CellUtil::ExtractRotation (cellRotMatrix, noteInfo.pCellElm); // was *noteInfo->pCellElm

    ElementHandle    textNode;
    if (SUCCESS != mdlNote_findTextNodeElement (textNode, noteInfo.pCellElm))
        return;

    bool parallelFrame = false;
    if (textNode.IsValid())
        {
        RotMatrix           textRotMatrix;
        getActualTextRotMatrix (&textRotMatrix, textNode);

        if (!areMatricesEqual (&textRotMatrix, &cellRotMatrix))
            parallelFrame = true;
        }

    if (!parallelFrame)
        {
        CurveVectorPtr  pathCurve = ICurvePathQuery::ElementToCurveVector (frameElement);
        DPoint3d        startPt, endPt;
        DVec3d          startTan, endTan;

        if (pathCurve.IsValid () && pathCurve->GetStartEnd (startPt, endPt, startTan, endTan))
            {
            DVec3d    cellXVec;

            cellRotMatrix.GetColumn (cellXVec, 0);
            startTan.Negate ();

            parallelFrame = cellXVec.IsParallelTo (startTan);
            }
        }

    switch (defaultFrameType)
        {
        case DIMSTYLE_VALUE_MLNote_FrameType_Hexagon:
            if (parallelFrame)
                noteInfo.frameTypeFromGeom = DIMSTYLE_VALUE_MLNote_FrameType_Hexagon;
            else
                noteInfo.frameTypeFromGeom = DIMSTYLE_VALUE_MLNote_FrameType_RotatedHexagon;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Box:
            if (parallelFrame)
                noteInfo.frameTypeFromGeom = DIMSTYLE_VALUE_MLNote_FrameType_Box;
            else
                noteInfo.frameTypeFromGeom = DIMSTYLE_VALUE_MLNote_FrameType_RotatedBox;
            break;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BradRushing     09/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int      note_getNoteInfo (ElementHandleCR elmP, DgnModelR model, NoteInfo& noteInfo)
    {
    int         numVerts  = 0;
    DPoint3d    shapePoints[5];

    switch (elmP.GetLegacyType())
        {
        case SHAPE_ELM:
            {
            if (SUCCESS != LineStringUtil::Extract (NULL, &numVerts, *elmP.GetElementCP(), model))
                return  ERROR;

            if (5 >= numVerts)
                LineStringUtil::Extract (noteInfo.oldOuterBox, NULL, *elmP.GetElementCP(), model);

            if (noteInfo.frameTypeFromGeom == -1)
                {
                switch (numVerts)
                    {
                    case 4:
                        noteInfo.frameTypeFromGeom = DIMSTYLE_VALUE_MLNote_FrameType_Triangle;
                        break;
                    case 5:
                        resolveFrameType (noteInfo, elmP, DIMSTYLE_VALUE_MLNote_FrameType_Box);
                        break;
                    case 6:
                        noteInfo.frameTypeFromGeom = DIMSTYLE_VALUE_MLNote_FrameType_Pentagon;
                        break;
                    case 7:
                        resolveFrameType (noteInfo, elmP, DIMSTYLE_VALUE_MLNote_FrameType_Hexagon);
                        break;
                    case 9:
                        noteInfo.frameTypeFromGeom = DIMSTYLE_VALUE_MLNote_FrameType_Octagon;
                        break;
                    }
                }
            break;
            }

        case CMPLX_SHAPE_ELM:
            {
            if (noteInfo.frameTypeFromGeom == -1)
                noteInfo.frameTypeFromGeom = DIMSTYLE_VALUE_MLNote_FrameType_Capsule;

            break;
            }

        case ELLIPSE_ELM:
            {
            if (noteInfo.frameTypeFromGeom == -1)
                noteInfo.frameTypeFromGeom = DIMSTYLE_VALUE_MLNote_FrameType_Circle;

            break;
            }

        case LINE_STRING_ELM:
            {
            if (noteInfo.frameTypeFromGeom == -1)
                noteInfo.frameTypeFromGeom = DIMSTYLE_VALUE_MLNote_FrameType_Line;

            if (SUCCESS != LineStringUtil::Extract (noteInfo.oldOuterBox, &numVerts, *elmP.GetElementCP(), model))
                return  ERROR;

            break;
            }

        case LINE_ELM:
            {
            noteInfo.leader = true;
            if (SUCCESS != LineStringUtil::Extract (noteInfo.oldLeaderPoints, &numVerts, *elmP.GetElementCP(), model))
                return  ERROR;

            noteInfo.pLeaderElm = elmP;
            break;
            }

        case TEXT_NODE_ELM:
            {
            extractShape (shapePoints, elmP);
            noteInfo.nodeID = elmP.GetElementId().GetValue();
            memcpy (noteInfo.oldInnerBox, shapePoints, sizeof (noteInfo.oldInnerBox));
            break;
            }

        case CELL_HEADER_ELM:
            {
            noteInfo.pCellElm = elmP;
            break;
            }
        }

    return  SUCCESS;
    }


/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/04
+---------------+---------------+---------------+---------------+---------------+------*/
static  void            mdlNote_getHorizontalAttachment
(
TextElementJustification*   iHorAttachmentJust,    // <=
ElementHandleCR             dimElement,            //  =>
bool                        bForOldNote,           //  =>
ElementHandleCR             cellForReference      //  =>
)
    {
    UInt16              horAttachment = DIMSTYLE_VALUE_MLNote_HorAttachment_Left;
    mdlDim_getNoteHorAttachment (&horAttachment, dimElement);

    *iHorAttachmentJust = getHorizontalAttachmentSide (horAttachment, dimElement, bForOldNote, cellForReference);
    }

#ifdef UNUSED_FUNCTION
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/04
+---------------+---------------+---------------+---------------+---------------+------*/
static int             getVerAttachmentFromGeometry
(
NoteInfo *              noteInfo,
ElementHandleCR            cellElement,
int                     horAttachmentJust
)
    {
    int                 verAttachment = DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine;
    DPoint3d            shapePoints[5];
    ElementHandle          textNode;

    if (!cellElement.IsValid() || SUCCESS != mdlNote_findTextNodeElement (textNode, cellElement))
        return verAttachment;

    extractShape (shapePoints, textNode);

    DPoint3d            leaderPoint;
    CellUtil::ExtractOrigin (leaderPoint, cellElement);

    double nodeHeight = shapePoints[3].y - shapePoints[0].y;
    if (leaderPoint.y > shapePoints[3].y)
        {
        verAttachment = DIMSTYLE_VALUE_MLNote_VerAttachment_Top;
        }
    else if (leaderPoint.y + fc_uorTol > shapePoints[0].y + 0.5 * nodeHeight)
        {
        verAttachment = DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine;
        }
    else if (leaderPoint.y + fc_uorTol > shapePoints[0].y + getLineTextHeight (textNode, false))
        {
        verAttachment = DIMSTYLE_VALUE_MLNote_VerAttachment_Middle;
        }
    else if (leaderPoint.y + fc_uorTol > shapePoints[0].y)
        {
        verAttachment = DIMSTYLE_VALUE_MLNote_VerAttachment_BottomLine;
        }
    else
        {
        if (bsiDPoint3d_distance (&noteInfo->oldLeaderPoints[0], &noteInfo->oldLeaderPoints[1]) < fc_uorTol)
            {
            verAttachment = DIMSTYLE_VALUE_MLNote_VerAttachment_Bottom;
            }
        else
            {
            DPoint3d    nearPoint = shapePoints[0];
            DPoint3d    farPoint  = shapePoints[1];
            if (TextElementJustification::RightTop == horAttachmentJust)
                {
                nearPoint = shapePoints[1];
                farPoint  = shapePoints[0];
                }

            if (fabs (noteInfo->oldLeaderPoints[0].x - nearPoint.x) < fabs (noteInfo->oldLeaderPoints[0].x - farPoint.x))
                verAttachment = DIMSTYLE_VALUE_MLNote_VerAttachment_Bottom;
            else
                verAttachment = DIMSTYLE_VALUE_MLNote_VerAttachment_Underline;
            }
        }

    return verAttachment;
    }
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static double          getDimTextHeight
(
ElementHandleCR             dimElement,         //  =>
ElementHandleCR             cellElement         //  =>
)
    {
    if (dimElement.IsValid())
        return dimElement.GetElementCP()->ToDimensionElm().text.height;

    ElementHandle     textNode;
    if (SUCCESS == mdlNote_findTextNodeElement (textNode, cellElement))
        {
        DPoint2d    size;
        TextNodeHandler::GetFontSize (textNode, size);
        return size.y;
        }

    BeAssert (false);
    return 1.0;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        findLeftMargin
(
RectifyProperties * rectifyProps,       // <=>
NoteInfo *          pNoteInfo,          //  =>
ElementHandleCR        dimElement,         //  =>
double              dimTextHeight,      //  =>
DPoint3d *          dimPoint            //  =>
)
    {
    if (true == mdlDim_getNoteLeftMargin (&rectifyProps->props.dLeftMargin, dimElement))
        return;

    if (bsiDPoint3d_distance (&pNoteInfo->oldOuterBox[0], &pNoteInfo->oldOuterBox[1]) > mgds_fc_epsilon)
        {
        // Get the left margin from the previous boxframe or lineframe, if available
        if (bsiDPoint3d_distance (&pNoteInfo->oldOuterBox[2], &pNoteInfo->oldOuterBox[3]) > mgds_fc_epsilon)
            {
            // If it's a boxframe
            rectifyProps->props.dLeftMargin = fabs (pNoteInfo->oldInnerBox[0].x - pNoteInfo->oldOuterBox[0].x);
            }
        else
            {
            // If it's a lineframe
            if (DIMSTYLE_VALUE_MLNote_Justification_Left == HORJUSTMODE (rectifyProps->props.iHorAttachmentJust))
                rectifyProps->props.dLeftMargin = fabs (pNoteInfo->oldInnerBox[0].x - pNoteInfo->oldOuterBox[0].x);
            else
                rectifyProps->props.dLeftMargin = fabs (pNoteInfo->oldInnerBox[1].x - pNoteInfo->oldOuterBox[0].x);
            }
        }
    else
        {
        double      dElbow = bsiDPoint3d_distance (&pNoteInfo->oldLeaderPoints[0], &pNoteInfo->oldLeaderPoints[1]);
        bool        bComputeLeftMargin = true;
        DPoint3d    leaderPoint = {0.0, 0.0, 0.0};

        // If there is an in-line leader, use it. Otherwise use the dimension endpoint.
        if (dElbow > mgds_fc_epsilon)
            leaderPoint = pNoteInfo->oldLeaderPoints[0];
        else if (dimPoint)
            leaderPoint = *dimPoint;
        else
            bComputeLeftMargin = false;

        if (!bComputeLeftMargin)
            {
            rectifyProps->props.dLeftMargin = 0.5;
            }
        else
            {
            if (DIMSTYLE_VALUE_MLNote_VerAttachment_Underline == rectifyProps->props.iVerAttachment && dElbow > mgds_fc_epsilon)
                {
                if (DIMSTYLE_VALUE_MLNote_Justification_Left == HORJUSTMODE (rectifyProps->props.iHorAttachmentJust))
                    rectifyProps->props.dLeftMargin = fabs (pNoteInfo->oldInnerBox[1].x - leaderPoint.x);
                else
                    rectifyProps->props.dLeftMargin = fabs (pNoteInfo->oldInnerBox[0].x - leaderPoint.x);
                }
            else
                {
                if (DIMSTYLE_VALUE_MLNote_Justification_Left == HORJUSTMODE (rectifyProps->props.iHorAttachmentJust))
                    rectifyProps->props.dLeftMargin = fabs (pNoteInfo->oldInnerBox[0].x - leaderPoint.x);
                else
                    rectifyProps->props.dLeftMargin = fabs (pNoteInfo->oldInnerBox[1].x - leaderPoint.x);
                }
            }
        }

    rectifyProps->props.dLeftMargin /= dimTextHeight;
    rectifyProps->flags.dLeftMargin = true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        findLowerMargin
(
RectifyProperties * rectifyProps,       // <=>
NoteInfo *          pNoteInfo,          //  =>
ElementHandleCR        dimElement,         //  =>
double              dimTextHeight,      //  =>
DPoint3d *          dimPoint            //  =>
)
    {
    if (true == mdlDim_getNoteLowerMargin (&rectifyProps->props.dLowerMargin, dimElement))
        return;

    if (bsiDPoint3d_distance (&pNoteInfo->oldOuterBox[0], &pNoteInfo->oldOuterBox[1]) > mgds_fc_epsilon)
        {
        // Get the lower margin from the previous boxframe or lineframe, if available
        rectifyProps->props.dLowerMargin = fabs (pNoteInfo->oldInnerBox[0].y - pNoteInfo->oldOuterBox[0].y);
        }
    else if (DIMSTYLE_VALUE_MLNote_VerAttachment_Underline == rectifyProps->props.iVerAttachment)
        {
        // Get the lower margin from underline and textshape. If not underline, we don't care about the lower margin.
        if (bsiDPoint3d_distance (&pNoteInfo->oldLeaderPoints[0], &pNoteInfo->oldLeaderPoints[1]) > mgds_fc_epsilon)
            rectifyProps->props.dLowerMargin = fabs (pNoteInfo->oldInnerBox[0].y - pNoteInfo->oldLeaderPoints[0].y);
        else if (dimPoint)
            rectifyProps->props.dLowerMargin = fabs (pNoteInfo->oldInnerBox[0].y - dimPoint->y);
        }

    rectifyProps->props.dLowerMargin /= dimTextHeight;
    rectifyProps->flags.dLowerMargin = true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        findElbowLength
(
RectifyProperties * rectifyProps,       // <=>
NoteInfo *          pNoteInfo,          //  =>
ElementHandleCR     dimElement,         //  =>
double              dimTextHeight,      //  =>
ElementHandleCR     oldCellElement     //  =>
)
    {
    if (true == mdlDim_getNoteElbowLength (&rectifyProps->props.dElbowLength, dimElement))
        return;

    rectifyProps->props.dElbowLength = bsiDPoint3d_distance (&pNoteInfo->oldLeaderPoints[0], &pNoteInfo->oldLeaderPoints[1]);

    if (rectifyProps->props.dElbowLength > mgds_fc_epsilon && DIMSTYLE_VALUE_MLNote_VerAttachment_Underline == rectifyProps->props.iVerAttachment)
        {
        DPoint3d    shapePoints[5];
        extractShape (shapePoints, oldCellElement);
        adjustShape (shapePoints, rectifyProps->props.dLeftMargin, rectifyProps->props.dLowerMargin);
        rectifyProps->props.dElbowLength -= bsiDPoint3d_distance (&shapePoints[0], &shapePoints[1]);
        }

    rectifyProps->props.dElbowLength /= dimTextHeight;
    rectifyProps->flags.dElbowLength = true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        findNoteDistanceSettings
(
RectifyProperties * newProps,           // <=>
NoteInfo *          pNoteInfo,          //  =>
ElementHandleCR     dimElement,         //  => may be NULL
ElementHandleCR     cellElement            //  =>
)
    {
    DPoint3d        dimPoint;
    if (dimElement.IsValid())
        BentleyApi::mdlDim_extractPointsD (&dimPoint, dimElement, DimensionHandler::GetInstance().GetNumSegments (dimElement) - 1, 1);
    else
        CellUtil::ExtractOrigin (dimPoint, cellElement); 

    double dimTextHeight = getDimTextHeight (dimElement, cellElement);
    if (dimTextHeight < mgds_fc_epsilon)
        {
        BeDataAssert (false && "Found a zero size dimension text height");
        return;
        }

    /*-------------------------------------------------------------------
      Note about legacy notes :
      Pre 8.1 notes used 0.5 * textstyle's text height for lower margin,
      0.5 * textstyle's text width for left margin and 2.0 * text style's
      text height for elbow length. We introduced new settings in dimstyle
      to drive these three distances as factors of dimstyle's text height.
      While editing existing notes, we need to handle both cases. First
      see if the note has the new distance factors. If so, use them in
      conjunction with dimension's textheight. Otherwise, use the note's
      geometry to compute these distances. Remember to never look at the
      active dimstyle or active textstyle while editing existing notes.
    -------------------------------------------------------------------*/
    findLeftMargin  (newProps, pNoteInfo, dimElement, dimTextHeight, &dimPoint);
    findLowerMargin (newProps, pNoteInfo, dimElement, dimTextHeight, &dimPoint);
    findElbowLength (newProps, pNoteInfo, dimElement, dimTextHeight, cellElement);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void            findNoteSettings
(
RectifyProperties *     rectifyProps,       // <=>
ElementHandleCR         dimElement,         //  => may be null
NoteInfo *              noteInfo,           //  =>
ElementHandleCR         cellElement            //  =>
)
    {
    if (SUCCESS != mdlDim_getNoteLeaderDisplay (NULL, dimElement))
        {
        rectifyProps->props.bHasLeader = noteInfo->leader;
        rectifyProps->flags.bHasLeader = true;
        }

    // Legacy notes always attach by Auto. No need to figure it out from the cell geometry, so don't turn on rectify flag.
    UInt16 horAttachment     = DIMSTYLE_VALUE_MLNote_HorAttachment_Auto;
    mdlDim_getNoteHorAttachment (&horAttachment, dimElement);
    rectifyProps->props.iHorAttachmentJust = getHorizontalAttachmentSide (horAttachment, dimElement, true, cellElement);
    //The distance computation needs the horizontal and vertical attachment values
    if (true != mdlDim_getNoteVerLeftAttachment (&rectifyProps->props.iVerAttachment, dimElement) || true != mdlDim_getNoteVerRightAttachment (&rectifyProps->props.iVerAttachment, dimElement))
        {
        // Legacy notes always attach to TopLine. No need to figure it out from the cell geometry, so don't turn on rectify flag.
        rectifyProps->props.iVerAttachment = DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine;
        }

    // hor Just
    if (SUCCESS != mdlDim_getNoteHorizontalJustification ((DimStyleProp_MLNote_Justification *)&rectifyProps->props.iHorJust, dimElement))
        {
        ElementHandle textNode;
        if (SUCCESS == mdlNote_findTextNodeElement (textNode, cellElement))
            {
            TextElementJustification just = TextElementJustification::LeftTop;
            mdlNote_getTextNodeJust (just, textNode);

            rectifyProps->props.iHorJust = HORJUSTMODE (just);
            rectifyProps->flags.iHorJust = true;
            }
        }

    // Frame Type
    DimStyleProp_MLNote_FrameType frameType = DIMSTYLE_VALUE_MLNote_FrameType_None;
    if (SUCCESS != mdlDim_getNoteFrameType ((DimStyleProp_MLNote_FrameType *) &frameType, dimElement) && -1 != noteInfo->frameTypeFromGeom)
        {
        rectifyProps->props.iFrameType = noteInfo->frameTypeFromGeom;
        rectifyProps->flags.iFrameType = true;
        }

    findNoteDistanceSettings (rectifyProps, noteInfo, dimElement, cellElement);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/04
* NOTE : If dimension is associated with note cell, this function assumes that
* mdlNote_rectifyDimSettings was called. For new notes, all the settings will be
* set in updateFromDimStyle.
+---------------+---------------+---------------+---------------+---------------+------*/
static void            getNoteProperties
(
NoteProperties *        noteProps,          // <=
NoteInfo *              noteInfo,           //  =>
ElementHandleCR         dimElement,         //  =>
bool                    bForOldNote,        //  =>
ElementHandleCR         oldCellElement         //  =>
)
    {
    if (!dimElement.IsValid())
        {
        /*-------------------------------------------------------------------
           If it is an orphan cell that does not have an associated dimension
           then get all settings from cell geometry.
        -------------------------------------------------------------------*/
        RectifyProperties    rectifyProps;
        findNoteSettings (&rectifyProps, dimElement, noteInfo, oldCellElement);
        *noteProps = rectifyProps.props;
        }
    else
        {
        /*-------------------------------------------------------------------
            Get the note settings from dimension. Do not check the return
            status because these functions are capable of returning the
            correct values in all cases.
        -------------------------------------------------------------------*/
        mdlDim_getNoteLeaderDisplay (&noteProps->bHasLeader, dimElement);

        mdlNote_getHorizontalAttachment (&noteProps->iHorAttachmentJust, dimElement, bForOldNote, oldCellElement);

        if (DIMSTYLE_VALUE_MLNote_Justification_Left == HORJUSTMODE (noteProps->iHorAttachmentJust))
            mdlDim_getNoteVerLeftAttachment ((UInt16 *) &noteProps->iVerAttachment, dimElement);
        else
            mdlDim_getNoteVerRightAttachment ((UInt16 *) &noteProps->iVerAttachment, dimElement);

        mdlDim_getNoteLeftMargin (&noteProps->dLeftMargin, dimElement);

        mdlDim_getNoteLowerMargin (&noteProps->dLowerMargin, dimElement);

        mdlDim_getNoteElbowLength (&noteProps->dElbowLength, dimElement);

        mdlDim_getNoteHorizontalJustification ((DimStyleProp_MLNote_Justification *) &noteProps->iHorJust, dimElement);

        mdlDim_getNoteFrameType ((DimStyleProp_MLNote_FrameType *) &noteProps->iFrameType, dimElement);
        }

    /*-------------------------------------------------------------------
        Apply text height
    -------------------------------------------------------------------*/
    double dimTextHeight = getDimTextHeight (dimElement, oldCellElement);
    noteProps->dLeftMargin  *= dimTextHeight;
    noteProps->dLowerMargin *= dimTextHeight;
    noteProps->dElbowLength *= dimTextHeight;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void            note_getDimViewMatrix
(
RotMatrixR              viewMatrix,
ElementHandleCR         dimElement
)
    {
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
    if (NULL == hdlr)
        return;

    if (SUCCESS == hdlr->GetViewRotation (dimElement, viewMatrix))
        viewMatrix.InverseOf(viewMatrix);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
* Return : SUCCESS if tangents updated
+---------------+---------------+---------------+---------------+---------------+------*/
  StatusInt       mdlNote_updateCurveLeaderTangents
(
EditElementHandleR      dimElemHandle,          // in global
DgnModelP            modelRef
)
    {
    if (2 >  ((DimensionElm*) dimElemHandle.GetElementCP ())->nPoints)
        return ERROR;

    /*-------------------------------------------------------------------
      Get the note cell
    -------------------------------------------------------------------*/
    ElementId           cellElementID;
    EditElementHandle      cellElm;

    if (SUCCESS != mdlNote_getRootNoteCellId (&cellElementID, dimElemHandle) ||
        SUCCESS != cellElm.FindById (ElementId(cellElementID), modelRef))
        return ERROR;

    /*-------------------------------------------------------------------
      Transform to LCS so getHorizontalAttachmentFromCellGeometry can
      compare node origin and cell origin
    -------------------------------------------------------------------*/
    DPoint3d        origin;
    RotMatrix       rMatrix;

    CellUtil::ExtractOrigin (origin, cellElm); 
    CellUtil::ExtractRotation (rMatrix, cellElm); 

    Transform       transform;
    transform.InitIdentity ();
    rMatrix.TransposeOf(rMatrix);
    transform.InitProduct(transform,rMatrix);

    TransformInfo   tInfo (transform);

    cellElm.GetHandler().ApplyTransform (cellElm, tInfo);

    TextElementJustification iHorAttachmentJust = TextElementJustification::LeftTop;
    mdlNote_getHorizontalAttachment (&iHorAttachmentJust, dimElemHandle, false, cellElm);

    /*-------------------------------------------------------------------
      Get leader points and compute tangents
    -------------------------------------------------------------------*/
    DPoint3d            points[2];
    DVec3d              newEndTangent, *pEndTangent = NULL;
    BentleyApi::mdlDim_extractPointsD (points, dimElemHandle, dimElemHandle.GetElementCP()->ToDimensionElm().nPoints - 2, 2);

    UInt16              iLeaderType = 0;
    mdlDim_getNoteLeaderType (&iLeaderType, dimElemHandle);
    if (1 == iLeaderType) // Curve
        {
        RotMatrix   dimRotMatrixC;
        DimensionHandler::GetInstance().GetRotationMatrix (dimElemHandle, dimRotMatrixC);

        UInt16      rotation = DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal;
        mdlDim_getNoteTextRotation (&rotation, dimElemHandle);

        switch (rotation)
            {
            case DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal:
                {
                newEndTangent.x = -1.0;
                newEndTangent.y = 0.0;
                newEndTangent.z = 0.0;

                // Flip end tangent if attachment is on right side
                if (TextElementJustification::LeftTop != iHorAttachmentJust)
                    bsiDVec3d_scale (&newEndTangent, &newEndTangent, -1.0);

                dimRotMatrixC.Multiply(newEndTangent);
                break;
                }

            case DIMSTYLE_VALUE_MLNote_TextRotation_Vertical:
                {
                newEndTangent.x = 0.0;
                newEndTangent.y = -1.0;
                newEndTangent.z = 0.0;

                // Flip end tangent if attachment is on right side
                if (TextElementJustification::LeftTop != iHorAttachmentJust)
                    bsiDVec3d_scale (&newEndTangent, &newEndTangent, -1.0);

                dimRotMatrixC.Multiply(newEndTangent);
                break;
                }

            case DIMSTYLE_VALUE_MLNote_TextRotation_Inline:
                {
                newEndTangent.NormalizedDifference (points[0], points[1]);
                break;
                }
            }

        pEndTangent = &newEndTangent;
        }

    /*-------------------------------------------------------------------
      Set the new tangents
    -------------------------------------------------------------------*/
    StatusInt status = SUCCESS;
    if (pEndTangent)
        {
        DPoint3d oldTangent;
        if (!mdlDim_segmentGetCurveEndTangent (&oldTangent, dimElemHandle, 0) ||
            !bsiDPoint3d_pointEqualTolerance (&oldTangent, pEndTangent, mgds_fc_epsilon))
            {
            status = mdlDim_segmentSetCurveEndTangent (dimElemHandle, 0, pEndTangent);
            }
        }
    else
        {
        if (mdlDim_segmentGetCurveEndTangent (NULL, dimElemHandle, 0))
            {
            status = mdlDim_segmentSetCurveEndTangent (dimElemHandle, 0, NULL);
            }
        }

    return status;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void            applyNoteTransform
(
EditElementHandleR         elemHandle,        // in lcs
ElementHandleCR            dimElement,        // in global (may be null)
RotMatrix *             oldMatrix,
RotMatrix *             rMatrix,
DPoint3d *              org,
bool                    mirror
)
    {
    DPoint3d            origin = {0,0,0};
    if (org)
        origin = *org;

    if (!mirror)
        {
        Transform           tMatrix;
        tMatrix.InitIdentity ();
        tMatrix.SetTranslation (origin);

        tMatrix.InitProduct(tMatrix,*rMatrix);

        TransformInfo   transformInfo (tMatrix);
        elemHandle.GetHandler().ApplyTransform (elemHandle, transformInfo);
        }
    else
        {
        Transform           tMatrix;
        tMatrix.InitIdentity ();
        tMatrix.SetTranslation (origin);

        // Step 1 : Transform to the view
        RotMatrix   viewMatrix;
        if (dimElement.IsValid())
            note_getDimViewMatrix (viewMatrix, dimElement);
        else
            viewMatrix = *oldMatrix;

        tMatrix.InitProduct(tMatrix,viewMatrix);

        TransformInfo   transformInfo1 (tMatrix);

        elemHandle.GetHandler().ApplyTransform (elemHandle, transformInfo1);

        // Step 2 : Mirror within the plane. To find the mirror matrix,
        // divide the input matrix (which contains mirror transform)
        // by the view matrix.
        // (NEEDSWORK : For mirrored case, I am currently putting back
        // the old cell's matrix. This means that if the new note has
        // a different rotation setting, it would be ignored. I have to
        // figure out how to retain mirroring as well as new rotation.)
        RotMatrix mirrorMatrix;
        mirrorMatrix.InverseOf(*oldMatrix);
        mirrorMatrix.InitProduct(viewMatrix, mirrorMatrix);

        tMatrix.InitIdentity ();
        tMatrix.InitProduct(tMatrix,mirrorMatrix);

        TransformInfo   transformInfo2 (tMatrix);
        transformInfo2.SetOptions (TRANSFORM_OPTIONS_DisableMirrorCharacters);

        RotMatrix   mirrorPlane;
        mirrorPlane.InverseOf(viewMatrix);
        transformInfo2.SetMirrorPlane (mirrorPlane);

        elemHandle.GetHandler().ApplyTransform (elemHandle, transformInfo2);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void        initElementRotMatrix
(
DgnElementP          el
)
    {
    if (!el)
        return;

    RotMatrix       rMatrix;
    rMatrix.InitIdentity ();

    switch (el->GetLegacyType())
        {
        case CELL_HEADER_ELM:
            {
            if (el->Is3d())
                {
                memcpy (el->ToCell_3dR().transform, rMatrix.form3d, sizeof(el->ToCell_3d().transform));
                }
            else
                {
                el->ToCell_2dR().transform[0][0] = rMatrix.form3d[0][0];
                el->ToCell_2dR().transform[0][1] = rMatrix.form3d[0][1];
                el->ToCell_2dR().transform[1][0] = rMatrix.form3d[1][0];
                el->ToCell_2dR().transform[1][1] = rMatrix.form3d[1][1];
                }
            break;
            }

        case TEXT_NODE_ELM:
            {
            if (el->Is3d())
                rMatrix.GetQuaternion(el->ToText_node_3dR().quat, true);
            else
                el->ToText_node_2dR().rotationAngle = rMatrix.ColumnXAngleXY ();
            break;
            }

        case TEXT_ELM:
            {
            if (el->Is3d())
                rMatrix.GetQuaternion(el->ToText_3dR().quat, true);
            else
                el->ToText_2dR().rotationAngle = rMatrix.ColumnXAngleXY ();
            break;
            }
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void            forceIdentityTransform
(
EditElementHandleR         element,
bool                    handleComponents
)
    {
    if (!element.IsValid())
        return;

    initElementRotMatrix (element.GetElementP());

    if (!handleComponents)
        return;

    for (ChildEditElemIter child (element); child.IsValid(); child=child.ToNext())
        forceIdentityTransform (child, handleComponents);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt        setNoteTransform
(
EditElementHandleR      cellElm,           // in lcs
DVec3d *                xVec,              // in XY plane of creation
ElementHandleCR         dimElement,        // in global (may be null)
RotMatrix *             oldCellMatrix,
DPoint3d *              textNodeOrigin,
bool                    mirror
)
    {
    RotMatrix           newMatrixC;
    newMatrixC.InitIdentity ();

    if (dimElement.IsValid())
        {
        // Compute the new rotation matrix within the note plane
        DVec3d  yVec, zVec;

        zVec.x = zVec.y = 0.0;
        zVec.z = 1.0;
        bsiDVec3d_crossProduct (&yVec, &zVec, xVec);
        newMatrixC.InitFromColumnVectors(*xVec, yVec, zVec);

        // Rotate the new rotation matrix to global
        RotMatrix           planeMatrix;
        DimensionHandler::GetInstance().GetRotationMatrix (dimElement, planeMatrix);    // Use dimrotmatrix for hor and ver

        bsiRotMatrix_multiplyRotMatrixRotMatrix (&newMatrixC, &planeMatrix, &newMatrixC);
        }
    else
        {
        newMatrixC = *oldCellMatrix;
        }

    // Apply new rotation matrix
    applyNoteTransform (cellElm, dimElement, oldCellMatrix, &newMatrixC, NULL, mirror);

    // Move the edP back to it's original location
    Transform           tMatrix;
    tMatrix.InitIdentity ();
    tMatrix.SetTranslation (*textNodeOrigin);

    TransformInfo   tInfo (tMatrix);

    cellElm.GetHandler().ApplyTransform (cellElm, tInfo);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    SunandSandurkar                   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void        applyRotationToNoteCell
(
EditElementHandleR     cellElem,          // in lcs
ElementHandleCR        dimElement,        // in global (may be null)
RotMatrix *         oldCellMatrix,
DPoint3d const*     oldCellOrigin,
DPoint3d *          textNodeOrigin,
bool                mirror,
DgnModelP        modelRef
)
    {
    UInt16          rotation = DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal;
    DVec3d          direction;

    if (dimElement.IsValid())
        {
        mdlDim_getNoteTextRotation (&rotation, dimElement);

        // Inline rotation requires 2 points to determine the direction. If the dimension doesn't have
        // two points yet, treat it as horizontal rotation. This case comes up in the following scenario.
        // While drawing the first-point dynamics of a new note with Start-At-Text, we pass in a dummy dimension
        // in order to access the dimension settings. That dimension does not have any points yet.
        if (2 > dimElement.GetElementCP()->ToDimensionElm().nPoints && DIMSTYLE_VALUE_MLNote_TextRotation_Inline == rotation)
            rotation = DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal;
        }

    switch (rotation)
        {
        case DIMSTYLE_VALUE_MLNote_TextRotation_Inline:
            {
            // Get the last two points along the dimension. The last point on the dimension may
            // be stale since the cell may have moved. So get the cell origin.
            DPoint3d        points[2];
            BentleyApi::mdlDim_extractPointsD (points, dimElement, dimElement.GetElementCP()->ToDimensionElm().nPoints-2, 2);

            if (oldCellOrigin)
                points[1] = *oldCellOrigin;

            // Get the plane of the note. Do not use the view matrix since it does not include mirroring
            RotMatrix       planeMatrixC, planeMatrixR;
            DimensionHandler::GetInstance().GetRotationMatrix (dimElement, planeMatrixC);
            planeMatrixR.InverseOf(planeMatrixC);

            // Rotate the points to the plane lcs
            planeMatrixR.Multiply(points[0]);
            planeMatrixR.Multiply(points[1]);

            // Find the direction
            direction.NormalizedDifference (points[1], points[0]);

            if (direction.x < mgds_fc_epsilon)
                {
                direction.x *= - 1.0;
                direction.y *= - 1.0;
                }

            setNoteTransform (cellElem, &direction, dimElement, oldCellMatrix, textNodeOrigin, mirror);
            break;
            }

        case DIMSTYLE_VALUE_MLNote_TextRotation_Vertical:
            {
            direction.x = 0.0;
            direction.y = 1.0;
            direction.z = 0.0;
            setNoteTransform (cellElem, &direction, dimElement, oldCellMatrix, textNodeOrigin, mirror);
            break;
            }

        default:
        case DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal:
            {
            direction.x = 1.0;
            direction.y = 0.0;
            direction.z = 0.0;
            setNoteTransform (cellElem, &direction, dimElement, oldCellMatrix, textNodeOrigin, mirror);
            break;
            }
        }
    }

/*-------------------------------------------------------------------------------------*
* @bsimethod                                    SunandSandurkar             01/03
+---------------+---------------+---------------+---------------+---------------+------*/
 bool     BentleyApi::mdlNote_isPropertyUpdateAllowed
(
DimStyleProp    eProp,
int             option
)
    {
#if defined (BEIJING_DGNGRAPHICS_WIP_JS)
    if (ADIM_PARAMS_CREATE == option || ADIM_PARAMS_CREATE_FROMDWG == option || ADIM_PARAMS_CHANGE == option)
        return true;

    switch (eProp)
        {
        case DIMSTYLE_PROP_MLNote_Justification_INTEGER:
        case DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER:
        case DIMSTYLE_PROP_MLNote_TextRotation_INTEGER:
        case DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER:
        case DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER:
        case DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER:
            {
            static char     s_scNoteAutoUpdate[256] = "0";
            static UInt32   s_updateTime          = 0;
            UInt32          currentTime           = GetTickCount()/16;

            // allow a rebuild every five seconds
            if (!s_updateTime || (currentTime - s_updateTime) >= 300)
                {
                s_updateTime = GetTickCount()/16;

                mdlSystem_getCfgVar (s_scNoteAutoUpdate, "MS_NOTEAUTOUPDATE", sizeof (s_scNoteAutoUpdate));
                }

            switch (eProp)
                {
                case DIMSTYLE_PROP_MLNote_Justification_INTEGER:
                    return isPropertyInString (s_scNoteAutoUpdate, "HorizontalJustification");

                case DIMSTYLE_PROP_MLNote_VerticalJustification_INTEGER:
                    return isPropertyInString (s_scNoteAutoUpdate, "EditAbout");

                case DIMSTYLE_PROP_MLNote_TextRotation_INTEGER:
                    return isPropertyInString (s_scNoteAutoUpdate, "TextRotation");

                case DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER:
                    return isPropertyInString (s_scNoteAutoUpdate, "HorizontalAttachment");

                case DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER:
                    return isPropertyInString (s_scNoteAutoUpdate, "VerticalLeftAttachment");

                case DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER:
                    return isPropertyInString (s_scNoteAutoUpdate, "VerticalRightAttachment");
                }

            break;
            }

        default:
            return true;
        }
#endif
    return true;
    }

/*-------------------------------------------------------------------------------------*
* @bsimethod                                    SunandSandurkar             01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       note_harvestElements (NoteInfo& noteInfo, ElementHandleCR cellElement)// Has been transformed to LCS
    {
    if (!cellElement.IsValid() || CELL_HEADER_ELM != cellElement.GetLegacyType())
        return ERROR;
    
    noteInfo.frameTypeFromGeom = -1;
    StatusInt status = SUCCESS;
    if (SUCCESS != (status = note_getNoteInfo(cellElement, *cellElement.GetDgnModelP (), noteInfo)))
        return status;

    for (ChildElemIter child(cellElement, ExposeChildrenReason::Count); child.IsValid(); child = child.ToNext())
        {
        if (SUCCESS != (status = note_getNoteInfo (child, *cellElement.GetDgnModelP (), noteInfo)))
            break;
        }
    return status;
    }

/*-------------------------------------------------------------------------------------*
* @bsimethod                                    SunandSandurkar             01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       note_harvestCellOffset
(
NoteInfo *              noteInfo,
NoteProperties *        noteProps,
ElementHandleCR            cellElement,                // Has been transformed to LCS
ElementHandleCR            dimElement,             // May be NULL
bool                    isDimPersistent,
Transform *             tMatrix                 // To LCS. Used for any edp's extracted directly from cache.
)
    {
    if (!cellElement.IsValid() || !dimElement.IsValid())
        return SUCCESS;

    /*-------------------------------------------------------------------
     The idea is to create the leader line based on the horAttachment,
     verLeftAttachment and verRightAttachment. Additionally, the user may
     have moved the previous leader line from it's to-follow location by
     a certain offset. We need to make sure that we maintain this offset.
     0. Find the old default cell origin location
     1. Compute the manual offset
     2. Find the new location of the elbow endpoints
     3. Add the manual offset to the elbow endpoints
    -------------------------------------------------------------------*/

    DPoint3d    cellOrigin;
    CellUtil::ExtractOrigin (cellOrigin, cellElement);

    if (!cellElement.GetElementCP()->Is3d())
        cellOrigin.z = 0.0;

    // Find the previous point-to-follow based on hor and ver attachment flags. Note that the
    // margins and elbow length pertain to the old annotation scale because we are trying to
    // rebuild the old cell as is.
    DPoint3d        shapePoints[5];
    DPoint3d        tightShapePoints[5];
    memcpy (shapePoints, noteInfo->oldInnerBox, sizeof (shapePoints));
    memcpy (tightShapePoints, shapePoints, sizeof (tightShapePoints));
    adjustShape (shapePoints, noteProps->dLeftMargin, noteProps->dLowerMargin);

    DPoint3d    leaderPoints[2];
    computeHorizontalAttachmentLocation (&leaderPoints[0].x, &leaderPoints[1].x, shapePoints, noteProps);

    /* Get the text node elm descriptor */
    ElementHandle     textNode;
    if (SUCCESS != mdlNote_findTextNodeElement (textNode, cellElement))
        return ERROR;

    TextElementJustification just = TextElementJustification::LeftTop;
    mdlNote_getTextNodeJust (just, textNode);
    TextElementJustification dynamicJust = getDynamicJustification (dimElement, isDimPersistent, just);

    leaderPoints[0].y = leaderPoints[1].y = computeVerticalAttachmentLocation (tightShapePoints, textNode, noteProps->dLowerMargin, noteProps->iVerAttachment, dynamicJust);
    leaderPoints[0].z = leaderPoints[1].z = shapePoints[0].z;

    int             frame        = DIMSTYLE_VALUE_MLNote_FrameType_None;
    EditElementHandle      frameElement(noteInfo->pFrameEdP, true);

    if (!frameElement.IsValid() && SUCCESS == mdlDim_getNoteFrameType ((DimStyleProp_MLNote_FrameType *) &frame, dimElement) && DIMSTYLE_VALUE_MLNote_FrameType_None != frame)
        {
        EditElementHandle tmp;
        addTextFrame (tmp, cellElement, dimElement, noteInfo, noteProps);
        frameElement.SetElementDescr(tmp.ExtractElementDescr().get(), false);
        }

    if (DIMSTYLE_VALUE_MLNote_VerAttachment_Underline != noteProps->iVerAttachment && frameElement.IsValid())
        adjustLeaderLineForIntersection (leaderPoints, frameElement, shapePoints, noteProps);

    bsiDPoint3d_subtractDPoint3dDPoint3d (&noteInfo->oldOriginOffset, &cellOrigin, &leaderPoints[1]);

    return SUCCESS;
    }

/*-------------------------------------------------------------------------------------*
* @bsimethod                                    SunandSandurkar                 06/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void            getTextNodeTransform
(
DPoint3d *              textNodeOrigin,
RotMatrix *             textRefMatrix,
RotMatrix *             textMirrorMatrix,
bool    *               mirror,
ElementHandleCR            cellElement,
DgnModelP            modelRef
)
    {
    /*-------------------------------------------------------------------
     Find the rotation matrix that represents the rotation of the textnode. If
     the textnode has mismatches with the note cell, we use the cell matrix and
     note the mismatches inorder to put them back in the new note's textnode.
    -------------------------------------------------------------------*/
    ElementHandle    textNode;
    mdlNote_findTextNodeElement (textNode, cellElement);

    RotMatrix           cellRotMatrix;
    CellUtil::ExtractRotation (cellRotMatrix, cellElement); 

    TextNodeHandler::GetUserOrigin (textNode, *textNodeOrigin);
    
    RotMatrix           textRotMatrix;
    getActualTextRotMatrix (&textRotMatrix, textNode);
    *textRefMatrix = textRotMatrix;

    if (areMatricesEqual (&textRotMatrix, &cellRotMatrix))
        return;

    // Account for legacy note cells that did not store actual rotation matrix
    if (cellRotMatrix.IsIdentity())
        {
        *textRefMatrix = textRotMatrix;
        return;
        }

    double determinant;
    determinant = cellRotMatrix.Determinant ();
    bool isMirror = determinant < -mgds_fc_epsilon;
    // The differences are because of mirroring effects
    if (mirror)
        *mirror = isMirror;
    
    if (isMirror)
        *textRefMatrix = cellRotMatrix;

    if (textMirrorMatrix)
        *textMirrorMatrix = textRotMatrix;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       initializeTextNodeTransform
(
bool *                  upsideDownText,
bool *                  backwardsText,
EditElementHandleR         textNodeElem
)
    {
    // The idea is to clean up the textnode so it doesn't have any remains of
    // old translations or rotations that could interfere with the note
    // regeneration process. We will put back these settings at the very end.
    TextBlock       textBlock (textNodeElem);
    textBlock.SetForceTextNodeFlag (true);

    *upsideDownText = textBlock.GetProperties ().IsUpsideDown ();
    *backwardsText  = textBlock.GetProperties ().IsBackwards ();

    textBlock.SetIsUpsideDown (false);
    textBlock.SetUpsideDownOverrideFlag (true);
    textBlock.SetIsBackwards (false);
    textBlock.SetBackwardsOverrideFlag (true);

    RotMatrix       identity;
    identity.InitIdentity ();
    textBlock.SetOrientation (identity);

    DPoint3d        origin = {0.0, 0.0, 0.0};
    textBlock.SetUserOrigin (origin);

    textBlock.Reprocess ();
    EditElementHandle newElem;
    if (TextBlock::TO_ELEMENT_RESULT_Success != textBlock.ToElement (newElem, textNodeElem.GetDgnModelP (), &textNodeElem))
        return ERROR;
    
    textNodeElem.SetElementDescr(newElem.ExtractElementDescr().get(), false);
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 10/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void        setDimensionShield
(
EditElementHandleR      dimElement,
DimStyleProp            prop
)
    {
    DimensionHandler&   hdlr            = DimensionHandler::GetInstance();
    DimStylePropMaskPtr shieldFlags     = hdlr.GetOverrideFlags (dimElement);

    shieldFlags->SetPropertyBit (prop, true);

    mdlDim_setOverridesDirect (dimElement, shieldFlags.get(), false);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void         setValueInDimension
(
EditElementHandleR  dimElement,
DimStyleProp        prop,
void *              value
)
    {
    switch (prop)
        {
        case DIMSTYLE_PROP_MLNote_LeftMargin_DOUBLE:
            {
            double *    margin = (double *) value;
            mdlDim_setNoteLeftMargin (dimElement, margin);
            break;
            }
        case DIMSTYLE_PROP_MLNote_LowerMargin_DOUBLE:
            {
            double *    margin = (double *) value;
            mdlDim_setNoteLowerMargin (dimElement, margin);
            break;
            }
        case DIMSTYLE_PROP_MLNote_ElbowLength_DOUBLE:
            {
            double *    margin = (double *) value;
            mdlDim_setNoteElbowLength (dimElement, margin);
            break;
            }
        case DIMSTYLE_PROP_MLNote_ShowLeader_BOOLINT:
            {
            bool    * showLeader = (bool    *) value;
            mdlDim_setNoteLeaderDisplay (dimElement, *showLeader);
            break;
            }
        case DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER:
            {
            UInt16 *    attach = (UInt16 *) value;
            mdlDim_setNoteHorAttachment (dimElement, attach);
            break;
            }
        case DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER:
            {
            UInt16 *    attach = (UInt16 *) value;
            mdlDim_setNoteVerLeftAttachment (dimElement, attach);
            break;
            }
        case DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER:
            {
            UInt16 *    attach = (UInt16 *) value;
            mdlDim_setNoteVerRightAttachment (dimElement, attach);
            break;
            }
        case DIMSTYLE_PROP_MLNote_FrameType_INTEGER:
            {
            UInt16 *    frame = (UInt16 *) value;
            mdlDim_setNoteFrameType (dimElement, (DimStyleProp_MLNote_FrameType) *frame);
            break;
            }
        case DIMSTYLE_PROP_MLNote_Justification_INTEGER:
            {
            UInt16 *    just = (UInt16 *) value;
            mdlDim_setNoteHorizontalJustification (dimElement, (DimStyleProp_MLNote_Justification) *just);
            break;
            }
        }

    setDimensionShield (dimElement, prop);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void            rectifyNoteSettings
(
RectifyProperties *     rectifyProps,       // <=>
EditElementHandleR      dimElement,       // <=>
NoteInfo *              noteInfo,           //  =>
ElementHandleCR         cellElement        //  =>
)
    {
    findNoteSettings (rectifyProps, dimElement, noteInfo, cellElement);

    if (rectifyProps->flags.bHasLeader)
        setValueInDimension (dimElement, DIMSTYLE_PROP_MLNote_ShowLeader_BOOLINT, &rectifyProps->props.bHasLeader);

    if (rectifyProps->flags.iHorAttachmentJust)
        setValueInDimension (dimElement, DIMSTYLE_PROP_MLNote_HorAttachment_INTEGER, &rectifyProps->props.iHorAttachmentJust);

    if (rectifyProps->flags.iVerAttachment)
        {
        setValueInDimension (dimElement, DIMSTYLE_PROP_MLNote_VerLeftAttachment_INTEGER, &rectifyProps->props.iVerAttachment);
        setValueInDimension (dimElement, DIMSTYLE_PROP_MLNote_VerRightAttachment_INTEGER, &rectifyProps->props.iVerAttachment);
        }

    if (rectifyProps->flags.dLeftMargin)
        setValueInDimension (dimElement, DIMSTYLE_PROP_MLNote_LeftMargin_DOUBLE, &rectifyProps->props.dLeftMargin);

    if (rectifyProps->flags.dLowerMargin)
        setValueInDimension (dimElement, DIMSTYLE_PROP_MLNote_LowerMargin_DOUBLE, &rectifyProps->props.dLowerMargin);

    if (rectifyProps->flags.dElbowLength)
        setValueInDimension (dimElement, DIMSTYLE_PROP_MLNote_ElbowLength_DOUBLE, &rectifyProps->props.dElbowLength);

    if (rectifyProps->flags.iFrameType)
        setValueInDimension (dimElement, DIMSTYLE_PROP_MLNote_FrameType_INTEGER, &rectifyProps->props.iFrameType);

    if (rectifyProps->flags.iHorJust)
        setValueInDimension (dimElement, DIMSTYLE_PROP_MLNote_Justification_INTEGER, &rectifyProps->props.iHorJust);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool            hasRequiredSettings (ElementHandleCR dimElement)
    {
    // The following settings have to be specified. If absent, they cause ambiguity and
    // therefore have to be reverse-engineered from the cell geometry.
    if (SUCCESS != mdlDim_getNoteLeaderDisplay (NULL, dimElement) ||
        SUCCESS != mdlDim_getNoteHorizontalJustification (NULL, dimElement) ||
        SUCCESS != mdlDim_getNoteFrameType (NULL, dimElement) ||
        false   == mdlDim_getNoteLeftMargin (NULL, dimElement) ||
        false   == mdlDim_getNoteLowerMargin (NULL, dimElement) ||
        false   == mdlDim_getNoteElbowLength (NULL, dimElement))
        return false;

    // The following settings have default meanings when unspecified. They don't have to be
    // reverse-engineered.
    //  mdlDim_getNoteHorAttachment (NULL, (DgnElementP) dimElement);
    //  mdlDim_getNoteVerLeftAttachment (NULL, (DgnElementP) dimElement);
    //  mdlDim_getNoteVerRightAttachment (NULL, (DgnElementP) dimElement);
    //  mdlDim_getMultiJustVertical (NULL, (DgnElementP) dimElement);
    //  mdlDim_getNoteFrameScale (NULL, dimElement);
    //  mdlDim_getNoteLeaderType (NULL, (DgnElementP) dimElement);
    //  mdlDim_getNoteTextRotation (NULL, (DgnElementP) dimElement);

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/05
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt       BentleyApi::mdlNote_rectifyDimSettings
(
bool    *               changed,            // <=  true if any settings were changed. Use to determine if dim should be rewritten
EditElementHandleR      newDimElemHandle,   //  =>
ElementHandleCR         dimElemHandle,      //  =>
DgnModelP            modelRef            //  =>
)
    {
    MSElementDescrCP    dimEdP     = dimElemHandle.GetElementDescrCP ();
    if (!dimEdP)
        return ERROR;

    if (changed)
        *changed = false;

    if (hasRequiredSettings (dimElemHandle))
        return SUCCESS;

    newDimElemHandle.Duplicate (dimElemHandle);

    /*-------------------------------------------------------------------
       Get the associated cell
    -------------------------------------------------------------------*/
    ElementId cellElementID;
    if (SUCCESS != mdlNote_getRootNoteCellId (&cellElementID, newDimElemHandle))
        return ERROR;

    EditElementHandle      cellElemHandle;
    if (SUCCESS != cellElemHandle.FindById (ElementId(cellElementID), modelRef))
        return SUCCESS;

    /*-------------------------------------------------------------------
       Compute the transform from global to local
    -------------------------------------------------------------------*/
    DPoint3d            textNodeOrigin;
    Transform           tMatrix;
    tMatrix.InitIdentity ();

    RotMatrix           rMatrix, invRMatrix;
    ElementHandle textNodeElement;
    if (SUCCESS != mdlNote_findTextNodeElement (textNodeElement, cellElemHandle))
        {
        DimensionHandler::GetInstance().GetRotationMatrix (newDimElemHandle, rMatrix);
        invRMatrix.InverseOf(rMatrix);
        memset (&textNodeOrigin, 0, sizeof(textNodeOrigin));
        }
    else
        {
        getTextNodeTransform (&textNodeOrigin, &rMatrix, NULL, NULL, cellElemHandle, modelRef);
        invRMatrix.InverseOf(rMatrix);
        }

    tMatrix.SetFixedPoint (textNodeOrigin);
    tMatrix.InitProduct(tMatrix,invRMatrix);

    TransformInfo   tInfo (tMatrix);

    cellElemHandle.GetHandler().ApplyTransform (cellElemHandle, tInfo);
    newDimElemHandle.GetHandler().ApplyTransform (newDimElemHandle, tInfo);

    /*-------------------------------------------------------------------
      Harvest the elements from the old note cell, if available
    -------------------------------------------------------------------*/
    NoteInfo noteInfo;
    memset (&noteInfo, 0, sizeof(noteInfo));
    if (SUCCESS != note_harvestElements (noteInfo, cellElemHandle))
        return ERROR;

    /*-------------------------------------------------------------------
       Rectify the properties
    -------------------------------------------------------------------*/
    RectifyProperties rectifyProps;
    memset (&rectifyProps, 0, sizeof(rectifyProps));
    rectifyNoteSettings (&rectifyProps, newDimElemHandle, &noteInfo, cellElemHandle);

    bool    anyChanged = (0 != (*(UShort *) &rectifyProps.flags));

    if (changed)
        *changed = anyChanged ? true : false;

    if (anyChanged)
        {
        tMatrix.InitIdentity ();
        tMatrix.SetFixedPoint (textNodeOrigin);
        tMatrix.InitProduct(tMatrix,rMatrix);

        TransformInfo   tInfo (tMatrix);

        newDimElemHandle.GetHandler().ApplyTransform (newDimElemHandle, tInfo);
        }

    return SUCCESS;
    }

#ifdef WIP_VANCOUVER_MERGE // note
/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    AbeeshBasheer 06/22
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    mdlNote_areIdentical (ElementHandleCR lhs, ElementHandleCR rhs)
    {
    CompareElements comparer;
    EditElementHandle oldElement(lhs, true);
    EditElementHandle newElement(rhs, true);
    return comparer.areEqual (oldElement.GetElementDescrCP(), newElement.GetElementDescrCP(), CompareElements::FLAGS_IgnoreElementIDs|CompareElements::FLAGS_IgnoreLastModifiedTime);
    }
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlNote_rectifySettingsAndCommit
(
ElementHandleR             cellElemHandle,     //  =>
DgnModelP            modelRef            //  =>
)
    {
    ElementId               dimElmId;
    EditElementHandle      dimElemHandle;
    NoteCellHeaderHandler* noteHandler = dynamic_cast<NoteCellHeaderHandler*> (&cellElemHandle.GetHandler());
    if (NULL == noteHandler)
        return SUCCESS;

    // It is okay if the leader is not found
    if (SUCCESS != noteHandler->GetLeaderDimension (dimElmId, NULL, cellElemHandle) ||
        SUCCESS != dimElemHandle.FindById (dimElmId, modelRef))
        return SUCCESS;

    bool                changed = false;
    StatusInt           status  = SUCCESS;
    EditElementHandle      newDimElemHandle;

    if (SUCCESS == (status = mdlNote_rectifyDimSettings (&changed, newDimElemHandle, dimElemHandle, modelRef)) && changed)
        status = newDimElemHandle.ReplaceInModel (dimElemHandle.GetElementRef ());

    return status;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       getText
(
EditElementHandleR         textElemOut,
ElementHandleCP            newTextElemHandle,
ElementHandleCR            oldCellElm
)
    {
    MSElementDescrPtr textNodeEdP;

    if (newTextElemHandle && newTextElemHandle->IsValid ())
        {
        if (TEXT_NODE_ELM != newTextElemHandle->GetLegacyType())
            {
            BeAssert (false && "Did not find text_node_elm where expected");
            return ERROR;
            }

        textNodeEdP = newTextElemHandle->GetElementDescrCP()->Duplicate(true, true);
        }
    else
        {
        ElementHandle txtNodeElement;
        if (SUCCESS == mdlNote_findTextNodeElement (txtNodeElement, oldCellElm))
            textNodeEdP = txtNodeElement.GetElementDescrCP()->Duplicate(true, true);
        }

    if (!textNodeEdP.IsValid())
        return ERROR;

    textElemOut.SetElementDescr (textNodeEdP.get(), false); 
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool            isDimensionPersistent
(
ElementHandleCR            dimElemHandle
)
    {
    return (dimElemHandle.IsPersistent () || (NULL != dimElemHandle.GetElementDescrCP () && NULL != dimElemHandle.GetElementDescrCP ()->GetElementRef()));
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 12/05
+---------------+---------------+---------------+---------------+---------------+------*/
static void            switchAnnotationScale
(
NoteProperties *        noteProps,
double *                pNewAnnotScale,
double *                pOldAnnotScale
)
    {
    if (!pNewAnnotScale && !pOldAnnotScale)
        return;

    double          newAnnotScale    = 1.0;
    double          oldAnnotScale    = 1.0;

    if (NULL != pNewAnnotScale && fabs (*pNewAnnotScale - 0.0) > mgds_fc_epsilon)
        newAnnotScale = *pNewAnnotScale;

    // oldAnnotScale is valid only when applying a change in model annot scale.
    // Otherwise the old and current annot scales are the same.
    if (NULL == pOldAnnotScale)
        oldAnnotScale = newAnnotScale;
    else if (fabs (*pOldAnnotScale - 0.0) > mgds_fc_epsilon)
        oldAnnotScale = *pOldAnnotScale;

    noteProps->dLeftMargin  *= (newAnnotScale / oldAnnotScale);
    noteProps->dLowerMargin *= (newAnnotScale / oldAnnotScale);
    noteProps->dElbowLength *= (newAnnotScale / oldAnnotScale);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void            cleanTextNode
(
EditElementHandleR       textNodeElement
)
    {
    if (!textNodeElement.IsValid())
        return;

    forceIdentityTransform (textNodeElement, true);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 04/04
+---------------+---------------+---------------+---------------+---------------+------*/
enum NoteTextValidity
    {
    NOTETEXT_Valid      = 0,
    NOTETEXT_Blank      = 1,
    NOTETEXT_Invalid    = 2
    };

static NoteTextValidity     hasValidText
(
ElementHandleCP     newTextElemHandle
)
    {
    if (NULL == newTextElemHandle || !newTextElemHandle->IsValid ())
        return NOTETEXT_Blank;

    MSElementDescrCP    textNodeEdP = newTextElemHandle->PeekElementDescrCP ();
    if (NULL != textNodeEdP)
        {
        if (0 == textNodeEdP->Components().size())
            {
            BeDataAssert (0);
            return NOTETEXT_Invalid;
            }
        return NOTETEXT_Valid;
        }
    
    ChildElemIter childIter (*newTextElemHandle, ExposeChildrenReason::Count);
    return childIter.IsValid() ? NOTETEXT_Valid : NOTETEXT_Blank;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 02/06
+---------------+---------------+---------------+---------------+---------------+------*/
static DimensionElm const*  getSafeDimElement
(
ElementHandleCR            dimElemHandle               //  => may be null
)
    {
    if (dimElemHandle.IsValid ())
        return (DimensionElm *) dimElemHandle.GetElementCP ();
    return NULL;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void            mirrorElement
(
EditElementHandleR      elemHandle,
DPoint3d *              origin,
ElementHandleCR         dimElement,
RotMatrix *             fromMatrix,
RotMatrix *             toMatrix
)
    {
    // This function is called in the mirror case when text and cell have
    // different orientations. This function will rotate the input element
    // to match the toMatrix.

    RotMatrix mirrorMatrix;
    mirrorMatrix.InverseOf(*fromMatrix);
    mirrorMatrix.InitProduct(*toMatrix, mirrorMatrix);

    Transform tMatrix;
    tMatrix.InitIdentity ();
    tMatrix.InitProduct(tMatrix,mirrorMatrix);

    TransformInfo   transformInfo (tMatrix);
    UInt32          options = TRANSFORM_OPTIONS_DisableMirrorCharacters;

    transformInfo.SetOptions (options);

    RotMatrix   viewMatrix;
    if (dimElement.IsValid())
        {
        note_getDimViewMatrix (viewMatrix, dimElement);

        RotMatrix   mirrorPlane;
        mirrorPlane.InverseOf(viewMatrix);

        transformInfo.SetMirrorPlane (mirrorPlane);
        }
    elemHandle.GetHandler().ApplyTransform (elemHandle, transformInfo);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 03/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void            transformElement
(
EditElementHandleR      elemHandle,
DPoint3d *              origin,
ElementHandleCR         dimElement,
bool                    mirror,
Transform *             tMatrix,
RotMatrix *             rMatrix,
RotMatrix *             textMirrorMatrix
)
    {
    if (!mirror)
        {
        TransformInfo tInfo (*tMatrix);
        elemHandle.GetHandler().ApplyTransform (elemHandle, tInfo);
        return;
        }
    
    // Step1 : Flip the cell (or dimension) to match the text transform
    mirrorElement (elemHandle, origin, dimElement, rMatrix, textMirrorMatrix);

    // Step2 : Transform all of them to lcs (using text mirror matrix)
    Transform   tMatrix1;
    tMatrix1.InitIdentity ();
    tMatrix1.SetFixedPoint (*origin);

    RotMatrix   invTextMirrorMatrix;
    invTextMirrorMatrix.InverseOf(*textMirrorMatrix);
    tMatrix1.InitProduct(tMatrix1,invTextMirrorMatrix);

    TransformInfo tInfo (tMatrix1);
    elemHandle.GetHandler().ApplyTransform (elemHandle, tInfo);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void            insertInvisibleDimension
(
EditElementHandleR         newCellElem,
ElementHandleCP            dimElemHandle,
ElementHandleCP            oldDimElemHandle,
ElementHandleCR            oldCellElement
)
    {
    DgnElementCP         visibleDim       = NULL;
    MSElementDescrPtr   newInvisDimEdP;
    ElementHandle        oldInvisDim;
    bool                foundOld        = SUCCESS == mdlNote_findCellComponent (oldInvisDim, DIMENSION_ELM, oldCellElement);

    visibleDim = (DgnElementCP) (NULL != dimElemHandle ? getSafeDimElement (*dimElemHandle) : NULL);
    if (NULL == visibleDim)
        visibleDim = (DgnElementCP) (NULL != oldDimElemHandle ? getSafeDimElement (*oldDimElemHandle) : NULL);

    if (visibleDim)
        {
        // Always clone the visible dimension in order to match the endpoints
        newInvisDimEdP = MSElementDescr::Allocate (*visibleDim, *oldCellElement.GetDgnModelP ());
        newInvisDimEdP->ElementR().InvalidateElementId();
        }
    else if (foundOld)
        {
        newInvisDimEdP = oldInvisDim.GetElementDescrCP()->Duplicate();
        }
    else
        {
        // NEEDSWORK : In the future we should consider inserting a new dimension element and style that match the old note's settings.
        return;
        }

    if (foundOld)
        newInvisDimEdP->ElementR().SetElementId(oldInvisDim.GetElementId());

    newInvisDimEdP->ElementR().SetInvisible(true);

    newCellElem.GetElementDescrP()->AddComponent(*newInvisDimEdP);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       getOwnedCell
(
EditElementHandleR         cellElm,
ElementHandleCR            dimElemHandle,
DgnModelP            modelRef
)
    {
    ElementId cellID;
    if (SUCCESS != mdlNote_getRootNoteCellId (&cellID, dimElemHandle) ||
        SUCCESS != cellElm.FindById (ElementId(cellID), modelRef))
        return ERROR;

    ElementId ownerDimElementID;
    NoteCellHeaderHandler* noteHandler = dynamic_cast<NoteCellHeaderHandler*> (&cellElm.GetHandler());
    if (NULL == noteHandler)
        return ERROR;

    if (SUCCESS != noteHandler->GetLeaderDimension (ownerDimElementID, NULL, cellElm) || ownerDimElementID != dimElemHandle.GetElementCP ()->GetElementId())
        {
        cellElm.Invalidate();
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   retargetMissingField (EditElementHandleR textElem, ElementHandleCP newTextElemIn, ElementHandleCR leader)
    {
    if (NULL == newTextElemIn)
        return ERROR;

    DimensionElm const* dimElm = getSafeDimElement(leader);
    if (NULL == dimElm || !dimElm->GetDimTextCP(0)->flags.b.associative)
        return ERROR;
    
    if (dimElm->IsInvisible())
        return ERROR;

    AssocPoint assocPt;
    if (SUCCESS != AssociativePoint::ExtractPoint (assocPt, leader, 0, dimElm->nPoints))
        return ERROR;
    
    ElementRefP elemRef;
    DgnModelP modelRef;
    if (SUCCESS != AssociativePoint::GetRoot(&elemRef, &modelRef, NULL, NULL, assocPt, leader.GetDgnModelP()))
        return ERROR;

    TextBlockPtr newText = TextHandlerBase::GetFirstTextPartValue(*newTextElemIn);
    if (newText.IsNull())
        return ERROR;

#ifdef WIP_VANCOUVER_MERGE // text
    ElementHandle newtarget(elemRef, modelRef);
    if (SUCCESS != newText->ReTargetField(NULL, DisplayPath(newtarget.GetElementRef(), newtarget.GetDgnModelP())))
        return ERROR;
#endif

    EditElementHandle textEeh;
    if (TextBlock::TO_ELEMENT_RESULT_Success != newText->ToElement (textEeh, newTextElemIn->GetDgnModelP (), newTextElemIn))
        return ERROR;

    textElem.ReplaceElementDescr (textEeh.ExtractElementDescr().get());

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 04/04
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt        BentleyApi::mdlNote_updateSettings
(
EditElementHandleR         newCellElem,                // <=  new multiline note
ElementHandleCP            newTextElemIn,              //  => new text node with identity transform. NULL if removing entire text
DPoint3d const*              cellOrigin,                 //  => cell origin, required for new creation
ElementHandleCR            dimElemHandle,              //  => associated dim element with updated settings. If null, use geometry to derive settings
ElementHandleCP            oldDimElemHandle,           //  => associated dim element w/o  updated settings. If settings are same, pass NULL
ElementHandleCP            oldCellElemHandle,          //  => required only when dimElemHandle is invalid
DgnModelP            modelRef,                   //  => modelref to create in
bool                    bRetainCellOrigin,          //  => recreate note around the previous cell origin. If No, the cell is allowed to shift due to some settings.
double *                pNewAnnotScale,             //  => new annotation scale of note, if changing, else NULL
double *                pOldAnnotScale              //  => annotation scale of note, if changing, NULL otherwise
)
    {
    //TODO Refactor this code into smaller pieces
    StatusInt           status = SUCCESS;

    dumpTextNode   (newTextElemIn, "newTextElem");
    dumpDimension  (dimElemHandle, "dimElement");

    /*-------------------------------------------------------------------
      Change Propagation
      ==================
      In some cases, the text should be retained in its original textframe
      location. In other cases, the text should be moved in order to keep
      the cell origin constant.

      Setting                       Modify Leader Point to the 'other side'
      ========                      =======================================
      Dynamic Text Justification    Retain cell origin
                                    Shift text in frame

      Auto Hor Attachment           Retain cell origin
                                    Flip text

      Auto Ver Attachments          Retain cell origin
                                    Move text

      Inline Text Rotation          Retain cell origin
                                    Rotate text

      Setting Changed               Save Style
      ===============               ==========
      Text Justification            Retain cell origin
      ex. Left to Right             Shift text in frame

      Hor Attachment                Move cell origin
      ex. Left to Right             Retain text in frame

      Ver Attachments               Move cell origin
      ex. TopLine to Underline      Retain text in frame

      Text Rotation                 Retain cell origin
      ex. Horizonal to Vertical     Rotate text
    -------------------------------------------------------------------*/

    // Early out if removing the entire text
    switch (hasValidText (newTextElemIn))
        {
        case NOTETEXT_Blank:
            newCellElem.SetElementDescr (NULL, false);
            return SUCCESS;
        case NOTETEXT_Invalid:
            newCellElem.SetElementDescr (NULL, false);
            return ERROR;
        }

    /*-------------------------------------------------------------------
      Prepare the edP's by rotating to LCS about the textnode origin and
      rotmatrix. If we are modifying an existing note, then we use the
      rotmatrix from the textnode and inv-rotate all the components of
      the note. If this is a new note, then we get the rotmatrix from the
      dimension and inv-rotate only the dimension since the new text is
      provided in LCS. When we are done building the note, we will use
      the same rotmatrix to transform all components back to global.
      When regenerating an existing note, the new text that comes in is
      in global and needs to be inv-rotated. However, when creating a new
      note, the new text does not have any transform and so doesn't have
      to be inv-rotated.
    -------------------------------------------------------------------*/
    EditElementHandle      oldCellElm;
    ElementRefP             oldElemRef = NULL;
    if (NULL != oldCellElemHandle && oldCellElemHandle->IsValid ())
        {
        oldElemRef = oldCellElemHandle->GetElementRef();
        oldCellElm.Duplicate (*oldCellElemHandle);
        }
    else if (isDimensionPersistent (dimElemHandle)) // Don't check for cell id if creating new note
        {
        if (SUCCESS != getOwnedCell (oldCellElm, dimElemHandle, modelRef))
            return ERROR;
        oldElemRef = oldCellElm.GetElementRef();
        }

    DPoint3d            oldCellOrigin;
    if (bRetainCellOrigin && oldCellElm.IsValid())
        CellUtil::ExtractOrigin (oldCellOrigin, oldCellElm); 

    dumpCell (oldCellElm, "oldCellEdP");

    EditElementHandle      newTextElem;
    if (SUCCESS != getText (newTextElem, newTextElemIn, oldCellElm))
        return ERROR;

    retargetMissingField (newTextElem, newTextElemIn, dimElemHandle);

    bool     upsideDownText = false, backwardsText = false;
    if (SUCCESS != initializeTextNodeTransform (&upsideDownText, &backwardsText, newTextElem))
        return ERROR;

    DPoint3d            textNodeOrigin;
    Transform           tMatrix;
    tMatrix.InitIdentity ();

    bool                mirror = false;
    RotMatrix           rMatrix, invRMatrix, textMirrorMatrix;
    EditElementHandle      oldTextNodeElement;
    if (oldCellElm.IsValid() && SUCCESS == mdlNote_findTextNodeElement (oldTextNodeElement, oldCellElm))
        {
        getTextNodeTransform (&textNodeOrigin, &rMatrix, &textMirrorMatrix, &mirror, oldCellElm, modelRef);
        invRMatrix.InverseOf(rMatrix);
        }
    else if (dimElemHandle.IsValid ())
        {
        DimensionHandler::GetInstance().GetRotationMatrix (dimElemHandle, rMatrix);
        invRMatrix.InverseOf(rMatrix);
        memset (&textNodeOrigin, 0, sizeof(textNodeOrigin));
        }
    else
        {
        rMatrix.InitIdentity ();
        invRMatrix.InitIdentity ();
        memset (&textNodeOrigin, 0, sizeof(textNodeOrigin));
        }

    tMatrix.SetFixedPoint (textNodeOrigin);
    tMatrix.InitProduct(tMatrix,invRMatrix);

    // Flip the old cell and its contents to LCS
    if (oldCellElm.IsValid())
        {
        transformElement (oldCellElm, &textNodeOrigin, dimElemHandle, mirror, &tMatrix, &rMatrix, &textMirrorMatrix);
        // Reget the old textnodeEdP since the pointer may have changed.
        mdlNote_findTextNodeElement (oldTextNodeElement, oldCellElm);
        cleanTextNode (oldTextNodeElement);
        }

    /*-------------------------------------------------------------------
      Harvest the elements from the old note cell, if available
    -------------------------------------------------------------------*/
    NoteInfo noteInfo;
    memset (&noteInfo, 0, sizeof(noteInfo));
    if (oldCellElm.IsValid() && SUCCESS != (status = note_harvestElements (noteInfo, oldCellElm)))
        return status;

    /*-------------------------------------------------------------------
      Prepare new settings
    -------------------------------------------------------------------*/
    EditElementHandle dimElmLocal(dimElemHandle, true);
    if (getSafeDimElement (dimElemHandle))
        transformElement (dimElmLocal, &textNodeOrigin, dimElemHandle, mirror, &tMatrix, &rMatrix, &textMirrorMatrix);

    NoteProperties      noteProps;
    memset (&noteProps, 0, sizeof(noteProps));
    getNoteProperties (&noteProps, &noteInfo, dimElmLocal, false, oldCellElm);

    NoteProperties      oldNoteProps(noteProps);
    memset (&oldNoteProps, 0, sizeof(oldNoteProps));

    EditElementHandle oldDimElmLocal (oldDimElemHandle ? *oldDimElemHandle : dimElemHandle , true);
    if (oldDimElemHandle && oldDimElemHandle->IsValid () && oldDimElemHandle->GetElementCP ())
        {
        transformElement (oldDimElmLocal, &textNodeOrigin, dimElemHandle, mirror, &tMatrix, &rMatrix, &textMirrorMatrix);
        getNoteProperties (&oldNoteProps, &noteInfo, oldDimElmLocal, true, oldCellElm);
        }
    else
        {
        oldNoteProps = noteProps;
        }

    /*-------------------------------------------------------------------
      Set up the note props with the appropriate annotation scale
    -------------------------------------------------------------------*/
    if (pNewAnnotScale && pOldAnnotScale)
        {
        // If the modified dim was not given, then the old props
        // should be multiplied by the new annotation scale
        // in order to get new props.
        if (!getSafeDimElement (dimElemHandle))
            switchAnnotationScale (&noteProps, pNewAnnotScale, pOldAnnotScale);

        // If the old dim was not given, then the new props
        // should be multiplied by the old annotation scale
        // in order to get old props.
        if (!oldDimElemHandle)
            switchAnnotationScale (&oldNoteProps, pOldAnnotScale, pNewAnnotScale);
        }

    /*-------------------------------------------------------------------
      Harvest the old note cell offset
    -------------------------------------------------------------------*/
    if (SUCCESS != (status = note_harvestCellOffset (&noteInfo, &oldNoteProps, oldCellElm, oldDimElmLocal, isDimensionPersistent (dimElemHandle), &tMatrix)))
        return status;

    /*-------------------------------------------------------------------
      Override the text justification based on new settings
    -------------------------------------------------------------------*/
    TextElementJustification    oldJust = TextElementJustification::LeftTop;
    int                         newHorJustMode = 0;
    UInt16                      newVerJustMode = 0;

    // Get the old justification
    if (oldTextNodeElement.IsValid())
        mdlNote_getTextNodeJust (oldJust, oldTextNodeElement);
    else
        mdlNote_getTextNodeJust (oldJust, newTextElem);

    // Get the justification settings
    newHorJustMode = noteProps.iHorJust;
    if (!dimElmLocal.IsValid())
        {
        newVerJustMode = VERJUSTMODE (oldJust);
        }
    else
        {
        if (false == mdlDim_getMultiJustVertical (&newVerJustMode, dimElmLocal))
            newVerJustMode = VERJUSTMODE (oldJust);
        }

    // Compute the new justification
    TextElementJustification resolvedJust = TextElementJustification::LeftTop, dynamicJust = getDynamicJustification (dimElmLocal, isDimensionPersistent (dimElemHandle), oldJust);
    resolveJustification (&resolvedJust, dimElmLocal, oldJust, dynamicJust, newHorJustMode, newVerJustMode);

    // Set the new justification on the new text
    TextElementJustification curNewJust = TextElementJustification::LeftTop;
    mdlNote_getTextNodeJust (curNewJust, newTextElem);
    if (resolvedJust != curNewJust)
        {
        if (SUCCESS != setTextNodeJust (newTextElem, resolvedJust))
            return ERROR;
        }

    /*-------------------------------------------------------------------
      Start building the new note. Use the cell header from the template
      note to retain any associations.
    -------------------------------------------------------------------*/
    if (noteInfo.pCellElm.IsValid())
        {
        MSElementDescrP newCellEdP = new MSElementDescr(*noteInfo.pCellElm.GetElementCP(), *noteInfo.pCellElm.GetDgnModelP());
        newCellElem.SetElementDescr(newCellEdP, false);
        
        newTextElem.GetElementDescrP()->ElementR().SetElementId(ElementId(noteInfo.nodeID));
        forceIdentityTransform (newCellElem, false);
        }
    else
        {
        NormalCellHeaderHandler::CreateOrphanCellElement (newCellElem, NULL, modelRef->Is3d(), *modelRef);

//TODO Test Note in dynamics
//        if (!tcb->inDynamics) // I doubt this test is needed
        mdlNote_addNoteCellLinkage (newCellElem);
        }

    // transfer xattr changes and clone source to new element
    if (oldCellElm.IsValid())
        oldCellElm.GetElementDescrP()->DonateXAttributeChangeSetTo(newCellElem.GetElementDescrP());

    newCellElem.GetElementDescrP()->AddComponent(*newTextElem.ExtractElementDescr().get());
    // From here down, newTextElem is invalid

    EditElementHandle frameElement;
    addTextFrame (frameElement, newCellElem, dimElmLocal, &noteInfo, &noteProps);
    if (frameElement.IsValid())
        NormalCellHeaderHandler::AddChildElement (newCellElem, frameElement);

    DPoint3d            tightShapePoints[5], shapePoints[5];
    extractShape (shapePoints, newCellElem);
    memcpy (tightShapePoints, shapePoints, sizeof (shapePoints));
    adjustShape (shapePoints, noteProps.dLeftMargin, noteProps.dLowerMargin);

    DPoint3d        newLeaderPoints[2];
    computeHorizontalAttachmentLocation (&newLeaderPoints[0].x, &newLeaderPoints[1].x, shapePoints, &noteProps);

    ElementHandle newTextElement;
    mdlNote_findTextNodeElement (newTextElement, newCellElem);

    newLeaderPoints[0].y = newLeaderPoints[1].y = computeVerticalAttachmentLocation (tightShapePoints, newTextElement, noteProps.dLowerMargin, noteProps.iVerAttachment, dynamicJust);
    newLeaderPoints[0].z = newLeaderPoints[1].z = shapePoints[0].z;

    if (DIMSTYLE_VALUE_MLNote_VerAttachment_Underline != noteProps.iVerAttachment)
        adjustLeaderLineForIntersection (newLeaderPoints, frameElement, shapePoints, &noteProps);

    /*-------------------------------------------------------------------
      Add the previous offset
    -------------------------------------------------------------------*/
    bsiDPoint3d_addDPoint3dDPoint3d (&newLeaderPoints[0], (DVec3d *)&newLeaderPoints[0], (DVec3d *)&noteInfo.oldOriginOffset);
    bsiDPoint3d_addDPoint3dDPoint3d (&newLeaderPoints[1], (DVec3d *)&newLeaderPoints[1], (DVec3d *)&noteInfo.oldOriginOffset);

    /*-------------------------------------------------------------------
      Create leader
    -------------------------------------------------------------------*/
    if (noteProps.bHasLeader)
        {
        EditElementHandle  tmpEeh;
        DSegment3d      segment;

        segment.Init (newLeaderPoints[0], newLeaderPoints[1]);
        LineHandler::CreateLineElement (tmpEeh, noteInfo.pLeaderElm.IsValid() ? &noteInfo.pLeaderElm : NULL, segment, modelRef->Is3d(), *modelRef);
        if (noteInfo.pLeaderElm.IsValid())
            {
            ElementId leaderElemID = noteInfo.pLeaderElm.GetElementId ();
            if (leaderElemID.IsValid())
                tmpEeh.GetElementP ()->SetElementId(leaderElemID);
            }

        if (dimElemHandle.IsValid ())
            setSymbologyFromDimension (tmpEeh, dimElemHandle.GetElementCP()->ToDimensionElm());

        NormalCellHeaderHandler::AddChildElement (newCellElem, tmpEeh);
        }

    setCellOrigin (newCellElem.GetElementDescrP(), &newLeaderPoints[1]);

    /*-------------------------------------------------------------------
      Apply note rotation and rotate the new cell back to global
      about the textnode origin.
    -------------------------------------------------------------------*/
    applyRotationToNoteCell (newCellElem, dimElemHandle, &rMatrix, cellOrigin, &textNodeOrigin, mirror, modelRef);

    /*-------------------------------------------------------------------
      When creating a new note, move the cell origin to the specified location
    -------------------------------------------------------------------*/
    if (cellOrigin)
        {
        DPoint3d    shift, curOrigin;
        CellUtil::ExtractOrigin (curOrigin, newCellElem); 
        bsiDPoint3d_subtractDPoint3dDPoint3d (&shift, cellOrigin, &curOrigin);
        tMatrix.InitIdentity ();
        tMatrix.SetTranslation (shift);

        TransformInfo   tInfo (tMatrix);

        newCellElem.GetHandler().ApplyTransform (newCellElem, tInfo);
        }

    /*-------------------------------------------------------------------
      When updating an existing note, the rearrangement may have shifted
      its cell origin. Users expect the rearrangement to happen relative
      to the cell origin as the pivot point. Therefore, shift it back to
      where it was.
    -------------------------------------------------------------------*/
    if (bRetainCellOrigin)
        {
        DPoint3d cellOffset, newOrigin;
        CellUtil::ExtractOrigin (newOrigin, newCellElem); 
        bsiDPoint3d_subtractDPoint3dDPoint3d (&cellOffset, &oldCellOrigin, &newOrigin);
        tMatrix.InitIdentity ();
        tMatrix.SetTranslation (cellOffset);

        TransformInfo   tInfo (tMatrix);

        newCellElem.GetHandler().ApplyTransform (newCellElem, tInfo);
        }

    /*-------------------------------------------------------------------
      Create invisible dimension. Do this after the cell has been located
      in its final location.
    -------------------------------------------------------------------*/
    insertInvisibleDimension (newCellElem, &dimElemHandle, oldDimElemHandle, oldCellElm);

    /*-------------------------------------------------------------------
      Add the dimension dependency on cell if it is missing. The
      intention here is to "fix" existing notes that do not have the
      dependency from the note cell to the leader dimension. We also
      do this type of fixing when the leader endpoints are modified.
    -------------------------------------------------------------------*/
    mdlNote_addDimDependencyToCell (newCellElem);

    if (SUCCESS != NormalCellHeaderHandler::SetCellRange (newCellElem))
        return ERROR;

    // Update component count...
    newCellElem.GetElementDescrP()->Validate ();

    if (oldCellElm.IsValid())
        {
        // NOTE: Propagate old elementRef/modelRef for updateDimDependentNote...
        newCellElem.GetElementDescrP()->SetElementRef(oldElemRef);
        }
    else
        {
        // NOTE: Need modelRef for display of new notes...NULL is ignored!
        newCellElem.GetElementDescrP()->SetDgnModel(*modelRef); // Should not be necessary...create sets this...
        }

    NoteCellHeaderHandler::GetInstance().UpdateOffsetAssociation(newCellElem, dimElemHandle);
    
#if defined (SET_DYNAMIC_RANGE_ON_ALL_ANNOTATIONS)
    if (mdlNote_getAnnotationScale (NULL, newCellEdP))
        newCellEdP->el.hdr.dhdr.props.b.dynamicRange = true;
#endif

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BentleyApi::mdlNote_updateDimSettings
(
EditElementHandleR         newNoteElement,
ElementHandleCR            oldNoteElement,
ElementHandleCR            newDimElemHandle
)
    {
    ElementHandle    oldDimElemHandle;
    if (SUCCESS != mdlNote_findCellComponent (oldDimElemHandle, DIMENSION_ELM, oldNoteElement))
        return ERROR;

    ElementHandle    textElemHandle;
    if (SUCCESS != mdlNote_findCellComponent (textElemHandle, TEXT_NODE_ELM, oldNoteElement))
        return ERROR;

    if (SUCCESS != mdlNote_updateSettings (newNoteElement, &textElemHandle, NULL, newDimElemHandle, &oldDimElemHandle, &oldNoteElement, newDimElemHandle.GetDgnModelP(), true, NULL, NULL))
        return ERROR;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 04/04
*
* NOTE : Every caller who wants to regenerate notes should call mdlNote_rectifySettingsAndCommit
*        before calling this function. mdlNote_create function assumes that all the
*        settings are available, and will not do any reverse-engineering rectification.
*
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt        BentleyApi::mdlNote_create
(
EditElementHandleR      newCellElemHandle,          // <=  new multiline note
ElementHandleCP         newTextElemHandle,          //  => new text node with identity transform. NULL if removing entire text
DPoint3d const*         cellOrigin,                 //  => cell origin, required for new creation
ElementHandleCR         dimElemHandle,              //  => associated dim element with updated settings. If null, use geometry to derive settings
ElementHandleCP         oldCellElemHandle,          //  => required only when dimElemHandle is invalid
DgnModelP            modelRef,                   //  => modelref to create in
bool                    bRetainCellOrigin,          //  => recreate note around the previous cell origin. If No, the cell is allowed to shift due to some settings.
double *                pNewAnnotScale,             //  => new annotation scale of note, if changing, else NULL
double *                pOldAnnotScale              //  => annotation scale of note, if changing, NULL otherwise
)
    {
    return mdlNote_updateSettings (newCellElemHandle, newTextElemHandle, cellOrigin, dimElemHandle, NULL, oldCellElemHandle, modelRef, bRetainCellOrigin, pNewAnnotScale, pOldAnnotScale);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
NOTE : Before calling this function, call mdlNote_rectifySettingsAndCommit in order to
       update the settings on the dimension element.
+---------------+---------------+---------------+---------------+---------------+------*/
  StatusInt       BentleyApi::mdlNote_replaceText
(
EditElementHandleR         newCellElemHandle,
ElementHandleCR            oldCellElemHandle,
ElementHandleCR            newTextElemHandle,
DgnModelP            modelRef
)
    {
    // Find the dimension element
    ElementId           dimElmId;
    EditElementHandle      dimElem;
    NoteCellHeaderHandler* noteHandler = dynamic_cast<NoteCellHeaderHandler*> (&oldCellElemHandle.GetHandler());
    if (NULL == noteHandler)
        return ERROR;
    if (SUCCESS == noteHandler->GetLeaderDimension (dimElmId, NULL, oldCellElemHandle))
        dimElem.FindById (dimElmId, modelRef);

    // Regenerate the note
    return mdlNote_create (newCellElemHandle, &newTextElemHandle, NULL, dimElem, &oldCellElemHandle, modelRef, true, NULL, NULL);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
+---------------+---------------+---------------+---------------+---------------+------*/
  StatusInt       mdlNote_translate
(
EditElementHandleR         newCellElemHandle,
ElementHandleR             oldCellElemHandle,
DPoint3d &              origin,
DgnModelP            modelRef
)
    {
    ElementId           dimElmId;
    EditElementHandle      dimElem;
    NoteCellHeaderHandler* noteHandler = dynamic_cast<NoteCellHeaderHandler*> (&oldCellElemHandle.GetHandler());
    if (NULL == noteHandler)
        return ERROR;

    if (SUCCESS == noteHandler->GetLeaderDimension(dimElmId, NULL, oldCellElemHandle))
        dimElem.FindById (dimElmId, modelRef);

    ElementHandle    textElemHandle;
    mdlNote_findTextNodeElement (textElemHandle, oldCellElemHandle);

    return mdlNote_create (newCellElemHandle, &textElemHandle, &origin, dimElem, &oldCellElemHandle, modelRef, false, NULL, NULL);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 02/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               BentleyApi::mdlNote_findTextNode
(
EditElementHandleR         textElemHandle,
ElementHandleCR            dimElemHandle,
DgnModelP            modelRef
)
    {
    ElementId           cellElementID;
    EditElementHandle      cellElm;
    if (SUCCESS != mdlNote_getRootNoteCellId (&cellElementID, dimElemHandle) ||
        SUCCESS != cellElm.FindById (cellElementID, modelRef))
        return ERROR;

    ElementHandle    oldtextNode;
    if (SUCCESS != mdlNote_findTextNodeElement (oldtextNode, cellElm))
        return ERROR;

    textElemHandle.Duplicate (oldtextNode);
    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_VANCOUVER_MERGE // note dependencies

static  StatusInt      updateDimDependentNote (ElementHandleR newDimElemHandle)
    {
    ElementRefP elemRef = mdlNote_getRootNoteCellElmRef (newDimElemHandle);
    /*-------------------------------------------------------------------
       Find the old text
    -------------------------------------------------------------------*/
    EditElementHandle  textElemHandle;
    if (SUCCESS != mdlNote_findTextNode (textElemHandle, newDimElemHandle, newDimElemHandle.GetDgnModelP()))
        return ERROR;

    /*-------------------------------------------------------------------
      Recreate and commit the note
    -------------------------------------------------------------------*/
    EditElementHandle  newCellElemHandle;

    if (SUCCESS != mdlNote_updateSettings (newCellElemHandle, &textElemHandle, NULL, newDimElemHandle, &newDimElemHandle, NULL, newDimElemHandle.GetDgnModelP(), true, NULL, NULL))
        return ERROR;

    if (NULL != elemRef)
        {
        EditElementHandle oldCEll(elemRef, newDimElemHandle.GetDgnModelP());
        if (mdlNote_areIdentical (newCellElemHandle, oldCEll))
            return SUCCESS;
        }
    else
        {
        elemRef = newCellElemHandle.GetElementRef ();
        
        if (!elemRef && newCellElemHandle.PeekElementDescrCP ())
            elemRef = newCellElemHandle.PeekElementDescrCP ()->GetElementRef();
        }

    return newCellElemHandle.ReplaceInModel (elemRef);
    }
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 07/06
+---------------+---------------+---------------+---------------+---------------+------*/
 bool BentleyApi::mdlNote_isDimInNoteCell
(
ElementRefP     dimElemRef,
DgnModelP    modelRef
)
    {
    if (!dimElemRef)
        return false;

    ElementRefP     parentElemRef = dimElemRef->GetParentElementRef();
    if (!parentElemRef)
        return false;

    ElementHandle   parentElmHandle (parentElemRef);
    return BentleyApi::mdlNote_isNote (parentElmHandle);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 11/04
* This function is called when a leader dimension is written to file or deleted from file.
* The idea is to regenerate the cell. Note that the cell regeneration may require the
* leader dimension's curve tangents to be recomputed and rewritten.
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_VANCOUVER_MERGE // note dependencies
static void    regenerateNote
(
ElementId                iNoteCellID,
DgnModelP             modelRef,
DependencyLinkage const* linkageP
)
    {
    DependencyRoot      roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];

    if (0 >= linkageP->nRoots ||
        0 == DependencyManagerLinkage::GetRoots (roots, modelRef, *linkageP, 0))
        {
        return;    
        }

    ElementRefP  dimElmRef = roots[0].ref;

    if (NULL == dimElmRef || dimElmRef->IsDeleted() ||
        DIMENSION_ELM != dimElmRef->GetLegacyType())
        {
        return;
        }

    EditElementHandle      dimElemHandle (dimElmRef, modelRef);
    ElementId           iRootDimID = ElementId(dimElemHandle.GetElementCP ()->ehdr.uniqueId);

    // Invisible dimension in note cell can trigger an infinite loop of changes.
    if (mdlNote_isDimInNoteCell (dimElmRef, modelRef))
        return;

    /*-------------------------------------------------------------------
      Regenerate the note cell and master leader, and save them.
    -------------------------------------------------------------------*/
    if (SUCCESS == updateDimDependentNote (dimElemHandle) &&
        SUCCESS == mdlNote_updateCurveLeaderTangents (dimElemHandle, modelRef))
        dimElemHandle.ReplaceInModel (dimElmRef);

    /*-------------------------------------------------------------------
      Go thru the remaining dependents and update their tangents
    -------------------------------------------------------------------*/
    ElementRefP  rootElem = modelRef->GetDgnProject().FindElementById (iNoteCellID);

    if (NULL == rootElem->GetFirstDependent())
        return;

    DPoint3d    tangents[2];
    bool        hasStartTangent = mdlDim_segmentGetCurveStartTangent (&tangents[0], dimElemHandle, 0);
    bool        hasEndTangent   = mdlDim_segmentGetCurveEndTangent   (&tangents[1], dimElemHandle, 0);

    if ( ! (hasStartTangent || hasEndTangent))
        return;

    DgnElement   elmBuf;

    for (DependentElemRef thisDep = rootElem->GetFirstDependent(); NULL != thisDep; thisDep = thisDep->GetNext())
        {
        ElementRefP depRef = thisDep->GetElementRef();

        if (DIMENSION_ELM != depRef->GetLegacyType() ||
            depRef->IsDeleted() ||
            iRootDimID    == depRef->GetElementId())
            {
            continue;
            }

        if (0 >= depRef->GetElement(&elmBuf, sizeof(elmBuf)))
            { BeAssert (0); continue; }

        EditElementHandle dimElm (&elmBuf, depRef->GetDgnModelP());
        
        if (hasStartTangent)
            mdlDim_segmentSetCurveStartTangent (dimElm, 0, &tangents[0]);

        if (hasEndTangent)
            mdlDim_segmentSetCurveEndTangent   (dimElm, 0, &tangents[1]);
        
        dimElm.ReplaceInModel(depRef);
        }

    return;
    }
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Abeesh.Basheer 10/09
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_VANCOUVER_MERGE // note dependencies
struct NoteAppIdRootsChanged : Bentley::DgnPlatform::DependencyManagerLinkage::IRootsChangedCallback
    {
    virtual UInt32      AddRef() const override {return 1;}
    virtual UInt32      Release() const override {return 1;}
    virtual WString    GetDescription () const override {return L"NoteAppIDRootsChanged";}
    virtual StatusInt   OnRootsChanged (ElementHandleCR depEh, DependencyLinkage const& linkage, UInt8* rootStatusP, UInt8 selfStatus) override
        {
        StatusInt           status = SUCCESS;

        if (DEPENDENCYAPPVALUE_Dimension != linkage.appValue)
            return status;

        if (selfStatus == DEPENDENCY_STATUS_DELETED || (selfStatus == DEPENDENCY_STATUS_CHANGED && *rootStatusP == DEPENDENCY_STATUS_UNCHANGED))
            return status;

        // Note: Don't need to check for selfStatus in order to react to the cell changes (move, rotate, etc).
        // The dimension has an assoc point dependency on the cell and adjusts itself, after which this dependency
        // also gets invoked. Thus any dynamic settings in the note cell get applied.
        switch (*rootStatusP)
            {
            case DEPENDENCY_STATUS_CHANGED:
                {
                regenerateNote (depEh.GetElementId(), depEh.GetDgnModelP(), &linkage);
                break;
                }

            case DEPENDENCY_STATUS_DELETED:
                {
                // Find the next dimension that is dependent on this cell and add a cell dependency to it
                EditElementHandle      cellElem(depEh, true);
                
                DependencyManagerLinkage::DeleteLinkageFromMSElement (cellElem.GetElementP(), DEPENDENCYAPPID_Note, DEPENDENCYAPPVALUE_Dimension);

                mdlNote_addDimDependencyToCell (cellElem);
                cellElem.ReplaceInModel (depEh.GetElementRef ());

                // Since the root has changed, use the latest linkage data
                DependencyLinkage const*  depLinkage;
                if (SUCCESS == DependencyManagerLinkage::GetLinkageFromMSElement (&depLinkage, cellElem.GetElementCP(), DEPENDENCYAPPID_Note, DEPENDENCYAPPVALUE_Dimension))
                    regenerateNote (depEh.GetElementId(), depEh.GetDgnModelP(), depLinkage);
                break;
                }
            }
        return status;
        }
    };
#endif

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            mdlNote_staticInitialize(void)
    {
#ifdef WIP_VANCOUVER_MERGE // note dependencies
    static NoteAppIdRootsChanged s_static_noteAppIdRootsChanged;
    DependencyManagerLinkage::RegisterRootsChangedCallback (DEPENDENCYAPPID_Note, &s_static_noteAppIdRootsChanged);
#endif
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BentleyApi::mdlNote_getRootDimension
(
ElementId *             pRootDimId,
DgnElementCR             cellElem
)
    {
    StatusInt                   status = SUCCESS;
    DependencyLinkageAccessor depLinkage;
    if (SUCCESS == (status = DependencyManagerLinkage::GetLinkageFromMSElement (&depLinkage, &cellElem, DEPENDENCYAPPID_Note, DEPENDENCYAPPVALUE_Dimension)))
        {
        if (pRootDimId)
            *pRootDimId = ElementId(depLinkage->root.elemid[0]);
        }

    return status;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 05/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BentleyApi::mdlNote_addDimDependencyToCell
(
EditElementHandleR         noteCellElm
)
    {
    if (SUCCESS == DependencyManagerLinkage::GetLinkageFromMSElement (NULL, noteCellElm.GetElementCP(), DEPENDENCYAPPID_Note, DEPENDENCYAPPVALUE_Dimension))
        return ERROR;//MDLERR_ALREADYEXISTS;

    ElementId           dimElementId;
    NoteCellHeaderHandler* noteHandler = dynamic_cast<NoteCellHeaderHandler*> (&noteCellElm.GetHandler());
    if (NULL == noteHandler)
        return SUCCESS;

    if (SUCCESS == noteHandler->GetLeaderDimension (dimElementId, NULL, noteCellElm))
        {
        DependencyLinkage   depLinkage;

        DependencyManagerLinkage::DefineElementIDDependency (depLinkage, DEPENDENCYAPPID_Note, DEPENDENCYAPPVALUE_Dimension,
                                                      DEPENDENCY_ON_COPY_DeepCopyRootsAcrossFiles, dimElementId.GetValue());

        DependencyManagerLinkage::AppendLinkage (noteCellElm, depLinkage, 0);
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    SunandSandurkar 07/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlNote_addDependencyToCell (ElementHandleCR dimensionElement)
    {
    ElementRefP elemRef = mdlNote_getRootNoteCellElmRef (dimensionElement);
    if (NULL == elemRef)
        return ERROR;

    EditElementHandle  cellElm (elemRef);
    
    if (SUCCESS != mdlNote_addDimDependencyToCell (cellElm))
        return ERROR;

    return cellElm.ReplaceInModel (elemRef);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BradRushing     09/95
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BentleyApi::mdlNote_appendNoteCellLinkage
(
DgnElement *             pNoteCell
)
    {
    if  (pNoteCell != NULL && pNoteCell->GetLegacyType() == CELL_HEADER_ELM)
        {
        MSNoteLinkage   noteLinkage;

        /* Set linkage header. */
        memset (&noteLinkage.header, '\0', sizeof(noteLinkage).header);
        noteLinkage.header.primaryID = MLNOTE_USERATTR_SIGNATURE;
        noteLinkage.header.user = 1;

        strcpy (noteLinkage.data.name, "Note");
        noteLinkage.data.size = static_cast<unsigned long>(strlen (noteLinkage.data.name));

        /* Convert, pad and append linkage to elem. */
        return linkage_appendToElement (pNoteCell, &noteLinkage.header, &noteLinkage.data, s_compiled_DataDefID_NoteData);
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BentleyApi::mdlNote_addNoteCellLinkage (EditElementHandleR cellElement)
    {
    if (CELL_HEADER_ELM != cellElement.GetLegacyType() || mdlElement_attributePresent(cellElement.GetElementCP(), MLNOTE_USERATTR_SIGNATURE, NULL))
        return  BSIERROR;

    // get a copy of the cell element - use malloc to reduce the chance of stack overflow in deeply nested cells.
    DgnElementP  cellHeader = (DgnElementP) malloc(MAX_V8_ELEMENT_SIZE);
    cellElement.GetElementCP()->CopyTo (*cellHeader);

    // append the note linkage
    StatusInt   status = mdlNote_appendNoteCellLinkage (cellHeader);
    if (BSISUCCESS == status)
        cellElement.ReplaceElement (cellHeader);

    free (cellHeader);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteCellHeaderHandler::GetNoteLeader(ElementHandleR leader, ElementHandleCR noteCell)
    {
    /*-------------------------------------------------------------------
       First find the dimension that the cell is dependent on
    -------------------------------------------------------------------*/
    ElementId           iRootDimID;
    StatusInt status = SUCCESS;
    if (SUCCESS != (status = mdlNote_getRootDimension (&iRootDimID, *noteCell.GetElementCP())))
        return (BentleyStatus)status;

    ElementRefP          rootElmRef = NULL;
    if (NULL == (rootElmRef = noteCell.GetDgnModelP()->GetDgnProject().Models().FindElementById (iRootDimID)))
        return ERROR;

    if (DIMENSION_ELM != rootElmRef->GetLegacyType())
        return ERROR;
    
    leader = ElementHandle(rootElmRef);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteCellHeaderHandler::GetInvisibleDimension (ElementHandleR dimElement, ElementHandleCR noteCell)
    {
    /*-------------------------------------------------------------------
       If the cell is not dependent on a dimension, go through all the
       cell's dependents and find the first valid note dimension element
    -------------------------------------------------------------------*/
    ElementRefP  rootElem = noteCell.GetDgnModelP()->GetDgnProject().Models().FindElementById (noteCell.GetElementId());//WIP_NOTES_TEST_AB // Why is it fetching the elemref again. TODO test

    if (NULL == rootElem)
        return ERROR;

#ifdef WIP_VANCOUVER_MERGE // note dependencies

    for (DependentElemRef thisDep = rootElem->GetFirstDependent(); NULL != thisDep; thisDep = thisDep->GetNext())
        {
        ElementRefP depRef = thisDep->GetElementRef();

        if (DIMENSION_ELM != depRef->GetLegacyType() || depRef->IsDeleted())
            continue;

        DgnElementCP pDimElm = depRef->GetUnstableMSElementCP();

        if (pDimElm->ToDimensionElm().dimcmd != DimensionType::Note)
            continue;

        dimElement = ElementHandle(depRef, noteCell.GetDgnModelP());
        return SUCCESS;
        }

#endif

    return ERROR;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    DonFu           09/02
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       NoteCellHeaderHandler::GetLeaderDimension
(
ElementId &             dimId,
DPoint3d *              pHookPoint,
ElementHandleCR            noteCell
)
    {
    dimId.Invalidate();

    if (NULL == noteCell.GetDgnModelP())
        { BeAssert (0); return ERROR; }

    ElementHandle leaderElement;
    if (SUCCESS != GetNoteLeader (leaderElement, noteCell))
        {
        //Try to get the invisible dimension
        BentleyStatus status = SUCCESS;
        if (SUCCESS != (status = GetInvisibleDimension(leaderElement, noteCell)))
            return status;
        }
    
    dimId = leaderElement.GetElementId();

    /* the last point in note dimension is the hook point */
    DimensionElm const* pDim = &leaderElement.GetElementCP()->ToDimensionElm();
    if  (pHookPoint != NULL && pDim->nPoints > 0)
        *pHookPoint = pDim->GetPoint(pDim->nPoints-1);

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    DonFu           10/02
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP     BentleyApi::mdlNote_getRootNoteCellElmRef (ElementHandleCR dimensionElement)
    {
    if (!dimensionElement.IsValid() || DIMENSION_ELM !=dimensionElement.GetLegacyType())
        return NULL;

    DependencyLinkageAccessor depLinkage;
    if (SUCCESS != DependencyManagerLinkage::GetLinkageFromMSElement (&depLinkage, dimensionElement.GetElementCP(), DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_AssocPoint))
        return NULL;
    
    DependencyRoot      roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];
    for (UInt16 index = 0; index < depLinkage->nRoots; ++index)
        {
        int num = DependencyManagerLinkage::GetRoots (roots, dimensionElement.GetDgnModelP(), *depLinkage, index);

        /* We only expect one root element, i.e. either simple cases or two-element case: */
        if  ((num == 1 || num == 2 || num == 4) && roots[0].ref != NULL &&
             !roots[0].ref->IsDeleted() &&
             roots[0].ref->GetLegacyType() == CELL_HEADER_ELM)
            {
            /* by definition, the 1st element in cell must be a text/text node: */
            ElementRefP  textNodeRef = NULL;
            SubElementRefVecP subElements = roots[0].ref->GetSubElements();
            if (NULL != subElements && subElements->size() > 0)
                textNodeRef = subElements->at(0);

            if (textNodeRef != NULL &&
                (textNodeRef->GetLegacyType() == TEXT_ELM ||
                 textNodeRef->GetLegacyType() == TEXT_NODE_ELM))
                {
                return  roots[0].ref;
                }
            }
        }
    
    return  NULL;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    DonFu           09/02
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  BentleyApi::mdlNote_getRootNoteCellId (ElementId* pNoteCellId, ElementHandleCR dimensionElement)
    {
    ElementRefP  noteCellRef = mdlNote_getRootNoteCellElmRef (dimensionElement);

    if  (noteCellRef != NULL)
        {
        if  (pNoteCellId != NULL)
            *pNoteCellId = noteCellRef->GetElementId();

        return SUCCESS;
        }

    return ERROR;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    DonFu           10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BentleyApi::mdlNote_setNoteAssocPoint
(
EditElementHandleR  dimElement,
ElementId           noteCellId,
int                 pointNo
)
    {
    /*-------------------------------------------------------------------
    This is used when we have the note cell ID but do not have note cell
    element created yet.  It is assumed that existing point happens to
    have correct associative point.  This function just adds linkage and
    set the dimension flag.
    -------------------------------------------------------------------*/
    DgnElementP      elem = dimElement.GetElementP ();
    if (elem != NULL && noteCellId.IsValid() && pointNo < elem->ToDimensionElm().nPoints)
        {
        AssocPoint      assocPoint;

        AssociativePoint::InitOrigin (assocPoint, /*option*/0);

        if (SUCCESS == AssociativePoint::SetRoot(assocPoint, noteCellId, 0) &&
            SUCCESS == AssociativePoint::InsertPoint(dimElement, assocPoint, pointNo, elem->ToDimensionElm().nPoints+1))
            {
            elem = dimElement.GetElementP ();
            elem->ToDimensionElmR().GetDimTextP(pointNo)->flags.b.associative = true;

            return BSISUCCESS;
            }
        }

    return BSIERROR;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    DonFu           10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  BentleyApi::mdlNote_getHookLineFromNoteCell (DPoint3dP hookLinePoint, ElementHandleCR dimensionElement)
    {
    ElementRefP  noteCellRef = mdlNote_getRootNoteCellElmRef (dimensionElement);
    if  (NULL == noteCellRef)
        return ERROR;

    SubElementRefVecP subElements = noteCellRef->GetSubElements();
    if (NULL == subElements)
        return ERROR;

    /* note cell contains a valid text element, now look for the elbow line: */
    for (SubElementRefVec::iterator iter = subElements->begin(); iter != subElements->end(); ++iter)
        {
        if ((*iter)->GetLegacyType() == LINE_ELM)
            {
            ElementHandle  lineEh (*iter);
            return LineStringUtil::Extract (hookLinePoint, NULL, *lineEh.GetElementCP (), *dimensionElement.GetDgnModelP ());
            }
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 10/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isNoteCellDisplayOn (ElementHandleCR thisElm, ViewContextR context)
    {
    ViewFlags const*    flags = context.GetViewFlags ();

    if (!flags || flags->dimens) // Dimensions on in view...display!
        return true;

    ElementRefP  elRef = thisElm.GetElementRef ();

    if (NULL == elRef) // Note not persistent...display!
        return true;

#ifdef WIP_VANCOUVER_MERGE // note dependencies

    DependentElemRef    dep;

    // Iterate over dependents of this element looking for a dimension...
    for (dep = elRef->GetFirstDependent(); dep; dep = dep->GetNext())
        {
        ElementRefP      depRef;

        if (NULL != (depRef = dep->GetElementRef()) && DIMENSION_ELM == depRef->GetLegacyType())
            return false;
        }

#endif

    return true; // Note has no dimensions...display!
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlNote_getAnnotationScale
(
double *        annotationScale,
ElementHandleCR    noteCellElemHandle
)
    {
    if (annotationScale)
        *annotationScale = 1.0;

    ElementHandle    textnodeElement;
    if (SUCCESS != mdlNote_findTextNodeElement (textnodeElement, noteCellElemHandle))
        {
        BeDataAssert (false && "Expected textnode in note not found");

        return false;
        }

    TextParamWide       params;
    TextNodeHandler::GetTextParams (textnodeElement, params);

    if (annotationScale)
        *annotationScale = params.annotationScale;

    return params.exFlags.annotationScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            NoteCellHeaderHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_SubType_Note));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                10/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            NoteCellHeaderHandler::_GetDescription
(
ElementHandleCR    el,
WStringR        descr,
UInt32          desiredLength
)
    {
    _GetTypeName (descr, desiredLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            NoteCellHeaderHandler::_GetPathDescription
(
ElementHandleCR    el,
WStringR        descr,
DisplayPathCP    path,
WCharCP       levelStr,
WCharCP       modelStr,
WCharCP       groupStr,
WCharCP       delimiter
)
    {
    _GetDescription (el, descr, 100);  // start with element's description

    if (levelStr && '\0' != levelStr[0]) descr.append(delimiter).append(levelStr);
    if (modelStr && '\0' != modelStr[0]) descr.append(delimiter).append(modelStr);
    if (groupStr && '\0' != groupStr[0]) descr.append(delimiter).append(groupStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void            NoteCellHeaderHandler::_GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials)
    {
    ChildElemIter   firstChild (thisElm, ExposeChildrenReason::Count);

    if (!firstChild.IsValid ())
        {
        T_Super::_GetElemDisplayParams (thisElm, params, wantMaterials);

        return;
        }

    // use first child for display params so that we have level for LevelSymb w/DrawFiltered...
    DisplayHandlerP handler = firstChild.GetDisplayHandler();
    if (NULL == handler)
        return;
    handler->GetElemDisplayParams (firstChild, params, wantMaterials);

    params.SetIsRenderable (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            NoteCellHeaderHandler::DrawChildren (ElementHandleCR thisElm, ViewContextR context)
    {
    ViewFlags const*  flagsCP = context.GetViewFlags ();

    if (flagsCP && flagsCP->text_nodes)
        {
        ViewFlags   orig = *flagsCP;
        ViewFlags   working = *flagsCP;

        working.text_nodes = false; // Don't display text node number for notes...
        context.SetViewFlags (&working);  //  It is true, set it to false

        UseChildren (thisElm, context, false);

        context.SetViewFlags (&orig);  // Set it back to true

        return;
        }

    UseChildren (thisElm, context, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                06/2006
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt               propagateAnnotationScaleToNoteText (EditElementHandleR noteCell, ChangeAnnotationScale&  changeContextIn)
    {
    MSElementDescrPtr scaledTextNodeEdP = noteCell.GetElementDescrP()->Components().begin()->get()->Duplicate(true, true);
    EditElementHandle elemHandle (scaledTextNodeEdP.get(), false);
    
    AnnotationScaleAction   action  = AnnotationScaleAction::Update;
    double      newAnnotationScale  = 1.0;
    mdlChangeAnnotationScale_getAction (&changeContextIn, &action, &newAnnotationScale);
    
    if (SUCCESS != TextHandlerBase::UpdateAnnotationScale (elemHandle, action, newAnnotationScale, true))
        return ERROR;

    
    noteCell.GetElementDescrP()->Components().begin()->get()->ReplaceElement(*elemHandle.GetElementP());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt mdlNote_getSettingsDimension (EditElementHandleR dimElemHandle, ElementHandleCR elHandle)
    {
    ElementId           dimElmId;
    
    NoteCellHeaderHandler* noteHandler = dynamic_cast<NoteCellHeaderHandler*> (&elHandle.GetHandler());
    if (NULL == noteHandler)
        return ERROR;

    if (SUCCESS == noteHandler->GetLeaderDimension (dimElmId, NULL, elHandle) &&
        SUCCESS == dimElemHandle.FindById (dimElmId, elHandle.GetDgnModelP ()))
        return SUCCESS;

    return mdlNote_findCellComponent (dimElemHandle, DIMENSION_ELM, elHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Ferguson   10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt            text_propagateModelAnnotationScaleToNote (EditElementHandleR cellElement, ChangeAnnotationScale& changeContextIn)
    {
    double                          newAnnotationScale   = 1.0;
    double                          oldAnnotationScale   = 1.0;

    if (!cellElement.IsValid())
        return ERROR;

    mdlNote_getAnnotationScale (&oldAnnotationScale, cellElement);

    mdlChangeAnnotationScale_getAction (&changeContextIn, NULL, &newAnnotationScale);

    // Scale a copy of the input cell edP
    EditElementHandle scaledCellElement (cellElement, true);
    StatusInt status = propagateAnnotationScaleToNoteText (scaledCellElement, changeContextIn);
    if (SUCCESS != status)
        return ERROR;

    bool                useOldOrigin = true;
    DPoint3d            origin;
    EditElementHandle   dimElemHandle;
    NoteCellHeaderHandler* noteHandler = dynamic_cast<NoteCellHeaderHandler*> (&cellElement.GetHandler());
    if (NULL == noteHandler)
        return ERROR;

    /*  Get the dim element associated with the cell. It is okay if it is missing. */
    if (SUCCESS == mdlNote_getSettingsDimension (dimElemHandle, cellElement))
        {
        double      annotationScale = 1.0;
        AnnotationScaleAction   action          = AnnotationScaleAction::Update;
        mdlChangeAnnotationScale_getAction (&changeContextIn, &action, &annotationScale);

        TransformInfo       transInfo;
        transInfo.SetOptions (TRANSFORM_OPTIONS_ApplyAnnotationScale);
        transInfo.SetAnnotationScale (annotationScale);
        transInfo.SetAnnotationScaleAction (action);

        dimElemHandle.GetHandler().ApplyTransform (dimElemHandle, transInfo);

        mdlDim_extractPointsD (&origin, dimElemHandle, dimElemHandle.GetElementCP()->ToDimensionElm().nPoints-1, 1);
        useOldOrigin = false;
        }
    else
        {
        BeDataAssert (false);
        return ERROR;
        }

    // Rebuild the note
    ElementHandle newTextElemHandle;
    if (SUCCESS != (status = mdlNote_findTextNodeElement (newTextElemHandle, scaledCellElement)))
        return status;
    
    return mdlNote_create (cellElement, &newTextElemHandle, useOldOrigin ? NULL : &origin, dimElemHandle, &cellElement,
                    cellElement.GetDgnModelP(), useOldOrigin, &newAnnotationScale, &oldAnnotationScale);
    }

/*=================================================================================**//**
* Note Cell Element Stroker
* @bsiclass                                                     Deepak.Malkan   10/2007
+===============+===============+===============+===============+===============+======*/
struct  StrokeNoteCellElm : IAnnotationStrokeForCache
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   10/2007
+---------------+---------------+---------------+---------------+---------------+------*/
void    StrokeScaledForCache (ElementHandleCR thisElm, ViewContextR context, AnnotationDisplayParameters const& parms) override
    {
    //ChangeAnnotationScale*  changeContext = mdlChangeAnnotationScale_new (thisElm.GetDgnModelP ()); removed in graphite
    EditElementHandle workElm (thisElm, true);
    //mdlChangeAnnotationScale_setAction (changeContext, AnnotationScaleAction::Update, parms.GetDesiredScale());
    //text_propagateModelAnnotationScaleToNote (workElm, *changeContext);
    //mdlChangeAnnotationScale_free (&changeContext);

    //  Account for aspectRatioSkew -- must be done last!
    ViewContext::ContextMark    mark (context, workElm);
    if (parms.HasAspectRatioSkew ())
        {
        TransformInfo tinfo (parms.GetAspectRatioSkew());
        tinfo.SetOptions (TRANSFORM_OPTIONS_AnnotationSizeMatchSource);
        thisElm.GetHandler().ApplyTransform (workElm, tinfo);
        workElm.GetElementDescrP ()->Validate ();

        //context.RemoveAspectRatioSkew (parms.GetAspectRatioSkew()); removed in graphite
        }

    _StrokeForCache (CachedDrawHandle(&workElm), context, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   10/2007
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    ElementHandleCR thisElm = *dh.GetElementHandleCP();
    NoteCellHeaderHandler*  handler = (NoteCellHeaderHandler*) thisElm.GetDisplayHandler ();
    handler->DrawChildren (thisElm, context);
    }
}; // StrokeNodeCellElm

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   09/2007
+---------------+---------------+---------------+---------------+---------------+------*/
bool    NoteCellHeaderHandler::_GetAnnotationScale (double* scale, ElementHandleCR el) const
    {
    return (true == mdlNote_getAnnotationScale (scale, el));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            NoteCellHeaderHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    UInt32      info = context.GetDisplayInfo (IsRenderable (thisElm));

    if (0 == (info & DISPLAY_INFO_Edge))
        return;

    if (!isNoteCellDisplayOn (thisElm, context))
        return;

    StrokeNoteCellElm     stroker;
    StrokeAnnotationElm annotationStroker (this, thisElm, context, &stroker);
    annotationStroker.DrawUsingContext ();

#ifdef OLD
    double refAnnotScale = 1.0;
    if (SUCCESS == context.GetRefAnnotationScale (&refAnnotScale))
        {
        double          curScale  = 1.0;

        if (mdlNote_getAnnotationScale (&curScale, thisElm.GetElementDescrCP ()) && refAnnotScale != curScale)
            {
            //DrawWithAnnotationScale (thisElm, context, refAnnotScale);
            //return;
            }
        }

    DrawChildren (thisElm, context);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/04
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    hasMirrorTransform (ElementHandleCR elemHandle, TransformInfoCR trans)
    {
    if ((trans.GetOptions () & TRANSFORM_OPTIONS_DisableMirrorCharacters) && NULL != trans.GetMirrorPlane ())
        {
        double      determ;
        RotMatrix   rMatrix;

        rMatrix.InitFrom(*( trans.GetTransform ()));
        determ = rMatrix.Determinant ();

        if (determ < -mgds_fc_epsilon)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            NoteCellHeaderHandler::_IsTransformGraphics (ElementHandleCR elemHandle, TransformInfoCR tInfo)
    {
    // If text should not be mirrored, treat it as a special case
    if (hasMirrorTransform (elemHandle, tInfo))
        return false;

    if (tInfo.GetOptions () & TRANSFORM_OPTIONS_ApplyAnnotationScale ||
        (tInfo.GetOptions () & TRANSFORM_OPTIONS_AnnotationSizeMatchSource && mdlNote_getAnnotationScale (NULL, elemHandle)))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 05/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       NoteCellHeaderHandler::_OnTransform (EditElementHandleR elHandle, TransformInfoCR tInfo)
    {
    // Transform should be applied as 
    //                                  Annotations | Non Annotations
    // translate+Rotate+Scale       |               |   apply all
    // Annotation Scale             |    step 1     |
    // Translation of origin points |    step 2     |
    // Translate + Rotate           |    step 2'    |
    // ----------------------------------------------------------------
    // To understand step2a... think of an annotation inside a non-annotation cell. When the cell is scaled the scale does
    // not change the size of the annotation, but it does change the origin.

    /*-------------------------------------------------------------------
        Step 1 : Propagate annotation scale
    -------------------------------------------------------------------*/
    if (tInfo.GetOptions () & TRANSFORM_OPTIONS_ApplyAnnotationScale)
        {
        ChangeAnnotationScale *  changeContext = mdlChangeAnnotationScale_new (elHandle.GetDgnModelP ());
        mdlChangeAnnotationScale_setAction (changeContext, tInfo.GetAnnotationScaleAction (), tInfo.GetAnnotationScale ());

        text_propagateModelAnnotationScaleToNote (elHandle, *changeContext);
        mdlChangeAnnotationScale_free (&changeContext);
        }

    /*-------------------------------------------------------------------
        Step 2 : Propagate transform
    -------------------------------------------------------------------*/
    if (tInfo.GetOptions () & TRANSFORM_OPTIONS_AnnotationSizeMatchSource && mdlNote_getAnnotationScale (NULL, elHandle))
        {
        // We have to explicitly transform the associated leader dimension to find out the
        // destination of the cell. This is not the same as unplugging the scale component
        // of the transform and applying it to the note because the latter approach applies
        // only to independent elements. It won't work for annotations inside cells.
        //Here's the example.
        // When propagating annotation scale, the transforminfo contains the scale both as
        // annotation scale and as geometry scale. After applying annotation scale to note
        // cell above, the geometry scale could cause further translation of its origin
        // if we following the latter approach. Instead, find the new location of the
        // leader endpoint and translate the cell to that location.

        // a. Read the leader dim from cache.
        EditElementHandle      dimElemHandle;
        NoteCellHeaderHandler* noteHandler = dynamic_cast<NoteCellHeaderHandler*> (&elHandle.GetHandler());
        if (NULL == noteHandler)
            return ERROR;

        if (SUCCESS == mdlNote_getSettingsDimension (dimElemHandle, elHandle))
            {
            RotMatrix rotation;
            bsiTransform_getMatrix (tInfo.GetTransform(), &rotation);
            {
            DVec3d scaleVector;
            rotation.NormalizeColumnsOf (rotation, scaleVector);
            }

            Transform rotTransform;
            bsiTransform_initFromMatrix(&rotTransform, &rotation);
            TransformInfo oldTinfo (rotTransform);
            EditElementHandle oldDimElemHandle(dimElemHandle, true);
            oldDimElemHandle.GetHandler().ApplyTransform (oldDimElemHandle, oldTinfo);
            DPoint3d    oldEndPoint;
            DgnElementCP dim = oldDimElemHandle.GetElementCP ();
            BentleyApi::mdlDim_extractPointsD (&oldEndPoint, dimElemHandle, dim->ToDimensionElm().nPoints-1, 1);

            // b. transform the leader dim.
            TransformInfo modifiedTransInfo = tInfo;
            modifiedTransInfo.SetOptions (modifiedTransInfo.GetOptions () & ~TRANSFORM_OPTIONS_ApplyAnnotationScale);
            dimElemHandle.GetHandler().ApplyTransform (dimElemHandle, modifiedTransInfo);
            dim = dimElemHandle.GetElementCP ();

            // c. find out the new location of the leader endpoint
            DPoint3d newEndPoint;
            BentleyApi::mdlDim_extractPointsD (&newEndPoint, dimElemHandle, dim->ToDimensionElm().nPoints-1, 1);

            // d. translate the note cell to that new location.
            DPoint3d offset;
            bsiDPoint3d_subtractDPoint3dDPoint3d (&offset, &newEndPoint, &oldEndPoint);

            Transform transform;
            bsiTransform_initFromMatrixAndTranslation (&transform, &rotation, &offset);
            TransformInfo trans(transform);

            return T_Super::_OnTransform (elHandle, trans);
            }
        }

    return T_Super::_OnTransform (elHandle, tInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
//StatusInt       NoteCellHeaderHandler::_OnPreprocessCopy (EditElementHandleR thisElm, ElementCopyContextP ccP) removed in graphite
//    {
//    if (SUCCESS != T_Super::_OnPreprocessCopy (thisElm, ccP))
//        return ERROR;
//
//    return _OnPreprocessCopyAnnotationScale (thisElm, ccP);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       NoteCellHeaderHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Complex & geometry.GetOptions ()))
        return ERROR;

    ChildElemIter childEh (eh, ExposeChildrenReason::Count);

    if (!childEh.IsValid ())
        return ERROR;

    for (; childEh.IsValid (); childEh = childEh.ToNext ())
        {
        // Do not include the invisible dimension...
        if (!childEh.GetElementCP ()->IsGraphic() || childEh.GetElementCP ()->IsInvisible())
            continue;

        EditElementHandle childEeh;

        childEeh.Duplicate (childEh);

        if (!childEeh.IsValid ())
            continue;

        childEeh.GetElementP()->SetComplexComponent(false); // Clear complex component bit...
        dropGeom.Insert (childEeh);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            NoteCellHeaderHandler::_EditProperties (EditElementHandleR eeh, PropertyContextR context)
    {
    T_Super::_EditProperties (eeh, context);

    // If purpose is just a simple id remap we don't need to regenerate note...
    if (EditPropertyPurpose::Change != context.GetIEditPropertiesP ()->_GetEditPropertiesPurpose () || !context.GetElementChanged ())
        return;

    // If properties being edited don't affect layout we don't beed to regenerate note...
    if (0 == ((ELEMENT_PROPERTY_Font | ELEMENT_PROPERTY_TextStyle | ELEMENT_PROPERTY_DimStyle | ELEMENT_PROPERTY_ElementTemplate) & context.GetElementPropertiesMask ()))
        return;

    ElementHandle      txtElemHandle;
    if (SUCCESS != mdlNote_findCellComponent (txtElemHandle, TEXT_NODE_ELM, eeh))
        return;

    EditElementHandle  newElemHandle;
    if (SUCCESS != BentleyApi::mdlNote_replaceText (newElemHandle, eeh, txtElemHandle, context.GetDestinationDgnModel ()))
        return;

    eeh.ReplaceElementDescr (newElemHandle.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BentleyApi::mdlNote_isNote (ElementHandleCR thisElm)
    {
    if (CELL_HEADER_ELM != thisElm.GetLegacyType())
        return false;

    if (!mdlElement_attributePresent (thisElm.GetElementCP (), MLNOTE_USERATTR_SIGNATURE, NULL))
        return false;
    
#if defined (NOT_NOW) // What's the point of this assert...just to annoy people?!? This note cell is garbage, you can't fix it...
    if (thisElm.PeekElementDescrCP ())
        BeAssert ((NULL != thisElm.PeekElementDescrCP ()->h.firstElem) && (TEXT_NODE_ELM == thisElm.PeekElementDescrCP ()->h.firstElem->el.GetLegacyType()));
#endif
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sunand.Sandurkar                10/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool NoteCellHeaderHandler::_ClaimElement (ElementHandleCR thisElm)
    {
    return mdlNote_isNote (thisElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus findChildTextEh (ElementHandleCR noteEh, ElementHandleP childTextEh)
    {
    // Notes are flat, thus don't need to recurse through children.
    
    for (ChildElemIter childIter (noteEh, ExposeChildrenReason::Count); childIter.IsValid (); childIter = childIter.ToNext ())
        {
        ITextQueryCP textQuery = dynamic_cast<ITextQueryCP>(&childIter.GetHandler ());
        if (NULL == textQuery)
            continue;
        
        if (!textQuery->IsTextElement (childIter))
            {
            BeAssert (false);
            continue;
            }
            
        if (NULL != childTextEh)
            *childTextEh = childIter;
        
        return SUCCESS;
        }
    
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2013
//---------------------------------------------------------------------------------------
bool NoteCellHeaderHandler::_DoesSupportFields (ElementHandleCR) const { return true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+------- --------+---------------+---------------+------*/
ITextPartIdPtr NoteCellHeaderHandler::_GetTextPartId (ElementHandleCR noteEh, HitPathCR) const
    {
    // Notes only ever only have zero or one text parts, thus we don't need to squirrel any data away.
    // We are purposefully finding text regardless of where the user actually hit the note.
    
    if (SUCCESS != findChildTextEh (noteEh, NULL))
        return NULL;
    
    return ITextPartId::Create ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void NoteCellHeaderHandler::_GetTextPartIds (ElementHandleCR noteEh, ITextQueryOptionsCR options, T_ITextPartIdPtrVectorR textPartIds) const
    {
    // Notes only ever only have zero or one text parts, thus we don't need to squirrel any data away.
    
    if (SUCCESS != findChildTextEh (noteEh, NULL))
        return;
    
    textPartIds.push_back (ITextPartId::Create ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
TextBlockPtr NoteCellHeaderHandler::_GetTextPart (ElementHandleCR noteEh, ITextPartIdCR) const
    {
    ElementHandle childTextEh;
    if (SUCCESS != findChildTextEh (noteEh, &childTextEh))
        return NULL;
    
    ITextQueryCP            childTextQuery  = childTextEh.GetITextQuery ();
    T_ITextPartIdPtrVector  textPartIds;
    
    childTextQuery->GetTextPartIds (childTextEh, *ITextQueryOptions::CreateDefault (), textPartIds);
    
    if (1 != textPartIds.size ())
        { BeAssert (false); return NULL; }
    
    return childTextQuery->GetTextPart (childTextEh, *textPartIds[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ITextEdit::ReplaceStatus NoteCellHeaderHandler::_ReplaceTextPart (EditElementHandleR noteEeh, ITextPartIdCR, TextBlockCR updatedText)
    {
    if (SUCCESS != mdlNote_rectifySettingsAndCommit (noteEeh, noteEeh.GetDgnModelP ()))
        return ReplaceStatus_Error;

    ElementHandle currentTextPartEh;
    if (SUCCESS != findChildTextEh (noteEeh, &currentTextPartEh))
        return ReplaceStatus_Error;
    
    EditElementHandle newTextPartEeh;
    
    if (TextBlock::TO_ELEMENT_RESULT_Success != updatedText.ToElement (newTextPartEeh, noteEeh.GetDgnModelP (), &currentTextPartEh))
        return ReplaceStatus_Error;
    
    if (SUCCESS != BentleyApi::mdlNote_replaceText (noteEeh, noteEeh, newTextPartEeh, noteEeh.GetDgnModelP ()))
        return ReplaceStatus_Error;
    
    return ReplaceStatus_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       NoteCellHeaderHandler::_OnFenceStretch (EditElementHandleR elemHandle, TransformInfoCR transform,
                            FenceParamsP fp, FenceStretchFlags options)
    {
    StatusInt status = SUCCESS;
    DPoint3d origin;
    if (SUCCESS != (status = CellUtil::ExtractOrigin (origin, elemHandle)))
        return status;

    transform.GetTransform()->multiply(&origin);
    return mdlNote_translate (elemHandle, elemHandle, origin, elemHandle.GetDgnModelP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                   08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
struct NoteDimensionData: IDimCreateData
    {
    private:
        DimensionStyleCR    m_dimStyle;
        DgnTextStylePtr     m_textStyle;
        RotMatrix           m_matrix;

    public:
    NoteDimensionData (DimensionStyleCR style, TextBlockCR text)
        :m_dimStyle (style)
        {
        if (BSISUCCESS != style.GetTextStyleProp(m_textStyle, DIMSTYLE_PROP_Text_TextStyle_TEXTSTYLE))
            m_textStyle = DgnTextStyle::Create(*style.GetDgnProject());
        m_matrix = text.GetOrientation();
        }
    virtual DimensionStyleCR                _GetDimStyle()      const {return m_dimStyle;}
    virtual DgnTextStyleCR                  _GetTextStyle()     const {return *m_textStyle;}
    virtual Symbology                       _GetSymbology()     const {return Symbology();}
    virtual LevelId                         _GetLevelID()       const {return LevelId();}
    virtual int                             _GetViewNumber()    const {return -1;}
    virtual RotMatrixCR                     _GetDimRMatrix()    const {return m_matrix;}
    virtual RotMatrixCR                     _GetViewRMatrix()   const {return m_matrix;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                   08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteCellHeaderHandler::CreateNote (EditElementHandleR noteElem, EditElementHandleR leaderElement, TextBlockCR text, 
                                               DimensionStyleCR dimStyle, bool is3d, DgnModelR modelRef, StdDPointVector const& noteLeaderPoints)
    {
    BentleyStatus status = SUCCESS;
    if (SUCCESS != (status = DimensionHandler::CreateDimensionElement (leaderElement, NoteDimensionData(dimStyle, text), DimensionType::Note, is3d, modelRef)))
        return status;
    
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&leaderElement.GetHandler());
    if (NULL == hdlr)
        return ERROR;

    int count = static_cast<int>(noteLeaderPoints.size());
    for (int index = 0; index < count; ++index)
        {
        DimText dimText;
        dimStyle.InitDimText (dimText, leaderElement, index);
        if (SUCCESS != (status = hdlr->InsertPointDirect (leaderElement, &noteLeaderPoints[index], NULL, dimText, index)))
            return status;
        }
    
    EditElementHandle newElementTemplate;
    ElementUtil::InitializeGraphicsElement (newElementTemplate, modelRef, TEXT_NODE_ELM, is3d, sizeof (Text_node_3d));

    EditElementHandle textElement;
    if (TextBlock::TO_ELEMENT_RESULT_Success != text.ToElement (textElement, &modelRef, &newElementTemplate))
        return ERROR;
    
    ElementUtil::InitializeGraphicsElement (noteElem, modelRef, CELL_HEADER_ELM, is3d, sizeof (Cell_3d));
    return (BentleyStatus) mdlNote_create (noteElem, &textElement, &noteLeaderPoints[count-1], leaderElement, NULL, &modelRef, false, NULL, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteCellHeaderHandler::AddDimensionNoteCellAssociation (EditElementHandleR dimElement, ElementId cellId)
    {
    BentleyStatus status = SUCCESS;
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
    if (NULL == hdlr)
        return ERROR;
    
    int nPoints = hdlr->GetNumPoints (dimElement);
    if (nPoints <= 0)
        return ERROR;

    AssocPoint  assocPoint;
    AssociativePoint::InitOrigin (assocPoint, /*options*/0);
    if (SUCCESS != (status = AssociativePoint::SetRoot (assocPoint, cellId, 0)))
        return status;

    DimText dimText = *dimElement.GetElementCP()->ToDimensionElm().GetDimTextCP(nPoints -1);
    if (SUCCESS != (status = hdlr->DeletePoint (dimElement, nPoints -1)))
        return status;

    if (SUCCESS != (status = hdlr->InsertPointDirect (dimElement, NULL, &assocPoint, dimText, nPoints -1)))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    abeesh.basheer                  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteCellHeaderHandler::AddToModel (EditElementHandleR noteElement, EditElementHandleR leaderElement, DgnModelR dgnCache)
    {
    BentleyStatus status = SUCCESS;
    if (noteElement.IsValid())
        {
        noteElement.SetDgnModel (dgnCache); // Shouldn't be necessary...should have been set when created...
        if (SUCCESS != (status = (BentleyStatus)noteElement.AddToModel()))
            return status;
        }
    
    ElementRefP oldElementRef = noteElement.GetElementRef();

    if (leaderElement.IsValid())
        {
        if (noteElement.IsValid() && SUCCESS != (status = AddDimensionNoteCellAssociation(leaderElement, noteElement.GetElementId())))
            return status;
        
        leaderElement.SetDgnModel (dgnCache); // Shouldn't be necessary...should have been set when created...
        if (SUCCESS != (status = (BentleyStatus)leaderElement.AddToModel ()))
            return status;
        }
    
    if (!noteElement.IsValid())
        return status;

    noteElement.GetElementDescrP();
    EditElementHandle invsibleDim;
    if (SUCCESS != (status = (BentleyStatus)mdlNote_findCellComponent (invsibleDim, DIMENSION_ELM, noteElement)))
        return status;

    if (SUCCESS != (status = AddDimensionNoteCellAssociation(invsibleDim, noteElement.GetElementId())))
        return status;

    if (SUCCESS != (status = (BentleyStatus)mdlNote_addDimDependencyToCell(noteElement)))
        return status;

    if (SUCCESS != (status = (BentleyStatus)noteElement.ReplaceInModel(oldElementRef)))
        return status;
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionStylePtr NoteCellHeaderHandler::GetNoteDimensionStyle(ElementHandleCR noteElement)
    {
    ElementId leaderId;
    if (SUCCESS != GetLeaderDimension (leaderId, NULL, noteElement))
        return NULL;

    EditElementHandle leaderElement;
    if (SUCCESS != leaderElement.FindById (leaderId, noteElement.GetDgnModelP()))
        return NULL;

    DimensionHandler* dimHandler = dynamic_cast<DimensionHandler*> (&leaderElement.GetHandler());
    if (NULL == dimHandler)
        return NULL;

    return dimHandler->GetDimensionStyle(leaderElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteCellHeaderHandler::SetNoteDimensionStyle(EditElementHandleR noteElement, EditElementHandleP leaderElement, DimensionStyleCR dimStyle)
    {
    if (NULL != leaderElement)
        {
        DimensionHandler* dimHandler = dynamic_cast<DimensionHandler*> (&leaderElement->GetHandler());
        if (NULL == dimHandler)
            return ERROR;

        dimHandler->ApplyDimensionStyle(*leaderElement, dimStyle, false);
        }

    ElementHandle dimElement;
    BentleyStatus status = SUCCESS;
    if (SUCCESS != (status = GetInvisibleDimension (dimElement, noteElement)))
        return status;
    
    DimensionHandler* dimHandler = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
    if (NULL == dimHandler)
        return ERROR;

    EditElementHandle newDimElement(dimElement, true);
    dimHandler->ApplyDimensionStyle(newDimElement, dimStyle, false);
    EditElementHandle newNoteElement;
    if (SUCCESS != (status = (BentleyStatus) mdlNote_updateDimSettings (newNoteElement, noteElement, newDimElement)))
        return status;

    noteElement.ReplaceElementDescr (newNoteElement.ExtractElementDescr().get());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteCellHeaderHandler::SetupLeaderEndTangent(EditElementHandleR dimElemHandle, DimStyleProp_MLNote_TextRotation textRotation, bool rightSideAttachment)
    {
    if (!dimElemHandle.IsValid ())
        return ERROR;

    DimensionHandler& handler = DimensionHandler::GetInstance();
    
    DVec3d endTangent;
    if (DIMSTYLE_VALUE_MLNote_TextRotation_Inline == textRotation)
        {
        // Note : Tangents need to point into the curve
        int numPoints = handler.GetNumPoints(dimElemHandle);
        if (numPoints < 2)
            return ERROR;

        DPoint3d points[2];
        handler.ExtractPoint(dimElemHandle, points[0], numPoints-2);
        handler.ExtractPoint(dimElemHandle, points[1], numPoints-1);
        endTangent = DVec3d::FromStartEndNormalize(points[0], points[1]);
        }
    else
        {
        RotMatrix dimMatrixR;
        if (SUCCESS != handler.GetRotationMatrix (dimElemHandle, dimMatrixR))
            return ERROR;
        
        dimMatrixR.Transpose();
        endTangent.x = DIMSTYLE_VALUE_MLNote_TextRotation_Vertical == textRotation ? 0.0 : -1.0;
        endTangent.y = DIMSTYLE_VALUE_MLNote_TextRotation_Vertical == textRotation ? -1.0 : 0.0;
        endTangent.z = 0.0;
        // Flip end tangent if attachment is on right side
        if (rightSideAttachment)
            endTangent.Negate();

        dimMatrixR.MultiplyTranspose(endTangent);//unrotatePoint
        }

    return static_cast<BentleyStatus> (mdlDim_segmentSetCurveEndTangent (dimElemHandle, 0, &endTangent));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteCellHeaderHandler::UpdateOffsetAssociation (EditElementHandleR noteCell, ElementHandleCR leader)
    {
    DimensionElm const* dimElm = getSafeDimElement(leader);
    if (NULL == dimElm || !dimElm->GetDimTextCP(0)->flags.b.associative)
        return ERROR;
    
    if (dimElm->IsInvisible())
        return ERROR;
    DPoint3d offsetLocation;
    if (SUCCESS != GetOffsetAssociationLocation(noteCell, offsetLocation))
        return ERROR;
    
    AssocPoint assocPt;
    if (SUCCESS != AssociativePoint::ExtractPoint (assocPt, leader, 0, dimElm->nPoints))
        return ERROR;
    
    ElementRefP elemRef;
    DgnModelP modelRef;
    if (SUCCESS != AssociativePoint::GetRoot(&elemRef, &modelRef, NULL, NULL, assocPt, leader.GetDgnModelP()))
        return ERROR;
    DPoint3d leaderPoint = dimElm->GetPoint(0);
    if (bsiDPoint3d_pointEqualTolerance (&leaderPoint, &offsetLocation, mgds_fc_epsilon))
        return SUCCESS;
    ElementHandle target(elemRef);
    return SetupOffsetAssociation(noteCell, target, assocPt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteCellHeaderHandler::UpdateFieldTarget (EditElementHandleR noteCell, ElementHandleCR newtarget)
    {
    ElementHandle currentTextPartEh;
    if (SUCCESS != findChildTextEh (noteCell, &currentTextPartEh))
        return ERROR;
    
    TextBlockPtr newText = TextHandlerBase::GetFirstTextPartValue(currentTextPartEh);
    if (newText.IsNull())
        return ERROR;

#ifdef WIP_VANCOUVER_MERGE // text
    if (SUCCESS != newText->ReTargetField(NULL, DisplayPath(newtarget.GetElementRef(), newtarget.GetDgnModelP())))
        return ERROR;

    EditElementHandle newTextPartEeh;
    if (TextBlock::TO_ELEMENT_RESULT_Success != newText->ToElement (newTextPartEeh, noteCell.GetDgnModelP (), &currentTextPartEh))
        return ERROR;
    
    if (SUCCESS != BentleyApi::mdlNote_replaceText (noteCell, noteCell, newTextPartEeh, noteCell.GetDgnModelP ()))
        return ERROR;
#endif
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   NoteCellHeaderHandler::SetupOffsetAssociation (EditElementHandleR noteCell, ElementHandleCR targetElement, AssocPoint const& assoc)
    {
    DPoint3d cellOrigin;
    if (SUCCESS != CellUtil::ExtractOrigin (cellOrigin, noteCell))
        return ERROR;

    return RelativeOffsetAssociation::AddOffsetAssociation (noteCell, targetElement, assoc, cellOrigin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       NoteCellHeaderHandler::GetOffsetAssociationLocation (ElementHandleCR noteCell, DPoint3dR point)
    {
    if (SUCCESS != CellUtil::ExtractOrigin (point, noteCell))
        return ERROR;
        
    DVec3d offset;
    if (SUCCESS != RelativeOffsetAssociation::GetOffsetValue (noteCell, offset))
        return ERROR;

    point.Add(offset);
    return SUCCESS;
    }
