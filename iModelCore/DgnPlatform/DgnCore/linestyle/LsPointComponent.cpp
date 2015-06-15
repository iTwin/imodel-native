/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsPointComponent.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#include    <DgnPlatform/DgnCore/LsLocal.h>

#define LCPOINT_ANYVERTEX   (LCPOINT_LINEORG | LCPOINT_LINEEND | LCPOINT_LINEVERT)

/*---------------------------------------------------------------------------------**//**
* Stroke a LsPointComponent by processing the underlying stroke component and calling the "_ProcessSymbol" method at
* every place along the input line where a symbol should appear. Afterwards, process the symbols that appear at vertices.
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsPointComponent::_DoStroke (ViewContextP context, DPoint3dCP inPoints, int nPoints, LineStyleSymbCP modifiers) const
    {
    if (NULL == m_strokeComponent.get ())
        return  SUCCESS;

    if (HasStrokeSymbol() && (SUCCESS != m_strokeComponent->ProcessStroke (context, this, inPoints, nPoints, modifiers)))
        return  ERROR;

    DPoint3dCP  org   = inPoints;
    DPoint3dCP  end   = org + (nPoints-1);

    // process vertext symbols
    LsSymbolReferenceCP   sym;
    DPoint3d     dir, dir1, dir2;

    if (!modifiers->ContinuationXElems() && NULL != (sym = GetSymbolForVertexCP (LsSymbolReference::VERTEX_LineOrigin)))
        {
        dir.NormalizedDifference (org[1], *org);

        if (SUCCESS != sym->Output (context, modifiers, org, &dir))
            return  ERROR;
        }

    if (NULL != (sym = GetSymbolForVertexCP (LsSymbolReference::VERTEX_LineEnd)))
        {
        dir.NormalizedDifference (*end, *(end-1));
        if (SUCCESS != sym->Output (context, modifiers, end, &dir))
            return  ERROR;
        }

    // If treat as single segment is set we are on a curve or arc - in which case we can't support vertex symbols either
    if (modifiers->IsTreatAsSingleSegment())
        return  SUCCESS;

    if (NULL != (sym = GetSymbolForVertexCP (LsSymbolReference::VERTEX_Each)))
        {
        for (org++ ;org < end; org++)
            {
            dir1.NormalizedDifference (org[1], *org);
            dir2.NormalizedDifference (*org, *(org-1));

            bsiDPoint3d_interpolate (&dir, &dir1, 0.5, &dir2);
            dir.Normalize ();

            if (SUCCESS != sym->Output (context, modifiers, org, &dir))
                return  ERROR;
            }
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsPointComponent::HasStrokeSymbol () const
    {
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if (-1 != curr->m_strokeNo)
            return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceP LsPointComponent::GetSymbolReferenceP
(
T_SymbolsCollectionConstIter iter
) const
    {
    LsSymbolReferenceCP lsSym = &*iter;
    return const_cast <LsSymbolReferenceP> (lsSym);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceP LsPointComponent::GetSymbolP
(
size_t           index
) const
    {
    if (index >= m_symbols.size ())
        return NULL;
        
    T_SymbolsCollectionConstIter    entry = m_symbols.begin () + index;
    return GetSymbolReferenceP (entry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceCP LsPointComponent::GetSymbolCP
(
size_t           index
) const
    {
    return GetSymbolP (index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceP LsPointComponent::GetSymbolForStrokeP
(
int             strokeNo
) const
    {
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if ((0 == (curr->m_mod1 & LCPOINT_ANYVERTEX)) && (curr->m_mod1 & LCPOINT_ONSTROKE) && (curr->m_strokeNo == strokeNo))
            return GetSymbolReferenceP (curr);
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceCP LsPointComponent::GetSymbolForStrokeCP
(
int             strokeNo
) const
    {
    return GetSymbolForStrokeP (strokeNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceP LsPointComponent::GetSymbolForVertexP (LsSymbolReference::VertexMask vertexMask) const
    {
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if (0 != (curr->m_mod1 & vertexMask))
            return GetSymbolReferenceP (curr);
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceCP LsPointComponent::GetSymbolForVertexCP (LsSymbolReference::VertexMask vertexMask) const
    {
    return GetSymbolForVertexP (vertexMask);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsPointComponent::_ProcessSymbol
(
ViewContextP     context,
Centerline const* centerline,
LineStyleSymbCP   modifiers,
LsStrokeCP        pStroke,
int               strokeIndex,
int               endCondition
) const
    {
    LsSymbolReferenceCP    symRef = GetSymbolForStrokeCP (strokeIndex);

    if (NULL == symRef)
        return  false;

    double  outStrokeLen = pStroke->GetLength();
    DPoint3dCP const firstPoint = centerline->GetPointAt(0);
    DPoint3dCP const lastPoint  = firstPoint + (centerline->GetCount() - 1);
    double   const* const firstLen   = centerline->GetLengthAt(0);
    double   const* const lastLen    = firstLen + (centerline->GetCount() - 1);

    /*-----------------------------------------------------------------------------------
    If this symbol reference doesn't allow partial strokes then short circuit early

    The check for IsRigid is a throwback.  It might have been a bug in earlier versions
    that you didn't have to set the partial items correctly on the point symbol, or it
    might be a necessary default for strokes that bypass corners.  Either way it is something
    we must continue to support.
    -----------------------------------------------------------------------------------*/
    if (symRef->GetNoPartial() && !pStroke->IsRigid () && ((*firstLen > 0) || (*lastLen < outStrokeLen)))
        return  false;

    DPoint3d    segOrg, segEnd, segDir, symOrg;
    bool        checkOrigin = !symRef->GetDgnDb();

    switch (symRef->GetJustification())
        {
        case LCPOINT_ORIGIN:
            if (checkOrigin && (*firstLen > 0))
                return  false;

            segOrg = *firstPoint;
            segEnd = centerline->GetCount() == 1 ? *(firstPoint) : *(firstPoint+1);
            centerline->GetDirectionVector (segDir, segOrg, segEnd);
            symOrg.SumOf (segOrg,segDir, - *firstLen);
            break;

        case LCPOINT_CENTER:
            {
            double  centerOffset = outStrokeLen / 2.0;

            if (checkOrigin && (centerOffset >= *lastLen || centerOffset <= *firstLen))
                return  false;

            DPoint3dCP pPoint = firstPoint;
            double const*   pLen   = firstLen;
            while (pPoint < lastPoint && centerOffset > *pLen)
                {
                pPoint++;
                pLen++;
                }

            if (pPoint == firstPoint)
                {   // Symbol is before the segment origin
                segOrg = *pPoint;
                segEnd = centerline->GetCount() == 1 ? *(pPoint) : *(pPoint + 1);
                centerline->GetDirectionVector (segDir, segOrg, segEnd);
                symOrg.SumOf (segOrg,segDir, centerOffset - *pLen);
                }
            else
                {   // Symbol is within the segment or past the end
                segOrg = centerline->GetCount() == 1 ? *(pPoint) : *(pPoint-1);
                segEnd = *pPoint;
                centerline->GetDirectionVector (segDir, segOrg, segEnd);
                symOrg.SumOf (segOrg,segDir, centerOffset - *(pLen-1));
                }
            }
            break;

        case LCPOINT_END:
            if (checkOrigin && outStrokeLen > *lastLen)
                return  false;

            segOrg = centerline->GetCount() == 1 ? *(lastPoint) : *(lastPoint-1);
            segEnd = *(lastPoint);
            centerline->GetDirectionVector (segDir, segOrg, segEnd);
            symOrg.SumOf (segEnd,segDir, outStrokeLen - *lastLen);
            break;

        default:
            return  false;
        }

    /*-----------------------------------------------------------------------------------
    Setup clipping points if clip is enabled. Note: segOrg and segEnd (calculated above)
    are not necessarily the endpoints of the stroke so they aren't used as the clip points.

    See comment about IsRigid above.
    -----------------------------------------------------------------------------------*/
    DPoint3d    *pOrgClip=NULL, *pEndClip=NULL;
    DPoint3d    strokeOrg, strokeEnd;

    if (symRef->GetClipPartial() || pStroke->IsRigid ())
        {
        if (*firstLen > 0)
            {
            strokeOrg = *firstPoint;
            pOrgClip = &strokeOrg;
            }

        if (*lastLen < outStrokeLen)
            {
            strokeEnd = *lastPoint;
            pEndClip = &strokeEnd;
            }
        }

    double xScale = 1.0;
    /*-----------------------------------------------------------------------------------
    If this symbol allows stretch and the underlying stroke has been stretched then
    the stretch has to be applied to the x bvector of the transform.
    -----------------------------------------------------------------------------------*/
    if (symRef->GetStretchable())
        {
        double baseLen =  m_strokeComponent->GetConstStrokePtr(strokeIndex)->GetLength() * modifiers->GetScale();

        if ((outStrokeLen != baseLen) && baseLen > 0)
            xScale = outStrokeLen / baseLen;
        }

    symRef->Output (context, modifiers, &symOrg, &segDir, &xScale, pOrgClip, pEndClip);
    return  false;
    }

