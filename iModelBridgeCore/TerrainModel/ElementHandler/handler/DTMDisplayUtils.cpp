/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMDisplayUtils.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

#include "MrDTMDataRef.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

static ::DRange3d const s_fullNpcRange =
    {
    {0.0, 0.0, 0.0},
    {1.0, 1.0, 1.0}
    };

static ::DPoint3d const s_npcViewBox[8] =
    {
    {0,0,0},
    {1,0,0},
    {0,1,0},
    {1,1,0},
    {0,0,1},
    {1,0,1},
    {0,1,1},
    {1,1,1}
    };

enum purposeMethod
    {
    FromContext,
    FromScanCriteria,
    WholeDTM
    };

static purposeMethod purposeUseScanCriteria[] =
    {
    WholeDTM, //DrawPurpose::NotSpecified      = 0,
    FromContext, //DrawPurpose::Update            = 1,
    FromContext, //DrawPurpose::UpdateDynamic     = 2,
    FromContext, //DrawPurpose::UpdateHealing     = 3,
    FromContext, //DrawPurpose::Hilite            = 5,
    FromContext, //DrawPurpose::Unhilite          = 6,
    FromContext, //DRAW_PURPOSE_Created           = 7,
    FromContext, //DRAW_PURPOSE_Deleted           = 8,
    FromContext, //DrawPurpose::ChangedPre        = 9,
    FromContext, //DrawPurpose::ChangedPost       = 10,
    FromContext, //DrawPurpose::RestoredPre       = 11,
    FromContext, //DrawPurpose::RestoredPost      = 12,
    FromContext, //DRAW_PURPOSE_RestoredDeleted   = 13,
    FromContext, //DRAW_PURPOSE_RestoredUndeleted = 14,
    FromContext, //DrawPurpose::Dynamics          = 15,
    FromContext, //DRAW_PURPOSE_EraseBeforeHilite = 16,
    FromContext, //DRAW_PURPOSE_EraseBeforeModify = 17,
    FromContext, //DRAW_PURPOSE_GetDescr          = 18,
    FromContext, //DRAW_PURPOSE_Animation         = 19,
    FromContext, //DrawPurpose::RangeCalculation  = 20,
    FromContext, //DrawPurpose::Plot              = 21,
    FromContext, //DrawPurpose::Pick              = 22,
    FromContext, //DrawPurpose::Flash             = 23,
    FromContext, //DrawPurpose::TransientChanged  = 25,
    WholeDTM,    //DrawPurpose::CaptureGeometry   = 26,
    WholeDTM,    //DrawPurpose::GenerateThumbnail = 27,
    WholeDTM,    //DRAW_PURPOSE_DrawFenceElement  = 28,
    FromContext, //DrawPurpose::ForceRedraw       = 29,
    FromScanCriteria, //DrawPurpose::FenceAccept       = 30,
    FromContext, //DRAW_PURPOSE_CreateSymbol      = 31,
    WholeDTM,    //DrawPurpose::FitView           = 32,
    FromContext, //DRAW_PURPOSE_RasterProgressiveUpdate = 33,
    WholeDTM,    //DrawPurpose::XGraphicsCreate   = 34,
    WholeDTM,    //DrawPurpose::CaptureShadowList = 35,
    WholeDTM,    //DrawPurpose::ExportVisibleEdges = 36,
    FromContext, //DrawPurpose::InterferenceDetection = 37,
    FromScanCriteria, //DrawPurpose::CutXGraphicsCreate = 38,
    WholeDTM, //DrawPurpose::ModelFacet         = 39,
    WholeDTM, //DrawPurpose::Measure            = 40,
    WholeDTM, //DrawPurpose::VisibilityCalculation = 41
    };


