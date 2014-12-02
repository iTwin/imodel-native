/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/Dimension/DimTerminator.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*----------------------------------------------------------------------+
|                                                                       |
| name          adim_generateLineTerminator                             |
|                                                                       |
| author        JVB                                     10/89           |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      BentleyApi::adim_generateLineTerminator
(
AdimProcess     *ep,         /* => Function used to process element    */
DPoint3d        *start,      /* => Dimension line start point          */
DPoint3d        *end,        /* => Dimension line end point            */
int             termindx,     /* => Dimension terminator index          */
bool            isBallAndChain
)
    {
    DVec3d      direction;
    DPoint3d    org;
    double      leaderLen;
    double      tolerance = ep->GetDimElementCP()->geom.margin > 1.0 ? 1.0 : ep->GetDimElementCP()->geom.margin;
    int         status;

    if (bsiDPoint3d_distance(start, end) < tolerance)
        return  SUCCESS;

    direction.NormalizedDifference (*end, *start);
    DimensionType dimType = static_cast<DimensionType>(ep->GetDimElementCP()->dimcmd);

    /*-------------------------------------------------------------------
    If arrowOut (JIS) is on reverse terminator and draw short leader
    -------------------------------------------------------------------*/
    if (!isBallAndChain && ep->GetDimElementCP()->flag.termMode == 1 && (DIM_LINEAR(dimType) || DIM_ANGULAR(dimType)))
        {
        leaderLen = ep->GetDimElementCP()->version < 7 ? ep->GetDimElementCP()->geom.termWidth * 2.0 : ep->GetDimElementCP()->geom.margin;

        /* terminator's minimum length is ignored, get the fixed outside min leader */
        if (leaderLen > 0.0)
            BentleyApi::adim_getEffectiveMinLeaders (NULL, &leaderLen, ep);

        bsiDPoint3d_addScaledDVec3d (&org, end, &direction, leaderLen);

        if (status = adim_generateDimLine (ep, &org, end, DIM_MATERIAL_Terminator,
                                           termindx, termindx, TRIM_RIGHT,
                                           false, false, true))
            return (status);

        bsiDVec3d_scale (&direction, &direction, -1.0);
        }

    return (adim_generateTerminator (ep, end, &direction, termindx));
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      generateArrowhead - generate arrowhead terminator           |
|                                                                       |
| author    JVB                                         9/90            |
|                                                                       |
+----------------------------------------------------------------------*/
static int generateArrowhead
(
AdimProcess     *ep,        /* => Function used to process element    */
DimTermBlock   *termBlock,  /* => Terminator block                    */
DPoint3d       *origin,     /* => Symbol origin                       */
RotMatrix      *rMatrix,    /* => Symbol orientation                  */
UInt64       arrowID
)
    {
    DPoint2d      tile;
    DPoint3d      rpoint[4];
    double        halfHeight;

    if (termBlock && termBlock->flags.arrow == 1)
        {
        if (termBlock->arrow.symbol.font &&
            termBlock->arrow.symbol.symb)
            {
            tile.x = ep->GetDimElementCP()->geom.termWidth;
            tile.y = ep->GetDimElementCP()->geom.termHeight;
            return (adim_generateSymbol (ep, termBlock->arrow.symbol.font, termBlock->arrow.symbol.symb,
                                         origin, rMatrix, &tile, TextElementJustification::RightMiddle, DIM_MATERIAL_Terminator));
            }
        }
    else if (termBlock && termBlock->flags.arrow == 2)  /* shared cell with absolute size */
        {
        return (adim_generateCell (ep, arrowID, origin, rMatrix,
                ep->GetDimElementCP()->geom.termWidth, ep->GetDimElementCP()->geom.termHeight, DIM_MATERIAL_Terminator));
        }
    else if (termBlock && termBlock->flags.arrow == 3)  /* shared cell with scale factor */
        {
        if (DoubleOps::AlmostEqual (0.0, termBlock->scScale))
            return SUCCESS;

        return (adim_generateCellScale (ep, arrowID, origin, rMatrix, termBlock->scScale, DIM_MATERIAL_Terminator));
        }
    else
        {
        memset (rpoint, 0, sizeof(rpoint));

        halfHeight = ep->GetDimElementCP()->geom.termHeight * 0.5;

        rpoint[0].x = rpoint[2].x = - ep->GetDimElementCP()->geom.termWidth;
        rpoint[0].y = halfHeight;
        rpoint[2].y = - halfHeight;
        rpoint[3]   = rpoint[0];

        rMatrix->Multiply(rpoint, rpoint,  4);
        bsiDPoint3d_addDPoint3dArray (rpoint, origin, 4);

        return (adim_generateLineString (ep, rpoint,
                ep->GetDimElementCP()->flag.arrowhead ? 4 : 3,
                ep->GetDimElementCP()->flag.arrowhead, DIM_MATERIAL_Terminator));
        }

    return (SUCCESS);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      generateStroke - generate oblique stroke terminator         |
|                                                                       |
| author    JVB                                         9/90            |
|                                                                       |
+----------------------------------------------------------------------*/
static int generateStroke
(
AdimProcess    *ep,          /* => Function used to process element    */
DimTermBlock   *termBlock,   /* => Terminator block                    */
DPoint3d       *origin,      /* => Symbol origin                       */
RotMatrix      *rMatrix,     /* => Symbol orientation                  */
UInt64      cellId
)
    {
    DPoint2d      tile;
    DSegment3d stroke;
    double        halfWidth, halfHeight;

    if (termBlock && termBlock->flags.stroke == 1)
        {
        if (termBlock->stroke.symbol.font &&
            termBlock->stroke.symbol.symb)
            {
            tile.x = ep->GetDimElementCP()->geom.termWidth;
            tile.y = ep->GetDimElementCP()->geom.termHeight;
            return (adim_generateSymbol (ep, termBlock->stroke.symbol.font, termBlock->stroke.symbol.symb,
                                         origin, rMatrix, &tile, TextElementJustification::CenterMiddle, DIM_MATERIAL_Terminator));
            }
        }
    else if (termBlock && termBlock->flags.stroke == 2)  /* shared cell with absolute size */
        {
        return (adim_generateCell (ep, cellId, origin, rMatrix,
                                   ep->GetDimElementCP()->geom.termWidth, ep->GetDimElementCP()->geom.termHeight, DIM_MATERIAL_Terminator));
        }
    else if (termBlock && termBlock->flags.stroke == 3)   /* shared cell with scale factors */
        {
        return (adim_generateCellScale (ep, cellId, origin, rMatrix, termBlock->scScale, DIM_MATERIAL_Terminator));
        }
    else
        {
        memset (&stroke, 0, sizeof(stroke));
        halfWidth    =  ep->GetDimElementCP()->geom.termWidth  / 2.0;
        halfHeight   =  ep->GetDimElementCP()->geom.termHeight / 2.0;
        stroke.point[0].x =  halfWidth;
        stroke.point[0].y =  halfHeight;
        stroke.point[1].x = -halfWidth;
        stroke.point[1].y = -halfHeight;
        rMatrix->Multiply(&stroke.point[0], &stroke.point[0],  2);

        bsiDPoint3d_addDPoint3dArray (&stroke.point[0], origin, 2);

        return (adim_generateLine (ep, &stroke.point[0], &stroke.point[1], DIM_MATERIAL_Terminator));
        }

    return (SUCCESS);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      generateOrigin - common origin terminator                   |
|                                                                       |
| author    JVB                                         12/89           |
|                                                                       |
+----------------------------------------------------------------------*/
static int generateOrigin
(
AdimProcess  *ep,            /* => Function used to process element    */
DimTermBlock    *termBlock,  /* => Terminator block                    */
DPoint3d        *origin,     /* => Symbol origin                       */
RotMatrix       *rMatrix,    /* => Symbol orientation                  */
UInt64       cellId
)
    {
    DPoint2d     tile;
    double       radius;

    adim_getTileSize (&tile, NULL, ep->GetElemHandleCR());

    if (termBlock && termBlock->flags.origin == 1)
        {
        if (termBlock->origin.symbol.font && termBlock->origin.symbol.symb)
            {
            tile.x = ep->GetDimElementCP()->geom.termWidth;
            tile.y = ep->GetDimElementCP()->geom.termHeight;
            return (adim_generateSymbol (ep, termBlock->origin.symbol.font, termBlock->origin.symbol.symb,
                                         origin, rMatrix, &tile, TextElementJustification::CenterMiddle, DIM_MATERIAL_Terminator));
            }
        }
    else if (termBlock && termBlock->flags.origin == 2) /* shared cell with absolute size */
        {
        return (adim_generateCell (ep, cellId, origin, rMatrix,
                                   ep->GetDimElementCP()->geom.termHeight, ep->GetDimElementCP()->geom.termHeight, DIM_MATERIAL_Terminator));
        }
    else if (termBlock && termBlock->flags.origin == 3) /* shared cell with scale factors */
        {
        return (adim_generateCellScale (ep, cellId, origin, rMatrix, termBlock->scScale, DIM_MATERIAL_Terminator));
        }
    else
        {
        radius = ep->GetDimElementCP()->geom.termHeight / 2.0;
        return adim_generateCircle (ep, origin, radius, rMatrix, false, DIM_MATERIAL_Terminator);
        }

    return (SUCCESS);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      generateDot - generate dot (joint) terminator               |
|                                                                       |
| author    JVB                                         9/90            |
|                                                                       |
+----------------------------------------------------------------------*/
static int generateDot
(
AdimProcess *ep,             /* => Function used to process element    */
DimTermBlock   *termBlock,   /* => Terminator block                    */
DPoint3d       *origin,      /* => Symbol origin                       */
RotMatrix      *rMatrix,     /* => Symbol orientation                  */
UInt64      cellId
)
    {
    DPoint2d    tile;
    double      radius;

    if (termBlock && termBlock->flags.dot == 1)
        {
        if (termBlock->dot.symbol.font && termBlock->dot.symbol.symb)
            {
            tile.x = ep->GetDimElementCP()->geom.termWidth;
            tile.y = ep->GetDimElementCP()->geom.termHeight;
            return (adim_generateSymbol (ep, termBlock->dot.symbol.font, termBlock->dot.symbol.symb,
                                         origin, rMatrix, &tile, TextElementJustification::CenterMiddle, DIM_MATERIAL_Terminator));
            }
        }
    else if (termBlock && termBlock->flags.dot == 2)    /* shared cell with absolute size */
        {
        return (adim_generateCell (ep, cellId, origin, rMatrix,
                                   ep->GetDimElementCP()->geom.termHeight, ep->GetDimElementCP()->geom.termHeight, DIM_MATERIAL_Terminator));
        }
    else if (termBlock && termBlock->flags.dot == 3)    /* shared cell with scale factors */
        {
        return (adim_generateCellScale (ep, cellId, origin, rMatrix, termBlock->scScale, DIM_MATERIAL_Terminator));
        }
    else
        {
        radius = ep->GetDimElementCP()->geom.termHeight / 8.0;

        return (adim_generateCircle (ep, origin, radius, rMatrix, 2 == ep->GetDimElementCP()->flag.arrowhead,
                                     DIM_MATERIAL_Terminator));
        }

    return (SUCCESS);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      generateNoteTerm - generate note terminator                 |
|                                                                       |
| author    SunandSandurkar                             08/03           |
|                                                                       |
+----------------------------------------------------------------------*/
static int generateNoteTerm
(
AdimProcess    *ep,          /* => Function used to process element    */
DimTermBlock   *termBlock,   /* => Terminator block                    */
DPoint3d       *origin,      /* => Symbol origin                       */
RotMatrix      *rMatrix      /* => Symbol orientation                  */
)
    {
    int             iNoteTermType;
    double          halfHeight;
    DPoint3d        rpoint[4];
    UInt64          cellId = 0;
    DPoint2d        tile;

    mdlDim_extensionsGetNoteTerminatorType (&iNoteTermType, ep->pOverrides, 0);

    // If doing dynamics OR generating a cell which is not available, change the note terminator type to default
    if (iNoteTermType > 1 && SUCCESS != adim_getCellId (&cellId, ep->GetElemHandleCR(), DEPENDENCYAPPVALUE_NoteTerminator))
        iNoteTermType = 0;

    if (iNoteTermType == 1)                         /* symbol */
        {
        UInt16  iNoteTermChar;
        UInt32  iNoteTermFont;

        mdlDim_extensionsGetNoteTermChar (&iNoteTermChar, ep->pOverrides, 0);
        mdlDim_extensionsGetNoteTermFont (&iNoteTermFont, ep->pOverrides, 0);

        if (iNoteTermFont && iNoteTermChar)
            {
            tile.x = ep->GetDimElementCP()->geom.termWidth;
            tile.y = ep->GetDimElementCP()->geom.termHeight;
            return (adim_generateSymbol (ep, iNoteTermFont, iNoteTermChar, origin, rMatrix, &tile, TextElementJustification::RightMiddle, DIM_MATERIAL_Terminator));
            }
        }
    else if (iNoteTermType == 2)                    /* shared cell with absolute size */
        {
        return (adim_generateCell (ep, cellId, origin, rMatrix,
                ep->GetDimElementCP()->geom.termWidth, ep->GetDimElementCP()->geom.termHeight, DIM_MATERIAL_Terminator));
        }
    else if (iNoteTermType == 3 && termBlock)       /* shared cell with scale factor */
        {
        return (adim_generateCellScale (ep, cellId, origin, rMatrix, termBlock->scScale, DIM_MATERIAL_Terminator));
        }
    else                                            /* default */
        {
        memset (rpoint, 0, sizeof(rpoint));

        halfHeight = ep->GetDimElementCP()->geom.termHeight * 0.5;

        rpoint[0].x = rpoint[2].x = - ep->GetDimElementCP()->geom.termWidth;
        rpoint[0].y = halfHeight;
        rpoint[2].y = - halfHeight;
        rpoint[3]   = rpoint[0];

        rMatrix->Multiply(rpoint, rpoint,  4);

        bsiDPoint3d_addDPoint3dArray ((DVec3d*) rpoint, (DVec3d*) origin, 4);

        return (adim_generateLineString (ep, rpoint,
                ep->GetDimElementCP()->flag.arrowhead ? 4 : 3,
                ep->GetDimElementCP()->flag.arrowhead, DIM_MATERIAL_Terminator));
        }

    return (SUCCESS);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name      adim_generateTerminator                                     |
|                                                                       |
| author    JVB                                         7/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      BentleyApi::adim_generateTerminator
(
AdimProcess     *ep,         /* => function to process element         */
DPoint3d        *endpt,      /* => endpoint of arrow                   */
DVec3d          *direction,  /* => direction of arrow                  */
int             termindx     /* => dimension terminator index          */
)
    {
    DimTermBlock    *termBlock;
    RotMatrix       rMatrix;

    if (ep->pDerivedData && ep->pDerivedData->pTermDirs)
        {
        int type     = ADIM_GETTYPE (ep->partName);
        int iSegment = ADIM_GETSEG (ep->partName);

        if (ADTYPE_TERM_LEFT == type)
            {
            ep->pDerivedData->pTermDirs[iSegment].flags.hasLeftTerm = true;
            ep->pDerivedData->pTermDirs[iSegment].leftDir = *direction;
            }
        else if (ADTYPE_TERM_RIGHT == type)
            {
            ep->pDerivedData->pTermDirs[iSegment].flags.hasRightTerm = true;
            ep->pDerivedData->pTermDirs[iSegment].rightDir = *direction;
            }
        }
    
    UInt64 arrowID[4] = {0};
    termBlock = (DimTermBlock*) mdlDim_getOptionBlock (ep->GetElemHandleCR(), ADBLK_TERMINATOR, arrowID);
    BentleyApi::adim_getRMatrixFromDir (&rMatrix, direction, &ep->rMatrix, &ep->vuMatrix);

    ADIM_SETNAME (ep->partName, ADTYPE_INHERIT, ADSUB_TERMSYMBOL);

    switch (termindx)
        {
        case DIMSTYLE_VALUE_Terminator_Type_Arrow:
            return (generateArrowhead (ep, termBlock, endpt, &rMatrix, arrowID[DEPENDENCYAPPVALUE_ArrowHeadTerminator-1]));

        case DIMSTYLE_VALUE_Terminator_Type_Stroke:
            return (generateStroke (ep, termBlock, endpt, &rMatrix, arrowID[DEPENDENCYAPPVALUE_StrokeTerminator-1]));

        case DIMSTYLE_VALUE_Terminator_Type_Origin:
            return (generateOrigin (ep, termBlock, endpt, &rMatrix, arrowID[DEPENDENCYAPPVALUE_OriginTerminator-1]));

        case DIMSTYLE_VALUE_Terminator_Type_Dot:
            return (generateDot (ep, termBlock, endpt, &rMatrix, arrowID[DEPENDENCYAPPVALUE_DotTerminator-1]));

        case DIMSTYLE_VALUE_Terminator_Type_Note:
            return (generateNoteTerm (ep, termBlock, endpt, &rMatrix));
         }

    return (SUCCESS);
    }
