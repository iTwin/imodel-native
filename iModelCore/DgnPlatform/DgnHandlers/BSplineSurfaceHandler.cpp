/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/BSplineSurfaceHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define MAX_CLIPBATCH       200 // MAX_VERTICES    /* 121 */

/*=================================================================================**//**
* @bsiclass                                                     RayBentley  08/06
+===============+===============+===============+===============+===============+======*/
struct TrimCurveBoundaryHeader
{
UInt32          m_version;
UInt32          m_checksum;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
TrimCurveBoundaryHeader (BsurfBoundary const* pBoundary)
    {
    m_version = 0;
    m_checksum = ComputeChecksum (pBoundary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32   ComputeChecksum (BsurfBoundary const* pBoundary)
    {
    int         checksum = 0;

    for (UInt32 *pValue = (UInt32 *) pBoundary->points, *pEnd = (UInt32 *) (pBoundary->points + pBoundary->numPoints); pValue < pEnd; pValue++)
        checksum += *pValue;

    return checksum;
    }

}; // TrimCurveBoundaryHeader

/*=================================================================================**//**
* @bsiclass                                                     RayBentley  08/06
+===============+===============+===============+===============+===============+======*/
struct TrimCurveBuffer
{
UInt32          m_bytesToFollow;
struct
    {
    UInt32      m_order:8;
    UInt32      m_rational:1;
    UInt32      m_nonUniformKnots:1;
    UInt32      m_closed:1;
    UInt32      m_reserved:21;
    } m_params;
UInt32          m_numPoles;
UInt32          m_reserved;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32   BufferSize (TrimCurve const*    pTrim)
    {
    UInt32  bufferSize = sizeof (TrimCurveBuffer);
    if (0 != pTrim->curve.params.numKnots)
        bufferSize += sizeof(double) * (pTrim->curve.params.numPoles + pTrim->curve.params.order);

    if (pTrim->curve.rational)
        bufferSize += sizeof (double) * pTrim->curve.params.numPoles;

    bufferSize += sizeof (DPoint2d) * pTrim->curve.params.numPoles;

    return bufferSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void     SaveTrimCurveToBuffer (byte** ppBuffer, TrimCurve const*    pTrim)
    {
    TrimCurveBuffer*        pTrimCurveBuffer = (TrimCurveBuffer *) *ppBuffer;

    memset (pTrimCurveBuffer, 0, sizeof (*pTrimCurveBuffer));
    pTrimCurveBuffer->m_bytesToFollow = BufferSize (pTrim);
    pTrimCurveBuffer->m_numPoles = pTrim->curve.params.numPoles;
    pTrimCurveBuffer->m_params.m_order = pTrim->curve.params.order;
    pTrimCurveBuffer->m_params.m_rational = pTrim->curve.rational;
    pTrimCurveBuffer->m_params.m_closed   = pTrim->curve.params.closed;
    pTrimCurveBuffer->m_params.m_nonUniformKnots = (0 != pTrim->curve.params.numKnots);

    double*         pDoubles = (double*) (*ppBuffer + sizeof (TrimCurveBuffer));

    if (pTrimCurveBuffer->m_params.m_nonUniformKnots)
        {
        int         nKnots = pTrim->curve.params.numPoles + pTrim->curve.params.order;
        memcpy (pDoubles, pTrim->curve.GetKnotCP (), nKnots * sizeof (double));
        pDoubles += nKnots;
        }
    if (pTrim->curve.rational)
        {
        memcpy (pDoubles, pTrim->curve.GetWeightCP (), pTrim->curve.params.numPoles * sizeof (double));
        pDoubles += pTrim->curve.params.numPoles;
        }
    for (int i=0; i < pTrim->curve.params.numPoles; i++)
        {
        DPoint3d xyz = pTrim->curve.GetPole (i);
        memcpy (pDoubles, &xyz, sizeof (DPoint2d));
        pDoubles += 2;
        }

    (*ppBuffer) += pTrimCurveBuffer->m_bytesToFollow;
    }

}; // TrimCurveBuffer

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt saveTrimBoundaryLoopToXAttributes
(
MSElementDescrP     pDescr,
BsurfBoundary*      pBoundary
)
    {
    int                     bufferSize = sizeof (TrimCurveBoundaryHeader);
    byte*                   pBuffer;
    TrimCurve*              pTrim;

    for (pTrim = pBoundary->pFirst; NULL != pTrim; pTrim = pTrim->pNext)
        bufferSize += TrimCurveBuffer::BufferSize (pTrim);

    if (NULL == (pBuffer = (byte *) memutil_malloc (bufferSize, 'TrmC')))
        return DGNMODEL_STATUS_OutOfMemory;

    TrimCurveBoundaryHeader     header (pBoundary);

    memcpy (pBuffer, &header, sizeof (header));

    byte*           pTrimBuffer = pBuffer + sizeof (TrimCurveBoundaryHeader);
    for (pTrim = pBoundary->pFirst; NULL != pTrim; pTrim = pTrim->pNext)
        TrimCurveBuffer::SaveTrimCurveToBuffer (&pTrimBuffer, pTrim);

    return EditElementHandle (pDescr, false).ScheduleWriteXAttribute (XAttributeHandlerId (XATTRIBUTEID_BSurfTrimCurve, 0x00), 0, bufferSize, pBuffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        08/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt extractBoundaryChecksum (UInt32* pChecksum, ElementHandleP elHandle)
    {
    ElementHandle::XAttributeIter  xAttr = ElementHandle::XAttributeIter (*elHandle, XAttributeHandlerId (XATTRIBUTEID_BSurfTrimCurve, 0x00));

    if (!xAttr.IsValid ())
        return ERROR;

    byte const* pBuffer = (byte const *) xAttr.PeekData ();

    TrimCurveBoundaryHeader *pHeader = (TrimCurveBoundaryHeader *) pBuffer;

    *pChecksum = pHeader->m_checksum;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt extractTrimCurve (BsurfBoundary* pBoundary, ElementHandleP elHandle)
    {
    ElementHandle::XAttributeIter  xAttr = ElementHandle::XAttributeIter (*elHandle, XAttributeHandlerId (XATTRIBUTEID_BSurfTrimCurve, 0x00));

    if (!xAttr.IsValid ())
        return ERROR;

    byte const* pBuffer = (byte const *) xAttr.PeekData ();
    byte const* pBufferEnd = pBuffer + xAttr.GetSize();
    TrimCurve*  pLastTrim = NULL;

    for (pBuffer += sizeof (TrimCurveBoundaryHeader); pBuffer < pBufferEnd; )
        {
        TrimCurveBuffer const*  pTrimCurveBuffer = (TrimCurveBuffer*) pBuffer;

        TrimCurve*              pTrim = (TrimCurve *) BSIBaseGeom::Calloc (1, sizeof (TrimCurve));
        int numPoles = pTrimCurveBuffer->m_numPoles;
        if (SUCCESS != pTrim->curve.Allocate (numPoles,
                    pTrimCurveBuffer->m_params.m_order,
                    pTrimCurveBuffer->m_params.m_closed,
                    pTrimCurveBuffer->m_params.m_rational
                    ))
            return ERROR;

        int numKnots = (size_t)pTrim->curve.NumberAllocatedKnots ();
        int numDoubles = 2 * numPoles;  // always xy data ...
        if (pTrimCurveBuffer->m_params.m_nonUniformKnots)
            numDoubles += numKnots;
        if (pTrim->curve.rational)
            numDoubles += numPoles;
            
        assert (numDoubles * sizeof (double) < pTrimCurveBuffer->m_bytesToFollow);  // Really must be less than -- have not considered param data
        
        double*     pDoubles = (double *) (pBuffer + sizeof (TrimCurveBuffer));

         if (pTrimCurveBuffer->m_params.m_nonUniformKnots)
            {
            pTrim->curve.SetKnots (0, pDoubles, (size_t)numKnots);
            pDoubles += numKnots;
            }
        else
            {
            if (!pTrim->curve.ComputeUniformKnots ())
                return ERROR;
            }

        if (pTrim->curve.rational)
            {
            pTrim->curve.SetWeights (0, pDoubles, (size_t)numPoles);
            pDoubles += numPoles;
            }

        for (size_t i=0; i < (size_t)numPoles; i++)
            {
            pTrim->curve.SetPole (i, pDoubles[0], pDoubles[1], 0.0);
            pDoubles += 2;
            }

        if (NULL == pBoundary->pFirst)
            {
            pBoundary->pFirst = pLastTrim = pTrim;
            }
        else
            {
            pTrim->pPrevious = pLastTrim;
            if(pLastTrim)
                pLastTrim->pNext = pTrim;
            pLastTrim = pTrim;
            }

        pBuffer += pTrimCurveBuffer->m_bytesToFollow;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BSplineStatus   BSplineSurfaceHandler::IsValidSurface (MSBsplineSurfaceCR surface)
    {
    if (!surface.HasValidPoleAllocation ())
        return BSPLINE_STATUS_NoPoles;

    if (!surface.HasValidWeightAllocation ())
        return BSPLINE_STATUS_NoWeights;

    if (!surface.HasValidBoundaryAllocation ())
        return BSPLINE_STATUS_NoBounds;

    if (!surface.HasValidKnotAllocation ())
        return BSPLINE_STATUS_NoKnots;

    if (!surface.HasValidOrder ())
        return BSPLINE_STATUS_BadOrder;

    if (!surface.HasValidPoleCounts ())
        return BSPLINE_STATUS_TooFewPoles;

    return BSPLINE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    createSurfaceKnots
(
EditElementHandleR     eeh,
MSBsplineSurfaceCR  surface
)
    {
    int         numKnots = surface.uParams.numKnots + surface.vParams.numKnots;

    if (!numKnots)
        return true;

    int         elmSize = sizeof (Bspline_knot) + (numKnots - 1) * sizeof (double);

    if (elmSize/2 > MAX_V8_ELEMENT_SIZE)
        return false;

    DgnV8ElementBlank   out;
    DgnElementCP hdr = eeh.GetElementCP ();

    hdr->CopyTo (out);
    ElementUtil::SetRequiredFields (out, BSPLINE_KNOT_ELM, LevelId(hdr->GetLevel()), false, ElementUtil::ELEMDIM_NA);

    if (surface.uParams.numKnots)
        memcpy (out.ToBspline_knotR().knots, surface.uKnots + surface.uParams.order, surface.uParams.numKnots * sizeof (double));

    if (surface.vParams.numKnots)
        memcpy (&out.ToBspline_knotR().knots[surface.uParams.numKnots], surface.vKnots + surface.vParams.order, surface.vParams.numKnots * sizeof (double));

    out.SetSizeWordsNoAttributes(elmSize/2);
    eeh.GetElementDescrP()->AddComponent(* new MSElementDescr(out, *eeh.GetDgnModelP()));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    createSurfacePolesAndWeights
(
EditElementHandleR     eeh,
MSBsplineSurfaceCR  surface
)
    {
    int         i, numPoles = surface.uParams.numPoles * surface.vParams.numPoles;
    double      maxWeight = 0.0;

    if (surface.rational)
        {
        for (i=0; i < numPoles; i++)
            {
            double  absWeight = fabs (surface.weights[i]);

            if (absWeight > maxWeight)
                maxWeight = absWeight;
            }

        bsputil_unWeightPoles (surface.poles, surface.poles, surface.weights, numPoles);
        }

    DgnElementCP hdr = eeh.GetElementCP ();

    for (i=0; i < surface.vParams.numPoles; i++)
        {
        int         elmSize;
        DgnV8ElementBlank   out;

        elmSize = sizeof (Line_String_3d) + (surface.uParams.numPoles-1) * sizeof (DPoint3d);
        hdr->CopyTo (out);
        ElementUtil::SetRequiredFields (out, BSPLINE_POLE_ELM, LevelId(hdr->GetLevel()), false, ElementUtil::ELEMDIM_3d);

        out.ToLine_String_2dR().numverts = surface.uParams.numPoles;
        ElementUtil::PackLineWords3d (out.ToLine_String_2dR().vertice, surface.poles + i*surface.uParams.numPoles, surface.uParams.numPoles);

        out.SetSizeWordsNoAttributes(elmSize/2);
        DisplayHandler::ValidateElementRange (&out, eeh.GetDgnModelP());
        eeh.GetElementDescrP()->AddComponent(*new MSElementDescr(out, *eeh.GetDgnModelP()));

        if (surface.rational)
            {
            double  scale = (maxWeight >= 1.0 + 1.0e-15 ? maxWeight : 1.0);
            double* weights = surface.weights + i*surface.uParams.numPoles;

            elmSize = sizeof (Bspline_weight) + (surface.uParams.numPoles - 1) * sizeof (double);
            hdr->CopyTo (out);
            ElementUtil::SetRequiredFields (out, BSPLINE_WEIGHT_ELM, LevelId(hdr->GetLevel()), false, ElementUtil::ELEMDIM_NA);

            for (int iWt=0; iWt < surface.uParams.numPoles; iWt++)
                out.ToBspline_weightR().weights[iWt] = weights[iWt] / scale;

            out.SetSizeWordsNoAttributes(elmSize/2);
            eeh.GetElementDescrP()->AddComponent(*MSElementDescr::Allocate (out, *eeh.GetDgnModelP()).get());
            }
        }

    if (surface.rational)
        bsputil_weightPoles (surface.poles, surface.poles, surface.weights, numPoles);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    createSurfaceBoundaries
(
EditElementHandleR     eeh,
MSBsplineSurfaceCR  surface
)
    {
    for (int i=0; i < surface.GetIntNumBounds (); i++)
        {
        int             numPnts = 0;
        BsurfBoundary   bnd = surface.boundaries[i];
        bool            trimCurveSaved = false;

        do
            {
            bnd.points += numPnts;
            numPnts = bnd.numPoints < MAX_BNDRY_PTS ? bnd.numPoints : MAX_BNDRY_PTS;

            DgnV8ElementBlank   out;
            DgnElementCP hdr = eeh.GetElementCP ();

            hdr->CopyTo (out);
            ElementUtil::SetRequiredFields (out, BSURF_BOUNDARY_ELM, LevelId(hdr->GetLevel()), false, ElementUtil::ELEMDIM_NA);

            memcpy (out.ToBsurf_boundaryR().vertices, bnd.points, numPnts * sizeof (out.ToBsurf_boundary().vertices[0]));

            out.ToBsurf_boundaryR().number   = i+1;
            out.ToBsurf_boundaryR().numverts = numPnts;

            int         elmSize = sizeof (Bsurf_boundary) + (numPnts - 1) * sizeof (DPoint2d);

            out.SetSizeWordsNoAttributes(elmSize/2);

            MSElementDescrPtr boundEdP = new MSElementDescr(out, *eeh.GetDgnModelP());

            if (NULL != bnd.pFirst && !trimCurveSaved)
                {
                saveTrimBoundaryLoopToXAttributes (boundEdP.get(), &surface.boundaries[i]);
                trimCurveSaved = true;
                }

            eeh.GetElementDescrP()->AddComponent(*boundEdP.get());

            bnd.numPoints -= numPnts;

            } while (bnd.numPoints > 0);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void    createSurfaceHeader
(
EditElementHandleR     eeh,
DgnElementCP         in,                     //  => template element (or NULL)
MSBsplineSurfaceCR  surface,
DgnModelR        modelRef
)
    {
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, BSPLINE_SURFACE_ELM, LevelId(in->GetLevel()), false, ElementUtil::ELEMDIM_3d);
        }
    else
        {
        memset (&out, 0, sizeof (Bspline_surface));
        ElementUtil::SetRequiredFields (out, BSPLINE_SURFACE_ELM, LEVEL_DEFAULT_LEVEL_ID, false, ElementUtil::ELEMDIM_3d);
        }

    int         elmSize = sizeof (Bspline_surface);

    out.SetSizeWordsNoAttributes(elmSize/2);
    ElementUtil::CopyAttributes (&out, in);

    // Set up the curve flags
    memset (&out.ToBspline_surfaceR().flags, 0, sizeof (out.ToBspline_surface().flags));

    out.ToBspline_surfaceR().flags.order         = surface.uParams.order - 2;
    out.ToBspline_surfaceR().flags.curve_display = surface.display.curveDisplay != 0;
    out.ToBspline_surfaceR().flags.poly_display  = surface.display.polygonDisplay != 0;
    out.ToBspline_surfaceR().flags.rational      = surface.rational != 0;
    out.ToBspline_surfaceR().flags.closed        = surface.uParams.closed != 0;

    if (surface.type >= 0 && surface.type <= 8)
        out.ToBspline_surfaceR().flags.curve_type = surface.type;

    // Set up the surface flags
    out.ToBspline_surfaceR().bsurf_flags.v_order     = surface.vParams.order - 2 ;
    out.ToBspline_surfaceR().bsurf_flags.arcSpacing  = surface.display.rulesByLength;
    out.ToBspline_surfaceR().bsurf_flags.v_closed    = surface.vParams.closed;

    // Number of poles, etc.
    out.ToBspline_surfaceR().num_poles_u     = surface.uParams.numPoles;
    out.ToBspline_surfaceR().num_knots_u     = surface.uParams.numKnots;
    out.ToBspline_surfaceR().rule_lines_u    = surface.uParams.numRules;

    out.ToBspline_surfaceR().num_poles_v     = surface.vParams.numPoles;
    out.ToBspline_surfaceR().num_knots_v     = surface.vParams.numKnots;
    out.ToBspline_surfaceR().rule_lines_v    = surface.vParams.numRules;

    out.ToBspline_surfaceR().num_bounds      = surface.GetIntNumBounds ();
    out.SetIsHole(TO_BOOL(surface.holeOrigin));

    eeh.SetElementDescr(new MSElementDescr(out, modelRef), false);
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BSplineStatus   createSurfaceElement 
(
EditElementHandleR     eeh,
DgnElementCP         templateEl,
MSBsplineSurfaceCR  surface,
DgnModelR        modelRef
)
    {
    createSurfaceHeader (eeh, templateEl, surface, modelRef);

    if (!createSurfaceKnots (eeh, surface))
        return BSPLINE_STATUS_BadKnots;

    if (!createSurfaceBoundaries (eeh, surface))
        return BSPLINE_STATUS_NoBounds;

    if (!createSurfacePolesAndWeights (eeh, surface))
        return BSPLINE_STATUS_BadPoles;

    return (SUCCESS == eeh.GetDisplayHandler()->ValidateElementRange(eeh)) ? BSPLINE_STATUS_Success : BSPLINE_STATUS_BadBspElement;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BSplineStatus   createSurfaceElementAllowUVSwap 
(
EditElementHandleR     eeh,
ElementHandleCP        templateEh,
MSBsplineSurfaceCR  surface,
DgnModelR        modelRef
)
    {
    BSplineStatus   status = BSPLINE_STATUS_BadBspElement;

    if (surface.vParams.numKnots > MAX_VERTICES /*MAX_BSKNOTS*/ - surface.uParams.numKnots)
        {
        MSBsplineSurface    tmpSurf;

        if (SUCCESS != tmpSurf.CopyFrom (surface))
            return status;

        if (SUCCESS == tmpSurf.SwapUV ())
            status = createSurfaceElement (eeh, templateEh ? templateEh->GetElementCP () : NULL, tmpSurf, modelRef);
        
        tmpSurf.ReleaseMem ();
        }
    else
        {
        status = createSurfaceElement (eeh, templateEh ? templateEh->GetElementCP () : NULL, surface, modelRef);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             09/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     segmentSurface
(
MSBsplineSurface    *segment,
MSBsplineSurface    *surface,
DPoint2d            *initial,
DPoint2d            *final
)
    {
    return bspsurf_segmentSurface (segment, surface, initial, final);
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static BSplineStatus   createCellElement
(
EditElementHandleR     eeh,
ElementHandleCP        templateEh,
MSBsplineSurfaceCR  surface,
DgnModelR        modelRef
)
    {
    MSBsplineSurface    tmpSurface;

    if (SUCCESS != tmpSurface.CopyFrom (surface))
        return BSPLINE_STATUS_BadBspElement;

    NormalCellHeaderHandler::CreateOrphanCellElement (eeh, NULL, true, modelRef);

    BSplineStatus   status = BSPLINE_STATUS_BadBspElement;
    DPoint2d        origin, finish;
    EditElementHandle  surfaceEeh;

    origin.zero ();
    finish.setComponents (1.0, 1.0);

    do
        {
        DPoint2d            breakPoint1, breakPoint2;
        MSBsplineSurface    patch;

        breakPoint1.x = tmpSurface.uKnots[MAX_POLES];
        breakPoint1.y = 1.0;

        breakPoint2.x = tmpSurface.uKnots[MAX_POLES];
        breakPoint2.y = 0.0;

        memset (&patch, 0, sizeof (patch));

        if (SUCCESS != segmentSurface (&patch, &tmpSurface, &origin, &breakPoint1) ||
            SUCCESS != segmentSurface (&tmpSurface, &tmpSurface, &breakPoint2, &finish) ||
            BSPLINE_STATUS_Success != (status = createSurfaceElementAllowUVSwap (surfaceEeh, templateEh, patch, modelRef)))
            {
            patch.ReleaseMem ();
            tmpSurface.ReleaseMem ();

            return status;
            }
        NormalCellHeaderHandler::AddChildElement (eeh, surfaceEeh);

        patch.ReleaseMem();
        } while (tmpSurface.uParams.numPoles > MAX_POLES);

    status = createSurfaceElementAllowUVSwap (surfaceEeh, templateEh, tmpSurface, modelRef);

    tmpSurface.ReleaseMem ();

    if (BSPLINE_STATUS_Success != status)
        return status;

    NormalCellHeaderHandler::AddChildElement (eeh, surfaceEeh);
    NormalCellHeaderHandler::AddChildComplete (eeh);

    return (SUCCESS == eeh.GetDisplayHandler()->ValidateElementRange(eeh)) ? BSPLINE_STATUS_Success : BSPLINE_STATUS_BadBspElement;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                                      Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BSplineStatus   BSplineSurfaceHandler::CreateBSplineSurfaceElement
(
EditElementHandleR     eeh,
ElementHandleCP        templateEh,
MSBsplineSurfaceCR  surface,
DgnModelR        modelRef
)
    {
    BSplineStatus   status;

    if (BSPLINE_STATUS_Success != (status = BSplineSurfaceHandler::IsValidSurface (surface)))
        return status;


    if (!surface.AreUKnotsValid (false)
        || !surface.AreVKnotsValid (false))
        {
        MSBsplineSurface surface1;
        surface1.CopyFrom (surface);
        // CR #123496: guard against programmer filling in knots field incorrectly, or not at all
        // Remark (EDL) It just passed a validity test.  What is this valid but invalid thing?
        if (!surface1.AreUKnotsValid (false))
            surface1.ComputeUniformUKnots ();

        if (!surface1.AreVKnotsValid (false))
            surface1.ComputeUniformVKnots ();
        
        return CreateBSplineSurfaceElement (eeh, templateEh, surface1, modelRef);
        }

    if (surface.GetNumUPoles () <= MAX_POLES)
        {
        if (BSPLINE_STATUS_Success != (status = createSurfaceElementAllowUVSwap (eeh, templateEh, surface, modelRef)))
            return status;
        }
    else
        {
        if (BSPLINE_STATUS_Success != (status = createCellElement (eeh, templateEh, surface, modelRef)))
            return status;
        }

    return (eeh.IsValid () ? BSPLINE_STATUS_Success : BSPLINE_STATUS_BadBspElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void    extractPoleDataTo3d
(
DPoint3d        *outPolesP,
const DPoint2d  *inPolesP,
int             numPoles,
bool            is3d
)
    {
    if (is3d)
        {
        memcpy (outPolesP, inPolesP, numPoles * sizeof (DPoint3d));
        }
    else
        {
        for (int i=0; i < numPoles; i++, inPolesP++, outPolesP++)
            outPolesP->init (inPolesP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     checkVClosedForRepeatedPole
(
BsplineParam*   uTmpParamsP,
BsplineParam*   vTmpParamsP,
DgnElementCP     headerP,        // => bcurve elm
ElementHandleCP eh
)
    {
    ChildElemIter childEh (*eh, ExposeChildrenReason::Count);

    if (vTmpParamsP->numKnots)
        {
        vTmpParamsP->numPoles = vTmpParamsP->numKnots + 1;
        }
    else if (vTmpParamsP->order != vTmpParamsP->numPoles)
        {
        // closed non-Bezier surfaces with uniform knots have highest order continuity at the closure, but redundant end poles ruin this
        ChildElemIter   tmpIter = childEh.ToNext ();
        ChildElemIter firstPole, lastPole;
        ChildElemIter firstWeight, lastWeight;
        // Find the first and last pole and weight elements ...

        for (ChildElemIter child (*eh, ExposeChildrenReason::Count); child.IsValid(); child = child.ToNext ())
            {
            DgnElementCP     elmP = child.GetElementCP();
            if (elmP->GetLegacyType() == BSPLINE_POLE_ELM)
                {
                lastPole = child;
                if (!firstPole.IsValid ())
                    firstPole = child;
                }
            else if (elmP->GetLegacyType() == BSPLINE_WEIGHT_ELM)
                {
                lastWeight = child;
                if (!firstWeight.IsValid ())
                    firstWeight = child;
                }
            }


        if (firstPole.IsValid () && lastPole.IsValid ())
            {
            DgnElementCP     firstPoleElmP = firstPole.GetElementCP();
            DgnElementCP     lastPoleElmP = lastPole.GetElementCP();
            DPoint3d *p0 = (DPoint3d *) firstPoleElmP->ToBspline_pole_3d().poles;
            DPoint3d *p1 = (DPoint3d *) lastPoleElmP->ToBspline_pole_3d().poles;
            for (int i=0; i < uTmpParamsP->numPoles; i++)
                if (!bsiDPoint3d_pointEqual (&p0[i], &p1[i]))
                    return;

            if (headerP->ToBspline_surface().flags.rational && firstWeight.IsValid () && lastWeight.IsValid ())
                {
                DgnElementCP     firstWeightElmP = firstWeight.GetElementCP();
                DgnElementCP     lastWeightElmP = lastWeight.GetElementCP();
                double *w0 = (double *) firstWeightElmP->ToBspline_weight().weights;
                double *w1 = (double *) lastWeightElmP->ToBspline_weight().weights;
                for (int i = 0; i < uTmpParamsP->numPoles; i++)
                    if (w0[i] != w1[i])
                        return;
                }

            // Same weights, same poles.   Drop the final ones.  
            vTmpParamsP->numPoles--;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void     checkUClosedForRepeatedPole
(
BsplineParam*   uTmpParamsP,
BsplineParam*   vTmpParamsP,
DgnElementCP     headerP,        // => bcurve elm
ElementHandleCP eh
)
    {
    

    if (uTmpParamsP->numKnots)
        {
        uTmpParamsP->numPoles = uTmpParamsP->numKnots + 1;
        }
    else if (uTmpParamsP->order != uTmpParamsP->numPoles)
        {
        // closed non-Bezier surfaces with uniform knots have highest order continuity at the closure, but redundant end poles ruin this
        int         offset = uTmpParamsP->numPoles - 1;
        int         numPoleElements = 0;
        int         numWeightElements = 0;

        for (ChildElemIter child (*eh, ExposeChildrenReason::Count); child.IsValid(); child = child.ToNext ())
            {
            DgnElementCP     elmP = child.GetElementCP();

            switch (elmP->GetLegacyType())
                {
                case BSPLINE_POLE_ELM:
                    if (!bsiDPoint3d_pointEqual (elmP->ToBspline_pole_3d().poles, elmP->ToBspline_pole_3d().poles + offset))
                        return;
                    numPoleElements++;
                    break;

                case BSPLINE_WEIGHT_ELM:
                    if (elmP->ToBspline_weight().weights[0] != elmP->ToBspline_weight().weights[offset])
                        return;
                    numWeightElements++;
                    break;
                }
            }

        if (numPoleElements > 0 && (numWeightElements == 0 || numWeightElements == numPoleElements))
            uTmpParamsP->numPoles--;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        08/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static void    checkBoundaries
(
BsurfBoundary   *bound,
UInt32          checksum
)
    {
    UInt32      sum = 0;

    for (UInt32 *pValue = (UInt32 *) bound->points, *pEnd = (UInt32 *) (bound->points + bound->numPoints); pValue < pEnd; pValue++)
        sum += *pValue;

    if (sum != checksum && bound->pFirst)
        {
        bspTrimCurve_freeList (&bound->pFirst);
        bound->pFirst = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    processSurfaceComponent
(
BsplineParam    *uTmpParamsP,           /* <= number of poles etc. in U */
BsplineParam    *vTmpParamsP,           /* <= number of poles etc. in V */
DgnElement       *headerP,               /* <= Header element */
DPoint3d        **polesPP,              /* <= pole coordinates */
double          **uKnotsPP,             /* <= U knots (if nonuniform) */
double          **vKnotsPP,             /* <= V knots (if nonuniform) */
double          **weightsPP,            /* <= weights (if (rational) */
BsurfBoundary   **boundsPP,             /* <= boundary definitions */
int             *poleCounterP,
bool            *knotsFoundP,
int             *weightCounterP,
bool            *boundsFoundP,
ChildElemIter   &childIter              /* => Input element descriptor */
)
    {
    DgnElementCP     elP = childIter.GetElementCP();

    switch (elP->GetLegacyType())
        {
        case BSPLINE_POLE_ELM:
            {
            if (polesPP && *polesPP && *poleCounterP < vTmpParamsP->numPoles)
                extractPoleDataTo3d (*polesPP + uTmpParamsP->numPoles * (*poleCounterP)++, elP->ToBspline_pole_2d().poles, uTmpParamsP->numPoles, elP->Is3d());

            childIter = childIter.ToNext();
            break;
            }

        case BSPLINE_KNOT_ELM:
            {
            *knotsFoundP = true;

            if (uKnotsPP && *uKnotsPP)
                {
                double      tmpUKnots[MAX_KNOTS];

                bsputil_extractScaledValues (tmpUKnots, (double *) elP->ToBspline_knot().knots, uTmpParamsP->numKnots);

                bspknot_computeKnotVector (*uKnotsPP, uTmpParamsP, tmpUKnots);
                }

            if (vKnotsPP && *vKnotsPP)
                {
                double      tmpVKnots[MAX_KNOTS];

                bsputil_extractScaledValues (tmpVKnots, (double *) elP->ToBspline_knot().knots + uTmpParamsP->numKnots, vTmpParamsP->numKnots);

                bspknot_computeKnotVector (*vKnotsPP, vTmpParamsP, tmpVKnots);
                }

            childIter = childIter.ToNext();
            break;
            }

        case BSPLINE_WEIGHT_ELM:
            {
            if (weightsPP && *weightsPP && *weightCounterP < vTmpParamsP->numPoles)
                bspconv_extractWeightValues (*weightsPP + uTmpParamsP->numPoles * (*weightCounterP)++, (double *) elP->ToBspline_weight().weights, uTmpParamsP->numPoles);

            childIter = childIter.ToNext();
            break;
            }

        case BSURF_BOUNDARY_ELM:
            {
            *boundsFoundP = true;

            if (boundsPP && *boundsPP)
                {
                BsurfBoundary       *bbP;
                int                 boundIndex = elP->ToBsurf_boundary().number;
                UInt32              checksum = 0;

                do
                    {
                    bbP = *boundsPP + elP->ToBsurf_boundary().number - 1;

                    int allocSize = (bbP->numPoints + elP->ToBsurf_boundary().numverts) * sizeof (DPoint2d);

                    if (!bbP->points)
                        {
//                        if (NULL == (bbP->points = (DPoint2d *) dlmSystem_mdlMallocWithDescr (allocSize, (mdlDesc *) heapDescrP)))
                        if (NULL == (bbP->points = (DPoint2d *) BSIBaseGeom::Malloc (allocSize)))
                            return DGNMODEL_STATUS_OutOfMemory;
                        }
                    else
                        {
//                        if (NULL == (bbP->points = (DPoint2d *) dlmSystem_mdlReallocWithDescr (bbP->points, allocSize, (mdlDesc *) heapDescrP)))
                        if (NULL == (bbP->points = (DPoint2d *) BSIBaseGeom::Realloc (bbP->points, allocSize)))
                            return DGNMODEL_STATUS_OutOfMemory;
                        }

                    bsputil_extractScaledValues ((double *) (bbP->points + bbP->numPoints), (double *) elP->ToBsurf_boundary().vertices, elP->ToBsurf_boundary().numverts * 2);
                    bbP->numPoints += elP->ToBsurf_boundary().numverts;

                    extractTrimCurve (bbP, &childIter);

                    UInt32  sum;

                    if (SUCCESS == extractBoundaryChecksum (&sum, &childIter))
                        checksum = sum;

                    childIter = childIter.ToNext();
                    elP = childIter.GetElementCP();

                    if (elP->GetLegacyType() != BSURF_BOUNDARY_ELM || elP->ToBsurf_boundary().number != boundIndex)
                        {
                        checkBoundaries (bbP, checksum);
                        break;
                        }

                    } while (childIter.IsValid());
                }
            //If we don't want to extract boundaries.
            else
                childIter = childIter.ToNext();
            break;
            }

        default:
            return BSPLINE_STATUS_BadBspElement;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    extractSurface
(
DgnElement       *header,                /* <= Header element */
int             *type,                  /* <= surface type */
int             *rational,              /* <= rational (weights included) */
BsplineDisplay  *display,               /* <= display parameters */
BsplineParam    *uParams,               /* <= number of poles etc. in U */
BsplineParam    *vParams,               /* <= number of poles etc. in V */
DPoint3d        **poles,                /* <= pole coordinates */
double          **uKnots,               /* <= U knots (if nonuniform) */
double          **vKnots,               /* <= V knots (if nonuniform) */
double          **weights,              /* <= weights (if (rational) */
int             *holeOrigin,            /* <= type of boundaries */
int             *numBounds,             /* <= number of boundaries */
BsurfBoundary   **bounds,               /* <= boundary definitions */
ElementHandleCP elHandle                /* => Input element descriptor */
)
    {
    StatusInt       status = SUCCESS;
    int             allocSize, poleCounter, weightCounter, totalPoles;
    bool            boundsFound, knotsFound;
    BsplineParam    uTmpParams, vTmpParams;
    DgnElementCP     headerP = elHandle->GetElementCP();

    if (BSPLINE_SURFACE_ELM != headerP->GetLegacyType())
        return BSPLINE_STATUS_NoBspHeader;

    /* Don't let the user get bounds without knowing its size */
    if (bounds && !numBounds)
        return BSPLINE_STATUS_NoNumBounds;

    ChildElemIter   childIter (*elHandle, ExposeChildrenReason::Count);

    if (!childIter.IsValid())
        return BSPLINE_STATUS_NoPoles;

    if (header)
        {
        allocSize = headerP->GetSizeWords() * 2;
        memcpy (header, headerP, allocSize);
        }

    if (type)
        *type = headerP->ToBspline_surface().flags.curve_type;

    if (rational)
        *rational = headerP->ToBspline_surface().flags.rational;

    if (numBounds)
        *numBounds = headerP->ToBspline_surface().num_bounds;

    if (holeOrigin)
        *holeOrigin = headerP->ToBspline_surface().IsHole();

    if (display)
        {
        display->curveDisplay   = headerP->ToBspline_surface().flags.curve_display;
        display->polygonDisplay = headerP->ToBspline_surface().flags.poly_display;
        display->rulesByLength  = headerP->ToBspline_surface().bsurf_flags.arcSpacing;
        }

    uTmpParams.order    = headerP->ToBspline_surface().flags.order + 2;
    uTmpParams.closed   = headerP->ToBspline_surface().flags.closed;
    uTmpParams.numPoles = headerP->ToBspline_surface().num_poles_u;
    uTmpParams.numKnots = headerP->ToBspline_surface().num_knots_u;
    uTmpParams.numRules = headerP->ToBspline_surface().rule_lines_u;

    vTmpParams.order    = headerP->ToBspline_surface().bsurf_flags.v_order + 2;
    vTmpParams.closed   = headerP->ToBspline_surface().bsurf_flags.v_closed;
    vTmpParams.numPoles = headerP->ToBspline_surface().num_poles_v;
    vTmpParams.numKnots = headerP->ToBspline_surface().num_knots_v;
    vTmpParams.numRules = headerP->ToBspline_surface().rule_lines_v;

    /* Check for repeated poles on closed surfaces */
    if (vTmpParams.closed)
        checkVClosedForRepeatedPole (&uTmpParams, &vTmpParams, headerP, elHandle);

    if (uTmpParams.closed)
        checkUClosedForRepeatedPole (&uTmpParams, &vTmpParams, headerP, elHandle);

    if (uParams)
        *uParams = uTmpParams;

    if (vParams)
        *vParams = vTmpParams;

    /* Allocate pole buffer */
    totalPoles = uTmpParams.numPoles * vTmpParams.numPoles;

    if (poles)
        {
        allocSize = totalPoles * sizeof (DPoint3d);

//        if (NULL == (*poles = (DPoint3d *) dlmSystem_mdlMallocWithDescr (allocSize, (mdlDesc *) pHeapDescr)))
        if (NULL == (*poles = (DPoint3d *) BSIBaseGeom::Malloc (allocSize)))
            return DGNMODEL_STATUS_OutOfMemory;
        }

    /* allocate the u knot buffer */
    if (uKnots)
        {
        allocSize = bspknot_numberKnots (uTmpParams.numPoles, uTmpParams.order, uTmpParams.closed) * sizeof (double);

//        if (NULL == (*uKnots = (double *) dlmSystem_mdlMallocWithDescr (allocSize, (mdlDesc *) pHeapDescr)))
        if (NULL == (*uKnots = (double *) BSIBaseGeom::Malloc (allocSize)))
            {
            status = DGNMODEL_STATUS_OutOfMemory;

            goto wrapup;
            }
        }

    /* allocate the v knot buffer */
    if (vKnots)
        {
        allocSize = bspknot_numberKnots (vTmpParams.numPoles, vTmpParams.order, vTmpParams.closed) * sizeof (double);

//        if (NULL == (*vKnots = (double *) dlmSystem_mdlMallocWithDescr (allocSize, (mdlDesc *) pHeapDescr)))
        if (NULL == (*vKnots = (double *) BSIBaseGeom::Malloc (allocSize)))
            {
            status = DGNMODEL_STATUS_OutOfMemory;

            goto wrapup;
            }
        }

    /* Allocate the weight buffer */
    if (weights)
        {
        if (headerP->ToBspline_curve().flags.rational)
            {
            allocSize = totalPoles * sizeof(double);

//            if (NULL == (*weights = (double *) dlmSystem_mdlMallocWithDescr (allocSize, (mdlDesc *) pHeapDescr)))
            if (NULL == (*weights = (double *) BSIBaseGeom::Malloc (allocSize)))
                {
                status = DGNMODEL_STATUS_OutOfMemory;

                goto wrapup;
                }
            }
        else
            {
            *weights = NULL;
            }
        }

    /* allocate the boundary buffer */
    if (bounds)
        {
        if (headerP->ToBspline_surface().num_bounds)
            {
//            if (NULL == (*bounds = (BsurfBoundary *) dlmSystem_mdlCallocWithDescr (headerP->ToBspline_surface().num_bounds, sizeof(BsurfBoundary), (mdlDesc *) pHeapDescr)))
            if (NULL == (*bounds = (BsurfBoundary *) BSIBaseGeom::Calloc (headerP->ToBspline_surface().num_bounds, sizeof(BsurfBoundary))))
                {
                status = DGNMODEL_STATUS_OutOfMemory;

                goto wrapup;
                }
            }
        else
            {
            *bounds = NULL;
            }
        }

    /* If no component data is required, return now */
    if (!poles && !uKnots && !vKnots && !weights && !bounds)
        return SUCCESS;

    poleCounter = weightCounter = 0;
    knotsFound = boundsFound = false;

    // step through all of the children of the shape (ignoring their symbology)
    do
        {
        if (SUCCESS != (status = processSurfaceComponent (&uTmpParams, &vTmpParams, (DgnElement *) headerP,
                                                          poles, uKnots, vKnots, weights, bounds,
                                                          &poleCounter, &knotsFound, &weightCounter, &boundsFound,
                                                          childIter)))
            goto wrapup;

        } while (childIter.IsValid());

    if (poleCounter < vTmpParams.numPoles)
        {
        status = BSPLINE_STATUS_BadPoles;

        goto wrapup;
        }

    if ((headerP->ToBspline_surface().flags.rational) && (weightCounter < vTmpParams.numPoles))
        {
        status = BSPLINE_STATUS_BadWeights;

        goto wrapup;
        }

    if ((uTmpParams.numKnots || vTmpParams.numKnots) && !knotsFound)
        {
        status = BSPLINE_STATUS_NoKnots;

        goto wrapup;
        }

    if (headerP->ToBspline_surface().num_bounds && !boundsFound)
        {
        status = BSPLINE_STATUS_NoBounds;

        goto wrapup;
        }

    // If we didn't find a knot element, still need to set output...
    if (!knotsFound)
        {
        if (uKnots && *uKnots)
            bspknot_computeKnotVector (*uKnots, &uTmpParams, NULL);

        if (vKnots && *vKnots)
            bspknot_computeKnotVector (*vKnots, &vTmpParams, NULL);
        }

wrapup:
    if (SUCCESS != status)
        {
        if (poles && *poles)
            {
//            dlmSystem_mdlFreeWithDescr (*poles, (mdlDesc *) pHeapDescr);
            BSIBaseGeom::Free (*poles);
            *poles = NULL;
            }

        if (uKnots && *uKnots)
            {
//            dlmSystem_mdlFreeWithDescr (*uKnots, (mdlDesc *) pHeapDescr);
            BSIBaseGeom::Free (*uKnots);
            *uKnots = NULL;
            }

        if (vKnots && *vKnots)
            {
//            dlmSystem_mdlFreeWithDescr (*vKnots, (mdlDesc *) pHeapDescr);
            BSIBaseGeom::Free (*vKnots);
            *vKnots = NULL;
            }

        if (weights && *weights)
            {
//            dlmSystem_mdlFreeWithDescr (*weights, (mdlDesc *) pHeapDescr);
            BSIBaseGeom::Free (*weights);
            *weights = NULL;
            }

        if (bounds && *bounds)
            {
            for (int i=0; i < *numBounds; i++)
                {
                if ((*bounds+i)->points)
//                    dlmSystem_mdlFreeWithDescr ((*bounds+i)->points, (mdlDesc *) pHeapDescr);
                    BSIBaseGeom::Free ((*bounds+i)->points);
                 }

//            dlmSystem_mdlFreeWithDescr (*bounds, (mdlDesc *) pHeapDescr);
            BSIBaseGeom::Free (*bounds);
            *bounds = NULL;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BSplineSurfaceHandler::SurfaceFromElement (MSBsplineSurfaceR surface, ElementHandleCR eh)
    {
    if (BSPLINE_SURFACE_ELM != eh.GetLegacyType())
        return ERROR;

    memset (&surface, 0, sizeof (surface));

    if (SUCCESS != extractSurface (NULL, (int *) &surface.type, (int *) &surface.rational,
                                   &surface.display, &surface.uParams, &surface.vParams,
                                   &surface.poles, &surface.uKnots, &surface.vKnots, &surface.weights,
                                   (int *) &surface.holeOrigin, (int *) &surface.numBounds, &surface.boundaries,
                                   &eh))
        return ERROR;

    mdlBspline_validateSurfaceKnots (surface.uKnots, surface.vKnots, surface.poles, surface.weights, &surface.uParams, &surface.vParams);

    if (surface.rational)
        bsputil_weightPoles (surface.poles, surface.poles, surface.weights, surface.uParams.numPoles * surface.vParams.numPoles);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BSplineSurfaceHandler::_GetBsplineSurface (ElementHandleCR source, MSBsplineSurfacePtr& surface)
    {
    MSBsplineSurfacePtr  tmpSurface = MSBsplineSurface::CreatePtr ();

    if (SUCCESS != SurfaceFromElement (*tmpSurface, source))
        return ERROR;

    surface = tmpSurface;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   BSplineSurfaceHandler::_SetBsplineSurface (EditElementHandleR eeh, MSBsplineSurfaceCR surface)
    {
    EditElementHandle  newEeh;

    // NOTE: In case eeh is component use ReplaceElementDescr, Create methods uses SetElementDescr...
    if (BSPLINE_STATUS_Success != BSplineSurfaceHandler::CreateBSplineSurfaceElement (newEeh, &eeh, surface, *eeh.GetDgnModelP ()))
        return ERROR;

    return (BentleyStatus) eeh.ReplaceElementDescr (newEeh.ExtractElementDescr().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplineSurfaceHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_BSPLINE_SURFACE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      BSplineSurfaceHandler::_OnSnap (SnapContextP context, int snapPathIndex)
    {
    if (!context->IsSnappableElement (snapPathIndex))
        return SnapStatus::NotSnappable;

    if (SnapMode::Origin == context->GetSnapMode ())
        {
        DPoint3d        hitPoint;
        SnapPathP       snap = context->GetSnapPath ();
        ElementHandle   elHandle (snap->GetPathElem (snapPathIndex));

        GetRangeCenter (elHandle, hitPoint);

        context->ElmLocalToWorld (hitPoint);
        context->SetSnapInfo (snapPathIndex, SnapMode::Origin, context->GetSnapSprite (SnapMode::Origin), hitPoint, true, false);

        return SnapStatus::Success;
        }

    return context->DoDefaultDisplayableSnap (snapPathIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplineSurfaceHandler::_GetOrientation (ElementHandleCR thisElm, RotMatrixR rMatrix)
    {
    MSBsplineSurface    surface;

    if (SUCCESS != BSplineSurfaceHandler::SurfaceFromElement (surface, thisElm))
        {
        T_Super::_GetOrientation (thisElm, rMatrix);

        return;
        }

    if (surface.rational)
        bsputil_unWeightPoles (surface.poles, surface.poles, surface.weights, surface.uParams.numPoles * surface.vParams.numPoles);

    DVec3d      xVector, yVector;

    xVector.differenceOf (&surface.poles[1], &surface.poles[0]);
    yVector.differenceOf (&surface.poles[surface.uParams.numPoles], &surface.poles[0]);

    rMatrix.initFrom2Vectors (&xVector, &yVector);
    rMatrix.squareAndNormalizeColumns (&rMatrix, 0, 1);

    surface.ReleaseMem ();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct StrokeAsSurface : IStrokeForCache
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache (CachedDrawHandleCR dh, ViewContextR context, double pixelSize = 0.0) override
    {
    MSBsplineSurface    surface;

    if (SUCCESS != BSplineSurfaceHandler::SurfaceFromElement (surface, *dh.GetElementHandleCP()))
        return;

    context.GetIDrawGeom().DrawBSplineSurface (surface);

    surface.ReleaseMem ();
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct strokeSurfaceCurvesInfo
    {
    ElementHandleCP     thisElm;
    ViewContextP        context;
    MSBsplineSurfaceP   surfaceP;

    } StrokeSurfaceCurvesInfo;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static int drawSurfaceCurvePolygonCallback
(
void                    *userArgP,
DPoint3d                *pointsP,
int                     nPoints,
const MSBsplineSurface  *surfaceP,
double                  u0,
double                  u1,
double                  v0,
double                  v1
)
    {
    StrokeSurfaceCurvesInfo *infoP = (StrokeSurfaceCurvesInfo *) userArgP;
    ViewContextP   context = infoP->context;

    ElemDisplayParamsP currDisplayParams = context->GetCurrentDisplayParams();

    // Display dotted style control polygon...
    currDisplayParams->SetWeight (0);
    currDisplayParams->SetLineStyle (2);
    context->CookDisplayParams();

    context->GetIDrawGeom().DrawLineString3d (nPoints, pointsP, NULL);

    // Don't need to draw this for both directions...dupicate points...
    if (u0 != u1)
        return SUCCESS;

    // Display fat dots for poles...
    currDisplayParams->SetWeight (5);
    currDisplayParams->SetLineStyle (0);
    context->CookDisplayParams();

    context->GetIDrawGeom().DrawPointString3d (nPoints, pointsP, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawBSurfacePolygon (ElementHandleCR thisElm, ViewContextR context)
    {
    switch (context.GetDrawPurpose ())
        {
        case DrawPurpose::Pick:
        case DrawPurpose::FenceAccept:
            break; // Want for locate...

        case DrawPurpose::FitView:
            {
            if (!thisElm.GetElementCP ()->ToBspline_surface().flags.curve_display)
                break; // Want if only poly is displayed...

            return;
            }

        default:
            {
            if (context.GetIViewDraw().IsOutputQuickVision ())
                break; // Want for display/plot...

            return;
            }
        }

    // host may override polygon display a la ACAD's "splframe" global
    switch (T_HOST.GetGraphicsAdmin()._GetControlPolyDisplay ())
        {
        case DgnPlatformLib::Host::GraphicsAdmin::CONTROLPOLY_DISPLAY_Always:
            break;

        case DgnPlatformLib::Host::GraphicsAdmin::CONTROLPOLY_DISPLAY_Never:
            return;

        default: // CONTROLPOLY_DISPLAY_ByElement
            {
            if (!thisElm.GetElementCP ()->ToBspline_surface().flags.poly_display)
                return;
            }
        }

    MSBsplineSurface    surface;

    if (SUCCESS != BSplineSurfaceHandler::SurfaceFromElement (surface, thisElm))
        return;

    IPickGeomP   pick   = context.GetIPickGeom ();
    GeomDetailP  detail = pick ? &pick->GetGeomDetail () : NULL;

    if (NULL != detail)
        detail->SetElemArg (BSURF_ELEMARG_ControlPolygon);

    StrokeSurfaceCurvesInfo info;

    memset (&info, 0, sizeof (info));

    info.thisElm    = &thisElm;
    info.context    = &context;
    info.surfaceP   = &surface;

    bspproc_processSurfacePolygon (&surface, drawSurfaceCurvePolygonCallback, &info);

    surface.ReleaseMem ();

    if (NULL != detail)
        detail->SetElemArg (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplineSurfaceHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    // Support nonsensical option to not display surface in wireframe (Helpful to make sure bit gets set correctly for older V8 versions!).
    if (thisElm.GetElementCP ()->ToBspline_surface().flags.curve_display || !context.GetViewFlags () || MSRenderMode::Wireframe != context.GetViewFlags ()->renderMode)
        {
        StrokeAsSurface  stroker;

        context.DrawCached (thisElm, stroker, 1);
        }

    // NOTE: Changes current matsymb...not cached...
    drawBSurfacePolygon (thisElm, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSplineSurfaceHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    MSBsplineSurfacePtr  surface = MSBsplineSurface::CreatePtr ();

    if (SUCCESS != SurfaceFromElement (*surface, eeh))
        return;

    CurveVectorPtr  curves = WireframeGeomUtil::CollectCurves (*surface, true, true);

    if (!curves.IsValid ())
        return;
    
    ElementAgenda   agenda;

    if (SUCCESS != DraftingElementSchema::ToElements (agenda, *curves, &eeh, true, *eeh.GetDgnModelP ()))
        return;

    EditElementHandle cellEeh;

    if (SUCCESS != NormalCellHeaderHandler::CreateGroupCellElement (cellEeh, agenda, L"From Bspline Surface"))
        return;

    eeh.ReplaceElementDescr (cellEeh.ExtractElementDescr ().get ());
    eeh.GetHandler ().ConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings  09/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BSplineSurfaceHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR eh,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    MSBsplineSurfacePtr  surface = MSBsplineSurface::CreatePtr ();

    if (SUCCESS != SurfaceFromElement (*surface, eh))
        return ERROR;

    CurveVectorPtr  curves = WireframeGeomUtil::CollectCurves (*surface, true, true);

    if (!curves.IsValid ())
        return ERROR;
    
    ElementAgenda   agenda;

    if (SUCCESS != DraftingElementSchema::ToElements (agenda, *curves, &eh, true, *eh.GetDgnModelP ()))
        return ERROR;

    StatusInt   status = SUCCESS;

    EditElementHandleP curr = agenda.GetFirstP ();
    EditElementHandleP end  = curr + agenda.GetCount ();

    for (; curr < end && SUCCESS == status; curr++)
        status = curr->GetHandler ().FenceClip (inside, outside, *curr, fp, options);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BSplineSurfaceHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry)
    {
    if (0 == (DropGeometry::OPTION_Solids & geometry.GetOptions ()))
        return ERROR;

    if (DropGeometry::SOLID_Surfaces == geometry.GetSolidsOptions ())
        return ERROR; // Already a surface...

    MSBsplineSurfacePtr  surface = MSBsplineSurface::CreatePtr ();

    if (SUCCESS != SurfaceFromElement (*surface, eh))
        return ERROR;

    CurveVectorPtr  curves = WireframeGeomUtil::CollectCurves (*surface, true, true);

    if (!curves.IsValid ())
        return ERROR;
    
    return DraftingElementSchema::ToElements (dropGeom, *curves, &eh, true, *eh.GetDgnModelP ());
    }