#if defined (NOTNOW)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LsPointSymbolComponent::ToResource (void** outRsc, bool forResourceFile) const
    {
    PointSymRsc* pRsc;

#if defined (NOTNOW)
    if (SUCCESS != ToResourceNoSymbols ((void**)&pRsc))
        return ERROR;
#endif

    // Always makes a V8 element
    DPoint3d        offset = {0.0, 0.0, 0.0};
    PointSymRsc*    pPointSymRsc;
    // !! This assumes the first element is a cell header !!
    StatusInt status = mdlLineStyle_createSymbolResourceEx (&pPointSymRsc, m_elementChain->h.firstElem, &offset, ACTIVEMODEL, asV7Element, forResourceFile);

    /* Copy in the rest of the info from our element */
    if (SUCCESS == status && NULL != pPointSymRsc)
        {
        uint32_t oldType = pPointSymRsc->header.auxType; /* Hang onto type; set up by createSymbolResource for V7 */
        size_t dataSize = sizeof (*pPointSymRsc) - sizeof (pPointSymRsc->nBytes) - sizeof (pPointSymRsc->symBuf);
        memcpy (pPointSymRsc, pRsc, dataSize);
        pPointSymRsc->header.auxType = oldType;
        }
    memutil_free (pRsc);

    *outRsc = pPointSymRsc;
    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsPointComponent::_PostProcessLoad (DgnModelP modelRef)
    {
    if (m_postProcessed)
        return;
        
    m_postProcessed = true;
    if (NULL != m_strokeComponent.get ())
        m_strokeComponent->_PostProcessLoad (modelRef);
        
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if (NULL != curr->m_symbol.get ())
            curr->m_symbol->_PostProcessLoad (modelRef);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsPointComponent::_ClearPostProcess ()
    {
    m_postProcessed = false;
    if (NULL != m_strokeComponent.get ())
        m_strokeComponent->_ClearPostProcess ();
        
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if (NULL != curr->m_symbol.get ())
            curr->m_symbol->_ClearPostProcess ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsPointComponent::LsPointComponent (LsLocation const *pLocation) : LsComponent (pLocation)
    {
    m_postProcessed   = false;
    m_strokeComponent = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsPointComponent::~LsPointComponent()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsPointComponent::_ContainsComponent
(
LsComponentP    other
) const
    {
    if (LsComponent::_ContainsComponent(other))
        return  true;

    for (size_t i=0; i<GetNumberSymbols(); i++)
        {
        if (GetSymbolCP(i)->GetSymbolComponentCP()->_ContainsComponent(other))
            return true;
        }

    LsStrokePatternComponentCP stroke = GetStrokeComponentCP();
    return  stroke ? stroke->_ContainsComponent(other) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
double          LsPointComponent::_GetMaxWidth (DgnModelP modelRef) const
    {
    double width = 0.0;

    for (size_t i=0; i<GetNumberSymbols(); i++)
        {
        double thisWidth = GetSymbolCP(i)->_GetMaxWidth(modelRef);

        if (thisWidth > width)
            width = thisWidth;
        }

    return  width;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsPointComponent::SaveToResource (LinePointRsc& resource)
    {
    //  GetResourceSize () is smaller that sizeof (resource) if the LinePointRsc does not have any symbols.
    memset (&resource, 0, GetResourceSize ());
    CopyDescription (resource.descr);
    
    //  Seems odd, but lcType = LsComponentType::LineCode has always been used when there is no linecode.
    //  Leaving lcType causes _loadComponent to call LoadInternalComponent and it 
    resource.lcType = (uint32_t)LsComponentType::LineCode;
    LsComponentP    comp = m_strokeComponent.get ();
    if (NULL != comp)
        {
        LsLocationCP    loc = comp->GetLocation ();
        resource.lcType = (uint32_t)loc->GetComponentType ();
        resource.lcID   = loc->GetComponentId ().GetValue();
        }

    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if (NULL == curr->m_symbol.get ())
            continue;

        PointSymInfo*   pointSymInfo = resource.symbol + resource.nSym++;
        LsLocationCP    loc = curr->m_symbol->GetLocation ();
        pointSymInfo->symType = (uint32_t)loc->GetComponentType ();
        pointSymInfo->symID   = loc->GetComponentId ().GetValue();

        pointSymInfo->strokeNo  = (uint16_t)curr->m_strokeNo;
        pointSymInfo->mod1      = (uint16_t)curr->m_mod1;
        if (-1 == curr->m_strokeNo && (pointSymInfo->mod1 & LsSymbolReference::VERTEX_Any))
            pointSymInfo->strokeNo = 0;

        pointSymInfo->mod2      = 0;
        pointSymInfo->xOffset   = curr->m_offset.x;
        pointSymInfo->yOffset   = curr->m_offset.y;
        pointSymInfo->zAngle    = curr->m_angle * msGeomConst_degreesPerRadian;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsPointComponent::DeleteSymbolForStroke (LsSymbolReferenceCR symRef)
    {
#if defined (NOTNOW)
    for (T_SymbolsCollectionConstIter curr = m_symbols.begin (); curr != m_symbols.end (); ++curr)
        {
        if (&symRef == curr)
            {
            curr->m_symbol = NULL;
            for (LsSymbolReferenceP next = curr + 1; next < end; ++next, ++curr)
                *curr = *next
                
            m_nSymbols--;
            return BSISUCCESS;
            }
        }
#endif
        
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsSymbolReferenceR LsPointComponent::AppendSymbolForStroke 
(
LsSymbolComponentP symbol, 
double          xOffset, 
double          yOffset, 
double          radians, 
int             strokeNo
)
    {
    m_symbols.push_back (LsSymbolReference (symbol, this, 0, xOffset, yOffset, radians, strokeNo));
    return m_symbols.back ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
                LsSymbolReference::LsSymbolReference ()
 : m_symbol (NULL), m_parent (NULL), m_mod1 (0), m_angle (0.0), m_strokeNo (0)
    {
    m_offset.x = 0;
    m_offset.y = 0;
    m_offset.z = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
                LsSymbolReference::LsSymbolReference 
(
LsSymbolComponentP symbol, 
LsPointComponentP parent, 
uint32_t        mod1, 
double          xOffset, 
double          yOffset, 
double          radians,
int             strokeNo
)  : m_symbol (symbol), m_parent (parent), m_mod1 (mod1), m_angle (radians), m_strokeNo (strokeNo)
    {
    m_offset.x = xOffset;
    m_offset.y = yOffset;
    m_offset.z = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsPointComponent* LsPointComponent::LoadLinePoint
(
LsComponentReader*    reader
)
    {
    LinePointRsc*   lpRsc = (LinePointRsc*) reader->GetRsc();

    if (NULL == lpRsc)
        return  NULL;

    LsPointComponent* pointComp = new LsPointComponent (reader->GetSource());
    pointComp->SetDescription (Utf8String(lpRsc->descr, false).c_str());

#if defined(NOTNOW)
    //  Maybe this is necessary to avoid recursing
    LineStyleCacheManager::CacheAdd (pointComp);
#endif
    LsLocation      tmpLocation;
    tmpLocation.GetLineCodeLocation (reader);

    LsComponent*       subComp = LsCache::GetLsComponent (tmpLocation);
    if (NULL != subComp)
        {
        pointComp->m_strokeComponent = dynamic_cast <LsStrokePatternComponentP> (subComp);
        BeAssert (pointComp->m_strokeComponent.get ());
        }

    if (lpRsc->nSym > 0)
        {
        PointSymRscInfo*            pRscInfo = lpRsc->symbol;
        PointSymRscInfo*            pRscEnd  = pRscInfo + lpRsc->nSym;

        int symNum = 0;

        while (pRscInfo < pRscEnd)
            {
            tmpLocation.GetPointSymbolLocation (reader, symNum++);
            LsSymbolComponentP  symbol = (LsSymbolComponentP)LsCache::GetLsComponent (tmpLocation);

            // Apparently we have symbols that don't participate in the line style.  See TR #308324.
            // These should be removed when there is an opportunity like a file format change.
            bool skip = false;
            if (0 == (pRscInfo->mod1 & LsSymbolReference::VERTEX_Any))
                {
                if ((NULL == pointComp->GetStrokeComponentCP()) ||
                    (pRscInfo->strokeNo < 0 || pRscInfo->strokeNo >= pointComp->GetStrokeComponentCP()->GetStrokeCount()) )
                    {
                    skip = true;
                    }
                else if (0 == (pRscInfo->mod1 & LCPOINT_ONSTROKE))
                    {
                    skip = true;
                    }
                }

            //  If the stroke applies to a vertex, then the strokeNo should be ignored. 
            //  We accomplish that by setting it negative.
            if (!skip)
                pointComp->m_symbols.push_back (LsSymbolReference (symbol, pointComp, pRscInfo->mod1, pRscInfo->xOffset, pRscInfo->yOffset,
                                            pRscInfo->zAngle / msGeomConst_degreesPerRadian,
                                            (pRscInfo->mod1 & LsSymbolReference::VERTEX_Any) ? -1 : pRscInfo->strokeNo));

            pRscInfo++;
            }
        }

    return pointComp;
    }

size_t                       LsPointComponent::GetNumberSymbols()         const   {return m_symbols.size ();}
LsStrokePatternComponentP    LsPointComponent::GetStrokeComponentP ()     const   {return m_strokeComponent.get ();}
LsStrokePatternComponentCP    LsPointComponent::GetStrokeComponentCP ()   const   {return GetStrokeComponentP ();}
void                         LsPointComponent::SetStrokeComponent (LsStrokePatternComponentP p)   {m_strokeComponent = p;}