//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  01/11
//=======================================================================================
bool GetViewVectorPoints (DTMDrawingInfo& drawingInfo, ViewContextR context, DTMPtr dtm, DPoint3d& startPt, DPoint3d& endPt)
    {
    DPoint3d point;
    Transform trans;
    context.GetCurrLocalToFrustumTrans (trans);
    trans.inverseOf (&trans);

    point = context.GetIPickGeom ()->GetPickPointRoot();

    context.LocalToView (&startPt, &point, 1);
    context.ViewToNpc (&startPt, &startPt, 1);
    endPt = startPt;
    endPt.z = 1;
    startPt.z = -1;

    context.NpcToView (&startPt, &startPt, 1);
    context.ViewToLocal (&startPt, &startPt, 1);
    trans.multiply (&startPt);
    drawingInfo.GetUORToStorageTransformation()->multiply(&startPt);

    context.NpcToView(&endPt, &endPt, 1);
    context.ViewToLocal(&endPt, &endPt, 1);
    trans.multiply (&endPt);
    drawingInfo.GetUORToStorageTransformation()->multiply(&endPt);

    // Intersect line with the DTM Range
    DRange3d range;
    DPoint3d sP;
    DPoint3d eP;
    dtm->GetRange (range);

    endPt.x -= startPt.x;
    endPt.y -= startPt.y;
    endPt.z -= startPt.z;
    if (!range.intersectRay (nullptr, nullptr, &sP, &eP, &startPt, &endPt))
        return false;
    startPt = sP;
    endPt = eP;
    return true;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
static void FixFrustumOrder (::DPoint3dP polyhedron)
    {
    ::DVec3d u, v, w;
    u.differenceOf (polyhedron+NPC_001, polyhedron+NPC_000);
    v.differenceOf (polyhedron+NPC_010, polyhedron+NPC_000);
    w.differenceOf (polyhedron+NPC_100, polyhedron+NPC_000);

    if (u.tripleProduct (&v, &w) <= 0)
        return;

    // frustum has mirroring, reverse points
    for (int i=0; i<8; i+=2)
        {
        ::DPoint3d tmpPoint = polyhedron[i];
        polyhedron[i] = polyhedron[i+1];
        polyhedron[i+1] = tmpPoint;
        }
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  04/10
//=======================================================================================
bool GetUpdateRangeFromRangeNPCSubRect (ViewContextR viewContext, DTMUnitsConverter& conv, const ::DRange3d& range, ::DRange3d& updateRange, const ::DRange3d* npcSubRect)
    {
    ::DRange3d viewRange;
    ::DRange3d updateViewRange;
    ::DPoint3d updateViewRangePts[8];

    if (!npcSubRect)
        npcSubRect = &s_fullNpcRange;

    viewRange = range;

    conv.FullStorageToUors((::DPoint3dP)&viewRange, 2);

    // Convert Range to npc.
    updateViewRangePts[0].x = viewRange.low.x; updateViewRangePts[0].y = viewRange.low.y; updateViewRangePts[0].z = viewRange.low.z;
    updateViewRangePts[1].x = viewRange.low.x; updateViewRangePts[1].y = viewRange.high.y; updateViewRangePts[1].z = viewRange.low.z;
    updateViewRangePts[2].x = viewRange.high.x; updateViewRangePts[2].y = viewRange.high.y; updateViewRangePts[2].z = viewRange.low.z;
    updateViewRangePts[3].x = viewRange.high.x; updateViewRangePts[3].y = viewRange.low.y; updateViewRangePts[3].z = viewRange.low.z;
    updateViewRangePts[4].x = viewRange.low.x; updateViewRangePts[4].y = viewRange.low.y; updateViewRangePts[4].z = viewRange.high.z;
    updateViewRangePts[5].x = viewRange.low.x; updateViewRangePts[5].y = viewRange.high.y; updateViewRangePts[5].z = viewRange.high.z;
    updateViewRangePts[6].x = viewRange.high.x; updateViewRangePts[6].y = viewRange.high.y; updateViewRangePts[6].z = viewRange.high.z;
    updateViewRangePts[7].x = viewRange.high.x; updateViewRangePts[7].y = viewRange.low.y; updateViewRangePts[7].z = viewRange.high.z;
    viewContext.LocalToView(updateViewRangePts,updateViewRangePts, 8);
    viewContext.ViewToNpc(updateViewRangePts,updateViewRangePts, 8);
    ::DRange3d refClipRange;
    refClipRange.initFrom (updateViewRangePts, 8);

    // Clip this into view npc only.
    // If we have a subclip use this to clip against as well.
        if (!refClipRange.isContained(npcSubRect))
            if (!bsiDRange3d_intersect(&refClipRange, (::DRange3dP)npcSubRect, &refClipRange))
                {
                // Not in subrect.
                bsiDRange3d_init(&updateRange);
                return false;
                }

    // refClipRange now contains the area with the range in npc now need to convert this into view coordinates.

    ::DPoint3d    polyhedron[8];
    memcpy(polyhedron, s_npcViewBox, sizeof(polyhedron));

    // Could possibly expand here for non subrects.
    ::DMatrix4d   npcToLocal = viewContext.GetFrustumToNpc().M1;
    ::DMap4d      npcSubRectMap;
    npcSubRectMap.initFromRanges (&refClipRange.low, &refClipRange.high, &polyhedron[NPC_000], &polyhedron[NPC_111]);
    npcToLocal.productOf (&npcToLocal, &npcSubRectMap.M1);

    npcToLocal.multiplyAndRenormalize (polyhedron, polyhedron, NPC_CORNER_COUNT);
    viewContext.FrustumToLocal(polyhedron, polyhedron, NPC_CORNER_COUNT);
    FixFrustumOrder (polyhedron);

    // get enclosing bounding box around polyhedron (outside scan range).
    updateViewRange.initFrom (polyhedron, NPC_CORNER_COUNT);

    conv.FullUorsToStorage((::DPoint3dP)&updateViewRange, 2);

    // Remove the bits outside of the original range.
    viewRange = range;
    if (!updateViewRange.isContained(&viewRange))
        if (!bsiDRange3d_intersect(&updateViewRange, (::DRange3dP)&viewRange, &updateViewRange))
            bsiDRange3d_init (&updateViewRange);

    updateRange = updateViewRange;

    return true;
    }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  01/11
//=======================================================================================
bool GetUpdateRangeFromRange (ViewContextR viewContext, DTMUnitsConverter& conv, const ::DRange3d& range, ::DRange3d& updateRange, int pixelExpansion, bool allowHealingUpdate)
    {
    BSIRect tRect;
    bool m_useSubRect = false;
    ::DRange3d  npcSubRect;

    // No Viewport so return entire range.
    if (!viewContext.GetViewport())
        {
        updateRange = range;
        return true;
        }

    // Work out the dirty rectangle in npc.
    if (allowHealingUpdate && viewContext.GetViewport()->GetIViewOutput()->CheckNeedsHeal(&tRect))
        {
        m_useSubRect = true;
        // Look at the DirtyRect and get the range
        tRect.Expand (pixelExpansion);

        ::DRange3d viewRange2;
        viewRange2.low.init  (tRect.origin.x, tRect.corner.y, 0.0);
        viewRange2.high.init (tRect.corner.x, tRect.origin.y, 0.0);

        viewContext.ViewToNpc (&viewRange2.low, &viewRange2.low, 2);

        // this is due to fact that y's can be reversed from view to npc
        npcSubRect.initFrom (&viewRange2.low, 2);

        npcSubRect.low.z  = 0.0;                // make sure entire z range.
        npcSubRect.high.z = 1.0;

        }
    if (m_useSubRect)
        return GetUpdateRangeFromRangeNPCSubRect (viewContext, conv, range, updateRange, &npcSubRect);
    return GetUpdateRangeFromRangeNPCSubRect (viewContext, conv, range, updateRange, nullptr);
    }

bool IsWireframeRendering(ViewContextCR viewContext)
    {    
    // Check context render mode
    switch (viewContext.GetViewFlags()->renderMode)
        {
        case MSRenderMode::ConstantShade:
        case MSRenderMode::SmoothShade:
        case MSRenderMode::Phong:
        case MSRenderMode::RayTrace:
        case MSRenderMode::Radiosity:
        case MSRenderMode::ParticleTrace:
        case MSRenderMode::RenderLuxology:
            return false;
                       
        case MSRenderMode::Wireframe:
        case MSRenderMode::CrossSection:
        case MSRenderMode::Wiremesh:
        case MSRenderMode::HiddenLine:
        case MSRenderMode::SolidFill:
        case MSRenderMode::RenderWireframe:
            return true;
        }
        BeAssert(!"Unknown render mode");
        return true;
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 08/11
//=======================================================================================
void GetViewBoxFromContext(::DPoint3d viewBoxPts[], int nbViewBoxPts, ViewContextP context, DTMUnitsConverter& conv)
    {            
    BeAssert(nbViewBoxPts == _countof(s_npcViewBox));

    DTransform3d storageToUORTransformation;
    Transform    ltf, ftl;
    
    storageToUORTransformation.initFromTransform ( conv.GetStorageToUORTransformation () );
    context->NpcToFrustum ( viewBoxPts, s_npcViewBox, nbViewBoxPts );

    context->GetCurrLocalToFrustumTrans ( ltf );
    ftl.inverseOf ( &ltf );
    ftl.multiply ( viewBoxPts, nbViewBoxPts );
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
int elemUtil_setRange
(
 ScanRange*          range,
 DRange3d const*     rangeVec,
 bool             setZ,
 bool             is3d
 )
    {
    DRange3d   tVec = *rangeVec;

    if ((tVec.low.x < RMINDESIGNRANGE) || (tVec.high.x > RMAXDESIGNRANGE) ||
        (tVec.low.y < RMINDESIGNRANGE) || (tVec.high.y > RMAXDESIGNRANGE))
        return ERROR;

    if (is3d)
        {
        if ((tVec.low.z < RMINDESIGNRANGE) || (tVec.high.z > RMAXDESIGNRANGE))
            return ERROR;
        }
    else
        {
        if (setZ)
            {
            tVec.low.z = tVec.high.z = 0.0;
            }
        else
            {
            tVec.low.z  = (double) range->zlowlim;
            tVec.high.z = (double) range->zhighlim;
            }
        }

    DataConvert::DRange3dToScanRange (*range, tVec);
    return SUCCESS;
    }

static int s_faceToBoxPoint[6][4] =
    {
    {1,0,2,3},
    {4,5,7,6},
    {0,4,6,2},
    {1,3,7,5},
    {0,1,5,4},
    {2,6,7,3},
    };

/*==================================================================*/
/*                                                                  */
/* BoxBoxIntersectionRange Code                                     */
/*                                                                  */
/* Note : This code was kindly provided by Earlin Lutz.             */
/*==================================================================*/

// BARE C CODE -- to be inserted/included in file with includes etc.

// Initialize a DPlane3d from origin, x point, and y point.
// (Plane normal is unit length)
static void InitPlane (DPlane3d & plane, DPoint3d const & xyz0, DPoint3d const &xyz1, DPoint3d const &xyz2)
    {
    DVec3d normal;
    DVec3d   vector01, vector02;
    bsiDVec3d_subtractDPoint3dDPoint3d (&vector01, &xyz1, &xyz0);
    bsiDVec3d_subtractDPoint3dDPoint3d (&vector02, &xyz2, &xyz0);
    bsiDVec3d_normalizedCrossProduct (&normal, &vector01, &vector02);
    bsiDPlane3d_initFromOriginAndNormal (&plane, &xyz0, &normal);
    }

// Construct plane origin&normal for each of the 6 planes of an 8 point box in standard point order.
void InitBoxPlanes (DPoint3d const *pBoxPoints, DPlane3d *pPlanes)
    {
    for (int planeIndex = 0; planeIndex < 6; planeIndex++)
        {
        InitPlane (
                pPlanes[planeIndex], 
                pBoxPoints[s_faceToBoxPoint[planeIndex][0]],
                pBoxPoints[s_faceToBoxPoint[planeIndex][1]],
                pBoxPoints[s_faceToBoxPoint[planeIndex][3]]
                );
        }
    }

// Return the 5 point linestring around a face of a box.
int GetBoxFace (DPoint3d const *pBoxPoints, int faceSelect, DPoint3d *pPoints)
    {
    faceSelect = faceSelect % 6;
    for (int i = 0; i < 4; i++)
        {
        pPoints[i] = pBoxPoints[s_faceToBoxPoint[faceSelect][i]];
        }
    pPoints[4] = pPoints[0];
    return 5;
    }

// Clip a (small) CONVEX polygon to a plane.
// Clipped polygon is updated IN PLACE
// The number of points may decrease (possibly to zero).
// The number of points may increase.  For a convex polygon, the increase can only be one point.
//   (For a non convex polygon, the increase can be larger -- the algorithm is still correct under "parity" logic
//     that handles concave sections as XOR dropouts)
// USES FIXED BUFFER FOR 100 POINTS -- perfectly adequate for range box clips, but not for general case.
static void ClipConvexPolygonToPlane (DPoint3d *polygonPoints, int &n, DPlane3d & plane)
    {
    DPoint3d clippedPoints[100];
    int m = 0;
    double h0, h1;
    for (int i = 0; i < n; i++)
        {
        h1 = bsiDPlane3d_evaluate (&plane, &polygonPoints[i]);
        if (i == 0)
            {
            if (h1 <= 0.0)
               clippedPoints[m++] = polygonPoints[i];
            }
        else
            {
            if (h0 * h1 < 0.0)
                {
                double s = -h0 / (h1 - h0);
                bsiDPoint3d_interpolate (&clippedPoints[m++],
                    &polygonPoints[i-1], s, &polygonPoints[i]);
                }
            if (h1 <= 0.0)
                clippedPoints[m++] = polygonPoints[i];
            }
        h0 = h1;
        }
    if (m > 0)
        {
        if (!bsiDPoint3d_pointEqual (&clippedPoints[0], &clippedPoints[m-1]))
            clippedPoints[m++] = clippedPoints[0];
        memcpy (polygonPoints, clippedPoints, m * sizeof (DPoint3d));
        }
    n = m;
    }

// Use the planes of box2 as clippers for the faces of box1.
// Extend the (evolving) range each clipped face.
void ExtendRangeForBoxIntersect (DPoint3d const *pBoxPoints1, DPoint3d const *pBoxPoints2, DRange3d &range)
    {
    DPoint3d xyz[100];
    DPlane3d planeSet2[6];
    InitBoxPlanes (pBoxPoints2, planeSet2);
    for (int select1 = 0; select1 < 6; select1++)
        {
        int nXYZ = GetBoxFace (pBoxPoints1, select1, xyz);
        for (int select2 = 0; select2 < 6; select2++)
            ClipConvexPolygonToPlane (xyz, nXYZ, planeSet2[select2]);
        bsiDRange3d_extendByDPoint3dArray (&range, xyz, nXYZ);
        }
    }

// Return the range of the intersection of two boxes.
// Each box is described by 8 corner points.
// The corner points correspond to the "x varies fastest, then y, then z" ordering.
// Hence the point order for the unit cube coordinates and view box positions is
//  (0,0,0)  (i.e. lower left rear)
//  (1,0,0)  (i.e. lower right rear)
//  (0,1,0)  (i.e. upper left rear)
//  (1,1,0)  (i.e. upper right rear)
//  (0,0,1)  (i.e. lower left front)
//  (1,0,1)  (i.e. lower right front)
//  (0,1,1)  (i.e. upper left front)
//  (1,1,1)  (i.e. upper right front)
// The box sides are (ASSUMED TO BE) planar -- this is not tested.
// The box sides do  not have to be parallel -- skewed frustum of a perspective view box is ok.
//! @param [in] boxPoints1 8 points of first box.
//! @param [in] boxPoints2 8 points of second box.
//! @param [out] intersectionRange range of the intersection of the two boxes.
void BoxBoxIntersectionRange
(
DPoint3dCP boxPoints1,
DPoint3dCP boxPoints2,
DRange3d &intersectionRange
)
    {
    bsiDRange3d_init (&intersectionRange);
    ExtendRangeForBoxIntersect (boxPoints1, boxPoints2, intersectionRange);
    ExtendRangeForBoxIntersect (boxPoints2, boxPoints1, intersectionRange);
    }

/*==================================================================*/
/*                                                                  */
/* End Of The BoxBoxIntersectionRange Code                          */
/*                                                                  */
/*==================================================================*/
 
/*---------------------------------------------------------------------------------**//**
* This function computes the visible area based on the definition of the viewbox
* and the DTM ranges.
* The viewbox coordinate smust be provided in UOR and the correcponding uor to meter
* ratio must be provided.
* @bsimethod                                                    MathieuStPierre  2/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool GetVisibleAreaForView(::DPoint3d** areaPt, int& nbPts, const DPoint3d viewBox[], DRange3d& dtmRange, DRange3d& dtmIntersectionRange)
    {        
    // Work out which bit of the triangulation is displayed on screen.    
    DRange3d dtmViewRange;
    bool isVisible = false;
       
    if (dtmRange.IsEmpty ())
        return false;
    dtmViewRange = dtmRange;
     
    DPoint3d dtmBox[8];    

    //The BoxBoxIntersectionRange function needs a box to perform correctly, so add a very 
    //small artificial range for any coordinate whose real range equals zero.    
    if (dtmViewRange.low.x == dtmViewRange.high.x)
        {        
        dtmViewRange.low.x -= 0.0000001;        
        }

    if (dtmViewRange.low.y == dtmViewRange.high.y)
        {        
        dtmViewRange.low.y -= 0.0000001;        
        }

    if (dtmViewRange.low.z == dtmViewRange.high.z)
        {        
        dtmViewRange.low.z -= 0.0000001;        
        }

    bsiDRange3d_box2Points(&dtmViewRange, dtmBox);
                
    BoxBoxIntersectionRange(dtmBox, viewBox, dtmIntersectionRange);
     
    if (dtmIntersectionRange.high.x > dtmIntersectionRange.low.x && 
        dtmIntersectionRange.high.y > dtmIntersectionRange.low.y)
        {

        if (areaPt != 0)
            {            
            nbPts = 5;
            *areaPt = new DPoint3d[nbPts];

            DPoint3d lowPt(dtmIntersectionRange.low);
            DPoint3d highPt(dtmIntersectionRange.high);            
                 
            (*areaPt)[0].x = lowPt.x;
            (*areaPt)[0].y = lowPt.y;
            (*areaPt)[0].z = lowPt.z;
            
            (*areaPt)[1].x = (*areaPt)[0].x;
            (*areaPt)[1].y = highPt.y;
            (*areaPt)[1].z = (*areaPt)[0].z;
            
            (*areaPt)[2].x = highPt.x;
            (*areaPt)[2].y = (*areaPt)[1].y ;
            (*areaPt)[2].z = (*areaPt)[0].z;
            
            (*areaPt)[3].x = (*areaPt)[2].x ;
            (*areaPt)[3].y = (*areaPt)[0].y;
            (*areaPt)[3].z = (*areaPt)[0].z;

            (*areaPt)[4].x = (*areaPt)[0].x;
            (*areaPt)[4].y = (*areaPt)[0].y;
            (*areaPt)[4].z = (*areaPt)[0].z;
            }

        isVisible = true;
        }
    
    return isVisible;        
    } 

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 03/10
//=======================================================================================
bool GetVisibleFencePointsFromContext(::DPoint3d*& fencePt, int& nbPts, ViewContextP context, DTMUnitsConverter& conv, DRange3d& dtmRange)
    {
    purposeMethod method = WholeDTM;

    if (dtmRange.IsEmpty ())
        return false;
    if (_countof (purposeUseScanCriteria) > static_cast<size_t>(context->GetDrawPurpose ()))
        method = purposeUseScanCriteria [static_cast<size_t>(context->GetDrawPurpose ())];

        // No Viewport return the entire range
    if (!context->GetViewport() || method == WholeDTM)
        {
        nbPts    = 5;
        fencePt = new ::DPoint3d[nbPts];
        fencePt[0] = dtmRange.low;
        fencePt[1].x = dtmRange.high.x;
        fencePt[1].y = dtmRange.low.y;
        fencePt[1].z = 0;
        fencePt[2] = dtmRange.high;
        fencePt[3].x = dtmRange.low.x;
        fencePt[3].y = dtmRange.high.y;
        fencePt[3].z = 0;
        fencePt[4] = dtmRange.low;
        return true;
        }

    // Work out which bit of the triangulation is displayed on screen.
    DRange3d        dtmIntersectionRange, dtmViewRange = dtmRange;
        
    DPoint3d viewBox[_countof(s_npcViewBox)];

    if (method == FromContext)
        {        
        GetViewBoxFromContext(viewBox, _countof(viewBox), context, conv);
        }
    else
        {
        ScanCriteriaCP scP = context->GetScanCriteria();
        ScanRange range;
    
        scP->GetExtendedRangeTest (&range, 0);
        viewBox[0].x = (double)range.xlowlim;  viewBox[0].y = (double)range.ylowlim;  viewBox[0].z = (double)range.zlowlim;
        viewBox[1].x = (double)range.xhighlim; viewBox[1].y = (double)range.ylowlim;  viewBox[1].z = (double)range.zlowlim;
        viewBox[2].x = (double)range.xlowlim;  viewBox[2].y = (double)range.yhighlim; viewBox[2].z = (double)range.zlowlim;
        viewBox[3].x = (double)range.xhighlim; viewBox[3].y = (double)range.yhighlim; viewBox[3].z = (double)range.zlowlim;
        viewBox[4].x = (double)range.xlowlim;  viewBox[4].y = (double)range.ylowlim;  viewBox[4].z = (double)range.zhighlim;
        viewBox[5].x = (double)range.xhighlim; viewBox[5].y = (double)range.ylowlim;  viewBox[5].z = (double)range.zhighlim;
        viewBox[6].x = (double)range.xlowlim;  viewBox[6].y = (double)range.yhighlim; viewBox[6].z = (double)range.zhighlim;
        viewBox[7].x = (double)range.xhighlim; viewBox[7].y = (double)range.yhighlim; viewBox[7].z = (double)range.zhighlim;
        }

    conv.FullUorsToStorage ( viewBox );
       
    if (context->Is3dView() == false)
        {        
        DRange3d viewRange;

        bsiDRange3d_initFromArray(&viewRange, viewBox, _countof(s_npcViewBox));
        
        dtmViewRange.low.z = viewRange.low.z;
        dtmViewRange.high.z = viewRange.high.z;        
        }

    return GetVisibleAreaForView ( &fencePt, nbPts, viewBox, dtmViewRange, dtmIntersectionRange);
}

//=======================================================================================
// @bsimethod                                                    KeithBentley    03/03
//=======================================================================================
void DrawScanRange (ViewContextR context, ElementHandleCR element, const RefCountedPtr<DTMDataRef>& dtmDataRef)
    {
          // Don't do this when trying to compute range or drop!
    switch (context.GetDrawPurpose ())
        {
        case DrawPurpose::CaptureGeometry:
//ToDo        case DRAW_PURPOSE_GetDescr:
            return;
        }

    ScanRange scanRange(element.GetElementCP()->hdr.dhdr.range);
    if ((dtmDataRef != nullptr) && (dtmDataRef->IsMrDTM() == true))
        {        
        if ((context.GetViewport() != 0) &&                    
            (((MrDTMDataRef*)dtmDataRef.get())->IsVisibleForView(context.GetViewport()->GetViewNumber()) == false))
            {
            return;
            }
            
        if (((MrDTMDataRef*)dtmDataRef.get())->GetClipActivation() == true)
            {
            DRange3d  clippedExtent;                        

            if (((MrDTMDataRef*)dtmDataRef.get())->GetClippedExtent(clippedExtent) == SUCCESS)
                {
                DataConvert::DRange3dToScanRange(scanRange, clippedExtent);
                }    
            }
        }

    DPoint3d    p[8];
    DRange3d   range;

    DataConvert::ScanRangeToDRange3d (range, scanRange);

    p[0].x = p[3].x = p[4].x = p[5].x = range.low.x;
    p[1].x = p[2].x = p[6].x = p[7].x = range.high.x;
    p[0].y = p[1].y = p[4].y = p[7].y = range.low.y;
    p[2].y = p[3].y = p[5].y = p[6].y = range.high.y;
    p[0].z = p[1].z = p[2].z = p[3].z = range.low.z;
    p[4].z = p[5].z = p[6].z = p[7].z = range.high.z;

    context.DrawBox (p, true);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
DgnModelRefP GetModelRef (ElementHandleCR element)
    {
    DgnModelRefP modelRef = element.GetModelRef();
    if (modelRef) return modelRef;

    if (element.GetElementDescrCP())
        {
        modelRef = element.GetElementDescrCP()->h.dgnModelRef;
        }
    if (modelRef) return modelRef;

    modelRef = element.GetDgnModelP();
    if (modelRef) return modelRef;
    if (IViewManager::GetActiveViewSet().GetSelectedViewport())
        modelRef = IViewManager::GetActiveViewSet().GetSelectedViewport()->GetTargetModel();
    BeAssert (modelRef);
    return modelRef;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 06/13
//=======================================================================================
DgnModelP GetDgnModel(ElementHandleCR element)
    {
    DgnModelP dgnCache = element.GetDgnModelP();
    if (dgnCache)
        return dgnCache;

    return GetActivatedModel (element, nullptr)->GetDgnModelP();
    }


/// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
DgnModelRefP GetActivatedModel (ElementHandleCR element, ViewContextCP context)
    {
    if (context && context->GetViewport())
        return context->GetViewport()->GetRootModel();

    if (nullptr != &DgnViewLib::GetHost () && IViewManager::GetActiveViewSet ().GetSelectedViewport ())
        return IViewManager::GetActiveViewSet ().GetSelectedViewport ()->GetRootModel ();
    return element.GetModelRef()->GetRoot();
//ToDo    return DTMSessionMonitor::GetInstance().GetActive ();
    }

void RedrawElement (ElementHandleR element)
    {
    RedrawElems redraw (nullptr, DRAW_MODE_Erase, DrawPurpose::ChangedPre, true);
    redraw.SetViews (IViewManager::GetActiveViewSet(), 0xffff);
    redraw.DoRedraw (element);
    }

/*=================================================================================**//**
* DTMTranslationManager
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct DTMTranslationManager
    {
    static const short CURRENT_VERSION = 1;

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  01/11
//=======================================================================================
    static bool GetTransformationToUORMatrix (Transform& trsf, ElementHandleCR element)
        {
        XAttributeHandlerId handlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_TRANSLATION);
        ElementHandle::XAttributeIter xAttrHandle(element, handlerId, 1);

        void* data(xAttrHandle.IsValid() ? (void*)xAttrHandle.PeekData() : nullptr);

        if (data)
            {
            DataInternalizer source ((byte*)data, xAttrHandle.GetSize());
            short version;

            source.get (&version);
            source.get ((Int8*)&trsf, sizeof(trsf.form3d));
            return true;
            }
        return false;
        }

//=======================================================================================
// @bsimethod                                                    Daryl.Holmwood  01/11
//=======================================================================================
    static void SetTransformationToUORMatrix (const Transform& trsf, EditElementHandleR element)
        {
        DataExternalizer d;
        short version = CURRENT_VERSION;
        d.put (version);

        d.put ((Int8*)&trsf, sizeof(trsf.form3d));

        XAttributeHandlerId handlerId (TMElementMajorId, XATTRIBUTES_SUBID_DTM_TRANSLATION);
        element.ScheduleWriteXAttribute(handlerId, 1, (UInt32)d.getBytesWritten(), d.getBuf());
        }
    };


//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//
// Sets the matrix that transforms the points from the storage units of the DTM
// to the UORs that are expected by IViewContext methods
//
//=======================================================================================    
void setStorageToUORMatrix (const Transform& trsf, EditElementHandleR element)
    {
    DTMTranslationManager::SetTransformationToUORMatrix(trsf, element);
    }

//=======================================================================================
// @bsimethod                                                   Mathieu.St-Pierre 07/11
//
// Gets the matrix that transforms the points from the storage units of the DTM
// to the UORs that are expected by IViewContext methods
//
//=======================================================================================    
void getStorageToUORMatrix (Transform& trsf, ElementHandleCR element)
    {
    getStorageToUORMatrix(trsf, element.GetModelRef(), element);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//
// Gets the matrix that transforms the points from the storage units of the DTM
// to the UORs that are expected by IViewContext methods
//
//=======================================================================================    
void getStorageToUORMatrix (Transform& trsf, DgnModelRefP model, ElementHandleCR element, bool withExaggeration)
    {    
    // Read from element.
    if(DTMTranslationManager::GetTransformationToUORMatrix(trsf, element) == false)       
        {
        trsf.InitIdentity();

        // We assume here that the DTM is stored in meters. We need to discuss about
        // that with Rob.
        double uorPerMeter = dgnModel_getUorPerMeter (model->GetDgnModelP());
        DPoint3d ptGO;
        StatusInt siResult = dgnModel_getGlobalOrigin (model->GetDgnModelP(), &ptGO);
        assert (siResult == SUCCESS); 

        bool is3d = model->GetDgnModelP()->Is3d();

        // Scale the matrix to UORs and translate it to the GO."
        trsf.SetTranslation (ptGO);
        trsf.ScaleMatrixColumns (uorPerMeter, uorPerMeter, is3d ? uorPerMeter : 0); // If the file is not a 3d file we zeroes the Z (problem with picking if elevation is not zeroed)
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.St-Pierre 12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DoProgressiveDraw(RefCountedPtr<DTMDataRef>& ref, ViewContextP viewContext)
    {
    bool doProgressiveDraw = false;

    WString cfgVarValue;
   
    if ((DTMElementHandlerManager::GetMrDTMActivationRefCount() > 0) &&
        (ref->IsMrDTM() == true) &&               
        (SUCCESS == ConfigurationManager::GetVariable(cfgVarValue, L"STM_PROGRESSIVE_DISPLAY_ACTIVATION")) && 
        (cfgVarValue == L"1") && 
        (DTMElementHandlerManager::IsDrawForAnimation() == false))
        {
        assert(MrDTMDataRef::GetMrDTMProgressiveDisplayInterface() != 0);

        //This list is based on what Point Cloud is doing.
        switch (viewContext->GetDrawPurpose())
            {
            case DrawPurpose::NotSpecified:
            case DrawPurpose::Update:
            case DrawPurpose::UpdateDynamic:    
            case DrawPurpose::UpdateHealing:        // TR:293660
    //        case DrawPurpose::Hilite:
    //        case DrawPurpose::Unhilite:
    //        case DRAW_PURPOSE_Created:
    //        case DRAW_PURPOSE_Deleted:
    //        case DrawPurpose::ChangedPre:
            case DrawPurpose::ChangedPost:
    //        case DrawPurpose::RestoredPre:
            case DrawPurpose::RestoredPost:
    //        case DRAW_PURPOSE_RestoredDeleted:
    //        case DRAW_PURPOSE_RestoredUndeleted:
    /*        case DrawPurpose::Dynamics:*/
    //        case DRAW_PURPOSE_EraseBeforeHilite:
    //        case DRAW_PURPOSE_EraseBeforeModify:
    //        case DRAW_PURPOSE_GetDescr:
    //        case DRAW_PURPOSE_Animation:
    //        case DrawPurpose::RangeCalculation:
    //        case DrawPurpose::Plot:
    //        case DrawPurpose::Pick:
    //        case DrawPurpose::Flash:
    //        case DrawPurpose::TransientChanged:
    //        case DrawPurpose::CaptureGeometry:
    //        case DrawPurpose::GenerateThumbnail:
    //        case DRAW_PURPOSE_DrawFenceElement:
            case DrawPurpose::ForceRedraw:
    //        case DrawPurpose::FenceAccept:
    //        case DRAW_PURPOSE_CreateSymbol:
    //        case DrawPurpose::FitView:
    //        case DRAW_PURPOSE_RasterProgressiveUpdate:
    //        case DrawPurpose::XGraphicsCreate:
    //        case DrawPurpose::CaptureShadowList:
    //        case DrawPurpose::ExportVisibleEdges:
    //        case DrawPurpose::InterferenceDetection:
    //        case DrawPurpose::CutXGraphicsCreate:
    //        case DrawPurpose::ModelFacet:
    //        case DrawPurpose::Measure:
                {
                doProgressiveDraw = true;
                }
            }                     
        }
    
    return doProgressiveDraw;
    }


END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

#ifndef NDEBUG
#include <Logging\bentleylogging.h>
#include <Bentley/BeDebugLog.h>
#ifdef USEOUTPUTDEBUGSTRING
#include <Windows.h>
#endif

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

static bool s_isInitialized = false;
NativeLogging::ILogger* pLogger = nullptr;
bool s_logInfo = true;
bool s_logDebug = true;

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void InitializeLogger()
    {
    pLogger = NativeLogging::LoggingManager::GetLogger ( L"DTMElement" );
    if (pLogger)
        {
        s_logInfo = pLogger->isSeverityEnabled (NativeLogging::LOG_INFO);
        s_logDebug = pLogger->isSeverityEnabled (NativeLogging::LOG_DEBUG);
        };
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void LogInfo(const wchar_t* message)
    {
    if (!s_isInitialized)
        InitializeLogger();

    if (s_logInfo)
        pLogger->message (NativeLogging::LOG_INFO, message);
#ifdef USEOUTPUTDEBUGSTRING
    OutputDebugStringW(message);
    OutputDebugStringW (L"\n");
#endif
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void LogInfoV(const wchar_t* message ...)
    {
    if (!s_isInitialized)
        InitializeLogger();

    if (s_logInfo)
        {
        va_list va;
        va_start(va, message);
        WString b;

        pLogger->messageva (NativeLogging::LOG_INFO, message, va);
        va_end(va);
        }
#ifdef USEOUTPUTDEBUGSTRING
    va_list va;
    va_start(va, message);
    WString b;

    WString::VSprintf (b, message, va);
    OutputDebugStringW (b.GetWCharCP());
    OutputDebugStringW (L"\n");
    va_end(va);
#endif
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void LogDebug(const wchar_t* message)
    {
    if (!s_isInitialized)
        InitializeLogger();

    if (s_logDebug)
        pLogger->message (NativeLogging::LOG_DEBUG, message);
#ifdef USEOUTPUTDEBUGSTRING
    OutputDebugStringW (message);
    OutputDebugStringW (L"\n");
#endif
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 04/10
//=======================================================================================
void LogDebugV(const wchar_t* message ...)
    {
    if (!s_isInitialized)
        InitializeLogger();

    if (s_logDebug)
        {
        va_list va;
        va_start(va, message);
        pLogger->messageva (NativeLogging::LOG_DEBUG, message, va);
        va_end(va);
        }
#ifdef USEOUTPUTDEBUGSTRING
    va_list va;
    va_start(va, message);
    WString b;

    WString::VSprintf (b, message, va);
    OutputDebugStringW (b.GetWCharCP());
    OutputDebugStringW (L"\n");
    va_end(va);
#endif
    }

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
#endif

